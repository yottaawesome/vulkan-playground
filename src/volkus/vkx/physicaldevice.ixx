export module volkus:vkx.physicaldevice;
import std;
import :vkx.exports;
import :vkx.vulkanresource;

export namespace Volkus::vkx
{
	struct PhysicalDeviceProperties
	{
		VkPhysicalDevice Device{};
		VkPhysicalDeviceProperties Properties = 
			[](const auto& Device) noexcept -> VkPhysicalDeviceProperties
			{
				auto properties = VkPhysicalDeviceProperties{};
				if (Device)
					vkGetPhysicalDeviceProperties(Device, &properties);
				return properties;
			}(Device);

		constexpr auto IsDiscreteGpu(this auto&& self) -> bool
		{
			return self.Properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		}
		constexpr auto IsIntegratedGpu(this auto&& self) -> bool
		{
			return self.Properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		}
		constexpr auto IsVirtualGpu(this auto&& self) -> bool
		{
			return self.Properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
		}
		constexpr auto IsCpu(this auto&& self) -> bool
		{
			return self.Properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU;
		}
		constexpr auto IsOther(this auto&& self) -> bool
		{
			return self.Properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_OTHER;
		}
		constexpr auto IsUnknown(this auto&& self) -> bool
		{
			return self.Properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_UNKNOWN;
		}
		constexpr auto GetApiVersion(this auto&& self) -> Vk::VulkanApiVersion
		{
			return { self.Properties.apiVersion };
		}
		constexpr auto GetDriverVersion(this auto&& self) -> Vk::VulkanDriverVersion
		{
			return { self.Properties.driverVersion };
		}
		constexpr auto GetVendorId(this auto&& self) -> std::uint32_t
		{
			return self.Properties.vendorID;
		}
		constexpr auto GetDeviceId(this auto&& self) -> std::uint32_t
		{
			return self.Properties.deviceID;
		}
		constexpr auto GetDeviceName(this auto&& self) -> std::string_view
		{
			return self.Properties.deviceName;
		}
	};

	class PhysicalDevice : public VulkanResource<VkPhysicalDevice>
	{
	public:
		PhysicalDevice(VkPhysicalDevice device) 
			: VulkanResource<VkPhysicalDevice>(device)
		{}

		constexpr auto GetProperties(this auto&& self) -> VkPhysicalDeviceProperties
		{
			auto properties = VkPhysicalDeviceProperties{};
			vkGetPhysicalDeviceProperties(self.Get(), &properties);
			return properties;
		}

		constexpr auto IsDiscreteGpu(this auto&& self) -> bool
		{
			return self.GetProperties().deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		}

		auto GetName(this auto&& self) -> std::string_view
		{
			return self.GetProperties().deviceName;
		}

		auto FindQueueIndex(this auto&& self, VkQueueFlags requiredFlags) -> std::optional<std::uint32_t>
		{
			auto queueFamilyCount = std::uint32_t{};
			vkGetPhysicalDeviceQueueFamilyProperties(self.Get(), &queueFamilyCount, nullptr);
			auto queueFamilies = std::vector<VkQueueFamilyProperties>{ queueFamilyCount };
			vkGetPhysicalDeviceQueueFamilyProperties(self.Get(), &queueFamilyCount, queueFamilies.data());
			for (std::uint32_t i = 0; i < queueFamilyCount; ++i)
			{
				if ((queueFamilies[i].queueFlags & requiredFlags) == requiredFlags)
					return i;
			}
			return std::nullopt;
		}
	};

	class PhysicalDevices
	{
		PhysicalDevices(std::vector<VkPhysicalDevice> devices) 
			: m_devices(
				devices
				| std::views::transform([](VkPhysicalDevice device) { return PhysicalDevice{ device }; })
				| std::ranges::to<std::vector>()) 
		{ }
		
	private:
		std::vector<PhysicalDevice> m_devices;
	};
}