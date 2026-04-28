export module volkus:vkx.device;
import std;
import :vkx.exports;
import :vkx.error;
import :vkx.vulkanresource;

namespace Volkus::vkx
{
	struct DeviceDeleter
	{
		static void operator()(VkDevice device) noexcept
		{
			vkDestroyDevice(device, nullptr);
		}
	};
}

export namespace Volkus::vkx
{
	using DeviceUniquePtr = std::unique_ptr<std::remove_pointer_t<VkDevice>, DeviceDeleter>;

	auto CreateDeviceUniquePtr(
		VkPhysicalDevice physicalDevice,
		const VkDeviceCreateInfo& createInfo,
		bool loadVolkDevice
	) -> DeviceUniquePtr
	{
		if (not physicalDevice)
			throw std::runtime_error{ "Invalid physical device handle" };
		auto device = VkDevice{};
		auto result = Volkus::vkx::Result{ vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) };
		if (not result)
			throw VulkanError{ result, "Failed to create Vulkan device" };
		if (loadVolkDevice)
			volkLoadDevice(device);
		return DeviceUniquePtr{ device };
	}

	class Device : public VulkanResource<DeviceUniquePtr>
	{
	public:
		constexpr Device(DeviceUniquePtr deviceIn)
			: VulkanResource(std::move(deviceIn))
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
	};
}
