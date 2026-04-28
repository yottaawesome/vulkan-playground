module;

// This partition re-exports the Vulkan C API (via volk) into the `vk::` namespace
// and volk's loader entry points into the `volk::` namespace.
//
// NOTE: Implementation macros (VOLK_IMPLEMENTATION, VMA_IMPLEMENTATION) are
// deliberately NOT defined here. Those definitions live in a single
// non-module translation unit (vulkan.impl.cpp) so that the export surface
// can be recompiled cheaply without rebuilding the volk/VMA implementations,
// and so that the `vma::` exports can live in their own partition
// (:vma.exports) without duplicating those one-definition-rule symbols.
#include <volk.h>

export module vulkan26:vulkan.exports;

export namespace vk
{
	using
		::VkInstance,
		::VkInstanceCreateInfo,
		::VkPhysicalDevice,
		::VkDevice,
		::VkQueue,
		::VkCommandPool,
		::VkCommandBuffer,
		::VkSemaphore,
		::VkFence,
		::VkBuffer,
		::VkDeviceMemory,
		::VkImage,
		::VkImageView,
		::VkSampler,
		::VkShaderModule,
		::VkPipelineLayout,
		::VkPipeline,
		::VkRenderPass,
		::VkFramebuffer,
		::VkApplicationInfo,
		::VkStructureType,
		::VkResult,
		::VkPhysicalDevice,
		::VkPhysicalDeviceProperties,
		::VkPhysicalDeviceProperties2,
		::VkPhysicalDeviceType,
		::VkQueueFamilyProperties2,
		::VkQueueFlags,
		::VkQueueFlagBits,
		::VkDeviceQueueCreateInfo,
		::VkPhysicalDeviceVulkan12Features,
		::VkPhysicalDeviceVulkan13Features,
		::VkPhysicalDeviceFeatures,
		::VkDeviceCreateInfo,
		::VkPhysicalDeviceVulkan14Features,
		::VkSurfaceKHR,
		::VkSurfaceCapabilitiesKHR,
		::VkFormat,
		::VkSwapchainCreateInfoKHR,
		::VkSwapchainKHR,
		::VkExtent2D,
		::VkImageUsageFlags,
		::VkImageUsageFlagBits,
		::VkColorSpaceKHR,
		::VkSurfaceTransformFlagBitsKHR,
		::VkCompositeAlphaFlagBitsKHR,
		::VkPresentModeKHR,
		::VkImage,
		::VkFormatProperties,
		::VkFormatProperties2,
		::VkFormatFeatureFlagBits,
		::VkImageCreateInfo,
		::VkImageType,
		::VkSampleCountFlagBits,
		::VkImageTiling,
		::VkSharingMode,
		::VkImageLayout,
		::VkExtent3D,
		::VkImageViewCreateInfo,
		::VkImageViewType,
		::VkImageAspectFlagBits,
		::VkDeviceSize,
		::VkBufferUsageFlagBits,
		::VkBufferCreateInfo,
		::VkDeviceAddress,
		::VkFenceCreateInfo,
		::VkSemaphoreCreateInfo,
		::VkFenceCreateFlags,
		::VkFenceCreateFlagBits,
		::VkSemaphoreCreateFlags,
		::vkCreateSemaphore,
		::vkCreateFence,
		::vkDestroySemaphore,
		::vkDestroyFence,
		::vkDestroyImageView,
		::vkDestroyImage,
		::vkCreateImageView,
		::vkGetPhysicalDeviceFormatProperties,
		::vkGetSwapchainImagesKHR,
		::vkDestroySwapchainKHR,
		::vkCreateSwapchainKHR,
		::vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
		::vkDestroySurfaceKHR,
		::vkGetInstanceProcAddr,
		::vkGetDeviceProcAddr,
		::vkGetDeviceQueue,
		::vkCreateDevice,
		::vkDestroyDevice,
		::vkGetPhysicalDeviceQueueFamilyProperties2,
		::vkDestroyInstance,
		::vkGetPhysicalDeviceProperties,
		::vkGetPhysicalDeviceProperties2,
		::vkCreateInstance,
		::vkEnumeratePhysicalDevices
		;

	constexpr auto MakeVersion(uint32_t major, uint32_t minor, uint32_t patch) noexcept -> uint32_t
	{
		return VK_MAKE_VERSION(major, minor, patch);
	}

	namespace ApiVersion
	{
		constexpr auto V1_0 = VK_API_VERSION_1_0;
		constexpr auto V1_1 = VK_API_VERSION_1_1;
		constexpr auto V1_2 = VK_API_VERSION_1_2;
		constexpr auto V1_3 = VK_API_VERSION_1_3;
		constexpr auto V1_4 = VK_API_VERSION_1_4;
	}

	namespace DeviceExtension
	{
		constexpr auto Swapchain = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	}
}

// We need to use the volk header here. If we use the Vulkan header, the global 
// function pointers won't be populated, and we'll get runtime crashes when
// invoking Vulkan functions.
export namespace volk
{
	/* 
	* Volk has three loading levels — each one populates a different set of function pointers:
	*   1. volkInitialize() — global functions only (vkCreateInstance, vkEnumerateInstanceExtensionProperties, etc.)
	*   2. volkLoadInstance(instance) — instance-level functions (vkEnumeratePhysicalDevices, vkCreateDevice, vkGetPhysicalDeviceProperties, etc.)
	*   3. volkLoadDevice(device) — device-level functions (vkCreateBuffer, vkCmdDraw, etc.) — you'll need this later when you create a logical device.
	*/
	using
		::volkInitialize,
		::volkLoadInstance,
		::volkLoadDevice
		;

	auto Initialize() noexcept -> vk::VkResult
	{
		return volkInitialize();
	}
	auto LoadInstance(vk::VkInstance instance) noexcept -> void
	{
		volkLoadInstance(instance);
	}
	auto LoadDevice(vk::VkDevice device) noexcept -> void
	{
		volkLoadDevice(device);
	}
}
