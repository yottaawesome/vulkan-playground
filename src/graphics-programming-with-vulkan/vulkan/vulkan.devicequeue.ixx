export module vulkangfx:vulkan.devicequeue;
import std;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan
{
	// DeviceQueues are implicitly created as part of the logical device creation process, and are not explicitly destroyed. 
	// They are implicitly destroyed when the logical device is destroyed.
	class DeviceQueue
	{
	public:
		DeviceQueue(vkr::VkPhysicalDevice physicalDevice, vkr::VkDevice device, std::uint32_t queueFamilyIndex, std::uint32_t queueIndex)
			: physicalDevice(physicalDevice), queueFamilyIndex(queueFamilyIndex), queueIndex(queueIndex)
		{ 
			if (not physicalDevice)
				throw std::runtime_error("Invalid physical device handle.");
			if (not device)
				throw std::runtime_error("Invalid device handle.");
			vkr::vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &queue);
		}

		constexpr auto GetQueue(this const DeviceQueue& self) noexcept -> vkr::VkQueue { return self.queue; }

		auto GetQueueFamilyProperties(this DeviceQueue& self) noexcept -> vkr::VkQueueFamilyProperties 
		{ 
			vkr::VkQueueFamilyProperties queueFamilyProperties{};
			vkr::vkGetPhysicalDeviceQueueFamilyProperties(self.physicalDevice, &self.queueFamilyIndex, &queueFamilyProperties);
			return queueFamilyProperties;
		}

	private:
		vkr::VkPhysicalDevice physicalDevice{};
		std::uint32_t queueFamilyIndex = 0;
		std::uint32_t queueIndex = 0;
		vkr::VkQueue queue = nullptr;
	};
}