// Re-exports the Vulkan Memory Allocator (VMA) C API through a C++20 module
// partition. VMA's header transitively includes <vulkan/vulkan.h>, so this
// partition also brings the Vulkan symbols into scope for downstream code
// via :vk.exports.
//
// VMA_IMPLEMENTATION is NOT defined here. The implementation lives in a
// single non-module translation unit (vma.impl.cpp) so the export surface
// can be recompiled cheaply without rebuilding the (large) VMA implementation.
module;

#include <vma/vk_mem_alloc.h>

export module vmadefrag:vma.exports;

export import :vk.exports;

export namespace vma
{
	using
		// Allocator / allocation handles
		::VmaAllocator,
		::VmaAllocation,
		::VmaAllocationInfo,
		::VmaAllocatorCreateInfo,
		::VmaAllocatorCreateFlagBits,
		::VmaAllocationCreateInfo,
		::VmaAllocationCreateFlagBits,
		::VmaMemoryUsage,
		::VmaVulkanFunctions,

		// Statistics
		::VmaTotalStatistics,
		::VmaDetailedStatistics,
		::VmaStatistics,

		// Defragmentation
		::VmaDefragmentationInfo,
		::VmaDefragmentationContext,
		::VmaDefragmentationFlagBits,
		::VmaDefragmentationPassMoveInfo,
		::VmaDefragmentationMove,
		::VmaDefragmentationMoveOperation,
		::VmaDefragmentationStats,

		// Functions
		::vmaCreateAllocator,
		::vmaDestroyAllocator,
		::vmaCreateBuffer,
		::vmaDestroyBuffer,
		::vmaBindBufferMemory,
		::vmaGetAllocationInfo,
		::vmaCalculateStatistics,
		::vmaBeginDefragmentation,
		::vmaEndDefragmentation,
		::vmaBeginDefragmentationPass,
		::vmaEndDefragmentationPass
		;
}
