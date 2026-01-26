export module vulkantutorial:vulkanite.device.logicaldevice;
import std;
import :libs;

export namespace VulkanTutorial::Vulkanite::Device
{
	struct LogicalDevice
	{
		vk::raii::Device Device;

		auto operator->(this LogicalDevice& self) noexcept -> vk::raii::Device&
		{
			return self.Device;
		}

		operator bool(this const LogicalDevice& self) noexcept
		{
			return *self.Device != nullptr;
		}

		operator vk::raii::Device&(this LogicalDevice& self) noexcept
		{
			return self.Device;
		}

		auto GetQueue(
			this const LogicalDevice& self, 
			std::uint32_t familyIndex, 
			std::uint32_t queueIndex
		) -> vk::raii::Queue
		{
			return vk::raii::Queue{ self.Device, familyIndex, queueIndex };
		}
	};
}
