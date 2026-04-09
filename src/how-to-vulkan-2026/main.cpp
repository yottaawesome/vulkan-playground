import std;
import vulkan26;

// https://howtovulkan.com/
// https://github.com/SaschaWillems/HowToVulkan/blob/main/source/main.cpp
auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
try
{
	//
	//
	//
	//
	// Instance and physical device selection.
	auto init = sdl3::Init{ sdl3::InitFlags::Video };
	if (not sdl3::vk::SDL_Vulkan_LoadLibrary(nullptr))
		throw sdl3::Error::Error{};
	auto result = vk::Result{ volk::volkInitialize() };
	if (not result)
		throw vk::Error{ result.result };

	// Create the Vulkan instance.
	auto instance = 
		[] static -> vk::Instance
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
				throw vk::Error{ status.result };	
			return instance;
		}();
	volk::volkLoadInstance(instance.Get());

	// Get a list of the physical devices.
	auto physicalDevices =
		[&instance] -> std::vector<vk::VkPhysicalDevice>
		{
			auto physicalDeviceCount = std::uint32_t{};
			auto status = vk::Result{ vk::vkEnumeratePhysicalDevices(instance.Get(), &physicalDeviceCount, nullptr) };
			if (not status)
				throw vk::Error{ status.result };
			auto physicalDevices = std::vector<vk::VkPhysicalDevice>{ physicalDeviceCount };
			status = vk::Result{ vk::vkEnumeratePhysicalDevices(instance.Get(), &physicalDeviceCount, physicalDevices.data()) };
			if (not status)
				throw vk::Error{ status.result };
			return physicalDevices;
		}();

	// Pick the first discrete device.
	auto pickedDevice = 
		[&physicalDevices] -> vk::PhysicalDevice
		{
			for (const auto& device : physicalDevices)
			{
				auto props = vk::VkPhysicalDeviceProperties2{.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
				vk::vkGetPhysicalDeviceProperties2(device, &props);
				std::println("Found device: {}", props.properties.deviceName);
				if (props.properties.deviceType & vk::VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					return device;
			}
			throw std::runtime_error("No Vulkan-compatible discrete GPUs found.");
		}();

	//
	//
	//
	//
	// Logical device and queues.
	
	//
	// Queues. On most devices, the first queue family will support graphics, 
	// compute and transfer, but this is not guaranteed.
	auto suitableQueueFamilyIndex = 
		[](const vk::PhysicalDevice& pickedDevice) static->std::uint32_t
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
		}(pickedDevice);

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
		[](
			auto suitableQueueFamilyIndex,
			const vk::PhysicalDevice& pickedDevice
		) -> vk::Device
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
			auto result = vk::Result{vk::vkCreateDevice(pickedDevice.Get(), &deviceCI, nullptr, &device)};
			if (not result)
				throw vk::Error{ result.result };
			volk::LoadDevice(device);
			return vk::Device{device};
		}(suitableQueueFamilyIndex, pickedDevice);

	// Get the queue handle from the device.
	auto queue = device.GetQueue(suitableQueueFamilyIndex, 0);

	// Create the VMA allocator. We need to provide it with the Vulkan 
	// function pointers, so that it can call Vulkan functions internally.
	auto allocator = 
		[](
			const vk::PhysicalDevice& pickedDevice, 
			const vk::Device& device, 
			const vk::Instance& instance
		) -> vk::vma::Allocator
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
				throw vk::Error{ result.result };
			return allocator;
		}(pickedDevice, device, instance);

	auto window = sdl3::Window{ "Vulkan-2026", 1280u, 720u, sdl3::WindowFlags::Vulkan };
	auto surface = vk::Surface{vk::Surface::Create(instance.Get(), pickedDevice.Get(), window.CreateSurface(instance.Get()))};
	auto surfaceCaps = vk::VkSurfaceCapabilitiesKHR{};
	vk::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pickedDevice.Get(), surface.Get(), &surfaceCaps);

	//
	//
	//
	//
	// Swapchain creation.
	auto windowSize = window.GetWindowSize();
	auto swapchainExtent = vk::VkExtent2D{ surfaceCaps.currentExtent };
	if (surfaceCaps.currentExtent.width == 0xFFFFFFFF) 
	{
		swapchainExtent = { 
			.width = static_cast<std::uint32_t>(windowSize.x), 
			.height = static_cast<std::uint32_t>(windowSize.y)
		};
	}
	constexpr auto imageFormat = vk::VkFormat::VK_FORMAT_B8G8R8A8_SRGB;
	auto swapchainCreateInfo = vk::VkSwapchainCreateInfoKHR{
		// todo fill out
	};
	auto swapchain = vk::VkSwapchainKHR{};
	// todo create the swapchain with vkCreateSwapchainKHR, and check the result for errors.

	return 0;
}
catch(const std::exception& e)
{
	std::println("Error: {}", e.what());
	return 1;
}
