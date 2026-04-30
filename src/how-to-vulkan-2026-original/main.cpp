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

constexpr auto maxFramesInFlight = uint32_t{ 2 };
auto imageIndex = uint32_t{ 0 };
auto frameIndex = uint32_t{ 0 };
auto instance = VkInstance{ };
auto device = VkDevice{ };
auto queue = VkQueue{ };
auto surface = VkSurfaceKHR{ };
auto updateSwapchain = false;
auto swapchain = VkSwapchainKHR{ };
auto commandPool = VkCommandPool{ };
auto pipeline = VkPipeline{ };
auto pipelineLayout = VkPipelineLayout{ };
auto depthImage = VkImage{ };
auto allocator = VmaAllocator{ };
auto depthImageAllocation = VmaAllocation{ };
auto depthImageView = VkImageView{ };
auto swapchainImages = std::vector<VkImage>{};
auto swapchainImageViews = std::vector<VkImageView>{};
auto commandBuffers = std::array<VkCommandBuffer, maxFramesInFlight>{};
auto fences = std::array<VkFence, maxFramesInFlight>{};
auto presentSemaphores = std::array<VkSemaphore, maxFramesInFlight>{};
auto renderSemaphores = std::vector<VkSemaphore>{};
auto vBufferAllocation = VmaAllocation{ };
auto vBuffer = VkBuffer{ };
struct ShaderData 
{
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model[3];
	glm::vec4 lightPos{ 0.0f, -10.0f, 10.0f, 0.0f };
	uint32_t selected{ 1 };
} shaderData{};
struct ShaderDataBuffer 
{
	VmaAllocation allocation{ };
	VmaAllocationInfo allocationInfo{};
	VkBuffer buffer{ };
	VkDeviceAddress deviceAddress{};
};
auto shaderDataBuffers = std::array<ShaderDataBuffer, maxFramesInFlight>{};
struct Texture 
{
	VmaAllocation allocation{ };
	VkImage image{ };
	VkImageView view{ };
	VkSampler sampler{ };
};
auto textures = std::array<Texture, 3>{};
auto descriptorPool = VkDescriptorPool{ };
auto descriptorSetLayoutTex = VkDescriptorSetLayout{ };
auto descriptorSetTex = VkDescriptorSet{ };
auto slangGlobalSession = Slang::ComPtr<slang::IGlobalSession>{};
auto camPos = glm::vec3{ 0.0f, 0.0f, -6.0f };
auto objectRotations = std::array<glm::vec3, 3>{};
auto windowSize = glm::ivec2{};
struct Vertex 
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

