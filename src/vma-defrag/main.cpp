// VMA Defragmentation Sample (Vulkan 1.4)
// =============================================================================
// This sample demonstrates how to drive VMA's incremental defragmentation API
// in a real-world style: allocate a bunch of buffers through VMA, free a
// subset of them to create fragmentation, then run vmaBeginDefragmentation /
// BeginPass / EndPass / EndDefragmentation to compact the heap.
//
// Modern Vulkan 1.4 features used:
//   - VK_API_VERSION_1_4 instance + device
//   - bufferDeviceAddress feature (core 1.2)            -> vkGetBufferDeviceAddress
//   - VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT    -> VMA tracks BDA-eligible memory
//   - VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT         -> per-buffer BDA opt-in
//
// Why VMA's defragmentation is harder than memmove():
//   When VMA moves an allocation, it allocates fresh memory and asks YOU to
//   1. recreate the VkBuffer (handles are bound to a specific VkDeviceMemory
//      and offset, which both change),
//   2. copy the data across (memcpy for HOST_VISIBLE or vkCmdCopyBuffer for
//      DEVICE_LOCAL),
//   3. destroy the OLD VkBuffer (VMA frees the old memory itself).
//
//   VkDeviceAddresses are also tied to (memory + offset), so any cached BDA
//   for moved buffers must be re-queried after defragmentation.
//
// We use HOST_VISIBLE memory so the defragmentation copy is a plain memcpy
// between mapped pointers; this keeps the sample focused on the VMA flow.
// The same dance with vkCmdCopyBuffer would work for DEVICE_LOCAL memory
// (see README for notes).

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

import std;
import vmadefrag;

// -----------------------------------------------------------------------------
// One tracked buffer in the demo. We hold the VkBuffer + its VmaAllocation,
// remember the size and the byte pattern we filled it with so we can verify
// integrity post-defrag, and cache the device address so we can show that it
// changes when the buffer is moved.
// -----------------------------------------------------------------------------
struct TrackedBuffer
{
	vk::VkBuffer Buffer = nullptr;
	vma::VmaAllocation Allocation = nullptr;
	vk::VkDeviceSize Size = 0;
	vk::VkDeviceAddress Address = 0;
	std::uint8_t Pattern = 0;
	bool Alive = true;
};

// Find a queue family that supports graphics (and therefore transfer/copy too).
auto FindGraphicsQueueFamily(vk::VkPhysicalDevice phys) -> std::optional<std::uint32_t>
{
	auto count = std::uint32_t{ 0 };
	vk::vkGetPhysicalDeviceQueueFamilyProperties(phys, &count, nullptr);
	auto props = std::vector<VkQueueFamilyProperties>(count);
	vk::vkGetPhysicalDeviceQueueFamilyProperties(phys, &count, props.data());

	for (std::uint32_t i = 0; i < count; ++i)
		if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			return i;
	return std::nullopt;
}

// Query the device address of a buffer. Requires the bufferDeviceAddress
// feature to be enabled and the buffer to have been created with
// VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT.
auto QueryDeviceAddress(vk::VkDevice device, vk::VkBuffer buffer) -> vk::VkDeviceAddress
{
	auto info = vk::VkBufferDeviceAddressInfo{
		.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.pNext = nullptr,
		.buffer = buffer
	};
	return vk::vkGetBufferDeviceAddress(device, &info);
}

// Pretty-printer for the live buffer table.
void DumpTable(std::span<const TrackedBuffer> buffers, std::string_view title)
{
	std::println("\n---- {} ----", title);
	std::println("  idx  size       pattern  device-address       state");
	for (std::size_t i = 0; i < buffers.size(); ++i)
	{
		const auto& b = buffers[i];
		if (b.Alive)
		{
			std::println("  {:>3}  {:>6} B   0x{:02X}     0x{:016X}   live",
				i, b.Size, b.Pattern, b.Address);
		}
		else
		{
			std::println("  {:>3}  {:>6} B   0x{:02X}     {:>20}   freed",
				i, b.Size, b.Pattern, "-");
		}
	}
}

