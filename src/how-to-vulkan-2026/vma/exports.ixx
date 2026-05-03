module;

// This partition re-exports the Vulkan Memory Allocator C API into the
// `vma::` namespace. It `#include`s both <volk.h> and <vma/vk_mem_alloc.h>
// in the global module fragment because VMA's header depends on Vulkan
// types being in scope.
//
// Neither VOLK_IMPLEMENTATION nor VMA_IMPLEMENTATION is defined here — those
// must appear in exactly one translation unit in the program
// (see vulkan\vulkan.impl.cpp). Re-including these headers in additional
// TUs without the *_IMPLEMENTATION macros is safe; they are just forward
// declarations and inline helpers guarded by include guards.
#include <volk.h>
#include <vma/vk_mem_alloc.h>

export module vulkan26:vma.exports;

// Re-export the Vulkan surface so any consumer that imports :vma.exports
// automatically gets `vk::` and `volk::` too. This mirrors the real
// dependency direction (VMA sits on top of Vulkan).
export import :vulkan.exports;

export namespace vma
{
	using
		::VmaAllocator,
		::VmaAllocation,
		::VmaAllocationCreateInfo,
		::VmaAllocationInfo,
		::VmaAllocatorCreateInfo,
		::VmaVulkanFunctions,
		::VmaAllocatorCreateFlagBits,
		::VmaAllocationCreateFlagBits,
		::VmaMemoryUsage,
		::vmaCreateBuffer,
		::vmaCreateImage,
		::vmaCreateAllocator,
		::vmaDestroyAllocator,
		::vmaAllocateMemory,
		::vmaFreeMemory,
		::vmaDestroyImage,
		::vmaDestroyBuffer
		;
}
