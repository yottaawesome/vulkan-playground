module;

#define VULKAN_HPP_NO_CONSTRUCTORS
#define VK_USE_PLATFORM_WIN32_KHR 
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.hpp>

export module vulkangfx:vulkan.exports;

// Raw vulkan types and functions, not vk:: types.
export namespace vkr
{
	using
		::VkInstance,
		::VkInstanceCreateInfo,
		::VkStructureType,
		::VkPhysicalDeviceProperties,
		::VkApplicationInfo,
		::VkInstanceCreateFlags,
		::VkPhysicalDeviceType,
		::VkPhysicalDevice,
		::VkDevice,
		::VkDeviceCreateInfo,
		::VkQueue,
		::VkImage,
		::VkImageView,
		::VkWin32SurfaceCreateInfoKHR,
		::VkSurfaceKHR,
		::VkDebugUtilsMessageSeverityFlagsEXT,
		::VkDebugUtilsMessageTypeFlagsEXT,
		::VkResult,
		::VkExtensionProperties,
		::VkLayerProperties,
		::VkDebugUtilsMessengerCreateInfoEXT,
		::VkDebugUtilsMessengerEXT,
		::PFN_vkCreateDebugUtilsMessengerEXT,
		::VkDebugUtilsMessageSeverityFlagBitsEXT,
		::VkDebugUtilsMessageTypeFlagBitsEXT,
		::VkDebugUtilsMessengerCallbackDataEXT,
		::VkDebugUtilsMessageTypeFlagsEXT,
		::PFN_vkDestroyDebugUtilsMessengerEXT,
		::VkBool32,
		::VkWin32SurfaceCreateInfoKHR,
		::VkDeviceQueueCreateInfo,
		::VkDeviceQueueCreateFlags,
		::VkPhysicalDeviceFeatures,
		::VkQueueFamilyProperties,
		::VkQueueFlagBits,
		::VkDeviceCreateFlags,
		::VkPhysicalDeviceFeatures2,
		::VkPhysicalDeviceVulkan11Features,
		::VkPhysicalDeviceVulkan12Features,
		::VkPhysicalDeviceVulkan13Features,
		::VkPhysicalDeviceVulkan14Features,
		::VkSwapchainCreateInfoKHR,
		::VkSwapchainKHR,
		::VkSurfaceCapabilitiesKHR,
		::VkSurfaceFormatKHR,
		::VkPresentModeKHR,
		::VkExtent2D,
		::VkSurfaceFormat2KHR,
		::VkSurfaceFormatKHR,
		::VkFormat,
		::VkColorSpaceKHR,
		::VkImageUsageFlagBits,
		::VkCompositeAlphaFlagBitsKHR,
		::VkSharingMode,
		::VkImageViewCreateInfo,
		::VkComponentMapping,
		::VkComponentSwizzle,
		::VkImageViewCreateFlags,
		::VkImageViewType,
		::VkImageSubresourceRange,
		::VkImageAspectFlagBits,
		::VkShaderModule,
		::VkShaderModuleCreateInfo,
		::VkPipelineShaderStageCreateInfo,
		::VkShaderStageFlagBits,
		::VkDynamicState,
		::VkPipelineDynamicStateCreateInfo,
		::VkPipelineDynamicStateCreateFlags,
		::VkPipelineViewportStateCreateInfo,
		::VkPipelineViewportStateCreateFlags,
		::VkRect2D,
		::VkViewport,
		::VkPipelineVertexInputStateCreateInfo,
		::VkPipelineInputAssemblyStateCreateInfo,
		::VkPipelineRasterizationStateCreateInfo,
		::VkPrimitiveTopology,
		::VkPolygonMode,
		::VkFrontFace,
		::VkCullModeFlagBits,
		::VkCullModeFlags,
		::VkPipelineMultisampleStateCreateInfo,
		::VkSampleCountFlagBits,
		::VkSampleCountFlags,
		::VkPipelineColorBlendAttachmentState,
		::VkColorComponentFlagBits,
		::VkPipelineColorBlendStateCreateInfo,
		::VkPipelineLayout,
		::VkPipelineLayoutCreateInfo,
		::VkPipeline,
		::VkPhysicalDeviceDynamicRenderingFeatures,
		::VkRenderPass,
		::VkRenderPassCreateInfo,
		::VkPipelineRenderingCreateInfo,
		::VkGraphicsPipelineCreateInfo,
		::vkCreateGraphicsPipelines,
		::vkDestroyRenderPass,
		::vkDestroyPipeline,
		::vkDestroyPipelineLayout,
		::vkCreatePipelineLayout,
		::vkCreateShaderModule,
		::vkDestroyShaderModule,
		::vkCreateImageView,
		::vkDestroyImageView,
		::vkGetSwapchainImagesKHR,
		::vkGetPhysicalDeviceSurfacePresentModesKHR,
		::vkGetPhysicalDeviceSurfaceFormatsKHR,
		::vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
		::vkGetPhysicalDeviceSurfaceSupportKHR,
		::vkCreateSwapchainKHR,
		::vkDestroySwapchainKHR,
		::vkCreateDevice,
		::vkDestroyDevice,
		::vkEnumeratePhysicalDevices,
		::vkCreateWin32SurfaceKHR,
		::vkDestroySurfaceKHR,
		::vkCreateDebugUtilsMessengerEXT,
		::vkGetInstanceProcAddr,
		::vkEnumerateInstanceExtensionProperties,
		::vkEnumerateInstanceLayerProperties,
		::vkCreateInstance,
		::vkDestroyInstance,
		::vkEnumeratePhysicalDevices,
		::vkGetPhysicalDeviceProperties,
		::vkGetPhysicalDeviceQueueFamilyProperties,
		::vkCreateDevice,
		::vkDestroyDevice,
		::vkGetDeviceQueue,
		::vkCreateWin32SurfaceKHR,
		::vkDestroySurfaceKHR
		;

