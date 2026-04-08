// Vulkan Memory Pool Demo
// Created with Copilot, as I was too lazy to write it myself and I couldn't 
// find good references online for how to do defragging. This is just a simple 
// demo to show how you might implement manual defragmentation in a 
// sub-allocating memory pool.
//
// Demonstrates manual memory defragmentation with sub-allocating pools.
//
// Part 1 — CPU-side (HOST_VISIBLE memory):
//   Uses memmove on a persistently-mapped pointer to compact data.
//
// Part 2 — GPU-side (DEVICE_LOCAL memory):
//   Uses vkCmdCopyBuffer with pipeline barriers to move data on the GPU.
//   Shows the two-phase pattern: record commands, submit, then apply.

#include <vulkan/vulkan.h>

import std;
import vulkanmem;

// Find the first memory type index whose property flags contain all of
// the requested bits (e.g. HOST_VISIBLE | HOST_COHERENT).
auto FindMemoryType(
	VkPhysicalDevice physicalDevice,
	VkMemoryPropertyFlags requiredProperties
) -> std::optional<std::uint32_t>
{
	auto memProps = VkPhysicalDeviceMemoryProperties{};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

	for (std::uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
	{
		if ((memProps.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties)
			return i;
	}
	return std::nullopt;
}

// Fill a region of the mapped pool with a repeating byte pattern.
void FillPattern(Memory::MemoryPool& pool, const Memory::SubAllocation& alloc, unsigned char pattern)
{
	auto ptr = static_cast<unsigned char*>(pool.GetMappedPointer());
	std::memset(ptr + alloc.Offset, pattern, alloc.Size);
}

// Verify that a region of the mapped pool contains the expected byte pattern.
auto VerifyPattern(
	const Memory::MemoryPool& pool,
	const Memory::SubAllocation& alloc,
	unsigned char expected
) -> bool
{
	auto ptr = static_cast<const unsigned char*>(pool.GetMappedPointer());
	for (VkDeviceSize i = 0; i < alloc.Size; ++i)
	{
		if (ptr[alloc.Offset + i] != expected)
			return false;
	}
	return true;
}

// --------------- GPU Demo Helpers ---------------

struct StagingBuffer
{
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	void* mapped = nullptr;
	VkDeviceSize size = 0;
};

// Create a small dedicated staging buffer with HOST_VISIBLE memory.
auto CreateStagingBuffer(
	VkDevice device,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	std::uint32_t hostVisibleMemTypeIndex
) -> StagingBuffer
{
	auto sb = StagingBuffer{ .size = size };

	auto bufInfo = VkBufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	if (vkCreateBuffer(device, &bufInfo, nullptr, &sb.buffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create staging buffer.");

	auto memReqs = VkMemoryRequirements{};
	vkGetBufferMemoryRequirements(device, sb.buffer, &memReqs);

	auto allocInfo = VkMemoryAllocateInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReqs.size,
		.memoryTypeIndex = hostVisibleMemTypeIndex
	};
	if (vkAllocateMemory(device, &allocInfo, nullptr, &sb.memory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate staging memory.");

	vkBindBufferMemory(device, sb.buffer, sb.memory, 0);
	vkMapMemory(device, sb.memory, 0, size, 0, &sb.mapped);

	return sb;
}

void DestroyStagingBuffer(VkDevice device, StagingBuffer& sb)
{
	if (sb.mapped) { vkUnmapMemory(device, sb.memory); sb.mapped = nullptr; }
	if (sb.buffer) { vkDestroyBuffer(device, sb.buffer, nullptr); sb.buffer = VK_NULL_HANDLE; }
	if (sb.memory) { vkFreeMemory(device, sb.memory, nullptr); sb.memory = VK_NULL_HANDLE; }
}

// Begin recording a single-use command buffer.
void BeginOneTimeCommands(VkCommandBuffer cmd)
{
	auto beginInfo = VkCommandBufferBeginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(cmd, &beginInfo);
}

// End, submit, and wait for a command buffer to complete.
void SubmitAndWait(VkDevice device, VkQueue queue, VkCommandBuffer cmd)
{
	vkEndCommandBuffer(cmd);

	auto submitInfo = VkSubmitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd
	};

	auto fenceInfo = VkFenceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
	};
	auto fence = VkFence{};
	vkCreateFence(device, &fenceInfo, nullptr, &fence);
	vkQueueSubmit(queue, 1, &submitInfo, fence);
	vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(device, fence, nullptr);
}

// Upload a repeating byte pattern to a GPU buffer via a staging copy.
void UploadPattern(
	VkDevice device, VkQueue queue, VkCommandBuffer cmd,
	VkBuffer dstBuffer, VkDeviceSize size,
	unsigned char pattern, std::uint32_t hostVisibleMemTypeIndex
)
{
	auto staging = CreateStagingBuffer(
		device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, hostVisibleMemTypeIndex);
	std::memset(staging.mapped, pattern, size);

	BeginOneTimeCommands(cmd);
	auto copy = VkBufferCopy{ .srcOffset = 0, .dstOffset = 0, .size = size };
	vkCmdCopyBuffer(cmd, staging.buffer, dstBuffer, 1, &copy);
	SubmitAndWait(device, queue, cmd);

	DestroyStagingBuffer(device, staging);
}

// Read back a GPU buffer via staging and verify its contents match a byte pattern.
auto VerifyGpuPattern(
	VkDevice device, VkQueue queue, VkCommandBuffer cmd,
	VkBuffer srcBuffer, VkDeviceSize size,
	unsigned char expected, std::uint32_t hostVisibleMemTypeIndex
) -> bool
{
	auto staging = CreateStagingBuffer(
		device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, hostVisibleMemTypeIndex);

	BeginOneTimeCommands(cmd);
	auto copy = VkBufferCopy{ .srcOffset = 0, .dstOffset = 0, .size = size };
	vkCmdCopyBuffer(cmd, srcBuffer, staging.buffer, 1, &copy);
	SubmitAndWait(device, queue, cmd);

	auto ptr = static_cast<const unsigned char*>(staging.mapped);
	auto ok = true;
	for (VkDeviceSize i = 0; i < size; ++i)
	{
		if (ptr[i] != expected) { ok = false; break; }
	}

	DestroyStagingBuffer(device, staging);
	return ok;
}

auto main() -> int
try
{
	// --- Minimal Vulkan Setup ---
	// We only need an instance, physical device, and logical device.
	// No window, surface, or swapchain — just memory operations.

	auto appInfo = VkApplicationInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = "Vulkan Memory Pool Demo",
		.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		.pEngineName = nullptr,
		.engineVersion = 0,
		.apiVersion = VK_API_VERSION_1_0
	};

	auto instanceInfo = VkInstanceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr
	};

	auto instance = VkInstance{};
	if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
	{
		std::println(std::cerr, "Failed to create Vulkan instance.");
		return 1;
	}

	// Pick the first available physical device.
	auto deviceCount = std::uint32_t{ 0 };
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		std::println(std::cerr, "No Vulkan-capable GPU found.");
		vkDestroyInstance(instance, nullptr);
		return 1;
	}

	auto physicalDevices = std::vector<VkPhysicalDevice>(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
	auto physicalDevice = physicalDevices[0];

	auto deviceProps = VkPhysicalDeviceProperties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProps);
	std::println("GPU: {}", deviceProps.deviceName);
	std::println("Max memory allocations: {}", deviceProps.limits.maxMemoryAllocationCount);
	std::println("");

	// Create a logical device with one queue.
	auto queuePriority = 1.0f;
	auto queueInfo = VkDeviceQueueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueFamilyIndex = 0,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	auto deviceFeatures = VkPhysicalDeviceFeatures{};
	auto deviceInfo = VkDeviceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
		.pEnabledFeatures = &deviceFeatures
	};

	auto device = VkDevice{};
	if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS)
	{
		std::println(std::cerr, "Failed to create logical device.");
		vkDestroyInstance(instance, nullptr);
		return 1;
	}

	// Find a HOST_VISIBLE | HOST_COHERENT memory type so we can map and
	// directly read/write from the CPU without explicit flush/invalidate.
	auto memTypeIndex = FindMemoryType(
		physicalDevice,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	if (not memTypeIndex)
	{
		std::println(std::cerr, "No HOST_VISIBLE | HOST_COHERENT memory type found.");
		vkDestroyDevice(device, nullptr);
		vkDestroyInstance(instance, nullptr);
		return 1;
	}
	std::println("Using memory type index: {}", *memTypeIndex);

	// ==================================================================
	//  CPU Memory Pool Demo — HOST_VISIBLE defragmentation
	// ==================================================================

	auto cpuIntegrityOk = false;

	// The pool must be destroyed BEFORE vkDestroyDevice, because its
	// destructor calls vkUnmapMemory / vkFreeMemory on the device.
	// A nested scope ensures correct teardown order.
	{
		constexpr auto PoolSize = VkDeviceSize{ 4096 }; // 4 KB
		auto pool = Memory::MemoryPool{ device, PoolSize, *memTypeIndex };

		std::println("\n========================================");
		std::println(" Initial Pool State");
		std::println("========================================");
		pool.PrintState();

		// Allocate five blocks with different sizes and alignment requirements.
		//
		//   Name   Size   Align   Purpose (simulated)
		//   ----   ----   -----   -------------------
		//   A      256    64      Vertex data
		//   B      512    128     Index data
		//   C      128    64      Uniform buffer
		//   D      1024   256     Texture staging
		//   E      64     16      Push constant mirror

		auto allocA = pool.Allocate(256, 64);
		auto allocB = pool.Allocate(512, 128);
		auto allocC = pool.Allocate(128, 64);
		auto allocD = pool.Allocate(1024, 256);
		auto allocE = pool.Allocate(64, 16);

		if (not allocA or not allocB or not allocC or not allocD or not allocE)
		{
			std::println(std::cerr, "One or more allocations failed.");
			vkDestroyDevice(device, nullptr);
			vkDestroyInstance(instance, nullptr);
			return 1;
		}

		// Write recognisable byte patterns into each allocation.
		FillPattern(pool, *allocA, 0xAA);
		FillPattern(pool, *allocB, 0xBB);
		FillPattern(pool, *allocC, 0xCC);
		FillPattern(pool, *allocD, 0xDD);
		FillPattern(pool, *allocE, 0xEE);

		std::println("\n========================================");
		std::println(" After 5 Allocations (A-E)");
		std::println("========================================");
		pool.PrintState();

		// Free B and D to create fragmentation — two holes in the middle.
		std::println("\n>>> Freeing B (512 bytes) and D (1024 bytes)...\n");
		pool.Free(*allocB);
		pool.Free(*allocD);

		std::println("========================================");
		std::println(" After Freeing B and D (fragmented)");
		std::println("========================================");
		pool.PrintState();

		std::println("\n  Even though {} bytes are free, the largest",
			pool.GetFreeSize());
		std::println("  contiguous block is only {} bytes across {} fragments.",
			pool.GetLargestFreeBlock(), pool.GetFreeBlockCount());
		std::println("  A 2048-byte allocation would fail despite enough total space.");

		// Defragment — compact A, C, E to the front of the pool.
		std::println("\n>>> Defragmenting...\n");
		auto moves = pool.Defragment();

		std::println("  {} move(s) performed:", moves.size());
		for (const auto& m : moves)
		{
			std::println("    Moved {} bytes: offset {} -> {}",
				m.Size, m.SrcOffset, m.DstOffset);
		}

		// After defrag, allocA/C/E offsets are stale — update them from the
		// move list so VerifyPattern reads from the correct new offsets.
		for (const auto& m : moves)
		{
			if (allocA->Offset == m.SrcOffset) allocA->Offset = m.DstOffset;
			if (allocC->Offset == m.SrcOffset) allocC->Offset = m.DstOffset;
			if (allocE->Offset == m.SrcOffset) allocE->Offset = m.DstOffset;
		}

		std::println("\n========================================");
		std::println(" After Defragmentation");
		std::println("========================================");
		pool.PrintState();

		std::println("\n  Largest contiguous block is now {} bytes — pool is compact.",
			pool.GetLargestFreeBlock());

		// Verify data integrity: patterns should survive the compaction.
		cpuIntegrityOk =
			VerifyPattern(pool, *allocA, 0xAA) and
			VerifyPattern(pool, *allocC, 0xCC) and
			VerifyPattern(pool, *allocE, 0xEE);

		std::println("\n  CPU data integrity after defrag: {}",
			cpuIntegrityOk ? "PASSED" : "FAILED");

	} // <-- cpu pool destroyed here, BEFORE device teardown

	// ==================================================================
	//  GPU Memory Pool Demo — DEVICE_LOCAL defragmentation
	// ==================================================================

	auto gpuIntegrityOk = true;

	std::println("\n\n========================================");
	std::println(" GPU Memory Pool Demo (DEVICE_LOCAL)");
	std::println("========================================");

	{
		auto gpuMemTypeIndex = FindMemoryType(
			physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		if (not gpuMemTypeIndex)
		{
			std::println("  No DEVICE_LOCAL memory type — skipping GPU demo.");
		}
		else
		{
			std::println("  Using DEVICE_LOCAL memory type index: {}\n", *gpuMemTypeIndex);

			// Command infrastructure for GPU copies.
			auto queue = VkQueue{};
			vkGetDeviceQueue(device, 0, 0, &queue);

			auto cmdPoolInfo = VkCommandPoolCreateInfo{
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
				.queueFamilyIndex = 0
			};
			auto cmdPool = VkCommandPool{};
			vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool);

			auto cmdAllocInfo = VkCommandBufferAllocateInfo{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = cmdPool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1
			};
			auto cmd = VkCommandBuffer{};
			vkAllocateCommandBuffers(device, &cmdAllocInfo, &cmd);

			// Nested scope: GpuMemoryPool must be destroyed before the command pool.
			{
				// 8 KB DEVICE_LOCAL pool.
				// TRANSFER_SRC | TRANSFER_DST are required for staging and defrag copies.
				constexpr auto GpuPoolSize = VkDeviceSize{ 8192 };
				auto gpuPool = Memory::GpuMemoryPool{
					device, GpuPoolSize, *gpuMemTypeIndex,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT
					| VK_BUFFER_USAGE_TRANSFER_DST_BIT
					| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
				};

				std::println("--- Initial GPU Pool ---");
				gpuPool.PrintState();

				// Allocate three GPU buffers.
				auto gpuA = gpuPool.Allocate(512, 256);
				auto gpuB = gpuPool.Allocate(1024, 256);
				auto gpuC = gpuPool.Allocate(256, 256);

				if (not gpuA or not gpuB or not gpuC)
				{
					std::println(std::cerr, "  GPU allocation failed.");
					gpuIntegrityOk = false;
				}
				else
				{
					// Upload byte patterns to the GPU via staging.
					UploadPattern(device, queue, cmd, gpuA->Buffer, gpuA->Size, 0xAA, *memTypeIndex);
					UploadPattern(device, queue, cmd, gpuB->Buffer, gpuB->Size, 0xBB, *memTypeIndex);
					UploadPattern(device, queue, cmd, gpuC->Buffer, gpuC->Size, 0xCC, *memTypeIndex);

					std::println("\n--- After 3 GPU Allocations (A=512, B=1024, C=256) ---");
					gpuPool.PrintState();

					// Free B to create a hole in the middle.
					std::println("\n>>> Freeing GPU buffer B (1024 bytes)...\n");
					gpuPool.Free(*gpuB);

					std::println("--- After Freeing B (fragmented) ---");
					gpuPool.PrintState();

					// Phase 1: Record GPU defrag commands into a command buffer.
					std::println("\n>>> Recording defrag commands...\n");
					BeginOneTimeCommands(cmd);
					auto gpuMoves = gpuPool.RecordDefrag(cmd);

					std::println("  {} GPU copy command(s) recorded:", gpuMoves.size());
					for (const auto& m : gpuMoves)
					{
						std::println("    vkCmdCopyBuffer: {} bytes, offset {} -> {}",
							m.Size, m.SrcOffset, m.DstOffset);
					}

					// Submit to the GPU and wait for all copies to finish.
					std::println("\n>>> Submitting to GPU queue and waiting...\n");
					SubmitAndWait(device, queue, cmd);

					// Phase 2: Destroy old buffers and update internal tracking.
					gpuPool.ApplyDefrag(gpuMoves);

					// Update caller-side handles from the move list.
					for (const auto& m : gpuMoves)
					{
						if (gpuA->Buffer == m.OldBuffer) { gpuA->Buffer = m.NewBuffer; gpuA->Offset = m.DstOffset; }
						if (gpuC->Buffer == m.OldBuffer) { gpuC->Buffer = m.NewBuffer; gpuC->Offset = m.DstOffset; }
					}

					std::println("--- After GPU Defragmentation ---");
					gpuPool.PrintState();

					std::println("\n  Largest contiguous block: {} bytes.",
						gpuPool.GetLargestFreeBlock());

					// Verify data integrity by reading back from the GPU.
					gpuIntegrityOk =
						VerifyGpuPattern(device, queue, cmd, gpuA->Buffer, gpuA->Size, 0xAA, *memTypeIndex)
						and VerifyGpuPattern(device, queue, cmd, gpuC->Buffer, gpuC->Size, 0xCC, *memTypeIndex);

					std::println("\n  GPU data integrity after defrag: {}",
						gpuIntegrityOk ? "PASSED" : "FAILED");
				}

			} // <-- gpuPool destroyed here

			vkDestroyCommandPool(device, cmdPool, nullptr);
		}
	}

	// ------------------------------------------------------------------
	//  Cleanup
	// ------------------------------------------------------------------
	vkDeviceWaitIdle(device);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);

	return (cpuIntegrityOk and gpuIntegrityOk) ? 0 : 1;
}
catch (const std::exception& ex)
{
	std::println(std::cerr, "Fatal: {}", ex.what());
	return 1;
}

