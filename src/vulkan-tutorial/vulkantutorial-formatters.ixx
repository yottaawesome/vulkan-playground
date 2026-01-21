export module vulkantutorial:formatters;
import std;
import :libs;

export namespace VulkanTutorial
{
	template<typename T>
	concept Printable = requires(T t)
	{
		{ t.ToString() } -> std::convertible_to<std::string>;
	};

	struct Test
	{
		constexpr auto ToString() const -> std::string
		{
			return "Test";
		}
	};
}

export namespace std
{
	template<>
	struct formatter<vk::PhysicalDeviceType> : formatter<string_view>
	{
		constexpr auto format(vk::PhysicalDeviceType type, format_context& ctx) const
		{
			switch (type)
			{
			case vk::PhysicalDeviceType::eOther:
				return formatter<string_view>::format("Other", ctx);
			case vk::PhysicalDeviceType::eIntegratedGpu:
				return formatter<string_view>::format("Integrated GPU", ctx);
			case vk::PhysicalDeviceType::eDiscreteGpu:
				return formatter<string_view>::format("Discrete GPU", ctx);
			case vk::PhysicalDeviceType::eVirtualGpu:
				return formatter<string_view>::format("Virtual GPU", ctx);
			case vk::PhysicalDeviceType::eCpu:
				return formatter<string_view>::format("CPU", ctx);
			default:
				return formatter<string_view>::format("Unknown", ctx);
			}
		}
	};

	template<>
	struct formatter<vk::raii::PhysicalDevice> : formatter<string_view>
	{
		auto format(const vk::raii::PhysicalDevice& physicalDevice, format_context& ctx) const
		{
			auto properties = physicalDevice.getProperties();
			auto deviceName = std::string_view{ properties.deviceName };
			return formatter<string_view>::format(
				std::format(
					"Device name: {}, type: {}",
					deviceName,
					properties.deviceType
				),
				ctx
			);
		}
	};

	template<VulkanTutorial::Printable T>
	struct formatter<T> : formatter<string_view>
	{
		auto format(const T& value, format_context& ctx) const
		{
			return formatter<string_view>::format(value.ToString(), ctx);
		}
	};
}
