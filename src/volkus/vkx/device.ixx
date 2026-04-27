export module volkus:vkx.device;
import std;
import :vkx.exports;
import :vkx.error;
import :vkx.vulkanresource;

export namespace Volkus::vkx
{
	struct DeviceDeleter
	{
		static void operator()(VkDevice device) noexcept
		{
			vkDestroyDevice(device, nullptr);
		}
	};
	using DeviceUniquePtr = std::unique_ptr<std::remove_pointer_t<VkDevice>, DeviceDeleter>;

	class Device : public VulkanResource<DeviceUniquePtr>
	{
	public:
		constexpr Device(VkDevice deviceIn)
			: VulkanResource(DeviceUniquePtr{ deviceIn })
		{ }

		auto GetQueue(
			this const auto& self,
			std::uint32_t queueFamilyIndex,
			std::uint32_t queueIndex
		) -> VkQueue
		{
			auto queue = VkQueue{};
			vkGetDeviceQueue(self.Get(), queueFamilyIndex, queueIndex, &queue);
			return queue;
		}

		/*static auto Create(
			const PhysicalDevice& physicalDevice
			const VkDeviceCreateInfo& createInfo,
		) -> Device
		{
			auto device = VkDevice{};
			auto result = Volkus::vkx::Result{ vkCreateDevice(physicalDevice.Get(), &createInfo, nullptr, &device) };
			if (not result)
				throw VulkanError{ result, "Failed to create Vulkan device" };
			volkLoadDevice(device);
			return Device{ DeviceUniquePtr{ device } };
		}*/
	};
}
