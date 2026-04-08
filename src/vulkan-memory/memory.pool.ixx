// Memory pool with sub-allocation, alignment, and manual defragmentation.
//
// Design overview:
//   - Backed by a single VkDeviceMemory block, persistently mapped for CPU access.
//   - Maintains an ordered list of regions (free or used) covering the entire range.
//   - Allocates via first-fit with alignment-aware splitting.
//   - Defragments by compacting live data forward with std::memmove.
//
// In a production engine you would extend this to:
//   - Support DEVICE_LOCAL pools (defrag via vkCmdCopyBuffer on a transfer queue).
//   - Use best-fit or buddy allocation to reduce fragmentation.
//   - Track VkBuffer bindings so they can be recreated at new offsets after defrag.
//   - Manage multiple pools per memory type.

export module vulkanmem:memory.pool;
import std;
import :vulkan.exports;

export namespace Memory
{
	// Align a value up to the nearest multiple of alignment.
	// Alignment must be a power of two.
	constexpr auto AlignUp(
		vk::VkDeviceSize value,
		vk::VkDeviceSize alignment
	) noexcept -> vk::VkDeviceSize
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	// A contiguous region within the memory pool.
	// The pool is partitioned into an ordered, gap-free sequence of these.
	struct Region
	{
		vk::VkDeviceSize Offset = 0;
		vk::VkDeviceSize Size = 0;
		bool IsFree = true;
	};

	// Handle returned by MemoryPool::Allocate().
	// Hold onto it and pass it back to Free() when done.
	struct SubAllocation
	{
		vk::VkDeviceSize Offset = 0;
		vk::VkDeviceSize Size = 0;
	};

	// Describes one data relocation performed during defragmentation.
	// Callers can use this to update external references (buffer bindings, descriptors).
	struct DefragMove
	{
		vk::VkDeviceSize SrcOffset = 0;
		vk::VkDeviceSize DstOffset = 0;
		vk::VkDeviceSize Size = 0;
	};

	class MemoryPool
	{
	public:
		// Allocate a single VkDeviceMemory block and persistently map it.
		MemoryPool(
			vk::VkDevice device,
			vk::VkDeviceSize totalSize,
			std::uint32_t memoryTypeIndex
		) : m_device(device), m_totalSize(totalSize)
		{
			auto allocInfo = vk::VkMemoryAllocateInfo{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = nullptr,
				.allocationSize = totalSize,
				.memoryTypeIndex = memoryTypeIndex
			};

			if (vk::vkAllocateMemory(device, &allocInfo, nullptr, &m_memory) != vk::Success)
				throw std::runtime_error("MemoryPool: failed to allocate VkDeviceMemory.");

			if (vk::vkMapMemory(device, m_memory, 0, totalSize, 0, &m_mappedPtr) != vk::Success)
			{
				vk::vkFreeMemory(device, m_memory, nullptr);
				throw std::runtime_error("MemoryPool: failed to map memory.");
			}

			// The entire pool starts as one large free region.
			m_regions.push_back(Region{
				.Offset = 0,
				.Size = totalSize,
				.IsFree = true
			});
		}

		~MemoryPool()
		{
			if (m_memory)
			{
				vk::vkUnmapMemory(m_device, m_memory);
				vk::vkFreeMemory(m_device, m_memory, nullptr);
			}
		}

		MemoryPool(const MemoryPool&) = delete;
		auto operator=(const MemoryPool&) -> MemoryPool& = delete;
		MemoryPool(MemoryPool&&) = delete;
		auto operator=(MemoryPool&&) -> MemoryPool& = delete;

		// ---------------------------------------------------------------
		// Sub-allocate a region with the requested size and alignment.
		//
		// Strategy: first-fit — walk the free list and take the first
		// region that fits after alignment.  When alignment creates
		// padding at the start, that padding becomes its own small free
		// region.  Leftover space after the allocation also becomes free.
		//
		// Returns std::nullopt when the pool is too fragmented or full.
		// ---------------------------------------------------------------
		[[nodiscard]]
		auto Allocate(
			this MemoryPool& self,
			vk::VkDeviceSize size,
			vk::VkDeviceSize alignment = 1
		) -> std::optional<SubAllocation>
		{
			if (size == 0 or alignment == 0)
				return std::nullopt;

			for (auto it = self.m_regions.begin(); it != self.m_regions.end(); ++it)
			{
				if (not it->IsFree) continue;

				// Compute the first aligned offset inside this free region.
				auto alignedOffset = AlignUp(it->Offset, alignment);
				auto padding = alignedOffset - it->Offset;
				auto totalNeeded = padding + size;

				if (totalNeeded > it->Size) continue;

				// Split into up to three parts:
				//   [padding free] [allocation] [trailing free]
				auto remaining = it->Size - totalNeeded;
				auto pos = self.m_regions.erase(it);

				// 1. Alignment padding (if any) becomes a free region.
				if (padding > 0)
				{
					pos = self.m_regions.insert(pos, Region{
						.Offset = alignedOffset - padding,
						.Size = padding,
						.IsFree = true
					});
					++pos;
				}

				// 2. The allocated region.
				pos = self.m_regions.insert(pos, Region{
					.Offset = alignedOffset,
					.Size = size,
					.IsFree = false
				});
				++pos;

				// 3. Trailing space (if any) becomes a free region.
				if (remaining > 0)
				{
					self.m_regions.insert(pos, Region{
						.Offset = alignedOffset + size,
						.Size = remaining,
						.IsFree = true
					});
				}

				return SubAllocation{ .Offset = alignedOffset, .Size = size };
			}

			return std::nullopt;
		}

