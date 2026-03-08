export module vulkan26:vulkan.device;
import std;
import :vulkan.error;
import :vulkan.exports;

export namespace vk
{
	class Device
	{
	public:
		~Device()
		{
			if (device)
			{
				vkDestroyDevice(device, nullptr);
				device = nullptr;
			}
		}

		constexpr Device(VkDevice deviceIn)
			: device(deviceIn)
		{
			if (not device)
				throw Error{ VkResult::VK_ERROR_INITIALIZATION_FAILED };
		}

		// No copy semantics
		Device(Device const&) = delete;
		Device& operator=(Device const&) = delete;

		// Move semantics
		constexpr Device(Device&& other) noexcept
			: device(std::exchange(other.device, nullptr))
		{}
		constexpr auto operator=(Device&& other) noexcept -> Device&
		{
			if (this != &other)
			{
				if (device)
					vkDestroyDevice(device, nullptr);
				device = std::exchange(other.device, nullptr);
			}
			return *this;
		}

		constexpr auto Get(this const auto& self) noexcept -> VkDevice
		{
			return self.device;
		}

		struct QueueDescriptor
		{
			VkQueue Handle = nullptr;
			std::uint32_t FamilyIndex = 0;
			std::uint32_t Index = 0;
		};

		constexpr auto GetQueue(
			this const auto& self, 
			std::uint32_t queueFamilyIndex, 
			std::uint32_t queueIndex
		) noexcept -> QueueDescriptor
		{
			auto queue = VkQueue{};
			vkGetDeviceQueue(self.device, queueFamilyIndex, queueIndex, &queue);
			return { queue, queueFamilyIndex, queueIndex };
		}
	private:
		VkDevice device = nullptr;
	};
}
