module;

#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>
#include <vma/vk_mem_alloc.h>

export module volkus:vkx.exports;
import std;

// ---------------------------------------------------------------------------
// 1:1 re-exports of the underlying C API.
//
// `using` brings the original entity into the module purview so it can be
// re-exported. Works for typedefs, enums, real functions, and the extern
// function-pointer variables that volk declares for runtime-loaded entry
// points. Macros (anything `#define`-d) cannot be re-exported and need
// hand-written shims further down.
// ---------------------------------------------------------------------------

// General
export using 
	::VkResult,
	::VkBool32,
	::VkAllocationCallbacks
	;

export constexpr VkBool32 
	VkFalse = 0,
	VkTrue = 1;

// Create
export using 
	::VkStructureType,
	::VkInstanceCreateFlags
	;

// Debug
export using
	::VkDebugUtilsMessengerEXT,
	::VkDebugUtilsMessengerCreateInfoEXT,
	::VkDebugUtilsMessengerCallbackDataEXT,
	::VkDebugUtilsMessageSeverityFlagBitsEXT,
	::VkDebugUtilsMessageTypeFlagsEXT,
	::PFN_vkDebugUtilsMessengerCallbackEXT,
	::PFN_vkCreateDebugUtilsMessengerEXT,
	::PFN_vkDestroyDebugUtilsMessengerEXT,
	::VkDebugUtilsMessageSeverityFlagsEXT
	;

// Instance
export using
	::VkInstance,
	::VkInstanceCreateInfo,
	::VkApplicationInfo,
	::vkCreateInstance,
	::vkDestroyInstance,
	::vkGetInstanceProcAddr
	;

// Physical device
export using
	::VkPhysicalDevice,
	::VkPhysicalDeviceProperties,
	::VkPhysicalDeviceProperties2,
	::VkPhysicalDeviceType,
	::VkPhysicalDeviceFeatures,
	::VkPhysicalDeviceFeatures2,
	::VkPhysicalDeviceVulkan11Features,
	::VkPhysicalDeviceVulkan12Features,
	::VkPhysicalDeviceVulkan13Features,
	::VkPhysicalDeviceVulkan14Features,
	::VkPhysicalDeviceType,
	::VkQueueFamilyProperties,
	::VkQueueFamilyProperties2,
	::VkQueueFlagBits,
	::vkGetPhysicalDeviceProperties,
	::vkGetPhysicalDeviceProperties2,
	::vkGetPhysicalDeviceFormatProperties,
	::vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
	::vkGetPhysicalDeviceQueueFamilyProperties,
	::vkGetPhysicalDeviceQueueFamilyProperties2,
	::vkEnumeratePhysicalDevices,
	::vkGetPhysicalDeviceFeatures,
	::vkGetPhysicalDeviceFeatures2
	;

// Debug utils
export using
	::vkCreateDebugUtilsMessengerEXT,
	::vkDestroyDebugUtilsMessengerEXT,
	::VkDebugUtilsMessageTypeFlagBitsEXT,
	::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
	::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
	::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
	;

// Device
export using
	::VkDevice,
	::VkDeviceCreateInfo,
	::vkCreateDevice,
	::vkDestroyDevice
	;

// Queue
export using
	::VkDeviceQueueCreateInfo,
	::VkQueueFamilyProperties2,
	::VkQueueFlags,
	::VkQueue,
	::vkGetDeviceQueue,
	::vkGetDeviceProcAddr
	;

// Command pool
export using
	::VkCommandPoolCreateFlagBits,
	::VkCommandPoolCreateFlags,
	::VkCommandPoolCreateInfo,
	::VkCommandPool,
	::vkCreateCommandPool,
	::vkDestroyCommandPool
	;

// Command buffer
export using
	::VkCommandBuffer,
	::vkAllocateCommandBuffers,
	::vkFreeCommandBuffers
	;

// Synchronisation primitives
export using
	::VkSemaphore,
	::VkFence,
	::VkSemaphoreCreateInfo,
	::VkFenceCreateInfo,
	::vkCreateSemaphore,
	::vkDestroySemaphore,
	::vkCreateFence,
	::vkDestroyFence
	;

// Swapchain
export using
	::VkSwapchainCreateInfoKHR,
	::VkSwapchainKHR,
	::vkCreateSwapchainKHR,
	::vkDestroySwapchainKHR,
	::vkGetSwapchainImagesKHR,
	::vkAcquireNextImageKHR,
	::vkQueuePresentKHR
	;

// Buffer
export using
	::VkBuffer,
	::VkBufferCreateInfo,
	::VkBufferUsageFlags,
	::VkBufferUsageFlags2,
	::vkCreateBuffer,
	::vkGetDeviceBufferMemoryRequirements,
	::vkDestroyBuffer
	;

// Device memory
export using
	::VkDeviceMemory,
	::VkMemoryAllocateInfo,
	::VkMemoryMapFlags,
	::VkMemoryPropertyFlags,
	::vkAllocateMemory,
	::vkFreeMemory,
	::vkMapMemory,
	::vkUnmapMemory
	;

// Image
export using
	::VkImage,
	::VkImageCreateInfo,
	::VkImageAspectFlags,
	::VkImageAspectFlagBits,
	::VkImageUsageFlags,
	::VkImageUsageFlagBits,
	::VkImageLayout,
	::VkImageType,
	::vkCreateImage,
	::vkDestroyImage
	;

// Image view
export using
	::VkImageViewCreateInfo,
	::VkImageView,
	::VkImageViewType,
	::vkCreateImageView,
	::vkDestroyImageView
	;

// Sampler
export using
	::VkSampler,
	::VkSamplerCreateInfo,
	::vkCreateSampler,
	::vkDestroySampler
	;

