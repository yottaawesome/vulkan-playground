export module vulkangfx:vulkan.device;
import std;
import :raii;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan
{
	using VkDeviceUniquePtr = Raii::IndirectUniquePtr<vkr::VkDevice, vkr::vkDestroyDevice, nullptr>;

	struct DeviceFactory
	{
		struct CreateInfo
		{
			struct DeviceQueueCreateInfo
			{
				vkr::VkDeviceQueueCreateFlags Flags = 0;
				std::uint32_t QueueFamilyIndex = 0;
				std::vector<float> QueuePriorities{1.0f};

				constexpr auto ToVulkanStruct(this const DeviceQueueCreateInfo& self) -> vkr::VkDeviceQueueCreateInfo
				{
					return vkr::VkDeviceQueueCreateInfo{
						.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
						.pNext = nullptr,
						.flags = self.Flags,
						.queueFamilyIndex = self.QueueFamilyIndex,
						.queueCount = static_cast<std::uint32_t>(self.QueuePriorities.size()),
						.pQueuePriorities = self.QueuePriorities.data()
					};
				}
			};

			std::vector<DeviceQueueCreateInfo> QueueCreateInfos;
			std::vector<const char*> EnabledExtensions;
			vkr::VkPhysicalDeviceFeatures2 EnabledFeatures{};
			vkr::VkPhysicalDeviceVulkan11Features EnabledFeatures11{};
			vkr::VkPhysicalDeviceVulkan12Features EnabledFeatures12{};
			vkr::VkPhysicalDeviceVulkan13Features EnabledFeatures13{};
			vkr::VkPhysicalDeviceVulkan14Features EnabledFeatures14{};

			auto ToVulkanStruct(this CreateInfo& self, std::span<const vkr::VkDeviceQueueCreateInfo> queueCreateInfos) -> vkr::VkDeviceCreateInfo
			{
				self.EnabledFeatures.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				self.EnabledFeatures.pNext = &self.EnabledFeatures11;
				self.EnabledFeatures11.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
				self.EnabledFeatures11.pNext = &self.EnabledFeatures12;
				self.EnabledFeatures12.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
				self.EnabledFeatures12.pNext = &self.EnabledFeatures13;
				self.EnabledFeatures13.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
				self.EnabledFeatures13.pNext = &self.EnabledFeatures14;
				self.EnabledFeatures14.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
				self.EnabledFeatures14.pNext = nullptr;

				return vkr::VkDeviceCreateInfo{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
					.pNext = &self.EnabledFeatures,
					.flags = 0, // Device creation flags are reserved for future use and must be zero.
					.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size()),
					.pQueueCreateInfos = queueCreateInfos.empty() ? nullptr : queueCreateInfos.data(),
					.enabledExtensionCount = static_cast<uint32_t>(self.EnabledExtensions.size()),
					.ppEnabledExtensionNames = self.EnabledExtensions.empty() ? nullptr : self.EnabledExtensions.data(),
					.pEnabledFeatures = nullptr // Must be nullptr if using VkPhysicalDeviceFeatures2.
				};
			}
		}; 

		CreateInfo Info;
		vkr::VkPhysicalDevice PhysicalDevice = nullptr;

		auto operator()(this DeviceFactory& self) -> VkDeviceUniquePtr
		{
			if (not self.PhysicalDevice)
				throw std::invalid_argument("Physical device cannot be null.");
			auto queueCreateInfos = self.Info.QueueCreateInfos
				| std::ranges::views::transform([](const CreateInfo::DeviceQueueCreateInfo& info) { return info.ToVulkanStruct(); })
				| std::ranges::to<std::vector<vkr::VkDeviceQueueCreateInfo>>();
			auto createInfo = self.Info.ToVulkanStruct(queueCreateInfos);
			auto device = VkDeviceUniquePtr{};
			auto result = Result{ vkr::vkCreateDevice(self.PhysicalDevice, &createInfo, nullptr, std::out_ptr(device)) };
			if (not result)
				throw VulkanError{ result, "Failed to create logical device." };
			return device;
		}
	};

	class Device
	{
	public:
		Device(VkDeviceUniquePtr deviceIn)
			: device(std::move(deviceIn))
		{ }

		constexpr auto GetHandle(this const Device& self) noexcept -> vkr::VkDevice
		{
			return self.device.get();
		}

		constexpr auto GetPtr(this Device&& self) noexcept -> VkDeviceUniquePtr
		{
			return std::move(self.device);
		}

		auto WaitIdle(this const Device& self) -> Result
		{
			// TODO: Should we check for errors here?
			return { vkr::vkDeviceWaitIdle(self.device.get()) };
		}

	private:
		VkDeviceUniquePtr device;
	};
}
