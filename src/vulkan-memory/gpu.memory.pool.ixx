// GPU memory pool with command-buffer-based defragmentation.
//
// Unlike the CPU-side MemoryPool (which uses memmove on a mapped pointer),
// this pool manages VkBuffers bound to sub-offsets of a single VkDeviceMemory
// and moves data via vkCmdCopyBuffer — the standard approach for DEVICE_LOCAL
// memory that the CPU cannot access directly.
//
// Defragmentation is split into two phases:
//   Phase 1 — RecordDefrag(): record copy + barrier commands into a command buffer.
//   Phase 2 — ApplyDefrag():  after the GPU finishes, destroy old buffers and
//                              update internal tracking.
//
// This two-phase design is idiomatic Vulkan: the application decides when and
// how to submit work, and the pool never touches the queue itself.

export module vulkanmem:gpu.memory.pool;
import std;
import :vulkan.exports;
import :memory.pool; // for AlignUp

export namespace Memory
{
	// A GPU sub-allocation: a VkBuffer bound to a sub-region of the pool.
	struct GpuSubAllocation
	{
		vk::VkBuffer Buffer = nullptr;
		vk::VkDeviceSize Offset = 0;
		vk::VkDeviceSize Size = 0; // usable data size (what the caller requested)
	};

	// Describes one data relocation during GPU defragmentation.
	// OldBuffer will be destroyed by ApplyDefrag; NewBuffer replaces it.
	struct GpuDefragMove
	{
		vk::VkDeviceSize SrcOffset = 0;
		vk::VkDeviceSize DstOffset = 0;
		vk::VkDeviceSize Size = 0; // copy size (the buffer's usable data size)
		vk::VkBuffer OldBuffer = nullptr;
		vk::VkBuffer NewBuffer = nullptr;
	};

	class GpuMemoryPool
	{
	public:
		// Allocate a single VkDeviceMemory block of the given size.
		// commonUsage MUST include TRANSFER_SRC | TRANSFER_DST so that
		// buffers can participate in staging copies and defrag moves.
		GpuMemoryPool(
			vk::VkDevice device,
			vk::VkDeviceSize totalSize,
			std::uint32_t memoryTypeIndex,
			vk::VkBufferUsageFlags commonUsage
		) : m_device(device), m_totalSize(totalSize),
			m_memoryTypeIndex(memoryTypeIndex), m_usage(commonUsage)
		{
			auto allocInfo = vk::VkMemoryAllocateInfo{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = nullptr,
				.allocationSize = totalSize,
				.memoryTypeIndex = memoryTypeIndex
			};
			if (vk::vkAllocateMemory(device, &allocInfo, nullptr, &m_memory) != vk::Success)
				throw std::runtime_error("GpuMemoryPool: failed to allocate VkDeviceMemory.");

			m_regions.push_back(RegionInfo{
				.Offset = 0,
				.Size = totalSize,
				.IsFree = true
			});
		}

		~GpuMemoryPool()
		{
			for (auto& r : m_regions)
				if (r.Buffer)
					vk::vkDestroyBuffer(m_device, r.Buffer, nullptr);
			if (m_memory)
				vk::vkFreeMemory(m_device, m_memory, nullptr);
		}

		GpuMemoryPool(const GpuMemoryPool&) = delete;
		auto operator=(const GpuMemoryPool&) -> GpuMemoryPool& = delete;
		GpuMemoryPool(GpuMemoryPool&&) = delete;
		auto operator=(GpuMemoryPool&&) -> GpuMemoryPool& = delete;