// Shader
export using
	::VkShaderModule,
	::VkShaderModuleCreateInfo,
	::vkCreateShaderModule
	;

// Pipeline
export using
	::VkPipelineLayout,
	::VkPipelineLayoutCreateInfo,
	::VkPipeline,
	::VkPipelineStageFlags2,
	::vkCreatePipelineLayout,
	::vkCreateGraphicsPipelines,
	::vkCreateComputePipelines,
	::vkDestroyPipelineLayout,
	::vkDestroyPipeline,
	::vkGetPipelineCacheData,
	::vkCreatePipelineCache,
	::vkDestroyPipelineCache
	;

// Access
export using ::VkAccessFlags2;

// Descriptor set
export using
	::VkDescriptorSetLayout,
	::VkDescriptorSetLayoutCreateInfo,
	::VkDescriptorPool,
	::VkDescriptorPoolCreateInfo,
	::vkCreateDescriptorSetLayout,
	::vkDestroyDescriptorSetLayout,
	::vkCreateDescriptorPool,
	::vkDestroyDescriptorPool
	;

// volk loader entry points (real functions, not pointers)
export using
	::volkInitialize,
	::volkLoadInstance,
	::volkLoadDevice
	;

// VMA
export using
	::VmaAllocator,
	::VmaAllocation,
	::VmaAllocationCreateInfo,
	::VmaAllocationInfo,
	::VmaVulkanFunctions,
	::VmaAllocatorCreateFlagBits,
	::VmaAllocatorCreateFlags,
	::VmaMemoryUsage,
	::vmaCreateImage,
	::vmaDestroyImage,
	::vmaCreateAllocator,
	::vmaDestroyAllocator,
	::vmaAllocateMemory,
	::vmaFreeMemory,
	::vmaCreateBuffer,
	::vmaDestroyBuffer
	;

// ---------------------------------------------------------------------------
// Shims for things that can't be re-exported via `using`:
//   - macros (no underlying entity to bring in)
//   - `static const` constants (internal linkage; consumers couldn't reach
//     them through a using-declaration even though it would compile)
// Kept inside Vk:: so they don't pollute module scope and so the names can
// mirror their C counterparts without colliding with them.
// ---------------------------------------------------------------------------
export namespace Vk
{
	constexpr auto QueueFamilyIgnored = static_cast<std::uint32_t>(VK_QUEUE_FAMILY_IGNORED);

	constexpr auto MakeVersion(
		std::uint32_t major,
		std::uint32_t minor,
		std::uint32_t patch
	) noexcept -> std::uint32_t
	{
		return VK_MAKE_VERSION(major, minor, patch);
	}

	struct VulkanApiVersion
	{
		std::uint32_t Value = 0;
		std::uint32_t Major = VK_VERSION_MAJOR(Value);
		std::uint32_t Minor = VK_VERSION_MINOR(Value);
		std::uint32_t Patch = VK_VERSION_PATCH(Value);
	};

	struct VulkanDriverVersion
	{
		std::uint32_t Value = 0;
		std::uint32_t Major = VK_VERSION_MAJOR(Value);
		std::uint32_t Minor = VK_VERSION_MINOR(Value);
		std::uint32_t Patch = VK_VERSION_PATCH(Value);
	};

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

	namespace ApiVersion
	{
		constexpr auto V1_0 = VK_API_VERSION_1_0;
		constexpr auto V1_1 = VK_API_VERSION_1_1;
		constexpr auto V1_2 = VK_API_VERSION_1_2;
		constexpr auto V1_3 = VK_API_VERSION_1_3;
		constexpr auto V1_4 = VK_API_VERSION_1_4;
	}

	namespace InstanceExtension
	{
		constexpr auto
			Surface    = VK_KHR_SURFACE_EXTENSION_NAME,
			DebugUtils = VK_EXT_DEBUG_UTILS_EXTENSION_NAME
			;

#ifdef VK_USE_PLATFORM_WIN32_KHR
		constexpr auto PlatformSurface = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		constexpr auto PlatformSurface = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		constexpr auto PlatformSurface = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
		constexpr auto PlatformSurface = VK_EXT_METAL_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		constexpr auto PlatformSurface = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#endif
	}

	namespace DeviceExtension
	{
		constexpr auto
			Swapchain        = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			DynamicRendering = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
			;
	}

	// Layers do not apply to devices (deprecated), so no DeviceLayer namespace.
	namespace Layer
	{
		constexpr auto KhronosValidation = "VK_LAYER_KHRONOS_validation";
	}

	// NOTE: the *_2_* flag bits below (VkBufferUsageFlags2, VkPipelineStageFlags2,
	// VkAccessFlags2, …) are declared in vulkan_core.h as `static const` 64-bit
	// constants — they predate C23's wider enums, so the Vulkan headers can't use a
	// real enum for them. `static const` means internal linkage, which means
	// `export using ::VK_FOO_2_BAR;` *compiles* but is ill-formed for consumers
	// (MSVC warns C5304). They have to be re-exposed as constexpr shims instead,
	// the same way macros are.
	namespace BufferUsageFlagBits2
	{
		constexpr VkBufferUsageFlags2
			TransferSrc = VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT,
			TransferDst = VK_BUFFER_USAGE_2_TRANSFER_DST_BIT,
			UniformTexelBuffer = VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT,
			StorageTexelBuffer = VK_BUFFER_USAGE_2_STORAGE_TEXEL_BUFFER_BIT,
			UniformBuffer = VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT,
			StorageBuffer = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT,
			IndexBuffer = VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT,
			VertexBuffer = VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT,
			IndirectBuffer = VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT;
	}

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
}
