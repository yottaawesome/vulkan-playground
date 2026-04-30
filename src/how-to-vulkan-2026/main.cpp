import std;
import vulkan26;

struct ShaderDataBuffer
{
	vma::VmaAllocation Allocation{ nullptr };
	vma::VmaAllocationInfo AllocationInfo{};
	vk::VkBuffer Buffer{ nullptr };
	vk::VkDeviceAddress DeviceAddress{};

	auto Destroy(const vma::Allocator& allocator, const vk::Device& device) noexcept -> void
	{
		if (Buffer)
			vma::vmaDestroyBuffer(allocator.Get(), Buffer, Allocation);
	}
};

struct Texture 
{
	vma::VmaAllocation allocation{ nullptr };
	vk::VkImage image{ nullptr };
	vk::VkImageView view{ nullptr };
	vk::VkSampler sampler{ nullptr };
};

struct ShaderData
{
	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model[3];
	glm::vec4 LightPos{ 0.0f, -10.0f, 10.0f, 0.0f };
	std::uint32_t Selected{ 1 };
} shaderData;

// https://howtovulkan.com/
// https://github.com/SaschaWillems/HowToVulkan/blob/main/source/main.cpp
auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
try
{
	// 
	// 
	// Initialisation of SDL, and initial load of Volk (stage 1).
	auto init = sdl3::Init{ sdl3::InitFlags::Video };
	[] static -> void
	{
		if (not sdl3::vk::SDL_Vulkan_LoadLibrary(nullptr))
			throw sdl3::Error::Error{};
		auto result = vk::Result{ volk::volkInitialize() };
		if (not result)
			throw vk::Error{ result };
	}();

	//
	//
	// Instance and physical device selection.
	// Create the Vulkan instance.
	auto instance =
		[] static->vk::Instance
		{
			// Get SDL to tell us the required instance extensions for Vulkan.
			auto instanceExtensions =
				[] static->std::span<const char* const>
			{
				auto instanceExtensionsCount = std::uint32_t{};
				return { sdl3::vk::SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount), instanceExtensionsCount };
			}();
			// Start with the application info, to describe our application
			// to Vulkan. This can help with driver optimisations for 
			// popular games.
			auto appInfo =
				vk::VkApplicationInfo
			{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName = "Vulkan-2026",
				.applicationVersion = vk::MakeVersion(1, 0, 0),
				.pEngineName = "DorkEngine2000",
				.engineVersion = vk::MakeVersion(1, 0, 0),
				.apiVersion = vk::ApiVersion::V1_4
			};
			auto createInfo =
				vk::VkInstanceCreateInfo
			{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pApplicationInfo = &appInfo,
				.enabledExtensionCount = static_cast<std::uint32_t>(instanceExtensions.size()),
				.ppEnabledExtensionNames = instanceExtensions.data()
			};

			auto instance = vk::VkInstance{};
			auto status = vk::Result{ vk::vkCreateInstance(&createInfo, nullptr, &instance) };
			if (not status)
				throw vk::Error{ status };
			volk::volkLoadInstance(instance);
			return vk::Instance{ vk::InstanceUniquePtr{ instance} };
		}();

	//
	// Get a list of the physical devices.
	auto physicalDevices =
		[&instance] -> std::vector<vk::VkPhysicalDevice>
		{
			auto physicalDeviceCount = std::uint32_t{};
			auto status = vk::Result{ vk::vkEnumeratePhysicalDevices(instance.Get(), &physicalDeviceCount, nullptr) };
			if (not status)
				throw vk::Error{ status };
			auto physicalDevices = std::vector<vk::VkPhysicalDevice>{ physicalDeviceCount };
			status = vk::Result{ vk::vkEnumeratePhysicalDevices(instance.Get(), &physicalDeviceCount, physicalDevices.data()) };
			if (not status)
				throw vk::Error{ status };
			return physicalDevices;
		}();

	//
	// Pick the first discrete device.
	auto pickedDevice =
		[&physicalDevices] -> vk::PhysicalDevice
		{
			for (const vk::VkPhysicalDevice& device : physicalDevices)
			{
				auto props = vk::VkPhysicalDeviceProperties2{ .sType = vk::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
				vk::vkGetPhysicalDeviceProperties2(device, &props);
				std::println("Found device: {}", props.properties.deviceName);
				if (props.properties.deviceType & vk::VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					return device;
			}
			throw std::runtime_error("No Vulkan-compatible discrete GPUs found.");
		}();

	//
	//
	// Logical device and queues.
	// Queues. On most devices, the first queue family will support graphics, 
	// compute and transfer, but this is not guaranteed.
	auto suitableQueueFamilyIndex =
		[&pickedDevice] -> std::uint32_t
		{
			auto count = std::uint32_t{};
			vk::vkGetPhysicalDeviceQueueFamilyProperties2(pickedDevice.Get(), &count, nullptr);

			auto queueFamilies = std::vector<vk::VkQueueFamilyProperties2>{
				count,
				vk::VkQueueFamilyProperties2{.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 }
			};
			vk::vkGetPhysicalDeviceQueueFamilyProperties2(pickedDevice.Get(), &count, queueFamilies.data());

			for (auto [index, element] : queueFamilies | std::views::enumerate)
			{
				auto queueFlags = element.queueFamilyProperties.queueFlags;
				if (not (queueFlags & vk::VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT))
					continue;
				if (not (queueFlags & vk::VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT))
					continue;
				return static_cast<std::uint32_t>(index);
			}
			throw std::runtime_error("No suitable queue family found.");
		}();

	//
	// 
	// Check if the queue family supports presentation. This depends on the windowing system, 
	// and the physical device so we have to ask SDL to check for us.
	if (not sdl3::vk::SDL_Vulkan_GetPresentationSupport(instance.Get(), pickedDevice.Get(), suitableQueueFamilyIndex))
		throw std::runtime_error("The selected queue family does not support presentation.");

	//
	//
	// Creation of the logical device
	auto device =
		[](auto suitableQueueFamilyIndex, const vk::PhysicalDevice& pickedDevice) -> vk::Device
		{
			constexpr auto qfpriorities = 1.0f;
			auto queueCI = vk::VkDeviceQueueCreateInfo{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = suitableQueueFamilyIndex,
				.queueCount = 1,
				.pQueuePriorities = &qfpriorities
			};
			auto enabledVk12Features = vk::VkPhysicalDeviceVulkan12Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
				.descriptorIndexing = true,
				.shaderSampledImageArrayNonUniformIndexing = true,
				.descriptorBindingVariableDescriptorCount = true,
				.runtimeDescriptorArray = true,
				.bufferDeviceAddress = true
			};
			auto enabledVk13Features = vk::VkPhysicalDeviceVulkan13Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
				.pNext = &enabledVk12Features,
				.synchronization2 = true,
				.dynamicRendering = true,
			};
			const auto enabledVk14Features = vk::VkPhysicalDeviceVulkan14Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
				.pNext = &enabledVk13Features,
			};
			const auto enabledVk10Features = vk::VkPhysicalDeviceFeatures{
				.samplerAnisotropy = true
			};
			const auto deviceExtensions = std::vector<const char*>{ vk::DeviceExtension::Swapchain };
			auto deviceCI = vk::VkDeviceCreateInfo{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pNext = &enabledVk14Features,
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &queueCI,
				.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
				.ppEnabledExtensionNames = deviceExtensions.data(),
				.pEnabledFeatures = &enabledVk10Features
			};
			auto device = vk::VkDevice{};
			auto result = vk::Result{ vk::vkCreateDevice(pickedDevice.Get(), &deviceCI, nullptr, &device) };
			if (not result)
				throw vk::Error{ result };
			volk::LoadDevice(device);
			return vk::Device{ device };
		}(suitableQueueFamilyIndex, pickedDevice);

	// Get the queue handle from the device.
	auto queue = device.GetQueue(suitableQueueFamilyIndex, 0);

	// Create the VMA allocator. We need to provide it with the Vulkan 
	// function pointers, so that it can call Vulkan functions internally.
	auto allocator =
		[&pickedDevice, &device, &instance] -> vma::Allocator
		{
			auto vkFunctions = vma::VmaVulkanFunctions{
				.vkGetInstanceProcAddr = vk::vkGetInstanceProcAddr,
				.vkGetDeviceProcAddr = vk::vkGetDeviceProcAddr
			};
			auto allocatorCI = vma::VmaAllocatorCreateInfo{
				.flags = vma::VmaAllocatorCreateFlagBits::VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
				.physicalDevice = pickedDevice.Get(),
				.device = device.Get(),
				.pVulkanFunctions = &vkFunctions,
				.instance = instance.Get()
			};
			auto allocator = vma::VmaAllocator{};
			auto result = vk::Result{ vma::vmaCreateAllocator(&allocatorCI, &allocator) };
			if (not result)
				throw vk::Error{ result };
			return allocator;
		}();

	constexpr auto width = 1280u;
	constexpr auto height = 720u;
	auto window = sdl3::Window{ "Vulkan-2026", width, height, sdl3::WindowFlags::Vulkan };
	auto surface = vk::Surface{ vk::Surface::Create(instance.Get(), pickedDevice.Get(), window.CreateSurface(instance.Get())) };
	auto surfaceCaps = vk::VkSurfaceCapabilitiesKHR{};
	vk::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pickedDevice.Get(), surface.Get(), &surfaceCaps);

	//
	//
	// Swapchain creation.
	auto windowSize = glm::vec2{ window.GetWindowSize() };
	auto swapchainExtent =
		[&surfaceCaps, &windowSize] -> vk::VkExtent2D
		{
			if (surfaceCaps.currentExtent.width != 0xFFFFFFFF)
				return { surfaceCaps.currentExtent };
			return {
				.width = static_cast<std::uint32_t>(windowSize.x),
				.height = static_cast<std::uint32_t>(windowSize.y)
			};
		}();

	constexpr auto imageFormat = vk::VkFormat::VK_FORMAT_B8G8R8A8_SRGB;

	auto swapchain =
		[&device, &surface, &surfaceCaps, &swapchainExtent, &imageFormat] -> vk::Swapchain
		{
			auto swapchainCreateInfo = vk::VkSwapchainCreateInfoKHR{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.surface = surface.Get(),
				.minImageCount = surfaceCaps.minImageCount,
				.imageFormat = imageFormat,
				.imageColorSpace = vk::VkColorSpaceKHR::VK_COLORSPACE_SRGB_NONLINEAR_KHR,
				.imageExtent = {
					.width = swapchainExtent.width,
					.height = swapchainExtent.height
				},
				.imageArrayLayers = 1,
				.imageUsage = vk::VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				.preTransform = vk::VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
				.compositeAlpha = vk::VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				.presentMode = vk::VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR
			};
			auto swapchain = vk::VkSwapchainKHR{};
			auto result = vk::Result{ vk::vkCreateSwapchainKHR(device.Get(), &swapchainCreateInfo, nullptr, &swapchain) };
			if (not result)
				throw vk::Error{ result };
			return vk::Swapchain{ vk::SwapchainUniquePtr{ swapchain, vk::SwapchainDeleter{device.Get()} } };
		}();

	auto swapchainImages = std::vector<vk::VkImage>{ swapchain.GetSwapchainImages() };

	//
	//
	// Select a depth format. We need to find a format that supports being used as a depth-stencil attachment, and that is supported by the device.
	auto depthFormat =
		[&pickedDevice] -> vk::VkFormat
		{
			constexpr auto candidates = std::array{ vk::VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT, vk::VkFormat::VK_FORMAT_D24_UNORM_S8_UINT };
			const auto formatSupportsDepthAttachment =
				[&pickedDevice](vk::VkFormat candidate) -> bool
				{
					auto formatProps = vk::VkFormatProperties{};
					vk::vkGetPhysicalDeviceFormatProperties(pickedDevice.Get(), candidate, &formatProps);
					return (formatProps.optimalTilingFeatures & vk::VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
				};
			auto filter = candidates | std::ranges::views::filter(formatSupportsDepthAttachment);
			return filter.empty()
				? throw Error::RuntimeError{ "No suitable depth format found." }
			: filter.front();
		}();

	auto depthImage =
		[depthFormat, &allocator, &windowSize, &device] -> vk::DepthImage<vma::VmaImage>
		{
			auto depthImageCi = vk::VkImageCreateInfo{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType = vk::VkImageType::VK_IMAGE_TYPE_2D,
				.format = depthFormat,
				.extent{ 
					.width = static_cast<std::uint32_t>(windowSize.x), 
					.height = static_cast<std::uint32_t>(windowSize.y), 
					.depth = 1 
				},
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = vk::VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
				.tiling = vk::VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
				.usage = vk::VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				.initialLayout = vk::VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
			};
			auto allocCi = vma::VmaAllocationCreateInfo{
				.flags = vma::VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
				.usage = vma::VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO,
			};

			auto allocation = vma::VmaAllocation{};
			auto image = vk::VkImage{};
			auto result = vk::Result{
				vma::vmaCreateImage(
					allocator.Get(),
					&depthImageCi,
					&allocCi,
					&image,
					&allocation,
					nullptr
				) };
			if (not result)
				throw vk::Error{ result };
			auto imageUniquePtr = vma::VmaImageUniquePtr{ image, vma::VmaImageDeleter{allocator.Get(), allocation} };

			auto depthViewCi = vk::VkImageViewCreateInfo{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = image,
				.viewType = vk::VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
				.format = depthFormat,
				.subresourceRange{
					.aspectMask = vk::VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT,
					.levelCount = 1,
					.layerCount = 1
				}
			};
			auto depthImageView = vk::VkImageView{};
			result = vk::Result{
				vk::vkCreateImageView(
					device.Get(),
					&depthViewCi,
					nullptr,
					&depthImageView
				) };
			if (not result)
				throw vk::Error{ result };

			return {
				vma::VmaImage{ std::move(imageUniquePtr) },
				vk::ImageView{ vk::ImageViewUniquePtr{ depthImageView, vk::ImageViewDeleter{device.Get()} } }
			};
		}();

	//
	//
	// Loading a mesh
	auto meshData =
		[] static->Mesh::MeshData
		{
			auto tinyobjData = tinyobj::FileData{ tinyobj::FileData::From("assets\\suzanne.obj") };
			auto indexCount = std::uint64_t{ tinyobjData.Shapes[0].mesh.indices.size() };
			auto vertices = std::vector<Mesh::Vertex>{};
			auto indices = std::vector<std::uint16_t>{};
			// Load vertex and index data
			// The value of the position's and normal's y-axis, and the texture coordinate's v-axis are flipped. This is done to accommodate for Vulkan's coordinate system. Otherwise the model and the texture image would appear upside down.
			for (auto& index : tinyobjData.Shapes[0].mesh.indices)
			{
				auto v = Mesh::Vertex{
					.Pos = { tinyobjData.Attrib.vertices[index.vertex_index * 3], -tinyobjData.Attrib.vertices[index.vertex_index * 3 + 1], tinyobjData.Attrib.vertices[index.vertex_index * 3 + 2] },
					.Normal = { tinyobjData.Attrib.normals[index.normal_index * 3], -tinyobjData.Attrib.normals[index.normal_index * 3 + 1], tinyobjData.Attrib.normals[index.normal_index * 3 + 2] },
					.Uv = { tinyobjData.Attrib.texcoords[index.texcoord_index * 2], 1.0 - tinyobjData.Attrib.texcoords[index.texcoord_index * 2 + 1] }
				};
				vertices.push_back(v);
				indices.push_back(static_cast<std::uint16_t>(indices.size()));
			}
			return { vertices, indices };
		}();

	auto vertexIndexBuffer =
		[&meshData, &allocator, &device] -> vma::VmaBuffer
		{
			auto vBufSize = vk::VkDeviceSize{ sizeof(Mesh::Vertex) * meshData.Vertices.size() };
			auto iBufSize = vk::VkDeviceSize{ sizeof(std::uint16_t) * meshData.Indices.size() };
			// Note that the buffer combines vertex and index data.
			auto bufferCI = vk::VkBufferCreateInfo{
				.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size = vBufSize + iBufSize,
				.usage = vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT
			};
			auto vBufferAllocCI = vma::VmaAllocationCreateInfo{
				.flags =
					vma::VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
					| vma::VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
					| vma::VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT,
				.usage = vma::VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO
			};
			auto vBufferAllocInfo = vma::VmaAllocationInfo{};
			auto vBuffer = vk::VkBuffer{};
			auto vBufferAllocation = vma::VmaAllocation{};
			auto result = vk::Result{ vma::vmaCreateBuffer(allocator.Get(), &bufferCI, &vBufferAllocCI, &vBuffer, &vBufferAllocation, &vBufferAllocInfo) };
			if (not result)
				throw vk::Error{ result };

			std::memcpy(vBufferAllocInfo.pMappedData, meshData.Vertices.data(), vBufSize);
			std::memcpy(((char*)vBufferAllocInfo.pMappedData) + vBufSize, meshData.Indices.data(), iBufSize);
			return { vma::VmaBufferUniquePtr{ vBuffer, vma::VmaBufferDeleter{allocator.Get(), vBufferAllocation} } };
		}();

	constexpr auto MaxFramesInFlight = std::uint32_t{2};
	auto shaderDataBuffers = std::array<ShaderDataBuffer, MaxFramesInFlight>{};
	for (auto i = 0; i < MaxFramesInFlight; i++) 
	{
		auto uBufferCI = vk::VkBufferCreateInfo{
			.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = sizeof(ShaderData),
			.usage = vk::VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
		};
		auto uBufferAllocCI = vma::VmaAllocationCreateInfo{
			.flags = 
				vma::VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
				| vma::VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
				| vma::VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.usage = vma::VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO
		};
		auto result = vk::Result{ 
			vma::vmaCreateBuffer(
				allocator.Get(),
				&uBufferCI,
				&uBufferAllocCI,
				&shaderDataBuffers[i].Buffer,
				&shaderDataBuffers[i].Allocation,
				&shaderDataBuffers[i].AllocationInfo
			)};
		if (not result)
			throw vk::Error{ result };
	}

	// Sync objects
	auto fences = std::array<Vulkan26::Fence, MaxFramesInFlight>{
		Vulkan26::Fence{ Vulkan26::CreateFenceUniquePtr(device.Get(), vk::VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT) },
		Vulkan26::Fence{ Vulkan26::CreateFenceUniquePtr(device.Get(), vk::VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT) }
	};
	auto semaphores = std::array<Vulkan26::Semaphore, MaxFramesInFlight>{
		Vulkan26::Semaphore{ Vulkan26::CreateSemaphoreUniquePtr(device.Get(), 0)},
		Vulkan26::Semaphore{ Vulkan26::CreateSemaphoreUniquePtr(device.Get(), 0)}
	};
	auto renderSemaphores = std::vector<Vulkan26::Semaphore>{};
	for (auto index = 0; index < swapchainImages.size(); index++)
		renderSemaphores.push_back(Vulkan26::Semaphore{ Vulkan26::CreateSemaphoreUniquePtr(device.Get(), 0) });



	// Command pool
	auto commandPool = vk::CommandPool{
		device.Get(), 
		vk::VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 
		0
	};
	auto commandBuffers = 
		commandPool.CreateArray<MaxFramesInFlight>(vk::VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	
	// Texture images
	auto textures = std::array<Texture, 3>{};
	for (auto i = 0; i < textures.size(); i++)
	{
		auto ktxTexture = static_cast<ktx::ktxTexture*>(nullptr);
		auto filename = std::format("assets/suzanne{}.ktx", i);
	}


	// Cleanup buffers
	for(ShaderDataBuffer& buffer : shaderDataBuffers)
	{
		buffer.Destroy(allocator, device);
	}

	return 0;
}
catch (const std::exception& e)
{
	std::println("Error: {}", e.what());
	return 1;
}