		// ---------------------------------------------------------------
		// Sub-allocate a region: create a VkBuffer and bind it to the
		// pool's memory at a properly aligned offset.
		//
		// The buffer's actual memory requirements (alignment, size) are
		// queried from the driver — the caller's alignment is a minimum.
		// If the pool's memory type is incompatible with the buffer, or
		// no free region can satisfy the request, returns nullopt.
		// ---------------------------------------------------------------
		[[nodiscard]]
		auto Allocate(
			this GpuMemoryPool& self,
			vk::VkDeviceSize size,
			vk::VkDeviceSize alignment = 1
		) -> std::optional<GpuSubAllocation>
		{
			if (size == 0) return std::nullopt;

			// Create a VkBuffer to learn its memory requirements.
			auto bufferInfo = vk::VkBufferCreateInfo{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.size = size,
				.usage = self.m_usage,
				.sharingMode = vk::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = nullptr
			};

			auto buffer = vk::VkBuffer{ nullptr };
			if (vk::vkCreateBuffer(self.m_device, &bufferInfo, nullptr, &buffer) != vk::Success)
				return std::nullopt;

			auto memReqs = vk::VkMemoryRequirements{};
			vk::vkGetBufferMemoryRequirements(self.m_device, buffer, &memReqs);

			// The buffer must be compatible with the pool's memory type.
			if (not (memReqs.memoryTypeBits & (1u << self.m_memoryTypeIndex)))
			{
				vk::vkDestroyBuffer(self.m_device, buffer, nullptr);
				return std::nullopt;
			}

			// Use the stricter alignment and the actual memory footprint.
			auto effectiveAlignment = std::max(alignment, memReqs.alignment);
			auto effectiveSize = memReqs.size;

			// First-fit search through free regions.
			for (auto it = self.m_regions.begin(); it != self.m_regions.end(); ++it)
			{
				if (not it->IsFree) continue;

				auto alignedOffset = AlignUp(it->Offset, effectiveAlignment);
				auto padding = alignedOffset - it->Offset;
				auto totalNeeded = padding + effectiveSize;

				if (totalNeeded > it->Size) continue;

				// Bind the buffer to the pool memory at this offset.
				if (vk::vkBindBufferMemory(self.m_device, buffer, self.m_memory, alignedOffset) != vk::Success)
				{
					vk::vkDestroyBuffer(self.m_device, buffer, nullptr);
					return std::nullopt;
				}

				// Split: [padding free | allocation | trailing free]
				auto remaining = it->Size - totalNeeded;
				auto pos = self.m_regions.erase(it);

				if (padding > 0)
				{
					pos = self.m_regions.insert(pos, RegionInfo{
						.Offset = alignedOffset - padding,
						.Size = padding,
						.IsFree = true
					});
					++pos;
				}

				pos = self.m_regions.insert(pos, RegionInfo{
					.Offset = alignedOffset,
					.Size = effectiveSize,
					.BufferSize = size,
					.IsFree = false,
					.Alignment = effectiveAlignment,
					.Buffer = buffer
				});
				++pos;

				if (remaining > 0)
				{
					self.m_regions.insert(pos, RegionInfo{
						.Offset = alignedOffset + effectiveSize,
						.Size = remaining,
						.IsFree = true
					});
				}

				return GpuSubAllocation{
					.Buffer = buffer,
					.Offset = alignedOffset,
					.Size = size
				};
			}

			// No region could satisfy the request.
			vk::vkDestroyBuffer(self.m_device, buffer, nullptr);
			return std::nullopt;
		}

		// ---------------------------------------------------------------
		// Free a sub-allocation, destroying its VkBuffer.
		// Adjacent free regions are merged automatically.
		// ---------------------------------------------------------------
		void Free(this GpuMemoryPool& self, const GpuSubAllocation& alloc)
		{
			auto it = std::ranges::find_if(self.m_regions, [&](const RegionInfo& r) {
				return not r.IsFree and r.Buffer == alloc.Buffer;
			});

			if (it == self.m_regions.end())
				throw std::runtime_error("GpuMemoryPool::Free: allocation not found.");

			vk::vkDestroyBuffer(self.m_device, it->Buffer, nullptr);
			*it = RegionInfo{
				.Offset = it->Offset,
				.Size = it->Size,
				.IsFree = true
			};
			self.MergeAdjacentFreeRegions();
		}

