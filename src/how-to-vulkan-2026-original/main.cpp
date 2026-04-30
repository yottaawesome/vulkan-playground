/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#define VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION
#include <vulkan/vulkan.h>
#include <volk.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <filesystem>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "slang.h"
#include "slang-com-ptr.h"
#include <ktx.h>
#include <ktxvulkan.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

struct ShaderData 
{
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model[3];
	glm::vec4 lightPos{ 0.0f, -10.0f, 10.0f, 0.0f };
	std::uint32_t selected{ 1 };
};
struct ShaderDataBuffer 
{
	VmaAllocation allocation{ };
	VmaAllocationInfo allocationInfo{};
	VkBuffer buffer{ };
	VkDeviceAddress deviceAddress{};
};
struct Texture 
{
	VmaAllocation allocation{ };
	VkImage image{ };
	VkImageView view{ };
	VkSampler sampler{ };
};
struct Vertex 
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

auto UpdateSwapchain = false;

static inline void chk(VkResult result) 
{
	if (result != VkResult::VK_SUCCESS) 
	{
		std::cerr << "Vulkan call returned an error (" << result << ")\n";
		exit(result);
	}
}
static inline void chk(bool result)
{
	if (not result)
	{
		std::cerr << "Call returned an error\n";
		exit(result);
	}
}
static inline void chkSwapchain(VkResult result) 
{
	if (result < VkResult::VK_SUCCESS) 
	{
		if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR) 
		{
			UpdateSwapchain = true;
			return;
		}
		std::cerr << "Vulkan call returned an error (" << result << ")\n";
		exit(result);
	}
}

