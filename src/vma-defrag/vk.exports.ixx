// Re-exports a focused subset of the Vulkan C API through a C++20 module
// partition. The macro-heavy <vulkan/vulkan.h> header is included only here
// so the rest of the project can stay in clean module territory.
//
// We deliberately target Vulkan 1.4 features in this sample:
//   - bufferDeviceAddress (core 1.2, promoted)
//   - synchronization2     (core 1.3)
//   - vmaCreateAllocator with VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
module;

#include <vulkan/vulkan.h>

export module vmadefrag:vk.exports;

export namespace vk
{
	using
		// Result / structure types
		::VkResult,
		::VkStructureType,

		// Instance / physical device / device
		::VkInstance,
		::VkInstanceCreateInfo,
		::VkApplicationInfo,
		::VkPhysicalDevice,
		::VkPhysicalDeviceProperties,
		::VkPhysicalDeviceFeatures,
		::VkPhysicalDeviceFeatures2,
		::VkPhysicalDeviceVulkan12Features,
		::VkPhysicalDeviceVulkan13Features,
		::VkDevice,
		::VkDeviceCreateInfo,
		::VkDeviceQueueCreateInfo,
		::VkQueue,

		// Buffers / memory
		::VkBuffer,
		::VkBufferCreateInfo,
		::VkBufferUsageFlags,
		::VkBufferUsageFlagBits,
		::VkBufferCopy,
		::VkBufferDeviceAddressInfo,
		::VkDeviceAddress,
		::VkDeviceMemory,
		::VkDeviceSize,
		::VkSharingMode,
		::VkMemoryRequirements,

		// Command recording
		::VkCommandPool,
		::VkCommandPoolCreateInfo,
		::VkCommandBuffer,
		::VkCommandBufferAllocateInfo,
		::VkCommandBufferBeginInfo,
		::VkCommandBufferLevel,
		::VkSubmitInfo,
		::VkFence,
		::VkFenceCreateInfo,

		// Functions used directly outside the impl unit
		::vkCreateInstance,
		::vkDestroyInstance,
		::vkEnumeratePhysicalDevices,
		::vkGetPhysicalDeviceProperties,
		::vkGetPhysicalDeviceQueueFamilyProperties,
		::vkCreateDevice,
		::vkDestroyDevice,
		::vkGetDeviceQueue,
		::vkDeviceWaitIdle,
		::vkGetBufferDeviceAddress,
		::vkCreateBuffer,
		::vkDestroyBuffer,
		::vkCreateCommandPool,
		::vkDestroyCommandPool,
		::vkAllocateCommandBuffers,
		::vkBeginCommandBuffer,
		::vkEndCommandBuffer,
		::vkResetCommandBuffer,
		::vkQueueSubmit,
		::vkCreateFence,
		::vkDestroyFence,
		::vkWaitForFences,
		::vkResetFences,
		::vkCmdCopyBuffer
		;

	// Vulkan macros cannot cross module boundaries — wrap the constants we use.
	constexpr auto Success = VK_SUCCESS;
	constexpr auto Incomplete = VK_INCOMPLETE;
	constexpr auto ApiVersion1_4 = VK_API_VERSION_1_4;
}