		// ---------------------------------------------------------------
		// Phase 1: Record defrag commands into a command buffer.
		//
		// The command buffer must already be in the recording state
		// (after vkBeginCommandBuffer).
		//
		// For each allocation that needs to move forward:
		//   1. Create a new VkBuffer at the compacted offset.
		//   2. Record vkCmdCopyBuffer from old buffer to new.
		//   3. Insert a pipeline barrier between copies so each write
		//      finishes before the next copy reads overlapping memory.
		//
		// After submitting the command buffer and waiting for the fence,
		// pass the returned move list to ApplyDefrag().
		// ---------------------------------------------------------------
		auto RecordDefrag(
			this GpuMemoryPool& self,
			vk::VkCommandBuffer cmd
		) -> std::vector<GpuDefragMove>
		{
			auto moves = std::vector<GpuDefragMove>{};
			auto writeOffset = vk::VkDeviceSize{ 0 };

			// Plan: compute target offsets, respecting original alignment.
			struct PlannedMove
			{
				std::size_t RegionIndex;
				vk::VkDeviceSize TargetOffset;
			};
			auto plan = std::vector<PlannedMove>{};

			for (std::size_t i = 0; i < self.m_regions.size(); ++i)
			{
				auto& r = self.m_regions[i];
				if (r.IsFree) continue;

				auto target = AlignUp(writeOffset, r.Alignment);
				if (target != r.Offset)
					plan.push_back({ i, target });

				writeOffset = target + r.Size;
			}

			// Record copy + barrier commands for each planned move.
			for (std::size_t i = 0; i < plan.size(); ++i)
			{
				auto& [idx, target] = plan[i];
				auto& region = self.m_regions[idx];

				// Create a new VkBuffer at the compacted offset.
				auto bufInfo = vk::VkBufferCreateInfo{
					.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.size = region.BufferSize,
					.usage = self.m_usage,
					.sharingMode = vk::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
					.queueFamilyIndexCount = 0,
					.pQueueFamilyIndices = nullptr
				};

				auto newBuffer = vk::VkBuffer{ nullptr };
				if (vk::vkCreateBuffer(self.m_device, &bufInfo, nullptr, &newBuffer) != vk::Success)
					throw std::runtime_error("GpuMemoryPool::RecordDefrag: buffer creation failed.");

				if (vk::vkBindBufferMemory(self.m_device, newBuffer, self.m_memory, target) != vk::Success)
				{
					vk::vkDestroyBuffer(self.m_device, newBuffer, nullptr);
					throw std::runtime_error("GpuMemoryPool::RecordDefrag: buffer bind failed.");
				}

				// Record the copy from old buffer to new buffer.
				auto copy = vk::VkBufferCopy{
					.srcOffset = 0,
					.dstOffset = 0,
					.size = region.BufferSize
				};
				vk::vkCmdCopyBuffer(cmd, region.Buffer, newBuffer, 1, &copy);

				// Pipeline barrier: ensure this copy's writes complete before
				// the next copy reads from potentially overlapping memory.
				if (i + 1 < plan.size())
				{
					auto barrier = vk::VkMemoryBarrier{
						.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = static_cast<vk::VkAccessFlags>(
							vk::VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT),
						.dstAccessMask = static_cast<vk::VkAccessFlags>(
							vk::VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT
							| vk::VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT)
					};
					vk::vkCmdPipelineBarrier(
						cmd,
						static_cast<vk::VkPipelineStageFlags>(
							vk::VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT),
						static_cast<vk::VkPipelineStageFlags>(
							vk::VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT),
						0,             // dependencyFlags
						1, &barrier,   // memory barriers
						0, nullptr,    // buffer memory barriers
						0, nullptr     // image memory barriers
					);
				}

				moves.push_back(GpuDefragMove{
					.SrcOffset = region.Offset,
					.DstOffset = target,
					.Size = region.BufferSize,
					.OldBuffer = region.Buffer,
					.NewBuffer = newBuffer
				});
			}

			return moves;
		}

		// ---------------------------------------------------------------
		// Phase 2: Finalise defragmentation after GPU execution.
		//
		// Destroys old VkBuffers, updates internal region tracking,
		// and rebuilds the free list.
		//
		// MUST only be called after the command buffer from RecordDefrag()
		// has completed execution on the GPU (i.e., after the fence).
		// ---------------------------------------------------------------
		void ApplyDefrag(
			this GpuMemoryPool& self,
			std::span<const GpuDefragMove> moves
		)
		{
			// Swap old buffers for new ones in the region list.
			for (const auto& move : moves)
			{
				auto it = std::ranges::find_if(self.m_regions, [&](const RegionInfo& r) {
					return not r.IsFree and r.Buffer == move.OldBuffer;
				});
				if (it != self.m_regions.end())
				{
					vk::vkDestroyBuffer(self.m_device, it->Buffer, nullptr);
					it->Buffer = move.NewBuffer;
					it->Offset = move.DstOffset;
				}
			}

			// Rebuild the region list: remove free regions, sort used by offset,
			// then fill in gaps as new free regions.
			std::erase_if(self.m_regions, [](const RegionInfo& r) { return r.IsFree; });
			std::ranges::sort(self.m_regions, {}, &RegionInfo::Offset);

			auto rebuilt = std::vector<RegionInfo>{};
			auto cursor = vk::VkDeviceSize{ 0 };
			for (const auto& r : self.m_regions)
			{
				if (r.Offset > cursor)
				{
					rebuilt.push_back(RegionInfo{
						.Offset = cursor,
						.Size = r.Offset - cursor,
						.IsFree = true
					});
				}
				rebuilt.push_back(r);
				cursor = r.Offset + r.Size;
			}
			if (cursor < self.m_totalSize)
			{
				rebuilt.push_back(RegionInfo{
					.Offset = cursor,
					.Size = self.m_totalSize - cursor,
					.IsFree = true
				});
			}

			self.m_regions = std::move(rebuilt);
			self.MergeAdjacentFreeRegions();
		}

		// --- Accessors ------------------------------------------------

		[[nodiscard]]
		auto GetMemory(this const GpuMemoryPool& self) noexcept -> vk::VkDeviceMemory
		{
			return self.m_memory;
		}

