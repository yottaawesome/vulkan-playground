module;

#include <vulkan/vulkan_raii.hpp>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
//#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>

export module vulkantutorial:libs;

export namespace glfw
{
	using
		::GLFWwindow,
		::glfwInit,
		::glfwWindowHint,
		::glfwWindowShouldClose,
		::glfwPollEvents,
		::glfwDestroyWindow,
		::glfwTerminate,
		::glfwCreateWindow,
		::glfwGetRequiredInstanceExtensions,
		::glfwCreateWindowSurface,
		::glfwGetFramebufferSize,
		::glfwSetWindowUserPointer,
		::glfwSetFramebufferSizeCallback,
		::glfwGetWindowUserPointer,
		::glfwWaitEvents
		;

	constexpr auto
		ClientApi = GLFW_CLIENT_API,
		NoApi = GLFW_NO_API,
		Resizable = GLFW_RESIZABLE
		;
}

export namespace vk
{

}

export namespace vk::raii
{
	using 
		::vk::raii::Instance
		;
}