// Print the VMA allocator's view of the heap. Useful to show fragmentation
// before / after defrag.
void DumpStats(vma::VmaAllocator allocator, std::string_view title)
{
	auto stats = vma::VmaTotalStatistics{};
	vma::vmaCalculateStatistics(allocator, &stats);

	const auto& s = stats.total.statistics;
	std::println("\n  [{}]", title);
	std::println("    blocks         : {}",   s.blockCount);
	std::println("    block bytes    : {}",   s.blockBytes);
	std::println("    allocations    : {}",   s.allocationCount);
	std::println("    allocated bytes: {}",   s.allocationBytes);
	std::println("    unused bytes   : {}",   s.blockBytes - s.allocationBytes);
	std::println("    unused ranges  : {}",   stats.total.unusedRangeCount);
	std::println("    largest free   : {} B", stats.total.unusedRangeSizeMax);
}

// Allocate one tracked buffer through VMA. We request:
//   - SHADER_DEVICE_ADDRESS so vkGetBufferDeviceAddress works
//   - TRANSFER_SRC|DST so vkCmdCopyBuffer-based defrag would also work
//   - STORAGE_BUFFER as a representative shader-visible usage
// On the VMA side we ask for HOST-accessible memory so we can both fill the
// buffer with a recognisable pattern and run a memcpy-based defrag pass.
auto CreateTrackedBuffer(
	vma::VmaAllocator allocator,
	vk::VkDevice device,
	vk::VkDeviceSize size,
	std::uint8_t pattern
) -> TrackedBuffer
{
	auto bufInfo = vk::VkBufferCreateInfo{
		.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage =
			static_cast<vk::VkBufferUsageFlags>(
				vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT
				| vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT
				| vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
				| vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT),
		.sharingMode = vk::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE
	};

	auto allocCreate = vma::VmaAllocationCreateInfo{
		.flags =
			vma::VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
			| vma::VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = vma::VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
	};

	auto t = TrackedBuffer{ .Size = size, .Pattern = pattern };
	auto info = vma::VmaAllocationInfo{};
	if (vma::vmaCreateBuffer(allocator, &bufInfo, &allocCreate,
		&t.Buffer, &t.Allocation, &info) != vk::Success)
		throw std::runtime_error("vmaCreateBuffer failed.");

	// Fill the buffer with its byte pattern via the persistent mapping so we
	// can later assert the data survived the defrag move.
	std::memset(info.pMappedData, pattern, static_cast<std::size_t>(size));

	t.Address = QueryDeviceAddress(device, t.Buffer);
	return t;
}

// Verify the buffer's bytes still match its expected pattern.
auto VerifyTrackedBuffer(vma::VmaAllocator allocator, const TrackedBuffer& t) -> bool
{
	auto info = vma::VmaAllocationInfo{};
	vma::vmaGetAllocationInfo(allocator, t.Allocation, &info);
	auto* p = static_cast<const std::uint8_t*>(info.pMappedData);
	for (vk::VkDeviceSize i = 0; i < t.Size; ++i)
		if (p[i] != t.Pattern) return false;
	return true;
}

