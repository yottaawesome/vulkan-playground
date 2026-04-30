module;

#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>
#include <vma/vk_mem_alloc.h>

export module vulkanlib:vk.exports;
import std;

// ---------------------------------------------------------------------------
// 1:1 re-exports of the underlying C API.
//
// `using` brings the original entity into the module purview so it can be
// re-exported. Works for typedefs, enums, real functions, and the extern
// function-pointer variables that volk declares for runtime-loaded entry
// points. Macros (anything `#define`-d) cannot be re-exported and need
// hand-written shims further down. `static const` constants (e.g. the
// VK_*_2_BIT flag values introduced after the headers needed wider types
// than enums could provide) also have to be re-exposed via constexpr shims
// because their internal linkage is unreachable across module boundaries
// (MSVC C5304).
// ---------------------------------------------------------------------------

// =====================================================================
// Core
// =====================================================================
export using
	::VkResult,
	::VkBool32,
	::VkFlags,
	::VkFlags64,
	::VkStructureType,
	::VkAllocationCallbacks,
	::VkObjectType,
	::VkSystemAllocationScope,
	::VkInternalAllocationType,
	::VkSharingMode,
	::VkDependencyFlags,
	::VkDependencyInfo,
	::VkMemoryBarrier,
	::VkMemoryBarrier2,
	::VkBufferMemoryBarrier,
	::VkBufferMemoryBarrier2,
	::VkImageMemoryBarrier,
	::VkImageMemoryBarrier2
	;

export constexpr VkBool32
	VkFalse = 0,
	VkTrue  = 1;

// =====================================================================
// Geometry / common value types
// =====================================================================
export using
	::VkExtent2D,
	::VkExtent3D,
	::VkOffset2D,
	::VkOffset3D,
	::VkRect2D,
	::VkViewport,
	::VkComponentMapping,
	::VkComponentSwizzle,
	::VkClearValue,
	::VkClearColorValue,
	::VkClearDepthStencilValue,
	::VkClearAttachment,
	::VkClearRect
	;

// =====================================================================
// Format
// =====================================================================
export using
	::VkFormat,
	::VkFormatProperties,
	::VkFormatProperties2,
	::VkFormatFeatureFlags,
	::VkFormatFeatureFlagBits,
	::VkFormatFeatureFlags2
	;

// =====================================================================
// Instance
// =====================================================================
export using
	::VkInstance,
	::VkInstanceCreateInfo,
	::VkInstanceCreateFlags,
	::VkInstanceCreateFlagBits,
	::VkApplicationInfo,
	::VkExtensionProperties,
	::VkLayerProperties,
	::vkCreateInstance,
	::vkDestroyInstance,
	::vkGetInstanceProcAddr,
	::vkEnumerateInstanceVersion,
	::vkEnumerateInstanceExtensionProperties,
	::vkEnumerateInstanceLayerProperties
	;

// =====================================================================
// Debug utils (VK_EXT_debug_utils)
// =====================================================================
export using
	::VkDebugUtilsMessengerEXT,
	::VkDebugUtilsMessengerCreateInfoEXT,
	::VkDebugUtilsMessengerCreateFlagsEXT,
	::VkDebugUtilsMessengerCallbackDataEXT,
	::VkDebugUtilsMessengerCallbackDataFlagsEXT,
	::VkDebugUtilsObjectNameInfoEXT,
	::VkDebugUtilsLabelEXT,
	::VkDebugUtilsMessageSeverityFlagsEXT,
	::VkDebugUtilsMessageSeverityFlagBitsEXT,
	::VkDebugUtilsMessageTypeFlagsEXT,
	::VkDebugUtilsMessageTypeFlagBitsEXT,
	::PFN_vkDebugUtilsMessengerCallbackEXT,
	::PFN_vkCreateDebugUtilsMessengerEXT,
	::PFN_vkDestroyDebugUtilsMessengerEXT,
	::vkCreateDebugUtilsMessengerEXT,
	::vkDestroyDebugUtilsMessengerEXT,
	::vkSetDebugUtilsObjectNameEXT,
	::vkCmdBeginDebugUtilsLabelEXT,
	::vkCmdEndDebugUtilsLabelEXT,
	::vkCmdInsertDebugUtilsLabelEXT,
	::vkQueueBeginDebugUtilsLabelEXT,
	::vkQueueEndDebugUtilsLabelEXT
	;

