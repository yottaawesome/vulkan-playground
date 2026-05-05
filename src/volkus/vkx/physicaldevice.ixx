export module volkus:vkx.physicaldevice;
import std;
import vulkanlib;
import :vkx.vulkanresource;
import :vkx.physicaldeviceproperties;

export namespace Volkus::vkx
{
	class PhysicalDevice : public VulkanResource<VkPhysicalDevice>
	{
	public:
		PhysicalDevice(VkPhysicalDevice device) 
			: VulkanResource<VkPhysicalDevice>(device)
		{}

		constexpr auto GetProperties(this auto&& self) -> PhysicalDeviceProperties
		{
			return PhysicalDeviceProperties{ self.Get() };
		}

		constexpr auto IsDiscreteGpu(this auto&& self) -> bool
		{
			return self.GetProperties().IsDiscreteGpu();
		}

		auto GetName(this auto&& self) -> std::string_view
		{
			return self.GetProperties().GetDeviceName();
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
}