// -----------------------------------------------------------------------------
// Run a single defragmentation pass.
//
// Returns the VkResult from vmaEndDefragmentationPass:
//   VK_SUCCESS    -> defragmentation is fully complete, exit the outer loop.
//   VK_INCOMPLETE -> more passes are still possible / useful, continue.
//   anything else -> error.
//
// For each move VMA hands us, we:
//   1. Create a new VkBuffer with the same description as the moved buffer.
//   2. Bind it to the freshly-allocated dstTmpAllocation via vmaBindBufferMemory.
//   3. Copy the data across (memcpy via mapped pointers — works because we
//      requested HOST_ACCESS + MAPPED memory).
//   4. Destroy the OLD VkBuffer. (VMA frees the old VmaAllocation itself
//      after EndPass; we just replace the buffer handle in our tracking.)
//
// The default move operation (VMA_DEFRAGMENTATION_MOVE_OPERATION_COPY) tells
// VMA to swap the underlying allocation pointer so our stored
// `tracked.Allocation` value continues to be valid post-pass — it just refers
// to the new (compacted) memory.
// -----------------------------------------------------------------------------
auto RunDefragPass(
	vma::VmaAllocator allocator,
	vk::VkDevice device,
	vma::VmaDefragmentationContext ctx,
	std::span<TrackedBuffer> buffers
) -> vk::VkResult
{
	auto pass = vma::VmaDefragmentationPassMoveInfo{};
	auto begin = vma::vmaBeginDefragmentationPass(allocator, ctx, &pass);
	if (begin == vk::Success)
		return vk::Success; // nothing more to do

	if (begin != vk::Incomplete)
		throw std::runtime_error("vmaBeginDefragmentationPass failed.");

	std::println("    pass moves: {}", pass.moveCount);

	for (std::uint32_t i = 0; i < pass.moveCount; ++i)
	{
		auto& move = pass.pMoves[i];

		// Locate the TrackedBuffer entry that owns this allocation so we can
		// recreate its VkBuffer and update our cached handle/address.
		auto it = std::ranges::find_if(buffers, [&](const TrackedBuffer& b) {
			return b.Alive and b.Allocation == move.srcAllocation;
		});
		if (it == buffers.end())
			throw std::runtime_error("Defrag: unknown source allocation.");

		// Re-create a buffer with the same description as the original.
		auto bufInfo = vk::VkBufferCreateInfo{
			.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = it->Size,
			.usage =
				static_cast<vk::VkBufferUsageFlags>(
					vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT
					| vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT
					| vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
					| vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT),
			.sharingMode = vk::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE
		};

		auto newBuffer = vk::VkBuffer{ nullptr };
		if (vk::vkCreateBuffer(device, &bufInfo, nullptr, &newBuffer) != vk::Success)
			throw std::runtime_error("Defrag: vkCreateBuffer failed.");

		// Bind the new buffer to the temporary destination allocation that
		// VMA reserved at the compacted location.
		if (vma::vmaBindBufferMemory(allocator, move.dstTmpAllocation, newBuffer)
			!= vk::Success)
		{
			throw std::runtime_error("Defrag: vmaBindBufferMemory failed.");
		}

		// Copy the data. Because both src and dst allocations were created
		// with HOST_ACCESS|MAPPED, VMA gives us mapped pointers for both.
		auto srcInfo = vma::VmaAllocationInfo{};
		auto dstInfo = vma::VmaAllocationInfo{};
		vma::vmaGetAllocationInfo(allocator, move.srcAllocation, &srcInfo);
		vma::vmaGetAllocationInfo(allocator, move.dstTmpAllocation, &dstInfo);
		std::memcpy(dstInfo.pMappedData, srcInfo.pMappedData,
			static_cast<std::size_t>(it->Size));

		// Destroy the old VkBuffer handle. VMA itself owns the old allocation
		// and will free it after vmaEndDefragmentationPass.
		vma::vmaDestroyBuffer(allocator, it->Buffer, nullptr);
		// vmaDestroyBuffer with a null allocation just deletes the buffer;
		// VMA still tracks the old VmaAllocation internally for the swap.
		// (The allocation is *not* freed here — it gets freed inside
		// vmaEndDefragmentationPass when the move's swap completes.)

		// Update our tracking. After EndPass, move.srcAllocation will be
		// re-pointed at the new memory, so we keep the same Allocation handle.
		it->Buffer = newBuffer;

		// Default operation == COPY. Setting it explicitly is cheap and
		// documents intent for readers.
		move.operation =
			vma::VmaDefragmentationMoveOperation::VMA_DEFRAGMENTATION_MOVE_OPERATION_COPY;
	}

	// EndPass tells VMA: we performed all the COPYs you asked for; commit.
	auto end = vma::vmaEndDefragmentationPass(allocator, ctx, &pass);

	// Some buffers were given new VkBuffer handles; their device addresses
	// are tied to (VkDeviceMemory, offset) so they MUST be re-queried.
	for (auto& b : buffers)
		if (b.Alive)
			b.Address = QueryDeviceAddress(device, b.Buffer);

	if (end != vk::Success and end != vk::Incomplete)
		throw std::runtime_error("vmaEndDefragmentationPass failed.");

	return end;
}