// =====================================================================
// Physical device
// =====================================================================
export using
	::VkPhysicalDevice,
	::VkPhysicalDeviceType,
	::VkPhysicalDeviceProperties,
	::VkPhysicalDeviceProperties2,
	::VkPhysicalDeviceFeatures,
	::VkPhysicalDeviceFeatures2,
	::VkPhysicalDeviceVulkan11Features,
	::VkPhysicalDeviceVulkan12Features,
	::VkPhysicalDeviceVulkan13Features,
	::VkPhysicalDeviceVulkan14Features,
	::VkPhysicalDeviceLimits,
	::VkPhysicalDeviceMemoryProperties,
	::VkPhysicalDeviceMemoryProperties2,
	::VkPhysicalDeviceDynamicRenderingFeatures,
	::VkPhysicalDeviceSynchronization2Features,
	::VkMemoryType,
	::VkMemoryHeap,
	::VkMemoryHeapFlags,
	::VkMemoryHeapFlagBits,
	::vkEnumeratePhysicalDevices,
	::vkGetPhysicalDeviceProperties,
	::vkGetPhysicalDeviceProperties2,
	::vkGetPhysicalDeviceFeatures,
	::vkGetPhysicalDeviceFeatures2,
	::vkGetPhysicalDeviceFormatProperties,
	::vkGetPhysicalDeviceFormatProperties2,
	::vkGetPhysicalDeviceImageFormatProperties,
	::vkGetPhysicalDeviceMemoryProperties,
	::vkGetPhysicalDeviceMemoryProperties2,
	::vkGetPhysicalDeviceQueueFamilyProperties,
	::vkGetPhysicalDeviceQueueFamilyProperties2,
	::vkEnumerateDeviceExtensionProperties
	;

// =====================================================================
// Device
// =====================================================================
export using
	::VkDevice,
	::VkDeviceCreateInfo,
	::VkDeviceCreateFlags,
	::vkCreateDevice,
	::vkDestroyDevice,
	::vkDeviceWaitIdle,
	::vkGetDeviceProcAddr
	;

// =====================================================================
// Queue
// =====================================================================
export using
	::VkQueue,
	::VkDeviceQueueCreateInfo,
	::VkDeviceQueueCreateFlags,
	::VkQueueFamilyProperties,
	::VkQueueFamilyProperties2,
	::VkQueueFlags,
	::VkQueueFlagBits,
	::VkSubmitInfo,
	::VkSubmitInfo2,
	::VkCommandBufferSubmitInfo,
	::VkSemaphoreSubmitInfo,
	::VkPresentInfoKHR,
	::vkGetDeviceQueue,
	::vkGetDeviceQueue2,
	::vkQueueSubmit,
	::vkQueueSubmit2,
	::vkQueueWaitIdle,
	::vkQueuePresentKHR
	;

// =====================================================================
// Command pool / command buffer
// =====================================================================
export using
	::VkCommandPool,
	::VkCommandPoolCreateInfo,
	::VkCommandPoolCreateFlags,
	::VkCommandPoolCreateFlagBits,
	::VkCommandPoolResetFlags,
	::VkCommandPoolResetFlagBits,
	::VkCommandBuffer,
	::VkCommandBufferLevel,
	::VkCommandBufferAllocateInfo,
	::VkCommandBufferBeginInfo,
	::VkCommandBufferUsageFlags,
	::VkCommandBufferUsageFlagBits,
	::VkCommandBufferResetFlags,
	::VkCommandBufferResetFlagBits,
	::VkCommandBufferInheritanceInfo,
	::vkCreateCommandPool,
	::vkDestroyCommandPool,
	::vkResetCommandPool,
	::vkAllocateCommandBuffers,
	::vkFreeCommandBuffers,
	::vkBeginCommandBuffer,
	::vkEndCommandBuffer,
	::vkResetCommandBuffer
	;

