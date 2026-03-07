export module vulkan26:vulkan.logicaldevice;
import std;
import :vulkan.error;
import :vulkan.exports;

export namespace Vk
{
	class LogicalDevice
	{
	public:
		~LogicalDevice()
		{
			if (device)
				vkDestroyDevice(device, nullptr);
		}
		constexpr LogicalDevice(VkDevice deviceIn)
			: device(deviceIn)
		{
			if (not device)
				throw Error{ VkResult::VK_ERROR_INITIALIZATION_FAILED };
		}
		LogicalDevice(LogicalDevice const&) = delete;
		LogicalDevice& operator=(LogicalDevice const&) = delete;
		constexpr auto Get(this const auto& self) noexcept -> VkDevice
		{
			return self.device;
		}
	private:
		VkDevice device = nullptr;
	};
}
