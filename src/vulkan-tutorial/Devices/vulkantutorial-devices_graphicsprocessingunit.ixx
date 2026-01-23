export module vulkantutorial:devices_graphicsprocessingunit;
import std;
import :libs;

export namespace VulkanTutorial::Devices
{
	constexpr auto RequiredDeviceExtensions =
		std::array{
			vk::KHRSwapchainExtensionName,
			vk::KHRSpirv14ExtensionName,
			vk::KHRSynchronization2ExtensionName,
			vk::KHRCreateRenderpass2ExtensionName
	};
	struct GraphicsProcessingUnit
	{
		vk::raii::PhysicalDevice Device = nullptr;

		GraphicsProcessingUnit(vk::raii::PhysicalDevice device)
			: Device(std::move(device))
		{
		}

		auto operator->(this GraphicsProcessingUnit& self) -> vk::raii::PhysicalDevice&
		{
			return self.Device;
		}

		auto GetName(this const GraphicsProcessingUnit& self) -> std::string
		{
			auto properties = self.Device.getProperties();
			return std::string{ std::string_view{ properties.deviceName } };
		}

		operator bool(this const GraphicsProcessingUnit& self) noexcept
		{
			return *self.Device != nullptr;
		}

		auto ToString(this const GraphicsProcessingUnit& self) -> std::string
		{
			auto properties = self.Device.getProperties();
			return std::format(
				"Name: {}, Type: {}",
				self.GetName(),
				properties.deviceType
			);
		}

		auto SupportsGraphicsQueue(this const GraphicsProcessingUnit& self) -> bool
		{
			return self.QueryFamilySupport(vk::QueueFlagBits::eGraphics);
		}

		auto SupportsRequiredExtensions(this const GraphicsProcessingUnit& self) -> bool
		{
			if (not self)
				return false;

			auto extensions = std::vector<vk::ExtensionProperties>{ self.Device.enumerateDeviceExtensionProperties() };
			for (std::string_view required : RequiredDeviceExtensions)
			{
				bool supported = std::ranges::any_of(
					extensions,
					[required](const vk::ExtensionProperties& ext)
					{
						return ext.extensionName == required;
					});
				if (not supported)
				{
					std::println("Required device extension not supported on device {}: {}", self.GetName(), required);
					return false;
				}
			}
			return true;
		}

		auto QueryFamilySupport(
			this const GraphicsProcessingUnit& self,
			vk::QueueFlagBits requested
		) -> bool
		{
			auto queueFamilyProperties = std::vector{ self.Device.getQueueFamilyProperties() };
			return std::any_of(
				queueFamilyProperties.begin(),
				queueFamilyProperties.end(),
				[requested](const vk::QueueFamilyProperties& qfp)
				{
					return qfp.queueFlags & requested;
				}
			);
		}
	};
}