// Re-exports a focused subset of the Vulkan C API through a C++20 module partition.
// This isolates the macro-heavy vulkan.h header into a single translation unit,
// keeping the rest of the codebase in clean module territory.
module;

#include <vulkan/vulkan.h>

export module vulkanmem:vulkan.exports;

export namespace vk
{
	using
		::VkResult,
		::VkStructureType,
		::VkDevice,
		::VkDeviceMemory,
		::VkDeviceSize,
		::VkMemoryAllocateInfo,
		::VkMemoryPropertyFlags,
		::VkMemoryPropertyFlagBits,
		::vkAllocateMemory,
		::vkFreeMemory,
		::vkMapMemory,
		::vkUnmapMemory
		;

	// Vulkan macros cannot cross module boundaries; wrap the ones we need.
	constexpr auto Success = VK_SUCCESS;
}
