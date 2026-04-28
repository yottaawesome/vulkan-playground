export module volkus:vkx.physicaldeviceproperties;
import std;
import :vkx.exports;

export namespace Volkus::vkx
{
	struct PhysicalDeviceProperties
	{
		VkPhysicalDevice Device{};
		VkPhysicalDeviceProperties Properties =
			[](auto&& Device) noexcept -> VkPhysicalDeviceProperties
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
}
