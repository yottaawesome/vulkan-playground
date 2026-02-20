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
		::VkApplicationInfo,
		::VkInstanceCreateFlags,
		::VkPhysicalDevice,
		::VkDevice,
		::VkDeviceCreateInfo,
		::VkQueue,
		::VkWin32SurfaceCreateInfoKHR,
		::VkSurfaceKHR,
		::VkResult,
		::VkExtensionProperties,
		::VkLayerProperties,
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
}

// TODO: Left empty for now.
export namespace vk
{
	
}

// TODO: Left empty for now.
export namespace vk::raii
{
	
}