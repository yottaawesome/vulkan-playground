// Re-exports a focused subset of the Vulkan C API through a C++20 module partition.
// This isolates the macro-heavy vulkan.h header into a single translation unit,
// keeping the rest of the codebase in clean module territory.
module;

#include <vulkan/vulkan.h>

export module vulkanmem:vulkan.exports;

export namespace vk
{
	using
		// Core result / structure types
		::VkResult,
		::VkStructureType,

		// Device and memory
		::VkDevice,
		::VkDeviceMemory,
		::VkDeviceSize,
		::VkMemoryAllocateInfo,
		::VkMemoryPropertyFlags,
		::VkMemoryPropertyFlagBits,

		// Buffers
		::VkBuffer,
		::VkBufferCreateInfo,
		::VkBufferUsageFlags,
		::VkBufferUsageFlagBits,
		::VkBufferCopy,
		::VkSharingMode,
		::VkMemoryRequirements,

		// Command recording
		::VkCommandBuffer,

		// Pipeline barriers
		::VkMemoryBarrier,
		::VkPipelineStageFlags,
		::VkPipelineStageFlagBits,
		::VkAccessFlags,
		::VkAccessFlagBits,

		// Memory functions
		::vkAllocateMemory,
		::vkFreeMemory,
		::vkMapMemory,
		::vkUnmapMemory,

		// Buffer functions
		::vkCreateBuffer,
		::vkDestroyBuffer,
		::vkGetBufferMemoryRequirements,
		::vkBindBufferMemory,

		// Command recording functions
		::vkCmdCopyBuffer,
		::vkCmdPipelineBarrier
		;

	// Vulkan macros cannot cross module boundaries; wrap the ones we need.
	constexpr auto Success = VK_SUCCESS;
}
