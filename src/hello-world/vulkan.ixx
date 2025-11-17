module;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

export module vulkan;

export namespace GLFW
{
	using 
		::glfwWindowHint,
		::glfwCreateWindow,
		::glfwWindowShouldClose,
		::glfwPollEvents,
		::glfwDestroyWindow,
		::glfwTerminate,
		::glfwInit,
		::glfwGetRequiredInstanceExtensions,
		::GLFWwindow
		;

	constexpr auto 
		ClientApi = GLFW_CLIENT_API,
		NoApi = GLFW_NO_API,
		Resizable = GLFW_RESIZABLE
		;
}

export namespace Vulkan
{
	using
		::VkApplicationInfo,
		::VkInstance,
		::VkInstanceCreateInfo,
		::VkStructureType,
		::VkResult,
		::VkExtensionProperties,
		::VkLayerProperties,
		::VkBool32,
		::VkDebugUtilsMessageSeverityFlagBitsEXT,
		::VkDebugUtilsMessageTypeFlagsEXT,
		::VkDebugUtilsMessageTypeFlagBitsEXT,
		::VkDebugUtilsMessengerCallbackDataEXT,
		::VkDebugUtilsMessageSeverityFlagBitsEXT,
		::VkDebugUtilsMessengerEXT,
		::VkDebugUtilsMessengerCreateInfoEXT,
		::VkAllocationCallbacks,
		::PFN_vkCreateDebugUtilsMessengerEXT,
		::PFN_vkDestroyDebugUtilsMessengerEXT,
		::VkPhysicalDevice,
		::VkPhysicalDeviceFeatures,
		::VkPhysicalDeviceProperties,
		::VkPhysicalDeviceProperties2, // supersedes VkPhysicalDeviceProperties in Vulkan 1.1
		::VkPhysicalDeviceType,
		::VkQueueFamilyProperties,
		::VkQueueFlagBits,
		::VkDevice,
		::VkDeviceQueueCreateInfo,
		::VkDeviceQueueCreateInfo,
		::VkDeviceCreateInfo,
		::VkQueue,
		::vkGetDeviceQueue,
		::vkDestroyDevice,
		::vkCreateDevice,
		::vkGetPhysicalDeviceQueueFamilyProperties,
		::vkGetPhysicalDeviceProperties,
		::vkGetPhysicalDeviceProperties2, // supersedes vkGetPhysicalDeviceProperties in Vulkan 1.1
		::vkGetPhysicalDeviceFeatures,
		::vkEnumeratePhysicalDevices,
		::vkGetInstanceProcAddr,
		::vkCreateInstance,
		// https://docs.vulkan.org/refpages/latest/refpages/source/vkEnumerateInstanceExtensionProperties.html
		::vkEnumerateInstanceExtensionProperties,
		::vkDestroyInstance,
		::vkEnumerateInstanceLayerProperties
		;
	constexpr auto ApiVersion1 = VK_API_VERSION_1_0;
	constexpr auto DebugUtilExtensionName = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	constexpr auto VkFalse = VK_FALSE;
	constexpr auto VkNullHandle = VK_NULL_HANDLE;

	constexpr auto MakeVersion(int major, int minor, int patch) -> uint32_t
	{
		return VK_MAKE_VERSION(major, minor,  patch);
	}
}
