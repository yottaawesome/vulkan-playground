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
	if (not sdl3::Vk::SDL_Vulkan_LoadLibrary(nullptr))
		throw sdl3::Error::Error{};
	auto result = Vk::Result{ volk::volkInitialize() };
	if (not result)
		throw Vk::Error{ result.result };

	// Create the Vulkan instance.
	auto instance = 
		[] static -> Vk::Instance
		{
			// Get SDL to tell us the required instance extensions for Vulkan.
			auto instanceExtensions = 
				[] static->std::span<const char* const>
				{
					auto instanceExtensionsCount = std::uint32_t{};
					return { sdl3::Vk::SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount), instanceExtensionsCount };
				}();
			// Start with the application info, to describe our application
			// to Vulkan. This can help with driver optimisations for 
			// popular games.
			auto appInfo = 
				Vk::VkApplicationInfo
				{
					.sType = Vk::VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
					.pApplicationName = "Vulkan-2026",
					.applicationVersion = Vk::MakeVersion(1, 0, 0),
					.pEngineName = "DorkEngine2000",
					.engineVersion = Vk::MakeVersion(1, 0, 0),
					.apiVersion = Vk::ApiVersion::V1_4
				};
			auto createInfo = 
				Vk::VkInstanceCreateInfo
				{
					.sType = Vk::VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
					.pApplicationInfo = &appInfo,
					.enabledExtensionCount = static_cast<std::uint32_t>(instanceExtensions.size()),
					.ppEnabledExtensionNames = instanceExtensions.data()
				};

			auto instance = Vk::VkInstance{};
			auto status = Vk::Result{ Vk::vkCreateInstance(&createInfo, nullptr, &instance) };
			if (not status)
				throw Vk::Error{ status.result };	
			return instance;
		}();
	volk::volkLoadInstance(instance.Get());

	// Get a list of the physical devices.
	auto physicalDevices =
		[](auto& instance) static -> std::vector<Vk::VkPhysicalDevice>
		{
			auto physicalDeviceCount = std::uint32_t{};
			auto status = Vk::Result{ Vk::vkEnumeratePhysicalDevices(instance.Get(), &physicalDeviceCount, nullptr) };
			if (not status)
				throw Vk::Error{ status.result };
			auto physicalDevices = std::vector<Vk::VkPhysicalDevice>{ physicalDeviceCount };
			status = Vk::Result{ Vk::vkEnumeratePhysicalDevices(instance.Get(), &physicalDeviceCount, physicalDevices.data()) };
			if (not status)
				throw Vk::Error{ status.result };
			return physicalDevices;
		}(instance);

	// Pick the first discrete device.
	auto pickedDevice = 
		[](auto& physicalDevices) static->Vk::PhysicalDevice
		{
			for (const auto& device : physicalDevices)
			{
				auto props = Vk::VkPhysicalDeviceProperties2{.sType = Vk::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
				Vk::vkGetPhysicalDeviceProperties2(device, &props);
				std::println("Found device: {}", props.properties.deviceName);
				if (props.properties.deviceType & Vk::VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					return device;
			}
			throw std::runtime_error("No Vulkan-compatible discrete GPUs found.");
		}(physicalDevices);

	//
	//
	//
	//
	// Logical device and queues.
	
	//
	// Queues. On most devices, the first queue family will support graphics, 
	// compute and transfer, but this is not guaranteed.
	auto suitableQueueFamilyIndex = 
		[](const Vk::PhysicalDevice& pickedDevice) static->std::uint32_t
		{
			auto count = std::uint32_t{};
			Vk::vkGetPhysicalDeviceQueueFamilyProperties2(pickedDevice.Get(), &count, nullptr);

			auto queueFamilies = std::vector<Vk::VkQueueFamilyProperties2>{ 
				count, 
				Vk::VkQueueFamilyProperties2{.sType = Vk::VkStructureType::VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 } 
			};
			Vk::vkGetPhysicalDeviceQueueFamilyProperties2(pickedDevice.Get(), &count, queueFamilies.data());
		
			for (auto [index, element] : queueFamilies | std::views::enumerate) 
			{
				auto queueFlags = element.queueFamilyProperties.queueFlags;
				if (not (queueFlags & Vk::VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT))
					continue;
				if (not (queueFlags & Vk::VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT))
					continue;
				return static_cast<std::uint32_t>(index);
			}
			throw std::runtime_error("No suitable queue family found.");
		}(pickedDevice);

	//
	// 
	// Check if the queue family supports presentation. This depends on the windowing system, 
	// and the physical device so we have to ask SDL to check for us.
	if (not sdl3::Vk::SDL_Vulkan_GetPresentationSupport(instance.Get(), pickedDevice.Get(), suitableQueueFamilyIndex))
		throw std::runtime_error("The selected queue family does not support presentation.");

	//
	//
	// Creation of the logical device and retrieval of the queue.
	auto [device, queue] = 
		[](
			auto suitableQueueFamilyIndex, 
			const Vk::PhysicalDevice& pickedDevice
		) -> std::pair<Vk::VkDevice, Vk::VkQueue>
		{
			constexpr auto qfpriorities = 1.0f;
			auto queueCI = Vk::VkDeviceQueueCreateInfo{
				.sType = Vk::VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = suitableQueueFamilyIndex,
				.queueCount = 1,
				.pQueuePriorities = &qfpriorities
			};
			auto enabledVk12Features = Vk::VkPhysicalDeviceVulkan12Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
				.descriptorIndexing = true,
				.shaderSampledImageArrayNonUniformIndexing = true,
				.descriptorBindingVariableDescriptorCount = true,
				.runtimeDescriptorArray = true,
				.bufferDeviceAddress = true
			};
			const auto enabledVk13Features = Vk::VkPhysicalDeviceVulkan13Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
				.pNext = &enabledVk12Features,
				.synchronization2 = true,
				.dynamicRendering = true,
			};
			const auto enabledVk10Features = Vk::VkPhysicalDeviceFeatures{
				.samplerAnisotropy = true
			};
			const auto  deviceExtensions = std::vector<const char*>{ Vk::DeviceExtension::Swapchain };
			Vk::VkDeviceCreateInfo deviceCI{
				.sType = Vk::VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pNext = &enabledVk13Features,
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &queueCI,
				.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
				.ppEnabledExtensionNames = deviceExtensions.data(),
				.pEnabledFeatures = &enabledVk10Features
			};
			auto device = Vk::VkDevice{};
			auto result = Vk::Result{Vk::vkCreateDevice(pickedDevice.Get(), &deviceCI, nullptr, &device)};
			if (not result)
				throw Vk::Error{ result.result };

			auto queue = Vk::VkQueue{};
			Vk::vkGetDeviceQueue(device, suitableQueueFamilyIndex, 0, &queue);
			return {device, queue};
		}(suitableQueueFamilyIndex, pickedDevice);

	return 0;
}
catch(const std::exception& e)
{
	std::println("Error: {}", e.what());
	return 1;
}
