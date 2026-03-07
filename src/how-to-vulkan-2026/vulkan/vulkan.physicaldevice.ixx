export module vulkan26:vulkan.physicaldevice;
import std;
import :vulkan.error;
import :vulkan.exports;

export namespace Vk
{
	class PhysicalDevice
	{
	public:
		constexpr PhysicalDevice(VkPhysicalDevice physicalDeviceIn)
			: physicalDevice(physicalDeviceIn)
		{
			if (not physicalDevice)
				throw Error{ VkResult::VK_ERROR_INITIALIZATION_FAILED };
		}
		PhysicalDevice(PhysicalDevice const&) = delete;
		PhysicalDevice& operator=(PhysicalDevice const&) = delete;
		constexpr auto Get(this const auto& self) noexcept -> VkPhysicalDevice
		{
			return self.physicalDevice;
		}
		auto GetProperties(this const auto& self) -> VkPhysicalDeviceProperties
		{
			auto props = VkPhysicalDeviceProperties2{.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
			vkGetPhysicalDeviceProperties2(self.physicalDevice, &props);
			return props;
		}
		auto GetName(this const auto& self) -> std::string
		{
			auto props = self.GetProperties();
			return std::string{ props.properties.deviceName };
		}
		auto GetType(this const auto& self) -> VkPhysicalDeviceType
		{
			auto props = self.GetProperties();
			return props.properties.deviceType;
		}
		auto IsDiscrete(this const auto& self) -> bool
		{
			return self.GetType() & VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		}

		auto GetQueueFamilyProperties(this const auto& self) -> std::vector<VkQueueFamilyProperties2>
		{
			auto queueFamilyCount = std::uint32_t{};
			auto status = Vk::Result{ Vk::vkGetPhysicalDeviceQueueFamilyProperties2(self.physicalDevice, &queueFamilyCount, nullptr) };
			if (not status)
				throw Error{ status.result };
			auto queueFamilies = std::vector<VkQueueFamilyProperties2>{ queueFamilyCount };
			status = Vk::Result{ Vk::vkGetPhysicalDeviceQueueFamilyProperties2(self.physicalDevice, &queueFamilyCount, queueFamilies.data()) };
			if (not status)
				throw Error{ status.result };
			return queueFamilies;
		}
	private:
		VkPhysicalDevice physicalDevice = nullptr;
	};
}
