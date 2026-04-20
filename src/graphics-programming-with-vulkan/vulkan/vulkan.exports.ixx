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
	template<auto VConstant>
	struct Constant
	{
		static constexpr auto operator()() noexcept { return VConstant; }
		operator std::remove_cvref_t<decltype(VConstant)>(this auto&&) noexcept { return VConstant; }
	};

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
		::VkImageType,
		::VkWin32SurfaceCreateInfoKHR,
		::VkDeviceQueueCreateInfo,
		::VkDeviceQueueCreateFlags,
		::VkImageCreateInfo,
		::VkImageTiling,
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
		::VkCommandPool,
		::VkCommandPoolCreateInfo,
		::VkCommandPoolCreateFlagBits,
		::VkCommandPoolResetFlags,
		::VkCommandBuffer,
		::VkCommandBufferResetFlags,
		::VkCommandBufferLevel,
		::VkCommandBufferAllocateInfo,
		::VkSemaphore,
		::VkSemaphoreCreateInfo,
		::VkFence,
		::VkFenceCreateInfo,
		::VkSemaphoreCreateFlags,
		::VkFenceCreateFlags,
		::VkFenceCreateFlagBits,
		::VkSemaphoreTypeCreateInfo,
		::VkTimelineSemaphoreSubmitInfo,
		::VkSemaphoreType,
		::VkSemaphoreSignalInfo,
		::VkSemaphoreWaitInfo,
		::VkSubmitInfo,
		::VkPipelineStageFlags,
		::VkPipelineStageFlagBits,
		::VkCommandBufferBeginInfo,
		::VkRenderingInfo,
		::VkRenderingAttachmentInfo,
		::VkImageLayout,
		::VkAttachmentLoadOp,
		::VkAttachmentStoreOp,
		::VkClearValue,
		::VkClearColorValue,
		::VkPipelineBindPoint,
		::VkImageMemoryBarrier2,
		::VkDependencyInfo,
		::VkPipelineStageFlags2,
		::VkAccessFlags2,
		::VkAccessFlagBits2,
		::VkPresentInfoKHR,
		::VkPipelineStageFlagBits2,
		::VkBlendFactor,
		::VkBlendOp,
		::VkVertexInputBindingDescription,
		::VkVertexInputRate,
		::VkVertexInputAttributeDescription,
		::VkBuffer,
		::VkDeviceMemory,
		::VkBufferCreateInfo,
		::VkMemoryPropertyFlags,
		::VkPhysicalDeviceMemoryProperties,
		::VkBufferUsageFlagBits,
		::VkBufferUsageFlagBits2,
		::VkMemoryPropertyFlagBits,
		::VkMemoryRequirements,
		::VkMemoryAllocateInfo,
		::VkDeviceSize,
		::VkCommandBufferUsageFlagBits,
		::VkBufferUsageFlags,
		::VkBufferUsageFlags2,
		::VkBufferCopy,
		::VkIndexType,
		::VkPushConstantRange,
		::VkDescriptorSetLayoutBinding,
		::VkDescriptorType,
		::VkShaderStageFlagBits,
		::VkDescriptorSetLayoutCreateInfo,
		::VkDescriptorSetLayout,
		::VkDescriptorPool,
		::VkDescriptorSet,
		::VkDescriptorPoolCreateInfo,
		::VkDescriptorPoolSize,
		::VkDescriptorType,
		::VkDescriptorSetAllocateInfo,
		::VkDescriptorBufferInfo,
		::VkWriteDescriptorSet,
		::VkSampler,
		::VkSamplerCreateInfo,
		::VkSamplerAddressMode,
		::VkFilter,
		::VkBorderColor,
		::VkCompareOp,
		::VkSamplerMipmapMode,
		::VkImageUsageFlagBits,
		::VkBufferImageCopy,
		::VkImageMemoryBarrier,
		::VkAccessFlagBits,
		::vkCmdPipelineBarrier,
		::vkCmdCopyBufferToImage,
		::vkBindImageMemory,
		::vkDestroySampler,
		::vkCreateSampler,
		::vkCmdBindDescriptorSets,
		::vkCmdBindDescriptorSets2,
		::vkUpdateDescriptorSets,
		::vkAllocateDescriptorSets,
		::vkFreeDescriptorSets,
		::vkCreateDescriptorPool,
		::vkDestroyDescriptorPool,
		::vkDestroyDescriptorSetLayout,
		::vkCreateDescriptorSetLayout,
		::vkCmdPushConstants,
		::vkCmdBindIndexBuffer,
		::vkCmdDrawIndexed,
		::vkCmdCopyBuffer,
		::vkQueueWaitIdle,
		::vkCmdBindVertexBuffers,
		::vkMapMemory,
		::vkUnmapMemory,
		::vkBindBufferMemory,
		::vkFreeMemory,
		::vkDestroyBuffer,
		::vkAllocateMemory,
		::vkGetBufferMemoryRequirements,
		::vkGetPhysicalDeviceMemoryProperties,
		::vkCreateBuffer,
		::vkDeviceWaitIdle,
		::vkGetFenceStatus,
		::vkAcquireNextImageKHR,
		::vkGetSemaphoreCounterValue,
		::vkGetImageMemoryRequirements,
		::vkSignalSemaphore,
		::vkWaitSemaphores,
		::vkResetFences,
		::vkWaitForFences,
		::vkCreateSemaphore,
		::vkDestroySemaphore,
		::vkCreateFence,
		::vkDestroyFence,
		::vkAllocateCommandBuffers,
		::vkFreeCommandBuffers,
		::vkResetCommandPool,
		::vkResetCommandBuffer,
		::vkCreateCommandPool,
		::vkDestroyCommandPool,
		::vkCreateGraphicsPipelines,
		::vkDestroyRenderPass,
		::vkDestroyPipeline,
		::vkDestroyPipelineLayout,
		::vkCreatePipelineLayout,
		::vkCreateShaderModule,
		::vkDestroyShaderModule,
		::vkCreateImageView,
		::vkCreateImage,
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
		::vkDestroySurfaceKHR,
		::vkBeginCommandBuffer,
		::vkEndCommandBuffer,
		::vkCmdBeginRendering,
		::vkCmdEndRendering,
		::vkCmdBindPipeline,
		::vkCmdSetViewport,
		::vkCmdSetScissor,
		::vkCmdDraw,
		::vkCmdPipelineBarrier2,
		::vkQueueSubmit,
		::vkQueuePresentKHR,
		::vkDestroyImage
		;

	namespace VkBufferUsageFlagBits2Constants
	{
		constexpr auto TransferSrc = Constant<VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT>{};
		constexpr auto TransferDst = Constant<VK_BUFFER_USAGE_2_TRANSFER_DST_BIT>{};
		constexpr auto UniformTexelBuffer = Constant<VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT>{};
		constexpr auto StorageTexelBuffer = Constant<VK_BUFFER_USAGE_2_STORAGE_TEXEL_BUFFER_BIT>{};
		constexpr auto UniformBuffer = Constant<VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT>{};
		constexpr auto StorageBuffer = Constant<VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT>{};
		constexpr auto IndexBuffer = Constant<VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT>{};
		constexpr auto VertexBuffer = Constant<VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT>{};
		constexpr auto IndirectBuffer = Constant<VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT>{};
	}

	// Synchronization2 64-bit flag constants (cannot be exported via `using` as they are
	// static const variables, not enum values).
	constexpr auto QueueFamilyIgnored = static_cast<std::uint32_t>(VK_QUEUE_FAMILY_IGNORED);

	namespace PipelineStage2
	{
		constexpr VkPipelineStageFlags2
			None = VK_PIPELINE_STAGE_2_NONE,
			ColorAttachmentOutput = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			BottomOfPipe = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
	}

	namespace Access2
	{
		constexpr VkAccessFlags2
			None = VK_ACCESS_2_NONE,
			ColorAttachmentWrite = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	}

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

	struct VulkanVersion
	{
		std::uint32_t Major = 0;
		std::uint32_t Minor = 0;
		std::uint32_t Patch = 0;
		constexpr auto ToVersion(this VulkanVersion self) noexcept -> std::uint32_t
		{
			return VK_MAKE_VERSION(self.Major, self.Minor, self.Patch);
		}
		constexpr operator std::uint32_t(this VulkanVersion self) noexcept
		{
			return VK_MAKE_VERSION(self.Major, self.Minor, self.Patch);
		}

		static constexpr auto MakeVersion(
			std::uint32_t major,
			std::uint32_t minor,
			std::uint32_t patch
		) noexcept -> std::uint32_t
		{
			return VK_MAKE_VERSION(major, minor, patch);
		}
	};

	struct ApiVersion
	{
		std::uint32_t Major = 0;
		std::uint32_t Minor = 0;
		std::uint32_t Patch = 0;
		constexpr auto ToVersion(this ApiVersion self) noexcept -> std::uint32_t
		{
			return VK_MAKE_API_VERSION(0, self.Major, self.Minor, self.Patch);
		}
		explicit constexpr operator std::uint32_t(this ApiVersion self) noexcept
		{
			return VK_MAKE_API_VERSION(0, self.Major, self.Minor, self.Patch);
		}
		static constexpr auto MakeVersion(
			std::uint32_t major,
			std::uint32_t minor,
			std::uint32_t patch
		) noexcept -> std::uint32_t
		{
			// First parameter is variant.
			// See: https://docs.vulkan.org/refpages/latest/refpages/source/VK_MAKE_API_VERSION.html
			return VK_MAKE_API_VERSION(0, major, minor, patch);
		}
	};

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