		// ---------------------------------------------------------------
		// Return a sub-allocation to the pool.
		// Adjacent free regions are merged automatically.
		// ---------------------------------------------------------------
		void Free(this MemoryPool& self, const SubAllocation& allocation)
		{
			auto it = std::ranges::find_if(self.m_regions, [&](const Region& r) {
				return not r.IsFree
					and r.Offset == allocation.Offset
					and r.Size == allocation.Size;
			});

			if (it == self.m_regions.end())
				throw std::runtime_error("MemoryPool::Free: allocation not found.");

			it->IsFree = true;
			self.MergeAdjacentFreeRegions();
		}

		// ---------------------------------------------------------------
		// Defragment by compacting all live allocations to the front.
		//
		// How it works:
		//   1. Walk the region list.  For each used region, if there is
		//      free space before it, memmove its data forward.
		//   2. Rebuild the region list: used regions packed at the front,
		//      one large free region at the tail.
		//
		// Returns the list of moves so the caller can update any external
		// references (e.g., VkBuffer rebinds, descriptor set updates).
		//
		// Note: for DEVICE_LOCAL memory you would record vkCmdCopyBuffer
		// commands instead of memmove and submit them on a transfer queue.
		// ---------------------------------------------------------------
		auto Defragment(this MemoryPool& self) -> std::vector<DefragMove>
		{
			auto moves = std::vector<DefragMove>{};
			auto writeOffset = vk::VkDeviceSize{ 0 };
			auto ptr = static_cast<unsigned char*>(self.m_mappedPtr);

			// Pass 1: slide each used region forward to close gaps.
			for (auto& region : self.m_regions)
			{
				if (region.IsFree) continue;

				if (region.Offset != writeOffset)
				{
					// memmove is safe even when source and destination overlap.
					std::memmove(ptr + writeOffset, ptr + region.Offset, region.Size);
					moves.push_back(DefragMove{
						.SrcOffset = region.Offset,
						.DstOffset = writeOffset,
						.Size = region.Size
					});
					region.Offset = writeOffset;
				}
				writeOffset += region.Size;
			}

			// Pass 2: remove all free regions and append one at the tail.
			std::erase_if(self.m_regions, [](const Region& r) { return r.IsFree; });

			if (writeOffset < self.m_totalSize)
			{
				self.m_regions.push_back(Region{
					.Offset = writeOffset,
					.Size = self.m_totalSize - writeOffset,
					.IsFree = true
				});
			}

			return moves;
		}

		// --- Accessors ------------------------------------------------

		[[nodiscard]]
		auto GetMemory(this const MemoryPool& self) noexcept -> vk::VkDeviceMemory
		{
			return self.m_memory;
		}

		[[nodiscard]]
		auto GetMappedPointer(this const MemoryPool& self) noexcept -> void*
		{
			return self.m_mappedPtr;
		}

		[[nodiscard]]
		auto GetTotalSize(this const MemoryPool& self) noexcept -> vk::VkDeviceSize
		{
			return self.m_totalSize;
		}

		[[nodiscard]]
		auto GetUsedSize(this const MemoryPool& self) noexcept -> vk::VkDeviceSize
		{
			vk::VkDeviceSize used = 0;
			for (const auto& r : self.m_regions)
				if (not r.IsFree) used += r.Size;
			return used;
		}

		[[nodiscard]]
		auto GetFreeSize(this const MemoryPool& self) noexcept -> vk::VkDeviceSize
		{
			return self.m_totalSize - self.GetUsedSize();
		}

		[[nodiscard]]
		auto GetLargestFreeBlock(this const MemoryPool& self) noexcept -> vk::VkDeviceSize
		{
			vk::VkDeviceSize largest = 0;
			for (const auto& r : self.m_regions)
				if (r.IsFree and r.Size > largest) largest = r.Size;
			return largest;
		}

		[[nodiscard]]
		auto GetFreeBlockCount(this const MemoryPool& self) noexcept -> std::size_t
		{
			return static_cast<std::size_t>(
				std::ranges::count_if(self.m_regions, &Region::IsFree)
			);
		}

		[[nodiscard]]
		auto GetRegions(this const MemoryPool& self) noexcept -> std::span<const Region>
		{
			return self.m_regions;
		}

		// Print a human-readable dump of the pool's layout and metrics.
		void PrintState(this const MemoryPool& self)
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
				std::println("    [{:>6} .. {:>6}]  {:>6} bytes  {}",
					r.Offset, r.Offset + r.Size - 1, r.Size,
					r.IsFree ? "FREE" : "USED");
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
		vk::VkDevice m_device = nullptr;
		vk::VkDeviceMemory m_memory = nullptr;
		vk::VkDeviceSize m_totalSize = 0;
		void* m_mappedPtr = nullptr;
		std::vector<Region> m_regions;

		// Merge adjacent free regions into larger ones.
		// Called after every Free() to keep the list compact.
		void MergeAdjacentFreeRegions(this MemoryPool& self)
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