	auto VersionToString(std::uint32_t version) noexcept -> std::string
	{
		return std::format(
			"{}.{}.{}",
			VK_VERSION_MAJOR(version),
			VK_VERSION_MINOR(version),
			VK_VERSION_PATCH(version)
		);
	}

	auto PhysicalDeviceTypeToString(vkr::VkPhysicalDeviceType type) noexcept -> const char*
	{
		return string_VkPhysicalDeviceType(type);
	}

	constexpr auto False = VkBool32{ VK_FALSE };
	constexpr auto True = VkBool32{ VK_TRUE };

	// We don't use string_VkResult due to it being declared as static inline 
	// in the header, which means it won't be exported from this module.
	auto VkResultToString(VkResult result) noexcept -> const char*
	{
		return string_VkResult(result);
	}

	constexpr auto MakeVersion(
		std::uint32_t major, 
		std::uint32_t minor, 
		std::uint32_t patch
	) noexcept -> std::uint32_t
	{
		return VK_MAKE_VERSION(major, minor, patch);
	}

	constexpr auto MakeApiVersion(
		std::uint32_t major, 
		std::uint32_t minor, 
		std::uint32_t patch
	) noexcept -> std::uint32_t
	{
		// First parameter is variant.
		// See: https://docs.vulkan.org/refpages/latest/refpages/source/VK_MAKE_API_VERSION.html
		return VK_MAKE_API_VERSION(0, major, minor, patch);
	}

	enum class Versions : std::uint32_t
	{
		Vulkan10 = VK_API_VERSION_1_0,
		Vulkan11 = VK_API_VERSION_1_1,
		Vulkan12 = VK_API_VERSION_1_2,
		Vulkan13 = VK_API_VERSION_1_3,
		Vulkan14 = VK_API_VERSION_1_4
	};

	constexpr auto VkSuccess = VkResult{ VK_SUCCESS };

	namespace Extensions
	{
		constexpr auto 
			EXTDebugUtils = VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			SwapChain = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			Surface = VK_KHR_SURFACE_EXTENSION_NAME,
			DynamicRendering = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
			;
	}
	namespace Layers
	{
		constexpr auto KhronosValidationLayerName = "VK_LAYER_KHRONOS_validation";
	}

	namespace DebugUtilsMessageType
	{
		constexpr VkDebugUtilsMessageTypeFlagBitsEXT General = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
		constexpr VkDebugUtilsMessageTypeFlagBitsEXT Validation = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		constexpr VkDebugUtilsMessageTypeFlagBitsEXT Performance = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	}
}

// TODO: Left empty for now.
export namespace vk
{
	
}

// TODO: Left empty for now.
export namespace vk::raii
{
	
}