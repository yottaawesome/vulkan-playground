module;

#define VOLK_IMPLEMENTATION
#include <volk.h>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

export module volkus:vk.exports;
import std;

export namespace Vk
{
	template<auto VConstant>
	struct Constant
	{
		static constexpr auto operator()() noexcept { return VConstant; }
		operator std::remove_cvref_t<decltype(VConstant)>(this auto&&) noexcept { return VConstant; }
	};

	using Result = ::VkResult;

	// Instance
	using Instance = ::VkInstance;
	using InstanceCreateInfo = ::VkInstanceCreateInfo;
	using ApplicationInfo = ::VkApplicationInfo;
	constexpr auto& CreateInstance = ::vkCreateInstance;
	constexpr auto& DestroyInstance = ::vkDestroyInstance;
	constexpr auto& GetInstanceProcAddr = ::vkGetInstanceProcAddr;

	// Physical device
	using PhysicalDevice = ::VkPhysicalDevice;
	using PhysicalDeviceProperties = ::VkPhysicalDeviceProperties;
	using PhysicalDeviceProperties2 = ::VkPhysicalDeviceProperties2;
	using PhysicalDeviceType = ::VkPhysicalDeviceType;
	using PhysicalDeviceFeatures = ::VkPhysicalDeviceFeatures;
	using PhysicalDeviceFeatures2 = ::VkPhysicalDeviceFeatures2;
	using PhysicalDeviceVulkan11Features = ::VkPhysicalDeviceVulkan11Features;
	using PhysicalDeviceVulkan12Features = ::VkPhysicalDeviceVulkan12Features;
	using PhysicalDeviceVulkan13Features = ::VkPhysicalDeviceVulkan13Features;
	using PhysicalDeviceVulkan14Features = ::VkPhysicalDeviceVulkan14Features;
	constexpr auto& GetPhysicalDeviceProperties = ::vkGetPhysicalDeviceProperties;
	constexpr auto& GetPhysicalDeviceProperties2 = ::vkGetPhysicalDeviceProperties2;
	constexpr auto& GetPhysicalDeviceFormatProperties = ::vkGetPhysicalDeviceFormatProperties;
	constexpr auto& GetPhysicalDeviceSurfaceCapabilitiesKHR = ::vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
	constexpr auto& GetPhysicalDeviceQueueFamilyProperties2 = ::vkGetPhysicalDeviceQueueFamilyProperties2;
	constexpr auto& EnumeratePhysicalDevices = ::vkEnumeratePhysicalDevices;

	// Miscellaneous
	constexpr auto& CreateDebugUtilsMessengerEXT = ::vkCreateDebugUtilsMessengerEXT;
	constexpr auto& DestroyDebugUtilsMessengerEXT = ::vkDestroyDebugUtilsMessengerEXT;

	// Device
	using Device = ::VkDevice;
	using DeviceCreateInfo = ::VkDeviceCreateInfo;
	constexpr auto& CreateDevice = ::vkCreateDevice;
	constexpr auto& DestroyDevice = ::vkDestroyDevice;

	// Queue
	using DeviceQueueCreateInfo = ::VkDeviceQueueCreateInfo;
	using QueueFamilyProperties2 = ::VkQueueFamilyProperties2;
	using QueueFlags = ::VkQueueFlags;
	using Queue = ::VkQueue;
	constexpr auto& GetDeviceQueue = ::vkGetDeviceQueue;
	constexpr auto& GetDeviceProcAddr = ::vkGetDeviceProcAddr;

	// Command pool
	using CommandPoolCreateFlagBits = ::VkCommandPoolCreateFlagBits;
	using CommandPoolCreateFlags = ::VkCommandPoolCreateFlags;
	using CommandPoolCreateInfo = ::VkCommandPoolCreateInfo;
	using CommandPool = ::VkCommandPool;
	constexpr auto& CreateCommandPool = ::vkCreateCommandPool;
	constexpr auto& DestroyCommandPool = ::vkDestroyCommandPool;