// =====================================================================
// Synchronisation primitives
// =====================================================================
export using
	::VkSemaphore,
	::VkSemaphoreCreateInfo,
	::VkSemaphoreCreateFlags,
	::VkSemaphoreType,
	::VkSemaphoreTypeCreateInfo,
	::VkSemaphoreWaitInfo,
	::VkSemaphoreSignalInfo,
	::VkFence,
	::VkFenceCreateInfo,
	::VkFenceCreateFlags,
	::VkFenceCreateFlagBits,
	::VkEvent,
	::VkEventCreateInfo,
	::VkEventCreateFlags,
	::vkCreateSemaphore,
	::vkDestroySemaphore,
	::vkWaitSemaphores,
	::vkSignalSemaphore,
	::vkGetSemaphoreCounterValue,
	::vkCreateFence,
	::vkDestroyFence,
	::vkWaitForFences,
	::vkResetFences,
	::vkGetFenceStatus,
	::vkCreateEvent,
	::vkDestroyEvent,
	::vkSetEvent,
	::vkResetEvent,
	::vkGetEventStatus
	;

// =====================================================================
// Surface / Swapchain (KHR)
// =====================================================================
export using
	::VkSurfaceKHR,
	::VkSurfaceCapabilitiesKHR,
	::VkSurfaceCapabilities2KHR,
	::VkSurfaceFormatKHR,
	::VkSurfaceFormat2KHR,
	::VkSurfaceTransformFlagsKHR,
	::VkSurfaceTransformFlagBitsKHR,
	::VkCompositeAlphaFlagsKHR,
	::VkCompositeAlphaFlagBitsKHR,
	::VkColorSpaceKHR,
	::VkPresentModeKHR,
	::VkSwapchainKHR,
	::VkSwapchainCreateInfoKHR,
	::VkSwapchainCreateFlagsKHR,
	::VkSwapchainCreateFlagBitsKHR,
	::vkDestroySurfaceKHR,
	::vkGetPhysicalDeviceSurfaceSupportKHR,
	::vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
	::vkGetPhysicalDeviceSurfaceFormatsKHR,
	::vkGetPhysicalDeviceSurfacePresentModesKHR,
	::vkCreateSwapchainKHR,
	::vkDestroySwapchainKHR,
	::vkGetSwapchainImagesKHR,
	::vkAcquireNextImageKHR,
	::vkAcquireNextImage2KHR
	;

// =====================================================================
// Buffer
// =====================================================================
export using
	::VkBuffer,
	::VkBufferView,
	::VkBufferViewCreateInfo,
	::VkBufferCreateInfo,
	::VkBufferCreateFlags,
	::VkBufferCreateFlagBits,
	::VkBufferUsageFlags,
	::VkBufferUsageFlagBits,
	::VkBufferUsageFlags2,
	::VkBufferCopy,
	::VkBufferCopy2,
	::VkBufferImageCopy,
	::VkBufferImageCopy2,
	::VkCopyBufferInfo2,
	::VkCopyBufferToImageInfo2,
	::VkCopyImageToBufferInfo2,
	::vkCreateBuffer,
	::vkDestroyBuffer,
	::vkCreateBufferView,
	::vkDestroyBufferView,
	::vkBindBufferMemory,
	::vkBindBufferMemory2,
	::vkGetBufferMemoryRequirements,
	::vkGetBufferMemoryRequirements2,
	::vkGetDeviceBufferMemoryRequirements,
	::vkGetBufferDeviceAddress
	;

// =====================================================================
// Device memory
// =====================================================================
export using
	::VkDeviceMemory,
	::VkMemoryAllocateInfo,
	::VkMemoryAllocateFlags,
	::VkMemoryAllocateFlagBits,
	::VkMemoryAllocateFlagsInfo,
	::VkMemoryRequirements,
	::VkMemoryRequirements2,
	::VkMemoryMapFlags,
	::VkMemoryPropertyFlags,
	::VkMemoryPropertyFlagBits,
	::VkMappedMemoryRange,
	::vkAllocateMemory,
	::vkFreeMemory,
	::vkMapMemory,
	::vkUnmapMemory,
	::vkFlushMappedMemoryRanges,
	::vkInvalidateMappedMemoryRanges
	;

// =====================================================================
// Image
// =====================================================================
export using
	::VkImage,
	::VkImageCreateInfo,
	::VkImageCreateFlags,
	::VkImageCreateFlagBits,
	::VkImageType,
	::VkImageTiling,
	::VkImageLayout,
	::VkImageUsageFlags,
	::VkImageUsageFlagBits,
	::VkImageAspectFlags,
	::VkImageAspectFlagBits,
	::VkImageSubresource,
	::VkImageSubresourceLayers,
	::VkImageSubresourceRange,
	::VkSubresourceLayout,
	::VkImageCopy,
	::VkImageCopy2,
	::VkImageBlit,
	::VkImageBlit2,
	::VkBlitImageInfo2,
	::VkCopyImageInfo2,
	::VkImageResolve,
	::VkImageResolve2,
	::VkResolveImageInfo2,
	::vkCreateImage,
	::vkDestroyImage,
	::vkBindImageMemory,
	::vkBindImageMemory2,
	::vkGetImageMemoryRequirements,
	::vkGetImageMemoryRequirements2,
	::vkGetDeviceImageMemoryRequirements,
	::vkGetImageSubresourceLayout
	;

