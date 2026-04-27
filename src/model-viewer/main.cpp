import volkus;

auto wWinMain(
    Volkus::Win32::HINSTANCE, Volkus::Win32::HINSTANCE, Volkus::Win32::LPWSTR, int
) -> int
{
    auto instance = 
		[] -> Volkus::vkx::Instance
        {
			auto layers = std::vector<const char*>{ Vk::Layers::KhronosValidationLayerName };

            auto applicationInfo = VkApplicationInfo{
                .sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = "Model Viewer",
                .applicationVersion = Vk::MakeApiVersion(0, 1, 0),
                .pEngineName = "Volkus Engine",
                .engineVersion = Vk::MakeApiVersion(0, 1, 0),
                .apiVersion = Vk::MakeApiVersion(1, 4, 0)
            };
            auto createinfo = VkInstanceCreateInfo{
                .sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &applicationInfo,
                .enabledLayerCount = static_cast<std::uint32_t>(layers.size()),
                .ppEnabledLayerNames = layers.data(),
                .enabledExtensionCount = 0,
                .ppEnabledExtensionNames = nullptr
            };
            return Volkus::vkx::Instance::Create(createinfo, true);
        }();

	auto physicalDevice = 
        [&instance] -> Volkus::vkx::PhysicalDevice
        {
            auto physicalDevices = instance.EnumeratePhysicalDevices();
            for (VkPhysicalDevice device : physicalDevices)
            {
                auto properties = Volkus::vkx::PhysicalDeviceProperties{ .Device = device };
                auto apiVersion = properties.GetApiVersion();
                if (properties.IsDiscreteGpu() and apiVersion.Major >= 1 and apiVersion.Minor >= 4)
                    return Volkus::vkx::PhysicalDevice{ device };
            }
            throw std::runtime_error{ "No suitable physical device found" };
        }();

	auto queueIndex = 
        [&physicalDevice] -> std::uint32_t
        {
            auto queueIndex = physicalDevice.FindQueueIndex(
                VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT | VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT | VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT
            );
            if (not queueIndex)
                throw std::runtime_error{ "No suitable queue found" };
            return *queueIndex;
		}();

    [queueIndex]
    {
        auto enabledVk12Features = 
            VkPhysicalDeviceVulkan12Features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .descriptorIndexing = true,
                .shaderSampledImageArrayNonUniformIndexing = true,
                .descriptorBindingVariableDescriptorCount = true,
                .runtimeDescriptorArray = true,
                .bufferDeviceAddress = true
            };
        auto enabledVk13Features = 
            VkPhysicalDeviceVulkan13Features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                .pNext = &enabledVk12Features,
                .synchronization2 = true,
                .dynamicRendering = true,
            };
        auto enabledVk14Features = 
            VkPhysicalDeviceVulkan14Features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
                .pNext = &enabledVk13Features,
            };
        auto enabledVk10Features = 
            VkPhysicalDeviceFeatures{
                .samplerAnisotropy = true
            };

        auto queuePriority = 1.0f;
        auto queueCreateInfo = 
            VkDeviceQueueCreateInfo{
                .sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .flags = 0,
                .queueFamilyIndex = queueIndex,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            };
        auto deviceCI = 
            VkDeviceCreateInfo{
                .sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = &enabledVk14Features,
                .queueCreateInfoCount = 1,
                .pQueueCreateInfos = &queueCreateInfo,
                //.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
                //.ppEnabledExtensionNames = deviceExtensions.data(),
                .pEnabledFeatures = &enabledVk10Features
            };
    }();

	
        
    return 0;
}
