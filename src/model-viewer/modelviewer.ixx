export module modelviewer;
import std;
import volkus;

export namespace ModelViewer
{
	struct InstanceFactory : Volkus::vkx::InstanceFactory
	{
		auto GetLayers(this auto&& self) noexcept -> std::ranges::range auto
		{
			return std::array{ Vk::Layer::KhronosValidation };
		}

		auto GetExtensions(this auto&& self) noexcept -> std::ranges::range auto
		{
			return std::array{
				Vk::InstanceExtension::DebugUtils,
				Vk::InstanceExtension::PlatformSurface
			};
		}

		auto GetFlags(this auto&& self) noexcept
		{
			return VkInstanceCreateFlags{ 0 };
		}

		auto GetApplicationInfo(this auto&& self) noexcept
		{
			return VkApplicationInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pNext = nullptr,
				.pApplicationName = "Model Viewer",
				.applicationVersion = Vk::MakeVersion(1, 0, 0),
				.pEngineName = "No Engine",
				.engineVersion = Vk::MakeVersion(1, 0, 0),
				.apiVersion = Vk::ApiVersion::V1_4
			};
		}
	};

	struct DebugMessengerFactory : Volkus::vkx::DebugMessengerFactory
	{
		consteval auto GetSeverity(this auto&& self) noexcept -> VkDebugUtilsMessageSeverityFlagsEXT
		{
			return
				VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
				| VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
				| VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		}

		consteval auto GetMessageType(this auto&& self) noexcept -> VkDebugUtilsMessageTypeFlagsEXT
		{
			return
				VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
				| VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
				| VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		}

		consteval auto GetCallback(this auto&& self) noexcept -> Volkus::vkx::DebugCallbackSignature
		{
			return
				[](
					VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
					VkDebugUtilsMessageTypeFlagsEXT messageType,
					const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
					void* pUserData
				) -> VkBool32
				{
					if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
						std::println("Validation layer: {}", pCallbackData->pMessage);
					return VkFalse;
				};
		}
	};

	auto SelectPhysicalDevice(Volkus::vkx::Instance& instance) -> Volkus::vkx::PhysicalDevice
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
	}

	auto FindQueueIndex(Volkus::vkx::PhysicalDevice& physicalDevice) -> std::uint32_t
	{
		auto queueIndex = physicalDevice.FindQueueIndex(
			VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT | VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT | VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT
		);
		if (not queueIndex)
			throw std::runtime_error{ "No suitable queue found" };
		return *queueIndex;
	}

	auto CreateDevice(std::uint32_t queueIndex, Volkus::vkx::PhysicalDevice& physicalDevice) -> Volkus::vkx::Device
	{
		auto enabledVk14Features = 
			VkPhysicalDeviceVulkan14Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
			};
		auto enabledVk13Features =
			VkPhysicalDeviceVulkan13Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
				.pNext = &enabledVk14Features,
				.synchronization2 = true,
				.dynamicRendering = true,
		};
		auto enabledVk12Features =
			VkPhysicalDeviceVulkan12Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
				.pNext = &enabledVk13Features,
				.descriptorIndexing = true,
				.shaderSampledImageArrayNonUniformIndexing = true,
				.descriptorBindingVariableDescriptorCount = true,
				.runtimeDescriptorArray = true,
				.bufferDeviceAddress = true
		};
		auto enabledVk11Features =
			VkPhysicalDeviceVulkan11Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
				.pNext = &enabledVk12Features,
		};
		auto enabledVk10Features =
			VkPhysicalDeviceFeatures2{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
				.pNext = &enabledVk11Features,
				.features = {
					.samplerAnisotropy = true
				}
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

		auto deviceExtensions = std::array{ Vk::DeviceExtension::Swapchain };
		auto deviceCreateInfo =
			VkDeviceCreateInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pNext = &enabledVk10Features,
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &queueCreateInfo,
				.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
				.ppEnabledExtensionNames = deviceExtensions.data(),
				.pEnabledFeatures = nullptr // mutually exclusive with pNext chain of feature structs
		};
		return { Volkus::vkx::CreateDeviceUniquePtr(physicalDevice.Get(), deviceCreateInfo, true) };
	}
}