// =====================================================================
// Image view
// =====================================================================
export using
	::VkImageView,
	::VkImageViewCreateInfo,
	::VkImageViewCreateFlags,
	::VkImageViewType,
	::vkCreateImageView,
	::vkDestroyImageView
	;

// =====================================================================
// Sampler
// =====================================================================
export using
	::VkSampler,
	::VkSamplerCreateInfo,
	::VkSamplerCreateFlags,
	::VkSamplerAddressMode,
	::VkSamplerMipmapMode,
	::VkSamplerReductionMode,
	::VkFilter,
	::VkBorderColor,
	::vkCreateSampler,
	::vkDestroySampler
	;

// =====================================================================
// Shader
// =====================================================================
export using
	::VkShaderModule,
	::VkShaderModuleCreateInfo,
	::VkShaderModuleCreateFlags,
	::VkShaderStageFlags,
	::VkShaderStageFlagBits,
	::vkCreateShaderModule,
	::vkDestroyShaderModule
	;

// =====================================================================
// Pipeline
// =====================================================================
export using
	::VkPipeline,
	::VkPipelineCache,
	::VkPipelineCacheCreateInfo,
	::VkPipelineCacheCreateFlags,
	::VkPipelineLayout,
	::VkPipelineLayoutCreateInfo,
	::VkPipelineLayoutCreateFlags,
	::VkPipelineBindPoint,
	::VkPipelineCreateFlags,
	::VkPipelineCreateFlagBits,
	::VkPushConstantRange,
	::VkSpecializationInfo,
	::VkSpecializationMapEntry,
	::VkPipelineShaderStageCreateInfo,
	::VkPipelineShaderStageCreateFlags,
	::VkPipelineVertexInputStateCreateInfo,
	::VkPipelineInputAssemblyStateCreateInfo,
	::VkPipelineTessellationStateCreateInfo,
	::VkPipelineViewportStateCreateInfo,
	::VkPipelineRasterizationStateCreateInfo,
	::VkPipelineMultisampleStateCreateInfo,
	::VkPipelineDepthStencilStateCreateInfo,
	::VkPipelineColorBlendStateCreateInfo,
	::VkPipelineColorBlendAttachmentState,
	::VkPipelineDynamicStateCreateInfo,
	::VkPipelineRenderingCreateInfo,
	::VkGraphicsPipelineCreateInfo,
	::VkComputePipelineCreateInfo,
	::VkVertexInputBindingDescription,
	::VkVertexInputAttributeDescription,
	::VkVertexInputRate,
	::VkPrimitiveTopology,
	::VkPolygonMode,
	::VkCullModeFlags,
	::VkCullModeFlagBits,
	::VkFrontFace,
	::VkSampleCountFlags,
	::VkSampleCountFlagBits,
	::VkBlendFactor,
	::VkBlendOp,
	::VkLogicOp,
	::VkColorComponentFlags,
	::VkColorComponentFlagBits,
	::VkCompareOp,
	::VkStencilOp,
	::VkStencilOpState,
	::VkDynamicState,
	::VkPipelineStageFlags,
	::VkPipelineStageFlagBits,
	::VkPipelineStageFlags2,
	::vkCreatePipelineLayout,
	::vkDestroyPipelineLayout,
	::vkCreateGraphicsPipelines,
	::vkCreateComputePipelines,
	::vkDestroyPipeline,
	::vkCreatePipelineCache,
	::vkDestroyPipelineCache,
	::vkGetPipelineCacheData,
	::vkMergePipelineCaches
	;

// =====================================================================
// Access flags
// =====================================================================
export using
	::VkAccessFlags,
	::VkAccessFlagBits,
	::VkAccessFlags2
	;

