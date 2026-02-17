module;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <glslang/Public/ShaderLang.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module gfxwithvulkancourse:exports;

export namespace glfw
{
	using 
		::GLFWwindow,
		::glfwInit,
		::glfwWindowShouldClose,
		::glfwWindowHint,
		::glfwCreateWindow,
		::glfwDestroyWindow,
		::glfwPollEvents,
		::glfwGetRequiredInstanceExtensions,
		::glfwGetPhysicalDevicePresentationSupport,
		::glfwCreateWindowSurface,
		::glfwTerminate
		;

	constexpr auto
		ClientApi = GLFW_CLIENT_API,
		NoApi = GLFW_NO_API,
		Resizable = GLFW_RESIZABLE
		;
}

export namespace Win32
{
	using 
		::HINSTANCE,
		::LPWSTR,
		::UINT
		;
}



export namespace Vulkan
{
	using
		::VkInstance,
		::VkPhysicalDevice,
		::VkSurfaceKHR,
		::VkResult,
		::VkDevice,
		::VkQueue,
		::vkGetDeviceQueue
		;
	constexpr auto QueueFamilyIgnored = VK_QUEUE_FAMILY_IGNORED;
}

export namespace vk
{
	using 
		::vk::Result,
		::vk::PhysicalDevice,
		::vk::SurfaceKHR,
		::vk::Device,
		::vk::Queue,
		::vk::PhysicalDevice,
		::vkGetDeviceQueue
		;
}

export namespace vk::raii
{
	using
		::vk::raii::Context,
		::vk::raii::Instance,
		::vk::raii::DebugUtilsMessengerEXT,
		::vk::raii::SurfaceKHR,
		::vk::raii::Device,
		::vk::raii::Queue,
		::vk::raii::SwapchainKHR,
		::vk::raii::ImageView,
		::vk::raii::PipelineLayout,
		::vk::raii::Pipeline,
		::vk::raii::CommandPool,
		::vk::raii::CommandBuffers,
		::vk::raii::Semaphore,
		::vk::raii::Fence
		;
}