auto main() -> int
try
{
	// -------------------------------------------------------------------------
	// 1. Vulkan 1.4 instance.
	// -------------------------------------------------------------------------
	auto appInfo = vk::VkApplicationInfo{
		.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "VMA Defrag Sample",
		.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		.pEngineName = "vma-defrag",
		.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		.apiVersion = vk::ApiVersion1_4
	};
	auto instInfo = vk::VkInstanceCreateInfo{
		.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo
	};
	auto instance = vk::VkInstance{ nullptr };
	if (vk::vkCreateInstance(&instInfo, nullptr, &instance) != vk::Success)
	{
		std::println(std::cerr, "Failed to create Vulkan instance.");
		return 1;
	}

	// -------------------------------------------------------------------------
	// 2. Pick the first physical device and find a graphics queue family.
	// -------------------------------------------------------------------------
	auto count = std::uint32_t{ 0 };
	vk::vkEnumeratePhysicalDevices(instance, &count, nullptr);
	if (count == 0) { std::println(std::cerr, "No Vulkan GPU."); return 1; }
	auto physes = std::vector<vk::VkPhysicalDevice>(count);
	vk::vkEnumeratePhysicalDevices(instance, &count, physes.data());
	auto phys = physes[0];

	auto props = vk::VkPhysicalDeviceProperties{};
	vk::vkGetPhysicalDeviceProperties(phys, &props);
	std::println("GPU: {}", props.deviceName);

	auto queueFamily = FindGraphicsQueueFamily(phys);
	if (not queueFamily) { std::println(std::cerr, "No graphics queue."); return 1; }

	// -------------------------------------------------------------------------
	// 3. Logical device with bufferDeviceAddress + synchronization2 enabled.
	//    These features are core in Vulkan 1.2 / 1.3 respectively but must
	//    still be opted into via the feature structures.
	// -------------------------------------------------------------------------
	auto priority = 1.0f;
	auto qInfo = vk::VkDeviceQueueCreateInfo{
		.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = *queueFamily,
		.queueCount = 1,
		.pQueuePriorities = &priority
	};

	auto v13 = vk::VkPhysicalDeviceVulkan13Features{
		.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.synchronization2 = VK_TRUE,
	};
	auto v12 = vk::VkPhysicalDeviceVulkan12Features{
		.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &v13,
		.bufferDeviceAddress = VK_TRUE,
	};
	auto features2 = vk::VkPhysicalDeviceFeatures2{
		.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &v12,
	};
	auto devInfo = vk::VkDeviceCreateInfo{
		.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features2,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &qInfo,
	};
	auto device = vk::VkDevice{ nullptr };
	if (vk::vkCreateDevice(phys, &devInfo, nullptr, &device) != vk::Success)
	{
		std::println(std::cerr, "vkCreateDevice failed.");
		vk::vkDestroyInstance(instance, nullptr);
		return 1;
	}

	// -------------------------------------------------------------------------
	// 4. VMA allocator. Telling VMA which Vulkan version we target lets it
	//    pick the right entry points (e.g. KHR vs core memory model APIs).
	//    BUFFER_DEVICE_ADDRESS_BIT is required so VMA marks every memory
	//    block it allocates as eligible for vkGetBufferDeviceAddress.
	// -------------------------------------------------------------------------
	auto vmaInfo = vma::VmaAllocatorCreateInfo{
		.flags =
			vma::VmaAllocatorCreateFlagBits::VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = phys,
		.device = device,
		.instance = instance,
		.vulkanApiVersion = vk::ApiVersion1_4,
	};
	auto allocator = vma::VmaAllocator{ nullptr };
	if (vma::vmaCreateAllocator(&vmaInfo, &allocator) != vk::Success)
	{
		std::println(std::cerr, "vmaCreateAllocator failed.");
		vk::vkDestroyDevice(device, nullptr);
		vk::vkDestroyInstance(instance, nullptr);
		return 1;
	}

	auto exitCode = 0;
	{
		// ---------------------------------------------------------------------
		// 5. Allocate a fragmented set of buffers.
		//    Mixed sizes encourage VMA to interleave them on a few blocks;
		//    distinct byte patterns let us verify per-buffer survival later.
		// ---------------------------------------------------------------------
		auto plan = std::vector<std::pair<vk::VkDeviceSize, std::uint8_t>>{
			{  4 * 1024, 0xA1 }, {  16 * 1024, 0xA2 }, {   8 * 1024, 0xA3 },
			{ 32 * 1024, 0xA4 }, {   4 * 1024, 0xA5 }, {  64 * 1024, 0xA6 },
			{  8 * 1024, 0xA7 }, {  16 * 1024, 0xA8 }, {  32 * 1024, 0xA9 },
			{  4 * 1024, 0xAA }, {   8 * 1024, 0xAB }, {  16 * 1024, 0xAC },
		};
		auto buffers = std::vector<TrackedBuffer>{};
		buffers.reserve(plan.size());
		for (auto [sz, pat] : plan)
			buffers.push_back(CreateTrackedBuffer(allocator, device, sz, pat));

		DumpTable(buffers, "After initial allocations");
		DumpStats(allocator, "Initial");

		// ---------------------------------------------------------------------
		// 6. Free every other buffer to create holes.
		// ---------------------------------------------------------------------
		std::println("\n>>> Freeing every other buffer to fragment the heap...");
		for (std::size_t i = 1; i < buffers.size(); i += 2)
		{
			vma::vmaDestroyBuffer(allocator, buffers[i].Buffer, buffers[i].Allocation);
			buffers[i].Buffer = nullptr;
			buffers[i].Allocation = nullptr;
			buffers[i].Alive = false;
		}

		DumpTable(buffers, "After fragmenting");
		DumpStats(allocator, "Fragmented");

		// Stash addresses pre-defrag so we can show which ones changed.
		auto addressesBefore = std::vector<vk::VkDeviceAddress>{};
		for (const auto& b : buffers) addressesBefore.push_back(b.Address);

		// ---------------------------------------------------------------------
		// 7. Run the defragmentation loop.
		// ---------------------------------------------------------------------
		std::println("\n>>> Starting VMA defragmentation...");
		auto defragInfo = vma::VmaDefragmentationInfo{
			// FAST mode prioritises throughput; BALANCED / FULL do more work.
			.flags =
				vma::VmaDefragmentationFlagBits::VMA_DEFRAGMENTATION_FLAG_ALGORITHM_FAST_BIT,
		};
		auto ctx = vma::VmaDefragmentationContext{ nullptr };
		if (vma::vmaBeginDefragmentation(allocator, &defragInfo, &ctx) != vk::Success)
			throw std::runtime_error("vmaBeginDefragmentation failed.");

		// Iterate passes until VMA tells us there is nothing left to do.
		// Each pass produces a (possibly empty) batch of moves we must service.
		auto passNumber = 0;
		while (true)
		{
			std::println("  pass {}:", ++passNumber);
			auto r = RunDefragPass(allocator, device, ctx, buffers);
			if (r == vk::Success) break;
		}

		auto stats = vma::VmaDefragmentationStats{};
		vma::vmaEndDefragmentation(allocator, ctx, &stats);
		std::println("\n  Defragmentation summary:");
		std::println("    bytes moved          : {}", stats.bytesMoved);
		std::println("    bytes freed          : {}", stats.bytesFreed);
		std::println("    allocations moved    : {}", stats.allocationsMoved);
		std::println("    device-memory blocks freed: {}", stats.deviceMemoryBlocksFreed);

		DumpTable(buffers, "After defragmentation");
		DumpStats(allocator, "Defragmented");

		// ---------------------------------------------------------------------
		// 8. Show which device addresses changed (BDA is tied to memory layout).
		// ---------------------------------------------------------------------
		std::println("\n  Device-address changes:");
		auto changed = 0;
		for (std::size_t i = 0; i < buffers.size(); ++i)
		{
			if (not buffers[i].Alive) continue;
			if (buffers[i].Address != addressesBefore[i])
			{
				std::println("    buf {:>2}: 0x{:016X} -> 0x{:016X}",
					i, addressesBefore[i], buffers[i].Address);
				++changed;
			}
		}
		std::println("    {} live buffer(s) had their device address change.", changed);

		// ---------------------------------------------------------------------
		// 9. Verify byte patterns survived the moves.
		// ---------------------------------------------------------------------
		auto allOk = true;
		for (const auto& b : buffers)
			if (b.Alive and not VerifyTrackedBuffer(allocator, b)) allOk = false;
		std::println("\n  Data integrity after defrag: {}", allOk ? "PASSED" : "FAILED");
		if (not allOk) exitCode = 1;

		// ---------------------------------------------------------------------
		// 10. Cleanup remaining tracked buffers (frees their VMA allocations).
		// ---------------------------------------------------------------------
		for (auto& b : buffers)
			if (b.Alive)
				vma::vmaDestroyBuffer(allocator, b.Buffer, b.Allocation);
	}

	// -------------------------------------------------------------------------
	// 11. Tear-down. Order matters: allocator before device, device before instance.
	// -------------------------------------------------------------------------
	vma::vmaDestroyAllocator(allocator);
	vk::vkDeviceWaitIdle(device);
	vk::vkDestroyDevice(device, nullptr);
	vk::vkDestroyInstance(instance, nullptr);
	return exitCode;
}
catch (const std::exception& ex)
{
	std::println(std::cerr, "Fatal: {}", ex.what());
	return 1;
}
