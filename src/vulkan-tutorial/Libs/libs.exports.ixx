module;

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan.hpp>
#include <glslang/Public/ShaderLang.h>
#define GLFW_INCLUDE_VULKAN
//#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>

export module vulkantutorial:libs.exports;
//import std; // ICE

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
		::glfwCreateWindowSurface,
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
		::vk::ApplicationInfo,
		::vk::InstanceCreateInfo,
		::vk::SystemError,
		::vk::DebugUtilsMessageTypeFlagsEXT,
		::vk::DebugUtilsMessengerCallbackDataEXT,
		::vk::DebugUtilsMessageSeverityFlagBitsEXT,
		::vk::DebugUtilsMessageSeverityFlagsEXT,
		::vk::DeviceQueueCreateInfo,
		::vk::Bool32,
		::vk::ComponentSwizzle,
		::vk::DebugUtilsMessengerCreateInfoEXT,
		::vk::PhysicalDevice,
		::vk::PhysicalDeviceFeatures,
		::vk::PhysicalDeviceProperties,
		::vk::PhysicalDeviceLimits,
		::vk::DeviceCreateInfo,
		::vk::ExtensionProperties,
		::vk::StructureChain,
		::vk::SwapchainCreateInfoKHR,
		::vk::QueueFamilyProperties,
		::vk::PhysicalDeviceType,
		::vk::PhysicalDeviceVulkan13Features,
		::vk::PhysicalDeviceVulkan14Features,
		::vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
		::vk::QueueFlagBits,
		::vk::Result,
		::vk::ImageViewType,
		::vk::ImageViewCreateInfo,
		::vk::SurfaceCapabilitiesKHR,
		::vk::SwapchainKHR,
		::vk::ShaderModuleCreateInfo,
		::vk::SwapchainCreateFlagsKHR,
		::vk::PipelineShaderStageCreateInfo,
		::vk::PipelineVertexInputStateCreateInfo,
		::vk::PipelineInputAssemblyStateCreateInfo,
		::vk::PipelineViewportStateCreateInfo,
		::vk::PipelineRasterizationStateCreateInfo,
		::vk::PipelineMultisampleStateCreateInfo,
		::vk::PipelineColorBlendAttachmentState,
		::vk::PipelineColorBlendStateCreateInfo,
		::vk::DynamicState,
		::vk::Viewport,
		::vk::PipelineDynamicStateCreateInfo,
		::vk::ColorComponentFlagBits,
		::vk::ColorComponentFlags,
		::vk::PipelineRenderingCreateInfo,
		::vk::GraphicsPipelineCreateInfo,
		::vk::CommandPoolCreateInfo,
		::vk::PhysicalDeviceFeatures2,
		::vk::PhysicalDeviceVulkan11Features,
		::vk::SurfaceFormatKHR,
		::vk::CommandPoolCreateFlagBits,
		::vk::CommandPoolCreateFlags,
		::vk::CommandBufferLevel,
		::vk::AccessFlags2,
		::vk::PipelineStageFlags,
		::vk::PipelineStageFlags2,
		::vk::ImageLayout,
		::vk::ImageMemoryBarrier2,
		::vk::FenceCreateInfo,
		::vk::ClearValue,
		::vk::RenderingAttachmentInfo,
		::vk::PipelineBindPoint,
		::vk::RenderingInfo,
		::vk::ApiVersion14,
		::vk::EXTDebugUtilsExtensionName,
		::vk::KHRSwapchainExtensionName,
		::vk::KHRSpirv14ExtensionName,
		::vk::KHRSynchronization2ExtensionName,
		::vk::KHRCreateRenderpass2ExtensionName
		;

	constexpr auto MakeVersion(int x, int y, int z)
		noexcept -> unsigned
	{
		return VK_MAKE_VERSION(x, y, z);
	}

	export namespace raii
	{
		using
			::vk::raii::Instance,
			::vk::raii::Context,
			::vk::raii::DebugUtilsMessengerEXT,
			::vk::raii::PhysicalDevice,
			::vk::raii::Device,
			::vk::raii::SurfaceKHR,
			::vk::raii::Queue,
			::vk::raii::ShaderModule,
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
}

