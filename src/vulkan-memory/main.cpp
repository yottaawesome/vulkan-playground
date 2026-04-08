// Vulkan Memory Pool Demo
// Created with Copilot, as I was too lazy to write it myself and I couldn't 
// find good references online for how to do defragging. This is just a simple 
// demo to show how you might implement manual defragmentation in a 
// sub-allocating memory pool.
//
// Demonstrates manual memory defragmentation with a sub-allocating pool.
// Steps:
//   1. Minimal Vulkan setup (instance + device, no window needed).
//   2. Create a 4 KB memory pool backed by HOST_VISIBLE memory.
//   3. Make five sub-allocations with varying sizes and alignments.
//   4. Free two of them to create fragmentation (holes in the pool).
//   5. Defragment — compact live data forward, closing all gaps.
//   6. Verify data integrity after compaction.

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

	// Create a logical device with one queue (unused — we only need the device handle).
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

	// ------------------------------------------------------------------
	//  Memory Pool Demo
	// ------------------------------------------------------------------

	auto integrityOk = false;

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
		integrityOk =
			VerifyPattern(pool, *allocA, 0xAA) and
			VerifyPattern(pool, *allocC, 0xCC) and
			VerifyPattern(pool, *allocE, 0xEE);

		std::println("\n  Data integrity after defrag: {}",
			integrityOk ? "PASSED" : "FAILED");

	} // <-- pool destroyed here, BEFORE device teardown

	// ------------------------------------------------------------------
	//  Cleanup
	// ------------------------------------------------------------------
	vkDeviceWaitIdle(device);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);

	return integrityOk ? 0 : 1;
}
catch (const std::exception& ex)
{
	std::println(std::cerr, "Fatal: {}", ex.what());
	return 1;
}