int main(int argc, char* argv[])
{
	// Make sure asset folder is present from the current working directory
	if (not std::filesystem::is_directory("assets"))
	{
		std::cerr << "Could not locate assets folder from current working directory\n";
		exit(-1);
	}
	chk(SDL_Init(SDL_INIT_VIDEO));
	chk(SDL_Vulkan_LoadLibrary(NULL));
	volkInitialize();
	
	
	// Instance
	auto appInfo = VkApplicationInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO, 
		.pApplicationName = "How to Vulkan", 
		.apiVersion = VK_API_VERSION_1_4 
	};
	auto instanceExtensionsCount = std::uint32_t{ 0 };
	auto instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount);
	auto instanceCI = VkInstanceCreateInfo{
		.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledExtensionCount = instanceExtensionsCount,
		.ppEnabledExtensionNames = instanceExtensions,
	};
	auto instance = VkInstance{ };
	chk(vkCreateInstance(&instanceCI, nullptr, &instance));
	volkLoadInstance(instance);
	
	
	// Device
	auto deviceCount = std::uint32_t{ 0 };
	chk(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
	auto physicalDevices = std::vector<VkPhysicalDevice>(deviceCount);
	chk(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));
	auto deviceIndex = std::uint32_t{ 0 };
	if (argc > 1) 
	{
		deviceIndex = std::stoi(argv[1]);
		assert(deviceIndex < deviceCount);
	}
	auto deviceProperties = VkPhysicalDeviceProperties2{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 
	};

	auto physicalDevice = physicalDevices[deviceIndex];
	vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);
	auto device = VkDevice{ };
	std::cout << "Selected device: " << deviceProperties.properties.deviceName << "\n";
	
	
	// Find a queue family for graphics
	auto queue = VkQueue{ };
	auto queueFamilyCount = std::uint32_t{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	auto queueFamilies = std::vector<VkQueueFamilyProperties>(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
	auto queueFamily = std::uint32_t{ 0 };
	for (auto i = std::size_t{ 0 }; i < queueFamilies.size(); i++) 
	{
		if (queueFamilies[i].queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) 
		{
			queueFamily = static_cast<std::uint32_t>(i);
			break;
		}
	}
	chk(SDL_Vulkan_GetPresentationSupport(instance, physicalDevice, queueFamily));
	
	
	// Logical device
	auto const qfpriorities = float{ 1.0f };
	auto queueCI = VkDeviceQueueCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, 
		.queueFamilyIndex = queueFamily, 
		.queueCount = 1, 
		.pQueuePriorities = &qfpriorities 
	};
	auto enabledVk12Features = VkPhysicalDeviceVulkan12Features{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, 
		.descriptorIndexing = true, 
		.shaderSampledImageArrayNonUniformIndexing = true, 
		.descriptorBindingVariableDescriptorCount = true, 
		.runtimeDescriptorArray = true, 
		.bufferDeviceAddress = true 
	};
	auto enabledVk13Features = VkPhysicalDeviceVulkan13Features{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, 
		.pNext = &enabledVk12Features, 
		.synchronization2 = true, 
		.dynamicRendering = true 
	};
	auto enabledVk10Features = VkPhysicalDeviceFeatures{ 
		.samplerAnisotropy = true
	};
	auto const deviceExtensions = std::vector<const char*>{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	auto deviceCI = VkDeviceCreateInfo{
		.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &enabledVk13Features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCI,
		.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &enabledVk10Features
	};
	chk(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device));
	vkGetDeviceQueue(device, queueFamily, 0, &queue);
	
	
	// VMA
	auto vkFunctions = VmaVulkanFunctions{ 
		.vkGetInstanceProcAddr = vkGetInstanceProcAddr, 
		.vkGetDeviceProcAddr = vkGetDeviceProcAddr, 
		.vkCreateImage = vkCreateImage 
	};
	auto allocatorCI = VmaAllocatorCreateInfo{ 
		.flags = VmaAllocatorCreateFlagBits::VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT, 
		.physicalDevice = physicalDevice,
		.device = device, 
		.pVulkanFunctions = &vkFunctions, 
		.instance = instance 
	};
	auto allocator = VmaAllocator{ };
	chk(vmaCreateAllocator(&allocatorCI, &allocator));
	
	
	// Window and surface
	auto surface = VkSurfaceKHR{ };
	auto window = SDL_CreateWindow("How to Vulkan", 1280u, 720u, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	assert(window);
	chk(SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface));
	auto windowSize = glm::ivec2{};
	chk(SDL_GetWindowSize(window, &windowSize.x, &windowSize.y));
	auto surfaceCaps = VkSurfaceCapabilitiesKHR{};
	chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));
	auto swapchainExtent = VkExtent2D{ surfaceCaps.currentExtent };
	if (surfaceCaps.currentExtent.width == 0xFFFFFFFF) 
	{
		swapchainExtent = { 
			.width = static_cast<std::uint32_t>(windowSize.x), 
			.height = static_cast<std::uint32_t>(windowSize.y) 
		};
	}
	
	
	// Swap chain
	auto const imageFormat = VkFormat{ VkFormat::VK_FORMAT_B8G8R8A8_SRGB };
	auto swapchainCI = VkSwapchainCreateInfoKHR{
		.sType = VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = surfaceCaps.minImageCount,
		.imageFormat = imageFormat,
		.imageColorSpace = VkColorSpaceKHR::VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		.imageExtent = VkExtent2D{
			.width = swapchainExtent.width, 
			.height = swapchainExtent.height 
		},
		.imageArrayLayers = 1,
		.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR
	};
	auto swapchain = VkSwapchainKHR{ };
	chk(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));
	auto imageCount = std::uint32_t{ 0 };
	chk(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
	auto swapchainImages = std::vector<VkImage>{};
	swapchainImages.resize(imageCount);
	chk(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()));
	auto swapchainImageViews = std::vector<VkImageView>{};
	swapchainImageViews.resize(imageCount);
	for (auto i = std::uint32_t{}; i < imageCount; i++)
	{
		auto viewCI = VkImageViewCreateInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
			.image = swapchainImages[i], 
			.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D, 
			.format = imageFormat, 
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 
				.levelCount = 1, 
				.layerCount = 1 
			} 
		};
		chk(vkCreateImageView(device, &viewCI, nullptr, &swapchainImageViews[i]));
	}
	
	
	// Depth attachment
	auto depthFormatList = std::vector<VkFormat>{ VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT, VkFormat::VK_FORMAT_D24_UNORM_S8_UINT };
	auto depthFormat = VkFormat{ VkFormat::VK_FORMAT_UNDEFINED };
	for (auto& format : depthFormatList) 
	{
		auto formatProperties = VkFormatProperties2{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 
		};
		vkGetPhysicalDeviceFormatProperties2(physicalDevice, format, &formatProperties);
		if (formatProperties.formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) 
		{
			depthFormat = format;
			break;
		}
	}
	assert(depthFormat != VkFormat::VK_FORMAT_UNDEFINED);
	auto depthImageCI = VkImageCreateInfo{
		.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VkImageType::VK_IMAGE_TYPE_2D,
		.format = depthFormat,
		.extent{
			.width = static_cast<std::uint32_t>(windowSize.x), 
			.height = static_cast<std::uint32_t>(windowSize.y), 
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
		.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
		.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
	};
	auto allocCI = VmaAllocationCreateInfo{ 
		.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, 
		.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO 
	};
	auto depthImage = VkImage{ };
	auto depthImageAllocation = VmaAllocation{ };
	chk(vmaCreateImage(allocator, &depthImageCI, &allocCI, &depthImage, &depthImageAllocation, nullptr));
	auto depthViewCI = VkImageViewCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = depthImage, 
		.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D, 
		.format = depthFormat, 
		.subresourceRange = VkImageSubresourceRange{
			.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT, 
			.levelCount = 1, 
			.layerCount = 1 
		} 
	};
	auto depthImageView = VkImageView{ };
	chk(vkCreateImageView(device, &depthViewCI, nullptr, &depthImageView));
	
	
	// Mesh data
	auto attrib = tinyobj::attrib_t{};
	auto shapes = std::vector<tinyobj::shape_t>{};
	auto materials = std::vector<tinyobj::material_t>{};
	chk(tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr, "assets/suzanne.obj"));
	auto const indexCount = VkDeviceSize{ shapes[0].mesh.indices.size() };
	auto vertices = std::vector<Vertex>{};
	auto indices = std::vector<std::uint16_t>{};
	
	
	// Load vertex and index data
	for (auto& index : shapes[0].mesh.indices) 
	{
		auto v = Vertex{
			.pos = { 
				attrib.vertices[index.vertex_index * 3], 
				-attrib.vertices[index.vertex_index * 3 + 1], 
				attrib.vertices[index.vertex_index * 3 + 2] 
			},
			.normal = { 
				attrib.normals[index.normal_index * 3], 
				-attrib.normals[index.normal_index * 3 + 1], 
				attrib.normals[index.normal_index * 3 + 2] 
			},
			.uv = { 
				attrib.texcoords[index.texcoord_index * 2], 
				1.0 - attrib.texcoords[index.texcoord_index * 2 + 1] 
			}
		};
		vertices.push_back(v);
		indices.push_back(static_cast<std::uint16_t>(indices.size()));
	}
	auto vBufSize = VkDeviceSize{ sizeof(Vertex) * vertices.size() };
	auto iBufSize = VkDeviceSize{ sizeof(std::uint16_t) * indices.size() };
	auto bufferCI = VkBufferCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 
		.size = vBufSize + iBufSize, 
		.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT 
	};
	auto vBufferAllocCI = VmaAllocationCreateInfo{ 
		.flags = 
			VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT 
			| VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT 
			| VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT, 
		.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO 
	};
	auto vBufferAllocInfo = VmaAllocationInfo{};
	auto vBuffer = VkBuffer{ };
	auto vBufferAllocation = VmaAllocation{ };
	chk(vmaCreateBuffer(allocator, &bufferCI, &vBufferAllocCI, &vBuffer, &vBufferAllocation, &vBufferAllocInfo));
	memcpy(vBufferAllocInfo.pMappedData, vertices.data(), vBufSize);
	memcpy(((char*)vBufferAllocInfo.pMappedData) + vBufSize, indices.data(), iBufSize);
	
	
	// Shader data buffers
	constexpr auto maxFramesInFlight = std::uint32_t{ 2 };
	auto shaderDataBuffers = std::array<ShaderDataBuffer, maxFramesInFlight>{};
	for (auto i = 0; i < maxFramesInFlight; i++) 
	{
		auto uBufferCI = VkBufferCreateInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 
			.size = sizeof(ShaderData), 
			.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT 
		};
		auto uBufferAllocCI = VmaAllocationCreateInfo{ 
			.flags = 
				VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT 
				| VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT 
				| VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT, 
			.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO 
		};
		chk(vmaCreateBuffer(
			allocator, 
			&uBufferCI, 
			&uBufferAllocCI, 
			&shaderDataBuffers[i].buffer, 
			&shaderDataBuffers[i].allocation, 
			&shaderDataBuffers[i].allocationInfo
		));
		auto uBufferBdaInfo = VkBufferDeviceAddressInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, 
			.buffer = shaderDataBuffers[i].buffer 
		};
		shaderDataBuffers[i].deviceAddress = vkGetBufferDeviceAddress(device, &uBufferBdaInfo);
	}
	
	
	// Sync objects
	auto semaphoreCI = VkSemaphoreCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO 
	};
	auto fenceCI = VkFenceCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, 
		.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT 
	};
	auto fences = std::array<VkFence, maxFramesInFlight>{};
	auto presentSemaphores = std::array<VkSemaphore, maxFramesInFlight>{};
	for (auto i = 0; i < maxFramesInFlight; i++) 
	{
		chk(vkCreateFence(device, &fenceCI, nullptr, &fences[i]));
		chk(vkCreateSemaphore(device, &semaphoreCI, nullptr, &presentSemaphores[i]));
	}
	auto renderSemaphores = std::vector<VkSemaphore>{};
	renderSemaphores.resize(swapchainImages.size());
	for (auto& semaphore : renderSemaphores) 
	{
		chk(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
	}
	
	
	// Command pool
	auto commandPoolCI = VkCommandPoolCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, 
		.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 
		.queueFamilyIndex = queueFamily 
	};
	auto commandPool = VkCommandPool{ };
	chk(vkCreateCommandPool(device, &commandPoolCI, nullptr, &commandPool));
	auto cbAllocCI = VkCommandBufferAllocateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
		.commandPool = commandPool, 
		.commandBufferCount = maxFramesInFlight 
	};
	auto commandBuffers = std::array<VkCommandBuffer, maxFramesInFlight>{};
	chk(vkAllocateCommandBuffers(device, &cbAllocCI, commandBuffers.data()));
	
	
	// Texture images
	// For each texture: load a KTX file from disk, create a GPU image + view,
	// upload all mip levels via a staging buffer using a one-time command buffer,
	// transition the image to a shader-readable layout, then create a sampler
	// and append a descriptor entry to be bound later.
	auto textureDescriptors = std::vector<VkDescriptorImageInfo>{};
	auto textures = std::array<Texture, 3>{};
	for (auto i = 0; i < textures.size(); i++) 
	{
		// Load the KTX container from disk (includes all mip levels and pixel data).
		auto ktxTexture = static_cast<::ktxTexture*>(nullptr);
		auto filename = "assets/suzanne" + std::to_string(i) + ".ktx";
		ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

		// Create the destination GPU image, sized and formatted to match the KTX file.
		// Usage: TRANSFER_DST so we can copy pixels into it, SAMPLED so shaders can read it.
		auto texImgCI = VkImageCreateInfo{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VkImageType::VK_IMAGE_TYPE_2D,
			.format = ktxTexture_GetVkFormat(ktxTexture),
			.extent = {
				.width = ktxTexture->baseWidth, 
				.height = ktxTexture->baseHeight, 
				.depth = 1 
			},
			.mipLevels = ktxTexture->numLevels,
			.arrayLayers = 1,
			.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
			.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
			.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT,
			.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED
		};
		auto texImageAllocCI = VmaAllocationCreateInfo{ 
			.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO 
		};
		chk(vmaCreateImage(allocator, &texImgCI, &texImageAllocCI, &textures[i].image, &textures[i].allocation, nullptr));

		// Image view exposing all mip levels for shader sampling.
		auto texVewCI = VkImageViewCreateInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
			.image = textures[i].image, 
			.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D, 
			.format = texImgCI.format, 
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 
				.levelCount = ktxTexture->numLevels, 
				.layerCount = 1 
			} 
		};
		chk(vkCreateImageView(device, &texVewCI, nullptr, &textures[i].view));


		// Upload — copy KTX pixel data to a host-mapped staging buffer, then issue
		// a one-time GPU command buffer that copies it into the device-local image.

		// Staging buffer sized to the full KTX payload (all mip levels packed together).
		auto imgSrcBuffer = VkBuffer{};
		auto imgSrcAllocation = VmaAllocation{};
		auto imgSrcBufferCI = VkBufferCreateInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 
			.size = (std::uint32_t)ktxTexture->dataSize, 
			.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT 
		};
		auto imgSrcAllocCI = VmaAllocationCreateInfo{ 
			.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT, 
			.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO 
		};
		auto imgSrcAllocInfo = VmaAllocationInfo{};
		chk(vmaCreateBuffer(allocator, &imgSrcBufferCI, &imgSrcAllocCI, &imgSrcBuffer, &imgSrcAllocation, &imgSrcAllocInfo));
		// Copy the entire KTX blob (all mips) into the mapped staging buffer.
		memcpy(imgSrcAllocInfo.pMappedData, ktxTexture->pData, ktxTexture->dataSize);

		// Allocate a fence + one-time command buffer to drive the upload synchronously.
		auto fenceOneTimeCI = VkFenceCreateInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO 
		};
		auto fenceOneTime = VkFence{};
		chk(vkCreateFence(device, &fenceOneTimeCI, nullptr, &fenceOneTime));
		auto cbOneTime = VkCommandBuffer{};
		auto cbOneTimeAI = VkCommandBufferAllocateInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
			.commandPool = commandPool, 
			.commandBufferCount = 1 
		};
		chk(vkAllocateCommandBuffers(device, &cbOneTimeAI, &cbOneTime));
		auto cbOneTimeBI = VkCommandBufferBeginInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 
			.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		chk(vkBeginCommandBuffer(cbOneTime, &cbOneTimeBI));

		// Pre-copy barrier: transition the freshly-created image from UNDEFINED
		// to TRANSFER_DST_OPTIMAL so the upcoming buffer→image copy is legal.
		auto barrierTexImage = VkImageMemoryBarrier2{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
			.srcAccessMask = VK_ACCESS_2_NONE,
			.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.image = textures[i].image,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 
				.levelCount = ktxTexture->numLevels, 
				.layerCount = 1 
			}
		};
		auto barrierTexInfo = VkDependencyInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_DEPENDENCY_INFO, 
			.imageMemoryBarrierCount = 1, 
			.pImageMemoryBarriers = &barrierTexImage 
		};
		vkCmdPipelineBarrier2(cbOneTime, &barrierTexInfo);

		// Build one VkBufferImageCopy per mip level. KTX tells us the byte offset of
		// each mip within the blob; the destination extent halves at each level.
		auto copyRegions = std::vector<VkBufferImageCopy>{};
		for (auto j = std::uint32_t{}; j < ktxTexture->numLevels; j++)
		{
			auto mipOffset = ktx_size_t{ 0 };
			auto ret = ktxTexture_GetImageOffset(ktxTexture, j, 0, 0, &mipOffset);
			copyRegions.push_back({
				.bufferOffset = mipOffset,
				.imageSubresource = VkImageSubresourceLayers{
					.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 
					.mipLevel = (std::uint32_t)j, 
					.layerCount = 1
				},
				.imageExtent = VkExtent3D{
					.width = ktxTexture->baseWidth >> j, 
					.height = ktxTexture->baseHeight >> j, 
					.depth = 1 
				},
			});
		}
		// Issue the actual buffer→image copy for every mip level in one call.
		vkCmdCopyBufferToImage(
			cbOneTime, 
			imgSrcBuffer, 
			textures[i].image, 
			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			static_cast<std::uint32_t>(copyRegions.size()), 
			copyRegions.data()
		);

		// Post-copy barrier: transition TRANSFER_DST_OPTIMAL → READ_ONLY_OPTIMAL
		// and synchronise the transfer write against the fragment shader's read.
		auto barrierTexRead = VkImageMemoryBarrier2{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
			.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT,
			.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			.image = textures[i].image,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 
				.levelCount = ktxTexture->numLevels, 
				.layerCount = 1 
			}
		};
		barrierTexInfo.pImageMemoryBarriers = &barrierTexRead;
		vkCmdPipelineBarrier2(cbOneTime, &barrierTexInfo);
		chk(vkEndCommandBuffer(cbOneTime));

		// Submit the upload command buffer and block until the GPU finishes,
		// so it's safe to free the staging buffer immediately after.
		auto oneTimeSI = VkSubmitInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO, 
			.commandBufferCount = 1, 
			.pCommandBuffers = &cbOneTime 
		};
		chk(vkQueueSubmit(queue, 1, &oneTimeSI, fenceOneTime));
		chk(vkWaitForFences(device, 1, &fenceOneTime, true, std::numeric_limits<std::uint64_t>::max()));
		vkDestroyFence(device, fenceOneTime, nullptr);
		vmaDestroyBuffer(allocator, imgSrcBuffer, imgSrcAllocation);


		// Sampler
		auto samplerCI = VkSamplerCreateInfo{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VkFilter::VK_FILTER_LINEAR,
			.minFilter = VkFilter::VK_FILTER_LINEAR,
			.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.anisotropyEnable = true,
			.maxAnisotropy = 8.0f,
			.maxLod = (float)ktxTexture->numLevels,
		};
		chk(vkCreateSampler(device, &samplerCI, nullptr, &textures[i].sampler));
		// CPU-side KTX data is no longer needed once it's on the GPU.
		ktxTexture_Destroy(ktxTexture);

		// Record an image-info descriptor for this texture; the array of these
		// is later written into the combined-image-sampler descriptor set.
		textureDescriptors.push_back({ 
			.sampler = textures[i].sampler,
			.imageView = textures[i].view, 
			.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
		});
	}


	// Descriptor (indexing)
	auto descVariableFlag = VkDescriptorBindingFlags{ VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };
	auto descBindingFlags = VkDescriptorSetLayoutBindingFlagsCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO, 
		.bindingCount = 1, 
		.pBindingFlags = &descVariableFlag 
	};
	auto descLayoutBindingTex = VkDescriptorSetLayoutBinding{ 
		.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		.descriptorCount = static_cast<std::uint32_t>(textures.size()), 
		.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT 
	};
	auto descLayoutTexCI = VkDescriptorSetLayoutCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 
		.pNext = &descBindingFlags, 
		.bindingCount = 1, 
		.pBindings = &descLayoutBindingTex 
	};
	auto descriptorSetLayoutTex = VkDescriptorSetLayout{ };
	chk(vkCreateDescriptorSetLayout(device, &descLayoutTexCI, nullptr, &descriptorSetLayoutTex));
	auto poolSize = VkDescriptorPoolSize{ 
		.type = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		.descriptorCount = static_cast<std::uint32_t>(textures.size()) 
	};
	auto descPoolCI = VkDescriptorPoolCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, 
		.maxSets = 1, 
		.poolSizeCount = 1, 
		.pPoolSizes = &poolSize 
	};
	auto descriptorPool = VkDescriptorPool{ };
	chk(vkCreateDescriptorPool(device, &descPoolCI, nullptr, &descriptorPool));
	auto variableDescCount = std::uint32_t{ static_cast<std::uint32_t>(textures.size()) };
	auto variableDescCountAI = VkDescriptorSetVariableDescriptorCountAllocateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT, 
		.descriptorSetCount = 1, 
		.pDescriptorCounts = &variableDescCount 
	};
	auto texDescSetAlloc = VkDescriptorSetAllocateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, 
		.pNext = &variableDescCountAI, 
		.descriptorPool = descriptorPool, 
		.descriptorSetCount = 1, 
		.pSetLayouts = &descriptorSetLayoutTex 
	};
	auto descriptorSetTex = VkDescriptorSet{ };
	chk(vkAllocateDescriptorSets(device, &texDescSetAlloc, &descriptorSetTex));
	auto writeDescSet = VkWriteDescriptorSet{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, 
		.dstSet = descriptorSetTex, 
		.dstBinding = 0, 
		.descriptorCount = static_cast<std::uint32_t>(textureDescriptors.size()), 
		.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		.pImageInfo = textureDescriptors.data() 
	};
	vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);


	// Initialize Slang shader compiler
	auto slangGlobalSession = Slang::ComPtr<slang::IGlobalSession>{};
	slang::createGlobalSession(slangGlobalSession.writeRef());
	auto slangTargets = std::array{ 
		slang::TargetDesc{ 
			.format{SlangCompileTarget::SLANG_SPIRV}, 
			.profile{slangGlobalSession->findProfile("spirv_1_4")} 
		}
	};
	auto slangOptions = std::array{ 
		slang::CompilerOptionEntry{ 
			.name = slang::CompilerOptionName::EmitSpirvDirectly, 
			.value = slang::CompilerOptionValue{ slang::CompilerOptionValueKind::Int, 1 }
		} 
	};
	auto slangSessionDesc = slang::SessionDesc{ 
		.targets{slangTargets.data()}, 
		.targetCount{SlangInt(slangTargets.size())}, 
		.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR, 
		.compilerOptionEntries{slangOptions.data()}, 
		.compilerOptionEntryCount{std::uint32_t(slangOptions.size())} 
	};


	// Load shader
	auto slangSession = Slang::ComPtr<slang::ISession>{};
	slangGlobalSession->createSession(slangSessionDesc, slangSession.writeRef());
	auto slangModule = Slang::ComPtr<slang::IModule>{ 
		slangSession->loadModuleFromSource("triangle", "assets/shader.slang", nullptr, nullptr) 
	};
	auto spirv = Slang::ComPtr<ISlangBlob>{};
	slangModule->getTargetCode(0, spirv.writeRef());
	auto shaderModuleCI = VkShaderModuleCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, 
		.codeSize = spirv->getBufferSize(), 
		.pCode = (std::uint32_t*)spirv->getBufferPointer()
	};
	auto shaderModule = VkShaderModule{};
	chk(vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule));


	// Pipeline
	auto pushConstantRange = VkPushConstantRange{
		.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, 
		.size = sizeof(VkDeviceAddress) 
	};
	auto pipelineLayoutCI = VkPipelineLayoutCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, 
		.setLayoutCount = 1, 
		.pSetLayouts = &descriptorSetLayoutTex, 
		.pushConstantRangeCount = 1, 
		.pPushConstantRanges = &pushConstantRange 
	};
	auto pipelineLayout = VkPipelineLayout{ };
	chk(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout));
	auto shaderStages = std::vector<VkPipelineShaderStageCreateInfo>{
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, 
			.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, 
			.module = shaderModule, 
			.pName = "main"
		},
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, 
			.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, 
			.module = shaderModule, 
			.pName = "main" 
		}
	};
	auto vertexBinding = VkVertexInputBindingDescription{ 
		.binding = 0, 
		.stride = sizeof(Vertex), 
		.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX 
	};
	auto vertexAttributes = std::vector<VkVertexInputAttributeDescription>{
		{ 
			.location = 0, 
			.binding = 0, 
			.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT 
		},
		{ 
			.location = 1, 
			.binding = 0, 
			.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT, 
			.offset = offsetof(Vertex, normal) 
		},
		{ 
			.location = 2, 
			.binding = 0, 
			.format = VkFormat::VK_FORMAT_R32G32_SFLOAT, 
			.offset = offsetof(Vertex, uv) 
		},
	};
	auto vertexInputState = VkPipelineVertexInputStateCreateInfo{
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &vertexBinding,
		.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertexAttributes.size()),
		.pVertexAttributeDescriptions = vertexAttributes.data(),
	};
	auto inputAssemblyState = VkPipelineInputAssemblyStateCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, 
		.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST 
	};
	auto dynamicStates = std::vector<VkDynamicState>{ VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR };
	auto dynamicState = VkPipelineDynamicStateCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, 
		.dynamicStateCount = 2, 
		.pDynamicStates = dynamicStates.data() 
	};
	auto viewportState = VkPipelineViewportStateCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, 
		.viewportCount = 1, 
		.scissorCount = 1 
	};
	auto rasterizationState = VkPipelineRasterizationStateCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, 
		.lineWidth = 1.0f 
	};
	auto multisampleState = VkPipelineMultisampleStateCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, 
		.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT 
	};
	auto depthStencilState = VkPipelineDepthStencilStateCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, 
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL 
	};
	auto blendAttachment = VkPipelineColorBlendAttachmentState{ 
		.colorWriteMask = 0xF 
	};
	auto colorBlendState = VkPipelineColorBlendStateCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, 
		.attachmentCount = 1, 
		.pAttachments = &blendAttachment 
	};
	auto renderingCI = VkPipelineRenderingCreateInfo{ 
		.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO, 
		.colorAttachmentCount = 1, 
		.pColorAttachmentFormats = &imageFormat, 
		.depthAttachmentFormat = depthFormat 
	};
	auto pipelineCI = VkGraphicsPipelineCreateInfo{
		.sType = VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &renderingCI,
		.stageCount = 2,
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertexInputState,
		.pInputAssemblyState = &inputAssemblyState,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizationState,
		.pMultisampleState = &multisampleState,
		.pDepthStencilState = &depthStencilState,
		.pColorBlendState = &colorBlendState,
		.pDynamicState = &dynamicState,
		.layout = pipelineLayout
	};
	auto pipeline = VkPipeline{ };
	chk(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCI, nullptr, &pipeline));


	// Render loop
	auto lastTime = std::uint64_t{ SDL_GetTicks() };
	auto quit = false;
	auto frameIndex = std::uint32_t{ 0 };
	auto imageIndex = std::uint32_t{ 0 };
	auto camPos = glm::vec3{ 0.0f, 0.0f, -6.0f };
	auto shaderData = ShaderData{};
	auto objectRotations = std::array<glm::vec3, 3>{};
	while (not quit) 
	{
		// Sync
		// Wait until the GPU has finished using the per-frame resources we're
		// about to overwrite (command buffer, uniform buffer slot). The fence
		// was signalled by the previous submit using these same resources.
		chk(vkWaitForFences(device, 1, &fences[frameIndex], true, std::numeric_limits<std::uint64_t>::max()));
		chk(vkResetFences(device, 1, &fences[frameIndex]));
		// Acquire the next swapchain image to render into. The presentSemaphore
		// is signalled once the image is actually free for rendering — we make
		// the queue submit wait on it before any color-attachment writes.
		// chkSwapchain tolerates OUT_OF_DATE by flagging a swapchain rebuild.
		chkSwapchain(vkAcquireNextImageKHR(
			device, 
			swapchain, 
			std::numeric_limits<std::uint64_t>::max(),
			presentSemaphores[frameIndex], 
			nullptr, 
			&imageIndex
		));


		// Update shader data
		// Rebuild the per-frame UBO contents on the CPU and memcpy into the
		// host-mapped buffer for this frame slot. View matrix simply translates
		// by camPos; each of the 3 instances gets its own model matrix from a
		// translation along X plus a rotation quaternion driven by mouse input.
		shaderData.projection = glm::perspective(
			glm::radians(45.0f), 
			(float)windowSize.x / (float)windowSize.y, 
			0.1f, 
			32.0f
		);
		shaderData.view = glm::translate(glm::mat4(1.0f), camPos);
		for (auto i = 0; i < 3; i++) 
		{
			auto instancePos = 
				glm::vec3((float)(i - 1) * 3.0f, 0.0f, 0.0f);
			shaderData.model[i] = 
				glm::translate(glm::mat4(1.0f), instancePos) * glm::mat4_cast(glm::quat(objectRotations[i]));
		}
		memcpy(shaderDataBuffers[frameIndex].allocationInfo.pMappedData, &shaderData, sizeof(ShaderData));


		// Build command buffer
		// Start fresh: reset and re-record the per-frame command buffer. The
		// ONE_TIME_SUBMIT hint tells the driver this recording will only run once.
		auto cb = commandBuffers[frameIndex];
		chk(vkResetCommandBuffer(cb, 0));
		auto cbBI = VkCommandBufferBeginInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 
			.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT 
		};
		chk(vkBeginCommandBuffer(cb, &cbBI));

		// Pre-render barriers: transition both attachments from UNDEFINED to
		// ATTACHMENT_OPTIMAL before the render pass writes to them.
		//  - Swapchain image: previous contents are discarded (UNDEFINED is OK).
		//  - Depth image: old→new stages are LATE_FRAGMENT→EARLY_FRAGMENT to
		//    serialise depth writes against the previous frame's depth writes.
		auto outputBarriers = std::array{
			VkImageMemoryBarrier2{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				.image = swapchainImages[imageIndex],
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 
					.levelCount = 1, 
					.layerCount = 1 
				}
			},
			VkImageMemoryBarrier2{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
				.srcAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
				.dstAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				.image = depthImage,
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT | VkImageAspectFlagBits::VK_IMAGE_ASPECT_STENCIL_BIT, 
					.levelCount = 1, 
					.layerCount = 1 
				}
			}
		};
		auto barrierDependencyInfo = VkDependencyInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_DEPENDENCY_INFO, 
			.imageMemoryBarrierCount = 2, 
			.pImageMemoryBarriers = outputBarriers.data() 
		};
		vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);

		// Dynamic-rendering attachments. Color is cleared to black and stored;
		// depth is cleared to 1.0 and discarded after the pass (DONT_CARE),
		// since we don't need it again this frame.
		auto colorAttachmentInfo = VkRenderingAttachmentInfo{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = swapchainImageViews[imageIndex],
			.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue{
				.color{ 0.0f, 0.0f, 0.0f, 1.0f }
			}
		};
		auto depthAttachmentInfo = VkRenderingAttachmentInfo{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = depthImageView,
			.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.clearValue = { 
				.depthStencil = {1.0f,  0} 
			}
		};
		auto renderingInfo = VkRenderingInfo{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea{
				.extent{
					.width = static_cast<std::uint32_t>(windowSize.x), 
					.height = static_cast<std::uint32_t>(windowSize.y) 
				}
			},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo,
			.pDepthAttachment = &depthAttachmentInfo
		};
		// Begin a render pass (Vulkan 1.3 dynamic rendering — no VkRenderPass object).
		vkCmdBeginRendering(cb, &renderingInfo);

		// Viewport and scissor are dynamic state, so they must be set per frame
		// (also so they pick up the latest window size after a resize).
		auto vp = VkViewport{ 
			.width = static_cast<float>(windowSize.x), 
			.height = static_cast<float>(windowSize.y), 
			.minDepth = 0.0f, 
			.maxDepth = 1.0f 
		};
		vkCmdSetViewport(cb, 0, 1, &vp);
		auto scissor = VkRect2D{ 
			.extent{
				.width = static_cast<std::uint32_t>(windowSize.x), 
				.height = static_cast<std::uint32_t>(windowSize.y) 
			} 
		};
		// Bind the graphics pipeline and the texture array descriptor set.
		vkCmdBindPipeline(cb, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdSetScissor(cb, 0, 1, &scissor);
		vkCmdBindDescriptorSets(cb, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSetTex, 0, nullptr);
		// Vertex and index data live in the same buffer (vertices first, indices
		// after), so the index buffer is bound at offset vBufSize.
		auto vOffset = VkDeviceSize{ 0 };
		vkCmdBindVertexBuffers(cb, 0, 1, &vBuffer, &vOffset);
		vkCmdBindIndexBuffer(cb, vBuffer, vBufSize, VkIndexType::VK_INDEX_TYPE_UINT16);
		// Push the device address of this frame's UBO to the vertex shader so
		// it can read projection/view/model matrices via buffer reference.
		vkCmdPushConstants(
			cb, 
			pipelineLayout, 
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, 
			0, 
			sizeof(VkDeviceAddress), 
			&shaderDataBuffers[frameIndex].deviceAddress
		);
		// Draw the mesh 3 times (instanceCount = 3) — one Suzanne per slot.
		vkCmdDrawIndexed(cb, static_cast<std::uint32_t>(indexCount), 3, 0, 0, 0);
		vkCmdEndRendering(cb);

		// Pre-present barrier: transition the swapchain image from
		// ATTACHMENT_OPTIMAL to PRESENT_SRC_KHR so it's legal to hand to the
		// presentation engine. Only the prior color writes need to be visible.
		auto barrierPresent = VkImageMemoryBarrier2{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = 0,
			.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.image = swapchainImages[imageIndex],
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 
				.levelCount = 1, 
				.layerCount = 1 
			}
		};
		auto barrierPresentDependencyInfo = VkDependencyInfo{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_DEPENDENCY_INFO, 
			.imageMemoryBarrierCount = 1, 
			.pImageMemoryBarriers = &barrierPresent 
		};
		vkCmdPipelineBarrier2(cb, &barrierPresentDependencyInfo);
		chk(vkEndCommandBuffer(cb));


		// Submit to graphics queue
		// Wait on presentSemaphore (image acquired) at the color-output stage,
		// signal renderSemaphore when rendering finishes (consumed by present),
		// and signal the per-frame fence so the next iteration can reuse this
		// frame's resources once the GPU is done.
		// Note: VkSubmitInfo (1.0) takes 32-bit VkPipelineStageFlags — the
		// _2_ constant is 64-bit, hence the static_cast.
		auto waitStages = static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
		auto submitInfo = VkSubmitInfo{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &presentSemaphores[frameIndex],
			.pWaitDstStageMask = &waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &cb,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &renderSemaphores[imageIndex],
		};
		chk(vkQueueSubmit(queue, 1, &submitInfo, fences[frameIndex]));
		// Advance to the next frame slot for the next iteration's CPU work
		// (this lets up to maxFramesInFlight frames be in flight on the GPU).
		frameIndex = (frameIndex + 1) % maxFramesInFlight;
		// Present the just-rendered image, gated on the render-finished
		// semaphore. renderSemaphores are indexed by swapchain image so each
		// image has its own semaphore regardless of frame slot.
		auto presentInfo = VkPresentInfoKHR{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &renderSemaphores[imageIndex],
			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &imageIndex
		};
		chkSwapchain(vkQueuePresentKHR(queue, &presentInfo));


		// Event polling
		// Compute frame delta-time in seconds for use as input scaling below.
		auto elapsedTime = float{ (SDL_GetTicks() - lastTime) / 1000.0f };
		lastTime = SDL_GetTicks();
		// Drain SDL's event queue. Each iteration handles one event.
		for (SDL_Event event; SDL_PollEvent(&event);) 
		{
			if (event.type == SDL_EVENT_QUIT) 
			{
				quit = true;
				break;
			}
			// Left-mouse drag rotates the currently selected mesh around the
			// X (pitch) and Y (yaw) axes. Scaled by elapsedTime for framerate
			// independence.
			if (event.type == SDL_EVENT_MOUSE_MOTION and event.button.button == SDL_BUTTON_LEFT)
			{
				objectRotations[shaderData.selected].x -= (float)event.motion.yrel * elapsedTime;
				objectRotations[shaderData.selected].y += (float)event.motion.xrel * elapsedTime;
			}
			// Wheel zooms the camera along Z.
			if (event.type == SDL_EVENT_MOUSE_WHEEL)
			{
				camPos.z += (float)event.wheel.y * elapsedTime * 10.0f;
			}
			// +/- cycle through the three mesh slots (with wrap-around).
			if (event.type == SDL_EVENT_KEY_DOWN) 
			{
				if (event.key.key == SDLK_PLUS or event.key.key == SDLK_KP_PLUS)
					shaderData.selected = (shaderData.selected < 2) ? shaderData.selected + 1 : 0;
				if (event.key.key == SDLK_MINUS or event.key.key == SDLK_KP_MINUS)
					shaderData.selected = (shaderData.selected > 0) ? shaderData.selected - 1 : 2;
			}


			// Window resize
			// Defer the actual swapchain rebuild until after the event loop so
			// we don't tear down resources that the in-flight frame still uses.
			if (event.type == SDL_EVENT_WINDOW_RESIZED)
			{
				UpdateSwapchain = true;
			}
		}

		// Swapchain recreation. Triggered either by a resize event above or by
		// vkAcquireNextImageKHR/vkQueuePresentKHR returning OUT_OF_DATE_KHR.
		if (UpdateSwapchain) 
		{
			chk(SDL_GetWindowSize(window, &windowSize.x, &windowSize.y));
			UpdateSwapchain = false;
			// Wait for the device to go idle so it's safe to destroy resources.
			chk(vkDeviceWaitIdle(device));
			chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));
			// Recreate the swapchain at the new size, passing the old one in
			// oldSwapchain so the driver can recycle resources.
			swapchainCI.oldSwapchain = swapchain;
			swapchainCI.imageExtent = VkExtent2D{ 
				.width = static_cast<std::uint32_t>(windowSize.x), 
				.height = static_cast<std::uint32_t>(windowSize.y) 
			};
			chk(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));
			// Destroy the old per-image views before re-querying the new images.
			for (auto i = std::uint32_t{}; i < imageCount; i++) 
			{
				vkDestroyImageView(device, swapchainImageViews[i], nullptr);
			}
			// Re-query the new swapchain images (count may have changed).
			chk(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
			swapchainImages.resize(imageCount);
			chk(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()));
			swapchainImageViews.resize(imageCount);
			// Create a fresh image view for each new swapchain image.
			for (auto i = std::uint32_t{}; i < imageCount; i++) 
			{
				auto viewCI = VkImageViewCreateInfo{ 
					.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
					.image = swapchainImages[i], 
					.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D, 
					.format = imageFormat, 
					.subresourceRange = VkImageSubresourceRange{
						.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 
						.levelCount = 1, 
						.layerCount = 1
					} 
				};
				chk(vkCreateImageView(device, &viewCI, nullptr, &swapchainImageViews[i]));
			}
			// Render-finished semaphores are indexed per swapchain image, so
			// they need to be resized and re-created if the image count changed.
			for (auto& semaphore : renderSemaphores) 
			{
				vkDestroySemaphore(device, semaphore, nullptr);
			}
			renderSemaphores.resize(imageCount);
			for (auto& semaphore : renderSemaphores) 
			{
				chk(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
			}
			// Now that nothing references it, dispose of the old swapchain.
			vkDestroySwapchainKHR(device, swapchainCI.oldSwapchain, nullptr);
			// Recreate the depth attachment to match the new window size.
			vmaDestroyImage(allocator, depthImage, depthImageAllocation);
			vkDestroyImageView(device, depthImageView, nullptr);
			depthImageCI.extent = VkExtent3D{ 
				.width = static_cast<std::uint32_t>(windowSize.x), 
				.height = static_cast<std::uint32_t>(windowSize.y), 
				.depth = 1 
			};
			auto allocCI = VmaAllocationCreateInfo{ 
				.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, 
				.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO 
			};
			chk(vmaCreateImage(
				allocator, 
				&depthImageCI, 
				&allocCI, 
				&depthImage, 
				&depthImageAllocation, 
				nullptr
			));
			auto viewCI = VkImageViewCreateInfo{ 
				.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
				.image = depthImage, 
				.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D, 
				.format = depthFormat, 
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT, 
					.levelCount = 1, 
					.layerCount = 1 
				} 
			};
			chk(vkCreateImageView(device, &viewCI, nullptr, &depthImageView));
		}
	}


	// Tear down
	chk(vkDeviceWaitIdle(device));
	for (auto i = 0; i < maxFramesInFlight; i++) 
	{
		vkDestroyFence(device, fences[i], nullptr);
		vkDestroySemaphore(device, presentSemaphores[i], nullptr);
		vmaDestroyBuffer(allocator, shaderDataBuffers[i].buffer, shaderDataBuffers[i].allocation);
	}
	for (auto i = 0; i < renderSemaphores.size(); i++) 
	{
		vkDestroySemaphore(device, renderSemaphores[i], nullptr);
	}
	vmaDestroyImage(allocator, depthImage, depthImageAllocation);
	vkDestroyImageView(device, depthImageView, nullptr);
	for (auto i = 0; i < swapchainImageViews.size(); i++) 
	{
		vkDestroyImageView(device, swapchainImageViews[i], nullptr);
	}
	vmaDestroyBuffer(allocator, vBuffer, vBufferAllocation);
	for (auto i = 0; i < textures.size(); i++) 
	{
		vkDestroyImageView(device, textures[i].view, nullptr);
		vkDestroySampler(device, textures[i].sampler, nullptr);
		vmaDestroyImage(allocator, textures[i].image, textures[i].allocation);
	}
	vkDestroyDescriptorSetLayout(device, descriptorSetLayoutTex, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyShaderModule(device, shaderModule, nullptr);
	vmaDestroyAllocator(allocator);
	SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}