	// Command buffer
	using CommandBuffer = ::VkCommandBuffer;
	constexpr auto& AllocateCommandBuffers = ::vkAllocateCommandBuffers;
	constexpr auto& FreeCommandBuffers = ::vkFreeCommandBuffers;

	// Synchronisation primitives
	using Semaphore = ::VkSemaphore;
	using Fence = ::VkFence;
	using SemaphoreCreateInfo = ::VkSemaphoreCreateInfo;
	using FenceCreateInfo = ::VkFenceCreateInfo;
	constexpr auto& CreateSemaphore = ::vkCreateSemaphore;
	constexpr auto& DestroySemaphore = ::vkDestroySemaphore;
	constexpr auto& CreateFence = ::vkCreateFence;
	constexpr auto& DestroyFence = ::vkDestroyFence;

	// Swapchain
	using SwapchainCreateInfoKHR = ::VkSwapchainCreateInfoKHR;
	using SwapchainKHR = ::VkSwapchainKHR;
	constexpr auto& CreateSwapchainKHR = ::vkCreateSwapchainKHR;
	constexpr auto& DestroySwapchainKHR = ::vkDestroySwapchainKHR;
	constexpr auto& GetSwapchainImagesKHR = ::vkGetSwapchainImagesKHR;
	constexpr auto& AcquireNextImageKHR = ::vkAcquireNextImageKHR;
	constexpr auto& QueuePresentKHR = ::vkQueuePresentKHR;

	// Buffer
	using Buffer = ::VkBuffer;
	using BufferCreateInfo = ::VkBufferCreateInfo;
	using BufferUsageFlags = ::VkBufferUsageFlags;
	constexpr auto& CreateBuffer = ::vkCreateBuffer;
	constexpr auto& GetDeviceBufferMemoryRequirements = ::vkGetDeviceBufferMemoryRequirements;
	constexpr auto& DestroyBuffer = ::vkDestroyBuffer;

	// Device memory
	using DeviceMemory = ::VkDeviceMemory;
	using MemoryAllocateInfo = ::VkMemoryAllocateInfo;
	using MemoryMapFlags = ::VkMemoryMapFlags;
	using MemoryPropertyFlags = ::VkMemoryPropertyFlags;
	constexpr auto& AllocateMemory = ::vkAllocateMemory;
	constexpr auto& FreeMemory = ::vkFreeMemory;
	constexpr auto& MapMemory = ::vkMapMemory;
	constexpr auto& UnmapMemory = ::vkUnmapMemory;

	// Image
	using Image = ::VkImage;
	using ImageCreateInfo = ::VkImageCreateInfo;
	using ImageView = ::VkImageView;
	using ImageViewCreateInfo = ::VkImageViewCreateInfo;
	using ImageAspectFlags = ::VkImageAspectFlags;
	using ImageAspectFlagBits = ::VkImageAspectFlagBits;
	using ImageUsageFlags = ::VkImageUsageFlags;
	using ImageUsageFlagBits = ::VkImageUsageFlagBits;
	using ImageLayout = ::VkImageLayout;
	using ImageType = ::VkImageType;
	constexpr auto& CreateImage = ::vkCreateImage;
	constexpr auto& DestroyImage = ::vkDestroyImage;

	// Image view
	using ImageViewCreateInfo = ::VkImageViewCreateInfo;
	using ImageView = ::VkImageView;
	using ImageViewType = ::VkImageViewType;
	constexpr auto& CreateImageView = ::vkCreateImageView;
	constexpr auto& DestroyImageView = ::vkDestroyImageView;

	// Sampler
	using Sampler = ::VkSampler;
	using SamplerCreateInfo = ::VkSamplerCreateInfo;
	constexpr auto& CreateSampler = ::vkCreateSampler;
	constexpr auto& DestroySampler = ::vkDestroySampler;

	// Shader
	using ShaderModule = ::VkShaderModule;
	using ShaderModuleCreateInfo = ::VkShaderModuleCreateInfo;
	constexpr auto& CreateShaderModule = ::vkCreateShaderModule;

