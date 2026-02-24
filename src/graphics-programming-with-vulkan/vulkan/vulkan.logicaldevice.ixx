export module vulkangfx:vulkan.logicaldevice;
import std;
import :raii;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan
{
	using VkDeviceUniquePtr = Raii::IndirectUniquePtr<vkr::VkDevice, vkr::vkDestroyDevice, nullptr>;

	struct LogicalDeviceFactory
	{
		struct CreateInfo
		{
			struct DeviceQueueCreateInfo
			{
				vkr::VkDeviceQueueCreateFlags Flags;
				uint32_t QueueFamilyIndex;
				uint32_t QueueCount;
				std::vector<float> QueuePriorities{1.0f};

				constexpr auto ToVulkanStruct(this const DeviceQueueCreateInfo& self) -> vkr::VkDeviceQueueCreateInfo
				{
					//TODO: there must be a way of enforcing this at compile time.
					if (self.QueueCount != self.QueuePriorities.size())
						throw std::runtime_error("QueueCount does not match the size of QueuePriorities.");

					return vkr::VkDeviceQueueCreateInfo{
						.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
						.pNext = nullptr,
						.flags = self.Flags,
						.queueFamilyIndex = self.QueueFamilyIndex,
						.queueCount = self.QueueCount,
						.pQueuePriorities = self.QueuePriorities.data()
					};
				}
			};

			std::vector<DeviceQueueCreateInfo> QueueCreateInfos;
			std::vector<const char*> EnabledExtensions;
			std::vector<vkr::VkPhysicalDeviceFeatures*> EnabledFeatures;

			// vulkanQueueCreateInfosCache is needed to ensure that the transformed queue create infos remain valid for the duration of the device creation,
			// as the Vulkan API expects pointers to valid memory. This cache will be populated during the transformation process and passed to the Vulkan API.
			auto ToVulkanStruct(this CreateInfo& self, std::vector<vkr::VkDeviceQueueCreateInfo>& vulkanQueueCreateInfosCache) -> vkr::VkDeviceCreateInfo
			{
				vulkanQueueCreateInfosCache = self.QueueCreateInfos
					| std::ranges::views::transform([](const DeviceQueueCreateInfo& info) -> vkr::VkDeviceQueueCreateInfo { return info.ToVulkanStruct(); })
					| std::ranges::to<std::vector<vkr::VkDeviceQueueCreateInfo>>();
				return vkr::VkDeviceCreateInfo{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0, // Device creation flags are reserved for future use and must be zero.
					.queueCreateInfoCount = static_cast<std::uint32_t>(self.QueueCreateInfos.size()),
					.pQueueCreateInfos = self.QueueCreateInfos.empty() ? nullptr : vulkanQueueCreateInfosCache.data(),
					.enabledExtensionCount = static_cast<uint32_t>(self.EnabledExtensions.size()),
					.ppEnabledExtensionNames = self.EnabledExtensions.empty() ? nullptr : self.EnabledExtensions.data(),
					.pEnabledFeatures = self.EnabledFeatures.empty() ? nullptr : self.EnabledFeatures[0]
				};
			}
		}; 
		
		CreateInfo Info;
		vkr::VkPhysicalDevice PhysicalDevice = nullptr;

		auto operator()(this LogicalDeviceFactory& self) -> VkDeviceUniquePtr
		{
			if (not self.PhysicalDevice)
				throw std::invalid_argument("Physical device cannot be null.");
			auto vulkanQueueCreateInfosCache = std::vector<vkr::VkDeviceQueueCreateInfo>{};
			auto createInfo = vkr::VkDeviceCreateInfo{ self.Info.ToVulkanStruct(vulkanQueueCreateInfosCache) };
			auto device = VkDeviceUniquePtr{};
			auto result = Result{ vkr::vkCreateDevice(self.PhysicalDevice, &createInfo, nullptr, std::out_ptr(device)) };
			if (not result)
				throw VulkanError{ result, "Failed to create logical device." };
			return device;
		}
	};

	struct LogicalDevice
	{
		LogicalDevice(VkDeviceUniquePtr device)
			: Device(std::move(device))
		{ }

		constexpr auto Get() const noexcept -> vkr::VkDevice
		{
			return Device.get();
		}

		VkDeviceUniquePtr Device;
	};
}