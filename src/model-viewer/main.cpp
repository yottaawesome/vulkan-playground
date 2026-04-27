import volkus;
import modelviewer;

auto wWinMain(
    Volkus::Win32::HINSTANCE, Volkus::Win32::HINSTANCE, Volkus::Win32::LPWSTR, int
) -> int
{
	Volkus::Win32::Crt::SetAbortBehavior(
        Volkus::Win32::Crt::CallReportFault,
		Volkus::Win32::Crt::CallReportFault | Volkus::Win32::Crt::WriteAbortMsg
    );

	auto instance = ModelViewer::VulkanInstance{};

    auto debugMessenger = 
        [&instance] -> Volkus::vkx::DebugMessenger
        {
            auto debugInfo =
                VkDebugUtilsMessengerCreateInfoEXT{
                    .sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                    .pNext = nullptr,
                    .flags = 0,
                    .messageSeverity =
                        VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                        | VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                        | VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                    .messageType =
                        VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                        | VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                        | VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                    .pfnUserCallback =
                        [](
                            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                            void* pUserData
                        ) -> VkBool32
                        {
                            if(messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
								std::println("Validation layer: {}", pCallbackData->pMessage);
                            return VkFalse;
                        },
                    .pUserData = nullptr
                };
            return Volkus::vkx::DebugMessenger::CreateDebugMessenger(instance.Get(), &debugInfo, nullptr);
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

    auto device = 
        [queueIndex, &physicalDevice]
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

            auto deviceExtensions = std::array{Vk::DeviceExtension::Swapchain};
            auto deviceCreateInfo = 
                VkDeviceCreateInfo{
                    .sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .pNext = &enabledVk14Features,
                    .queueCreateInfoCount = 1,
                    .pQueueCreateInfos = &queueCreateInfo,
                    .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
                    .ppEnabledExtensionNames = deviceExtensions.data(),
                    .pEnabledFeatures = &enabledVk10Features
                };
		    return Volkus::vkx::Device::Create(physicalDevice.Get(), deviceCreateInfo, true);
        }();

    return 0;
}