		[[nodiscard]]
		auto GetTotalSize(this const GpuMemoryPool& self) noexcept -> vk::VkDeviceSize
		{
			return self.m_totalSize;
		}

		[[nodiscard]]
		auto GetUsedSize(this const GpuMemoryPool& self) noexcept -> vk::VkDeviceSize
		{
			vk::VkDeviceSize used = 0;
			for (const auto& r : self.m_regions)
				if (not r.IsFree) used += r.Size;
			return used;
		}

		[[nodiscard]]
		auto GetFreeSize(this const GpuMemoryPool& self) noexcept -> vk::VkDeviceSize
		{
			return self.m_totalSize - self.GetUsedSize();
		}

		[[nodiscard]]
		auto GetLargestFreeBlock(this const GpuMemoryPool& self) noexcept -> vk::VkDeviceSize
		{
			vk::VkDeviceSize largest = 0;
			for (const auto& r : self.m_regions)
				if (r.IsFree and r.Size > largest) largest = r.Size;
			return largest;
		}

		[[nodiscard]]
		auto GetFreeBlockCount(this const GpuMemoryPool& self) noexcept -> std::size_t
		{
			return static_cast<std::size_t>(
				std::ranges::count_if(self.m_regions, &RegionInfo::IsFree)
			);
		}

		// Print a human-readable dump of the pool's layout.
		void PrintState(this const GpuMemoryPool& self)
		{
			auto used = self.GetUsedSize();
			auto free = self.GetFreeSize();
			auto usedPct = static_cast<double>(used) * 100.0 / static_cast<double>(self.m_totalSize);
			auto freePct = 100.0 - usedPct;

			std::println("  Total: {} bytes | Used: {} ({:.1f}%) | Free: {} ({:.1f}%)",
				self.m_totalSize, used, usedPct, free, freePct);
			std::println("  Free blocks: {} | Largest free block: {} bytes",
				self.GetFreeBlockCount(), self.GetLargestFreeBlock());

			std::println("");
			std::println("  Regions:");
			for (const auto& r : self.m_regions)
			{
				if (r.IsFree)
				{
					std::println("    [{:>6} .. {:>6}]  {:>6} bytes  FREE",
						r.Offset, r.Offset + r.Size - 1, r.Size);
				}
				else
				{
					std::println("    [{:>6} .. {:>6}]  {:>6} bytes  USED  (buffer data: {} bytes)",
						r.Offset, r.Offset + r.Size - 1, r.Size, r.BufferSize);
				}
			}

			// ASCII bar — 60 chars wide.  '#' = used, '.' = free.
			constexpr int BarWidth = 60;
			auto bar = std::string(BarWidth, ' ');
			for (const auto& r : self.m_regions)
			{
				auto startChar = static_cast<int>(r.Offset * BarWidth / self.m_totalSize);
				auto endChar = static_cast<int>((r.Offset + r.Size) * BarWidth / self.m_totalSize);
				endChar = std::max(endChar, startChar + 1);
				endChar = std::min(endChar, BarWidth);
				auto fill = r.IsFree ? '.' : '#';
				for (int i = startChar; i < endChar; ++i)
					bar[static_cast<std::size_t>(i)] = fill;
			}
			std::println("");
			std::println("  [{}]", bar);
		}

	private:
		// Internal region tracker.  Compared to the CPU pool's Region,
		// this also stores the VkBuffer handle, the usable data size
		// (which may differ from the memory footprint), and the alignment
		// used at allocation time so compaction can respect it.
		struct RegionInfo
		{
			vk::VkDeviceSize Offset = 0;
			vk::VkDeviceSize Size = 0;       // actual memory footprint (memReqs.size)
			vk::VkDeviceSize BufferSize = 0;  // usable data (bufferCreateInfo.size)
			bool IsFree = true;
			vk::VkDeviceSize Alignment = 1;
			vk::VkBuffer Buffer = nullptr;
		};

		vk::VkDevice m_device = nullptr;
		vk::VkDeviceMemory m_memory = nullptr;
		vk::VkDeviceSize m_totalSize = 0;
		std::uint32_t m_memoryTypeIndex = 0;
		vk::VkBufferUsageFlags m_usage = 0;
		std::vector<RegionInfo> m_regions;

		void MergeAdjacentFreeRegions(this GpuMemoryPool& self)
		{
			for (std::size_t i = 0; i + 1 < self.m_regions.size(); )
			{
				if (self.m_regions[i].IsFree and self.m_regions[i + 1].IsFree)
				{
					self.m_regions[i].Size += self.m_regions[i + 1].Size;
					self.m_regions.erase(
						self.m_regions.begin()
						+ static_cast<std::ptrdiff_t>(i + 1)
					);
				}
				else
				{
					++i;
				}
			}
		}
	};
}
