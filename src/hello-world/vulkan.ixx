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
		::vkCreateInstance,
		::vkDestroyInstance
		;

	constexpr auto ApiVersion1 = VK_API_VERSION_1_0;
	
	constexpr auto MakeVersion(int major, int minor, int patch) -> uint32_t
	{
		return VK_MAKE_VERSION(major, minor,  patch);
	}
}