// =====================================================================
// Descriptor set
// =====================================================================
export using
	::VkDescriptorSet,
	::VkDescriptorSetLayout,
	::VkDescriptorSetLayoutCreateInfo,
	::VkDescriptorSetLayoutCreateFlags,
	::VkDescriptorSetLayoutBinding,
	::VkDescriptorSetLayoutBindingFlagsCreateInfo,
	::VkDescriptorBindingFlags,
	::VkDescriptorBindingFlagBits,
	::VkDescriptorPool,
	::VkDescriptorPoolCreateInfo,
	::VkDescriptorPoolCreateFlags,
	::VkDescriptorPoolCreateFlagBits,
	::VkDescriptorPoolSize,
	::VkDescriptorSetAllocateInfo,
	::VkDescriptorType,
	::VkWriteDescriptorSet,
	::VkCopyDescriptorSet,
	::VkDescriptorBufferInfo,
	::VkDescriptorImageInfo,
	::vkCreateDescriptorSetLayout,
	::vkDestroyDescriptorSetLayout,
	::vkCreateDescriptorPool,
	::vkDestroyDescriptorPool,
	::vkResetDescriptorPool,
	::vkAllocateDescriptorSets,
	::vkFreeDescriptorSets,
	::vkUpdateDescriptorSets
	;

// =====================================================================
// Render pass / dynamic rendering
// =====================================================================
export using
	::VkRenderPass,
	::VkRenderPassCreateInfo,
	::VkRenderPassCreateInfo2,
	::VkRenderPassBeginInfo,
	::VkAttachmentDescription,
	::VkAttachmentDescription2,
	::VkAttachmentReference,
	::VkAttachmentReference2,
	::VkAttachmentLoadOp,
	::VkAttachmentStoreOp,
	::VkSubpassDescription,
	::VkSubpassDescription2,
	::VkSubpassDependency,
	::VkSubpassDependency2,
	::VkSubpassContents,
	::VkFramebuffer,
	::VkFramebufferCreateInfo,
	::VkFramebufferCreateFlags,
	::VkRenderingInfo,
	::VkRenderingFlags,
	::VkRenderingFlagBits,
	::VkRenderingAttachmentInfo,
	::vkCreateRenderPass,
	::vkCreateRenderPass2,
	::vkDestroyRenderPass,
	::vkCreateFramebuffer,
	::vkDestroyFramebuffer
	;

// =====================================================================
// Query pool
// =====================================================================
export using
	::VkQueryPool,
	::VkQueryPoolCreateInfo,
	::VkQueryType,
	::VkQueryControlFlags,
	::VkQueryControlFlagBits,
	::VkQueryResultFlags,
	::VkQueryResultFlagBits,
	::vkCreateQueryPool,
	::vkDestroyQueryPool,
	::vkGetQueryPoolResults,
	::vkResetQueryPool
	;

// =====================================================================
// Command-buffer recording (vkCmd*)
// =====================================================================
export using
	::vkCmdBindPipeline,
	::vkCmdBindDescriptorSets,
	::vkCmdBindVertexBuffers,
	::vkCmdBindVertexBuffers2,
	::vkCmdBindIndexBuffer,
	::vkCmdDraw,
	::vkCmdDrawIndexed,
	::vkCmdDrawIndirect,
	::vkCmdDrawIndexedIndirect,
	::vkCmdDispatch,
	::vkCmdDispatchIndirect,
	::vkCmdCopyBuffer,
	::vkCmdCopyBuffer2,
	::vkCmdCopyImage,
	::vkCmdCopyImage2,
	::vkCmdCopyBufferToImage,
	::vkCmdCopyBufferToImage2,
	::vkCmdCopyImageToBuffer,
	::vkCmdCopyImageToBuffer2,
	::vkCmdBlitImage,
	::vkCmdBlitImage2,
	::vkCmdResolveImage,
	::vkCmdResolveImage2,
	::vkCmdClearColorImage,
	::vkCmdClearDepthStencilImage,
	::vkCmdClearAttachments,
	::vkCmdFillBuffer,
	::vkCmdUpdateBuffer,
	::vkCmdPipelineBarrier,
	::vkCmdPipelineBarrier2,
	::vkCmdSetEvent,
	::vkCmdSetEvent2,
	::vkCmdResetEvent,
	::vkCmdResetEvent2,
	::vkCmdWaitEvents,
	::vkCmdWaitEvents2,
	::vkCmdSetViewport,
	::vkCmdSetScissor,
	::vkCmdSetLineWidth,
	::vkCmdSetDepthBias,
	::vkCmdSetBlendConstants,
	::vkCmdSetDepthBounds,
	::vkCmdSetStencilCompareMask,
	::vkCmdSetStencilWriteMask,
	::vkCmdSetStencilReference,
	::vkCmdPushConstants,
	::vkCmdBeginRendering,
	::vkCmdEndRendering,
	::vkCmdBeginRenderPass,
	::vkCmdBeginRenderPass2,
	::vkCmdEndRenderPass,
	::vkCmdEndRenderPass2,
	::vkCmdNextSubpass,
	::vkCmdExecuteCommands,
	::vkCmdBeginQuery,
	::vkCmdEndQuery,
	::vkCmdResetQueryPool,
	::vkCmdWriteTimestamp,
	::vkCmdWriteTimestamp2,
	::vkCmdCopyQueryPoolResults
	;