	// Pipeline
	using PipelineLayout = ::VkPipelineLayout;
	using PipelineLayoutCreateInfo = ::VkPipelineLayoutCreateInfo;
	using Pipeline = ::VkPipeline;
	constexpr auto& CreatePipelineLayout = ::vkCreatePipelineLayout;
	constexpr auto& CreateGraphicsPipelines = ::vkCreateGraphicsPipelines;
	constexpr auto& CreateComputePipelines = ::vkCreateComputePipelines;
	constexpr auto& DestroyPipelineLayout = ::vkDestroyPipelineLayout;
	constexpr auto& DestroyPipeline = ::vkDestroyPipeline;
	constexpr auto& GetPipelineCacheData = ::vkGetPipelineCacheData;
	constexpr auto& CreatePipelineCache = ::vkCreatePipelineCache;
	constexpr auto& DestroyPipelineCache = ::vkDestroyPipelineCache;

	// Descriptor set layout
	using DescriptorSetLayout = ::VkDescriptorSetLayout;
	using DescriptorSetLayoutCreateInfo = ::VkDescriptorSetLayoutCreateInfo;
	using DescriptorPool = ::VkDescriptorPool;
	using DescriptorPoolCreateInfo = ::VkDescriptorPoolCreateInfo;
	constexpr auto& CreateDescriptorSetLayout = ::vkCreateDescriptorSetLayout;
	constexpr auto& DestroyDescriptorSetLayout = ::vkDestroyDescriptorSetLayout;
	constexpr auto& CreateDescriptorPool = ::vkCreateDescriptorPool;
	constexpr auto& DestroyDescriptorPool = ::vkDestroyDescriptorPool;

	constexpr auto QueueFamilyIgnored = static_cast<std::uint32_t>(VK_QUEUE_FAMILY_IGNORED);

	namespace BufferUsageFlagBits2Constants
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

export namespace volk
{
	/*
	* Volk has three loading levels — each one populates a different set of function pointers:
	*   1. volkInitialize() — global functions only (vkCreateInstance, vkEnumerateInstanceExtensionProperties, etc.)
	*   2. volkLoadInstance(instance) — instance-level functions (vkEnumeratePhysicalDevices, vkCreateDevice, vkGetPhysicalDeviceProperties, etc.)
	*   3. volkLoadDevice(device) — device-level functions (vkCreateBuffer, vkCmdDraw, etc.) — you'll need this later when you create a logical device.
	*/
	constexpr auto& Initialize = ::volkInitialize;
	constexpr auto& LoadInstance = ::volkLoadInstance;
	constexpr auto& LoadDevice = ::volkLoadDevice;
}

export namespace vma
{
	using Allocator = ::VmaAllocator;
	using Allocation = ::VmaAllocation;
	using AllocationCreateInfo = ::VmaAllocationCreateInfo;
	using AllocationInfo = ::VmaAllocationInfo;
	using VulkanFunctions = ::VmaVulkanFunctions;
	using AllocatorCreateFlagBits = ::VmaAllocatorCreateFlagBits;
	using AllocatorCreateFlags = ::VmaAllocatorCreateFlags;
	using SuballocationType = ::VmaSuballocationType;
	using BufferImageUsage = ::VmaBufferImageUsage;
	using MemoryUsage = ::VmaMemoryUsage;
	constexpr auto& CreateImage = ::vmaCreateImage;
	constexpr auto& DestroyImage = ::vmaDestroyImage;
	constexpr auto& CreateAllocator = ::vmaCreateAllocator;
	constexpr auto& DestroyAllocator = ::vmaDestroyAllocator;
	constexpr auto& AllocateMemory = ::vmaAllocateMemory;
	constexpr auto& FreeMemory = ::vmaFreeMemory;
	constexpr auto& CreateBuffer = ::vmaCreateBuffer;
	constexpr auto& DestroyBuffer = ::vmaDestroyBuffer;
}