static inline void chk(VkResult result) 
{
	if (result != VK_SUCCESS) 
	{
		std::cerr << "Vulkan call returned an error (" << result << ")\n";
		exit(result);
	}
}
static inline void chkSwapchain(VkResult result) 
{
	if (result < VK_SUCCESS) 
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR) 
		{
			updateSwapchain = true;
			return;
		}
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
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, 
		.pApplicationName = "How to Vulkan", 
		.apiVersion = VK_API_VERSION_1_3 
	};
	auto instanceExtensionsCount = uint32_t{ 0 };
	auto instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount);
	auto instanceCI = VkInstanceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledExtensionCount = instanceExtensionsCount,
		.ppEnabledExtensionNames = instanceExtensions,
	};
	chk(vkCreateInstance(&instanceCI, nullptr, &instance));
	volkLoadInstance(instance);
	
	
	// Device
	auto deviceCount = uint32_t{ 0 };
	chk(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
	auto devices = std::vector<VkPhysicalDevice>(deviceCount);
	chk(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));
	auto deviceIndex = uint32_t{ 0 };
	if (argc > 1) 
	{
		deviceIndex = std::stoi(argv[1]);
		assert(deviceIndex < deviceCount);
	}
	auto deviceProperties = VkPhysicalDeviceProperties2{ 
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 
	};
	vkGetPhysicalDeviceProperties2(devices[deviceIndex], &deviceProperties);
	std::cout << "Selected device: " << deviceProperties.properties.deviceName << "\n";
	
	
	// Find a queue family for graphics
	auto queueFamilyCount = uint32_t{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, nullptr);
	auto queueFamilies = std::vector<VkQueueFamilyProperties>(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, queueFamilies.data());
	auto queueFamily = uint32_t{ 0 };
	for (auto i = size_t{ 0 }; i < queueFamilies.size(); i++) 
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
		{
			queueFamily = static_cast<uint32_t>(i);
			break;
		}
	}
	chk(SDL_Vulkan_GetPresentationSupport(instance, devices[deviceIndex], queueFamily));
	
	
	// Logical device
	auto const qfpriorities = float{ 1.0f };
	auto queueCI = VkDeviceQueueCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, 
		.queueFamilyIndex = queueFamily, 
		.queueCount = 1, 
		.pQueuePriorities = &qfpriorities 
	};
	auto enabledVk12Features = VkPhysicalDeviceVulkan12Features{ 
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, 
		.descriptorIndexing = true, 
		.shaderSampledImageArrayNonUniformIndexing = true, 
		.descriptorBindingVariableDescriptorCount = true, 
		.runtimeDescriptorArray = true, 
		.bufferDeviceAddress = true 
	};
	auto enabledVk13Features = VkPhysicalDeviceVulkan13Features{ 
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, 
		.pNext = &enabledVk12Features, 
		.synchronization2 = true, 
		.dynamicRendering = true 
	};
	auto enabledVk10Features = VkPhysicalDeviceFeatures{ 
		.samplerAnisotropy = true
	};
	auto const deviceExtensions = std::vector<const char*>{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	auto deviceCI = VkDeviceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &enabledVk13Features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCI,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &enabledVk10Features
	};
	chk(vkCreateDevice(devices[deviceIndex], &deviceCI, nullptr, &device));
	vkGetDeviceQueue(device, queueFamily, 0, &queue);
	
	
	// VMA
	auto vkFunctions = VmaVulkanFunctions{ 
		.vkGetInstanceProcAddr = vkGetInstanceProcAddr, 
		.vkGetDeviceProcAddr = vkGetDeviceProcAddr, 
		.vkCreateImage = vkCreateImage 
	};
	auto allocatorCI = VmaAllocatorCreateInfo{ 
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT, 
		.physicalDevice = devices[deviceIndex], 
		.device = device, 
		.pVulkanFunctions = &vkFunctions, 
		.instance = instance 
	};
	chk(vmaCreateAllocator(&allocatorCI, &allocator));
	
	
	// Window and surface
	auto window = SDL_CreateWindow("How to Vulkan", 1280u, 720u, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	assert(window);
	chk(SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface));
	chk(SDL_GetWindowSize(window, &windowSize.x, &windowSize.y));
	auto surfaceCaps = VkSurfaceCapabilitiesKHR{};
	chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[deviceIndex], surface, &surfaceCaps));
	auto swapchainExtent = VkExtent2D{ surfaceCaps.currentExtent };
	if (surfaceCaps.currentExtent.width == 0xFFFFFFFF) 
	{
		swapchainExtent = { 
			.width = static_cast<uint32_t>(windowSize.x), 
			.height = static_cast<uint32_t>(windowSize.y) 
		};
	}
	
	
	// Swap chain
	auto const imageFormat = VkFormat{ VK_FORMAT_B8G8R8A8_SRGB };
	auto swapchainCI = VkSwapchainCreateInfoKHR{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = surfaceCaps.minImageCount,
		.imageFormat = imageFormat,
		.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		.imageExtent = VkExtent2D{
			.width = swapchainExtent.width, 
			.height = swapchainExtent.height 
		},
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR
	};
	chk(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));
	auto imageCount = uint32_t{ 0 };
	chk(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
	swapchainImages.resize(imageCount);
	chk(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()));
	swapchainImageViews.resize(imageCount);
	for (auto i = std::uint32_t{}; i < imageCount; i++)
	{
		auto viewCI = VkImageViewCreateInfo{ 
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
			.image = swapchainImages[i], 
			.viewType = VK_IMAGE_VIEW_TYPE_2D, 
			.format = imageFormat, 
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
				.levelCount = 1, 
				.layerCount = 1 
			} 
		};
		chk(vkCreateImageView(device, &viewCI, nullptr, &swapchainImageViews[i]));
	}
	
	
	// Depth attachment
	auto depthFormatList = std::vector<VkFormat>{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	auto depthFormat = VkFormat{ VK_FORMAT_UNDEFINED };
	for (auto& format : depthFormatList) 
	{
		auto formatProperties = VkFormatProperties2{ 
			.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 
		};
		vkGetPhysicalDeviceFormatProperties2(devices[deviceIndex], format, &formatProperties);
		if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) 
		{
			depthFormat = format;
			break;
		}
	}
	assert(depthFormat != VK_FORMAT_UNDEFINED);
	auto depthImageCI = VkImageCreateInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = depthFormat,
		.extent{
			.width = static_cast<uint32_t>(windowSize.x), 
			.height = static_cast<uint32_t>(windowSize.y), 
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	auto allocCI = VmaAllocationCreateInfo{ 
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, 
		.usage = VMA_MEMORY_USAGE_AUTO 
	};
	chk(vmaCreateImage(allocator, &depthImageCI, &allocCI, &depthImage, &depthImageAllocation, nullptr));
	auto depthViewCI = VkImageViewCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = depthImage, 
		.viewType = VK_IMAGE_VIEW_TYPE_2D, 
		.format = depthFormat, 
		.subresourceRange = VkImageSubresourceRange{
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, 
			.levelCount = 1, 
			.layerCount = 1 
		} 
	};
	chk(vkCreateImageView(device, &depthViewCI, nullptr, &depthImageView));
	
	
	// Mesh data
	auto attrib = tinyobj::attrib_t{};
	auto shapes = std::vector<tinyobj::shape_t>{};
	auto materials = std::vector<tinyobj::material_t>{};
	chk(tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr, "assets/suzanne.obj"));
	auto const indexCount = VkDeviceSize{ shapes[0].mesh.indices.size() };
	auto vertices = std::vector<Vertex>{};
	auto indices = std::vector<uint16_t>{};
	
	
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
		indices.push_back(static_cast<uint16_t>(indices.size()));
	}
	auto vBufSize = VkDeviceSize{ sizeof(Vertex) * vertices.size() };
	auto iBufSize = VkDeviceSize{ sizeof(uint16_t) * indices.size() };
	auto bufferCI = VkBufferCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 
		.size = vBufSize + iBufSize, 
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT 
	};
	auto vBufferAllocCI = VmaAllocationCreateInfo{ 
		.flags = 
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT 
			| VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT 
			| VMA_ALLOCATION_CREATE_MAPPED_BIT, 
		.usage = VMA_MEMORY_USAGE_AUTO 
	};
	auto vBufferAllocInfo = VmaAllocationInfo{};
	chk(vmaCreateBuffer(allocator, &bufferCI, &vBufferAllocCI, &vBuffer, &vBufferAllocation, &vBufferAllocInfo));
	memcpy(vBufferAllocInfo.pMappedData, vertices.data(), vBufSize);
	memcpy(((char*)vBufferAllocInfo.pMappedData) + vBufSize, indices.data(), iBufSize);
	
	
	// Shader data buffers
	for (auto i = 0; i < maxFramesInFlight; i++) 
	{
		auto uBufferCI = VkBufferCreateInfo{ 
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 
			.size = sizeof(ShaderData), 
			.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT 
		};
		auto uBufferAllocCI = VmaAllocationCreateInfo{ 
			.flags = 
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT 
				| VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT 
				| VMA_ALLOCATION_CREATE_MAPPED_BIT, 
			.usage = VMA_MEMORY_USAGE_AUTO 
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
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, 
			.buffer = shaderDataBuffers[i].buffer 
		};
		shaderDataBuffers[i].deviceAddress = vkGetBufferDeviceAddress(device, &uBufferBdaInfo);
	}
	
	
	// Sync objects
	auto semaphoreCI = VkSemaphoreCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO 
	};
	auto fenceCI = VkFenceCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, 
		.flags = VK_FENCE_CREATE_SIGNALED_BIT 
	};
	for (auto i = 0; i < maxFramesInFlight; i++) 
	{
		chk(vkCreateFence(device, &fenceCI, nullptr, &fences[i]));
		chk(vkCreateSemaphore(device, &semaphoreCI, nullptr, &presentSemaphores[i]));
	}
	renderSemaphores.resize(swapchainImages.size());
	for (auto& semaphore : renderSemaphores) 
	{
		chk(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
	}
	
	
	// Command pool
	auto commandPoolCI = VkCommandPoolCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, 
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 
		.queueFamilyIndex = queueFamily 
	};
	chk(vkCreateCommandPool(device, &commandPoolCI, nullptr, &commandPool));
	auto cbAllocCI = VkCommandBufferAllocateInfo{ 
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
		.commandPool = commandPool, 
		.commandBufferCount = maxFramesInFlight 
	};
	chk(vkAllocateCommandBuffers(device, &cbAllocCI, commandBuffers.data()));
	
	
	// Texture images
	// For each texture: load a KTX file from disk, create a GPU image + view,
	// upload all mip levels via a staging buffer using a one-time command buffer,
	// transition the image to a shader-readable layout, then create a sampler
	// and append a descriptor entry to be bound later.
	auto textureDescriptors = std::vector<VkDescriptorImageInfo>{};
	for (auto i = 0; i < textures.size(); i++) 
	{
		// Load the KTX container from disk (includes all mip levels and pixel data).
		auto ktxTexture = static_cast<::ktxTexture*>(nullptr);
		auto filename = "assets/suzanne" + std::to_string(i) + ".ktx";
		ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

		// Create the destination GPU image, sized and formatted to match the KTX file.
		// Usage: TRANSFER_DST so we can copy pixels into it, SAMPLED so shaders can read it.
		auto texImgCI = VkImageCreateInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = ktxTexture_GetVkFormat(ktxTexture),
			.extent = {
				.width = ktxTexture->baseWidth, 
				.height = ktxTexture->baseHeight, 
				.depth = 1 
			},
			.mipLevels = ktxTexture->numLevels,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		auto texImageAllocCI = VmaAllocationCreateInfo{ 
			.usage = VMA_MEMORY_USAGE_AUTO 
		};
		chk(vmaCreateImage(allocator, &texImgCI, &texImageAllocCI, &textures[i].image, &textures[i].allocation, nullptr));

		// Image view exposing all mip levels for shader sampling.
		auto texVewCI = VkImageViewCreateInfo{ 
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
			.image = textures[i].image, 
			.viewType = VK_IMAGE_VIEW_TYPE_2D, 
			.format = texImgCI.format, 
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
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
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 
			.size = (uint32_t)ktxTexture->dataSize, 
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT 
		};
		auto imgSrcAllocCI = VmaAllocationCreateInfo{ 
			.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, 
			.usage = VMA_MEMORY_USAGE_AUTO 
		};
		auto imgSrcAllocInfo = VmaAllocationInfo{};
		chk(vmaCreateBuffer(allocator, &imgSrcBufferCI, &imgSrcAllocCI, &imgSrcBuffer, &imgSrcAllocation, &imgSrcAllocInfo));
		// Copy the entire KTX blob (all mips) into the mapped staging buffer.
		memcpy(imgSrcAllocInfo.pMappedData, ktxTexture->pData, ktxTexture->dataSize);

		// Allocate a fence + one-time command buffer to drive the upload synchronously.
		auto fenceOneTimeCI = VkFenceCreateInfo{ 
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO 
		};
		auto fenceOneTime = VkFence{};
		chk(vkCreateFence(device, &fenceOneTimeCI, nullptr, &fenceOneTime));
		auto cbOneTime = VkCommandBuffer{};
		auto cbOneTimeAI = VkCommandBufferAllocateInfo{ 
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
			.commandPool = commandPool, 
			.commandBufferCount = 1 
		};
		chk(vkAllocateCommandBuffers(device, &cbOneTimeAI, &cbOneTime));
		auto cbOneTimeBI = VkCommandBufferBeginInfo{ 
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		chk(vkBeginCommandBuffer(cbOneTime, &cbOneTimeBI));

		// Pre-copy barrier: transition the freshly-created image from UNDEFINED
		// to TRANSFER_DST_OPTIMAL so the upcoming buffer→image copy is legal.
		auto barrierTexImage = VkImageMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
			.srcAccessMask = VK_ACCESS_2_NONE,
			.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.image = textures[i].image,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
				.levelCount = ktxTexture->numLevels, 
				.layerCount = 1 
			}
		};
		auto barrierTexInfo = VkDependencyInfo{ 
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, 
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
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
					.mipLevel = (uint32_t)j, 
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
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			static_cast<uint32_t>(copyRegions.size()), 
			copyRegions.data()
		);

		// Post-copy barrier: transition TRANSFER_DST_OPTIMAL → READ_ONLY_OPTIMAL
		// and synchronise the transfer write against the fragment shader's read.
		auto barrierTexRead = VkImageMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			.image = textures[i].image,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
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
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, 
			.commandBufferCount = 1, 
			.pCommandBuffers = &cbOneTime 
		};
		chk(vkQueueSubmit(queue, 1, &oneTimeSI, fenceOneTime));
		chk(vkWaitForFences(device, 1, &fenceOneTime, true, UINT64_MAX));
		vkDestroyFence(device, fenceOneTime, nullptr);
		vmaDestroyBuffer(allocator, imgSrcBuffer, imgSrcAllocation);


		// Sampler
		auto samplerCI = VkSamplerCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
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
			.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
		});
	}


	// Descriptor (indexing)
	auto descVariableFlag = VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };
	auto descBindingFlags = VkDescriptorSetLayoutBindingFlagsCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO, 
		.bindingCount = 1, 
		.pBindingFlags = &descVariableFlag 
	};
	auto descLayoutBindingTex = VkDescriptorSetLayoutBinding{ 
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		.descriptorCount = static_cast<uint32_t>(textures.size()), 
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT 
	};
	auto descLayoutTexCI = VkDescriptorSetLayoutCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 
		.pNext = &descBindingFlags, 
		.bindingCount = 1, 
		.pBindings = &descLayoutBindingTex 
	};
	chk(vkCreateDescriptorSetLayout(device, &descLayoutTexCI, nullptr, &descriptorSetLayoutTex));
	auto poolSize = VkDescriptorPoolSize{ 
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		.descriptorCount = static_cast<uint32_t>(textures.size()) 
	};
	auto descPoolCI = VkDescriptorPoolCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, 
		.maxSets = 1, 
		.poolSizeCount = 1, 
		.pPoolSizes = &poolSize 
	};
	chk(vkCreateDescriptorPool(device, &descPoolCI, nullptr, &descriptorPool));
	auto variableDescCount = uint32_t{ static_cast<uint32_t>(textures.size()) };
	auto variableDescCountAI = VkDescriptorSetVariableDescriptorCountAllocateInfo{ 
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT, 
		.descriptorSetCount = 1, 
		.pDescriptorCounts = &variableDescCount 
	};
	auto texDescSetAlloc = VkDescriptorSetAllocateInfo{ 
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, 
		.pNext = &variableDescCountAI, 
		.descriptorPool = descriptorPool, 
		.descriptorSetCount = 1, 
		.pSetLayouts = &descriptorSetLayoutTex 
	};
	chk(vkAllocateDescriptorSets(device, &texDescSetAlloc, &descriptorSetTex));
	auto writeDescSet = VkWriteDescriptorSet{ 
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, 
		.dstSet = descriptorSetTex, 
		.dstBinding = 0, 
		.descriptorCount = static_cast<uint32_t>(textureDescriptors.size()), 
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		.pImageInfo = textureDescriptors.data() 
	};
	vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);


	// Initialize Slang shader compiler
	slang::createGlobalSession(slangGlobalSession.writeRef());
	auto slangTargets = std::array{ 
		slang::TargetDesc{ 
			.format{SLANG_SPIRV}, 
			.profile{slangGlobalSession->findProfile("spirv_1_4")} 
		}
	};
	auto slangOptions = std::array{ 
		slang::CompilerOptionEntry{ 
			slang::CompilerOptionName::EmitSpirvDirectly, 
			{ slang::CompilerOptionValueKind::Int, 1 } 
		} 
	};
	auto slangSessionDesc = slang::SessionDesc{ 
		.targets{slangTargets.data()}, 
		.targetCount{SlangInt(slangTargets.size())}, 
		.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR, 
		.compilerOptionEntries{slangOptions.data()}, 
		.compilerOptionEntryCount{uint32_t(slangOptions.size())} 
	};


	// Load shader
	Slang::ComPtr<slang::ISession> slangSession;
	slangGlobalSession->createSession(slangSessionDesc, slangSession.writeRef());
	auto slangModule = Slang::ComPtr<slang::IModule>{ slangSession->loadModuleFromSource("triangle", "assets/shader.slang", nullptr, nullptr) };
	Slang::ComPtr<ISlangBlob> spirv;
	slangModule->getTargetCode(0, spirv.writeRef());
	auto shaderModuleCI = VkShaderModuleCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, 
		.codeSize = spirv->getBufferSize(), 
		.pCode = (uint32_t*)spirv->getBufferPointer()
	};
	auto shaderModule = VkShaderModule{};
	chk(vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule));


	// Pipeline
	auto pushConstantRange = VkPushConstantRange{
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT, 
		.size = sizeof(VkDeviceAddress) 
	};
	auto pipelineLayoutCI = VkPipelineLayoutCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, 
		.setLayoutCount = 1, 
		.pSetLayouts = &descriptorSetLayoutTex, 
		.pushConstantRangeCount = 1, 
		.pPushConstantRanges = &pushConstantRange 
	};
	chk(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout));
	auto shaderStages = std::vector<VkPipelineShaderStageCreateInfo>{
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, 
			.stage = VK_SHADER_STAGE_VERTEX_BIT, 
			.module = shaderModule, 
			.pName = "main"
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, 
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT, 
			.module = shaderModule, 
			.pName = "main" 
		}
	};
	auto vertexBinding = VkVertexInputBindingDescription{ 
		.binding = 0, 
		.stride = sizeof(Vertex), 
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX 
	};
	auto vertexAttributes = std::vector<VkVertexInputAttributeDescription>{
		{ 
			.location = 0, 
			.binding = 0, 
			.format = VK_FORMAT_R32G32B32_SFLOAT 
		},
		{ 
			.location = 1, 
			.binding = 0, 
			.format = VK_FORMAT_R32G32B32_SFLOAT, 
			.offset = offsetof(Vertex, normal) 
		},
		{ 
			.location = 2, 
			.binding = 0, 
			.format = VK_FORMAT_R32G32_SFLOAT, 
			.offset = offsetof(Vertex, uv) 
		},
	};
	auto vertexInputState = VkPipelineVertexInputStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &vertexBinding,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size()),
		.pVertexAttributeDescriptions = vertexAttributes.data(),
	};
	auto inputAssemblyState = VkPipelineInputAssemblyStateCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, 
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST 
	};
	auto dynamicStates = std::vector<VkDynamicState>{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	auto dynamicState = VkPipelineDynamicStateCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, 
		.dynamicStateCount = 2, 
		.pDynamicStates = dynamicStates.data() 
	};
	auto viewportState = VkPipelineViewportStateCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, 
		.viewportCount = 1, 
		.scissorCount = 1 
	};
	auto rasterizationState = VkPipelineRasterizationStateCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, 
		.lineWidth = 1.0f 
	};
	auto multisampleState = VkPipelineMultisampleStateCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, 
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT 
	};
	auto depthStencilState = VkPipelineDepthStencilStateCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, 
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL 
	};
	auto blendAttachment = VkPipelineColorBlendAttachmentState{ 
		.colorWriteMask = 0xF 
	};
	auto colorBlendState = VkPipelineColorBlendStateCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, 
		.attachmentCount = 1, 
		.pAttachments = &blendAttachment 
	};
	auto renderingCI = VkPipelineRenderingCreateInfo{ 
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO, 
		.colorAttachmentCount = 1, 
		.pColorAttachmentFormats = &imageFormat, 
		.depthAttachmentFormat = depthFormat 
	};
	auto pipelineCI = VkGraphicsPipelineCreateInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
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
	chk(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline));


	// Render loop
	auto lastTime = uint64_t{ SDL_GetTicks() };
	auto quit = false;
	while (not quit) 
	{
		// Sync
		chk(vkWaitForFences(device, 1, &fences[frameIndex], true, UINT64_MAX));
		chk(vkResetFences(device, 1, &fences[frameIndex]));
		chkSwapchain(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex));


		// Update shader data
		shaderData.projection = glm::perspective(glm::radians(45.0f), (float)windowSize.x / (float)windowSize.y, 0.1f, 32.0f);
		shaderData.view = glm::translate(glm::mat4(1.0f), camPos);
		for (auto i = 0; i < 3; i++) 
		{
			auto instancePos = glm::vec3((float)(i - 1) * 3.0f, 0.0f, 0.0f);
			shaderData.model[i] = glm::translate(glm::mat4(1.0f), instancePos) * glm::mat4_cast(glm::quat(objectRotations[i]));
		}
		memcpy(shaderDataBuffers[frameIndex].allocationInfo.pMappedData, &shaderData, sizeof(ShaderData));


		// Build command buffer
		auto cb = commandBuffers[frameIndex];
		chk(vkResetCommandBuffer(cb, 0));
		auto cbBI = VkCommandBufferBeginInfo{ 
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT 
		};
		chk(vkBeginCommandBuffer(cb, &cbBI));
		auto outputBarriers = std::array{
			VkImageMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				.image = swapchainImages[imageIndex],
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
					.levelCount = 1, 
					.layerCount = 1 
				}
			},
			VkImageMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
				.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
				.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				.image = depthImage,
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 
					.levelCount = 1, 
					.layerCount = 1 
				}
			}
		};
		auto barrierDependencyInfo = VkDependencyInfo{ 
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, 
			.imageMemoryBarrierCount = 2, 
			.pImageMemoryBarriers = outputBarriers.data() 
		};
		vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);
		auto colorAttachmentInfo = VkRenderingAttachmentInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = swapchainImageViews[imageIndex],
			.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue{
				.color{ 0.0f, 0.0f, 0.0f, 1.0f }
			}
		};
		auto depthAttachmentInfo = VkRenderingAttachmentInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = depthImageView,
			.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.clearValue = { 
				.depthStencil = {1.0f,  0} 
			}
		};
		auto renderingInfo = VkRenderingInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea{
				.extent{
					.width = static_cast<uint32_t>(windowSize.x), 
					.height = static_cast<uint32_t>(windowSize.y) 
				}
			},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo,
			.pDepthAttachment = &depthAttachmentInfo
		};
		vkCmdBeginRendering(cb, &renderingInfo);
		auto vp = VkViewport{ 
			.width = static_cast<float>(windowSize.x), 
			.height = static_cast<float>(windowSize.y), 
			.minDepth = 0.0f, 
			.maxDepth = 1.0f 
		};
		vkCmdSetViewport(cb, 0, 1, &vp);
		auto scissor = VkRect2D{ 
			.extent{
				.width = static_cast<uint32_t>(windowSize.x), 
				.height = static_cast<uint32_t>(windowSize.y) 
			} 
		};
		vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdSetScissor(cb, 0, 1, &scissor);
		vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSetTex, 0, nullptr);
		auto vOffset = VkDeviceSize{ 0 };
		vkCmdBindVertexBuffers(cb, 0, 1, &vBuffer, &vOffset);
		vkCmdBindIndexBuffer(cb, vBuffer, vBufSize, VK_INDEX_TYPE_UINT16);
		vkCmdPushConstants(
			cb, 
			pipelineLayout, 
			VK_SHADER_STAGE_VERTEX_BIT, 
			0, 
			sizeof(VkDeviceAddress), 
			&shaderDataBuffers[frameIndex].deviceAddress
		);
		vkCmdDrawIndexed(cb, static_cast<uint32_t>(indexCount), 3, 0, 0, 0);
		vkCmdEndRendering(cb);
		auto barrierPresent = VkImageMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.image = swapchainImages[imageIndex],
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
				.levelCount = 1, 
				.layerCount = 1 
			}
		};
		auto barrierPresentDependencyInfo = VkDependencyInfo{ 
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, 
			.imageMemoryBarrierCount = 1, 
			.pImageMemoryBarriers = &barrierPresent 
		};
		vkCmdPipelineBarrier2(cb, &barrierPresentDependencyInfo);
		chk(vkEndCommandBuffer(cb));


		// Submit to graphics queue
		auto waitStages = static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
		auto submitInfo = VkSubmitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &presentSemaphores[frameIndex],
			.pWaitDstStageMask = &waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &cb,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &renderSemaphores[imageIndex],
		};
		chk(vkQueueSubmit(queue, 1, &submitInfo, fences[frameIndex]));
		frameIndex = (frameIndex + 1) % maxFramesInFlight;
		auto presentInfo = VkPresentInfoKHR{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &renderSemaphores[imageIndex],
			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &imageIndex
		};
		chkSwapchain(vkQueuePresentKHR(queue, &presentInfo));


		// Event polling
		auto elapsedTime = float{ (SDL_GetTicks() - lastTime) / 1000.0f };
		lastTime = SDL_GetTicks();
		for (SDL_Event event; SDL_PollEvent(&event);) 
		{
			if (event.type == SDL_EVENT_QUIT) 
			{
				quit = true;
				break;
			}
			if (event.type == SDL_EVENT_MOUSE_MOTION and event.button.button == SDL_BUTTON_LEFT)
			{
				objectRotations[shaderData.selected].x -= (float)event.motion.yrel * elapsedTime;
				objectRotations[shaderData.selected].y += (float)event.motion.xrel * elapsedTime;
			}
			if (event.type == SDL_EVENT_MOUSE_WHEEL)
			{
				camPos.z += (float)event.wheel.y * elapsedTime * 10.0f;
			}
			if (event.type == SDL_EVENT_KEY_DOWN) 
			{
				if (event.key.key == SDLK_PLUS or event.key.key == SDLK_KP_PLUS)
					shaderData.selected = (shaderData.selected < 2) ? shaderData.selected + 1 : 0;
				if (event.key.key == SDLK_MINUS or event.key.key == SDLK_KP_MINUS)
					shaderData.selected = (shaderData.selected > 0) ? shaderData.selected - 1 : 2;
			}


			// Window resize
			if (event.type == SDL_EVENT_WINDOW_RESIZED)
			{
				updateSwapchain = true;
			}
		}

		if (updateSwapchain) 
		{
			chk(SDL_GetWindowSize(window, &windowSize.x, &windowSize.y));
			updateSwapchain = false;
			chk(vkDeviceWaitIdle(device));
			chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[deviceIndex], surface, &surfaceCaps));
			swapchainCI.oldSwapchain = swapchain;
			swapchainCI.imageExtent = VkExtent2D{ 
				.width = static_cast<uint32_t>(windowSize.x), 
				.height = static_cast<uint32_t>(windowSize.y) 
			};
			chk(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));
			for (auto i = std::uint32_t{}; i < imageCount; i++) 
			{
				vkDestroyImageView(device, swapchainImageViews[i], nullptr);
			}
			chk(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
			swapchainImages.resize(imageCount);
			chk(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()));
			swapchainImageViews.resize(imageCount);
			for (auto i = std::uint32_t{}; i < imageCount; i++) 
			{
				auto viewCI = VkImageViewCreateInfo{ 
					.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
					.image = swapchainImages[i], 
					.viewType = VK_IMAGE_VIEW_TYPE_2D, 
					.format = imageFormat, 
					.subresourceRange = VkImageSubresourceRange{
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
						.levelCount = 1, 
						.layerCount = 1
					} 
				};
				chk(vkCreateImageView(device, &viewCI, nullptr, &swapchainImageViews[i]));
			}
			for (auto& semaphore : renderSemaphores) 
			{
				vkDestroySemaphore(device, semaphore, nullptr);
			}
			renderSemaphores.resize(imageCount);
			for (auto& semaphore : renderSemaphores) 
			{
				chk(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
			}
			vkDestroySwapchainKHR(device, swapchainCI.oldSwapchain, nullptr);
			vmaDestroyImage(allocator, depthImage, depthImageAllocation);
			vkDestroyImageView(device, depthImageView, nullptr);
			depthImageCI.extent = VkExtent3D{ 
				.width = static_cast<uint32_t>(windowSize.x), 
				.height = static_cast<uint32_t>(windowSize.y), 
				.depth = 1 
			};
			auto allocCI = VmaAllocationCreateInfo{ 
				.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, 
				.usage = VMA_MEMORY_USAGE_AUTO 
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
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
				.image = depthImage, 
				.viewType = VK_IMAGE_VIEW_TYPE_2D, 
				.format = depthFormat, 
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, 
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