// =====================================================================
// Win32 surface (gated on VK_USE_PLATFORM_WIN32_KHR)
// =====================================================================
#ifdef VK_USE_PLATFORM_WIN32_KHR
export using
	::VkWin32SurfaceCreateInfoKHR,
	::VkWin32SurfaceCreateFlagsKHR,
	::vkCreateWin32SurfaceKHR,
	::vkGetPhysicalDeviceWin32PresentationSupportKHR
	;
#endif

// =====================================================================
// volk loader entry points (real functions, not pointers)
// =====================================================================
export using
	::volkInitialize,
	::volkInitializeCustom,
	::volkFinalize,
	::volkGetInstanceVersion,
	::volkLoadInstance,
	::volkLoadInstanceOnly,
	::volkLoadDevice,
	::volkLoadDeviceTable,
	::volkLoadInstanceTable,
	::volkGetLoadedInstance,
	::volkGetLoadedDevice
	;

export using
	::VolkInstanceTable,
	::VolkDeviceTable
	;

// =====================================================================
// Vulkan Memory Allocator (VMA)
// =====================================================================
export using
	::VmaAllocator,
	::VmaAllocatorCreateInfo,
	::VmaAllocatorCreateFlags,
	::VmaAllocatorCreateFlagBits,
	::VmaAllocation,
	::VmaAllocationCreateInfo,
	::VmaAllocationCreateFlags,
	::VmaAllocationCreateFlagBits,
	::VmaAllocationInfo,
	::VmaPool,
	::VmaPoolCreateInfo,
	::VmaPoolCreateFlags,
	::VmaPoolCreateFlagBits,
	::VmaMemoryUsage,
	::VmaVulkanFunctions,
	::VmaVirtualBlock,
	::VmaVirtualBlockCreateInfo,
	::VmaVirtualAllocation,
	::VmaVirtualAllocationCreateInfo,
	::VmaVirtualAllocationInfo,
	::VmaStatistics,
	::VmaDetailedStatistics,
	::VmaTotalStatistics,
	::VmaBudget,
	::vmaCreateAllocator,
	::vmaDestroyAllocator,
	::vmaGetAllocatorInfo,
	::vmaGetPhysicalDeviceProperties,
	::vmaGetMemoryProperties,
	::vmaGetMemoryTypeProperties,
	::vmaGetHeapBudgets,
	::vmaSetCurrentFrameIndex,
	::vmaCalculateStatistics,
	::vmaCreatePool,
	::vmaDestroyPool,
	::vmaAllocateMemory,
	::vmaAllocateMemoryPages,
	::vmaAllocateMemoryForBuffer,
	::vmaAllocateMemoryForImage,
	::vmaFreeMemory,
	::vmaFreeMemoryPages,
	::vmaGetAllocationInfo,
	::vmaSetAllocationName,
	::vmaSetAllocationUserData,
	::vmaMapMemory,
	::vmaUnmapMemory,
	::vmaFlushAllocation,
	::vmaInvalidateAllocation,
	::vmaCopyMemoryToAllocation,
	::vmaCopyAllocationToMemory,
	::vmaCreateBuffer,
	::vmaCreateBufferWithAlignment,
	::vmaDestroyBuffer,
	::vmaCreateImage,
	::vmaDestroyImage,
	::vmaBindBufferMemory,
	::vmaBindBufferMemory2,
	::vmaBindImageMemory,
	::vmaBindImageMemory2,
	::vmaCheckCorruption,
	::vmaCheckPoolCorruption
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
	// Sentinels
	constexpr auto QueueFamilyIgnored   = static_cast<std::uint32_t>(VK_QUEUE_FAMILY_IGNORED);
	constexpr auto QueueFamilyExternal  = static_cast<std::uint32_t>(VK_QUEUE_FAMILY_EXTERNAL);
	constexpr auto SubpassExternal      = static_cast<std::uint32_t>(VK_SUBPASS_EXTERNAL);
	constexpr auto AttachmentUnused     = static_cast<std::uint32_t>(VK_ATTACHMENT_UNUSED);
	constexpr auto RemainingMipLevels   = static_cast<std::uint32_t>(VK_REMAINING_MIP_LEVELS);
	constexpr auto RemainingArrayLayers = static_cast<std::uint32_t>(VK_REMAINING_ARRAY_LAYERS);
	constexpr auto WholeSize            = static_cast<std::uint64_t>(VK_WHOLE_SIZE);
	constexpr auto LodClampNone         = VK_LOD_CLAMP_NONE;

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
			Surface             = VK_KHR_SURFACE_EXTENSION_NAME,
			DebugUtils          = VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			GetSurfaceCaps2     = VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
			GetPhysDevProps2    = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
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
			Swapchain         = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			DynamicRendering  = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			Synchronization2  = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
			BufferDeviceAddr  = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			TimelineSemaphore = VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
			DescriptorIndex   = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			PushDescriptor    = VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
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
			TransferSrc                   = VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT,
			TransferDst                   = VK_BUFFER_USAGE_2_TRANSFER_DST_BIT,
			UniformTexelBuffer            = VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT,
			StorageTexelBuffer            = VK_BUFFER_USAGE_2_STORAGE_TEXEL_BUFFER_BIT,
			UniformBuffer                 = VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT,
			StorageBuffer                 = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT,
			IndexBuffer                   = VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT,
			VertexBuffer                  = VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT,
			IndirectBuffer                = VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT,
			ShaderDeviceAddress           = VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
	}

	namespace PipelineStage2
	{
		constexpr VkPipelineStageFlags2
			None                          = VK_PIPELINE_STAGE_2_NONE,
			TopOfPipe                     = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			DrawIndirect                  = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,
			VertexInput                   = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
			VertexShader                  = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
			FragmentShader                = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			EarlyFragmentTests            = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
			LateFragmentTests             = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			ColorAttachmentOutput         = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			ComputeShader                 = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
			AllTransfer                   = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT,
			Transfer                      = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			BottomOfPipe                  = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			Host                          = VK_PIPELINE_STAGE_2_HOST_BIT,
			AllGraphics                   = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
			AllCommands                   = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			Copy                          = VK_PIPELINE_STAGE_2_COPY_BIT,
			Resolve                       = VK_PIPELINE_STAGE_2_RESOLVE_BIT,
			Blit                          = VK_PIPELINE_STAGE_2_BLIT_BIT,
			Clear                         = VK_PIPELINE_STAGE_2_CLEAR_BIT,
			IndexInput                    = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT,
			VertexAttributeInput          = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT,
			PreRasterizationShaders       = VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT;
	}

	namespace Access2
	{
		constexpr VkAccessFlags2
			None                          = VK_ACCESS_2_NONE,
			IndirectCommandRead           = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT,
			IndexRead                     = VK_ACCESS_2_INDEX_READ_BIT,
			VertexAttributeRead           = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
			UniformRead                   = VK_ACCESS_2_UNIFORM_READ_BIT,
			InputAttachmentRead           = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT,
			ShaderRead                    = VK_ACCESS_2_SHADER_READ_BIT,
			ShaderWrite                   = VK_ACCESS_2_SHADER_WRITE_BIT,
			ColorAttachmentRead           = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
			ColorAttachmentWrite          = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			DepthStencilAttachmentRead    = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
			DepthStencilAttachmentWrite   = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			TransferRead                  = VK_ACCESS_2_TRANSFER_READ_BIT,
			TransferWrite                 = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			HostRead                      = VK_ACCESS_2_HOST_READ_BIT,
			HostWrite                     = VK_ACCESS_2_HOST_WRITE_BIT,
			MemoryRead                    = VK_ACCESS_2_MEMORY_READ_BIT,
			MemoryWrite                   = VK_ACCESS_2_MEMORY_WRITE_BIT,
			ShaderSampledRead             = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
			ShaderStorageRead             = VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
			ShaderStorageWrite            = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
	}
}
