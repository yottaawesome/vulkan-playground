export module vulkantutorial:vulkanite.device.physicaldevice;
import std;
import :libs;
import :concepts;
import :vulkanite.device.logicaldevice;

export namespace VulkanTutorial::Vulkanite::Device
{
	struct PhysicalDevice
	{
		vk::raii::PhysicalDevice Device = nullptr;

		constexpr PhysicalDevice() = default;

		PhysicalDevice(vk::raii::PhysicalDevice device)
			: Device(std::move(device))
		{ }

		auto operator->(this PhysicalDevice& self) -> vk::raii::PhysicalDevice*
		{
			return &self.Device;
		}

		auto GetName(this const PhysicalDevice& self) -> std::string
		{
			auto properties = self.Device.getProperties();
			return std::string{ std::string_view{ properties.deviceName } };
		}

		operator bool(this const PhysicalDevice& self) noexcept
		{
			return *self.Device != nullptr;
		}

		auto ToString(this const PhysicalDevice& self) -> std::string
		{
			auto properties = self.Device.getProperties();
			return std::format(
				"Name: {}, Type: {}",
				self.GetName(),
				properties.deviceType
			);
		}

		auto SupportsGraphicsQueue(this const PhysicalDevice& self) -> bool
		{
			return self.QueryFamilySupport(vk::QueueFlagBits::eGraphics);
		}

		auto FindQueueFamilyIndex(
			this const PhysicalDevice& self, 
			vk::QueueFlagBits requested
		) -> std::optional<std::uint32_t>
		{
			if (not self)
				return std::nullopt;
			auto queueFamilyProperties = std::vector<vk::QueueFamilyProperties>{ 
				self.Device.getQueueFamilyProperties() 
			};
			for (const auto& [index, queueProps] : std::views::enumerate(queueFamilyProperties))
				if (queueProps.queueFlags & requested)
					return static_cast<std::uint32_t>(index);
			return std::nullopt;
		}

		auto FindPresentQueueFamilyIndexForSurface(
			this const PhysicalDevice& self, 
			vk::SurfaceKHR surface
		) -> std::optional<std::uint32_t>
		{
			if (not self)
				return std::nullopt;
			auto queueFamilyProperties = std::vector{ self.Device.getQueueFamilyProperties() };
			for (const auto& [index, queueProps] : std::views::enumerate(queueFamilyProperties))
			{
				auto supportsPresent = self.Device.getSurfaceSupportKHR(
					static_cast<std::uint32_t>(index),
					surface
				);
				if (supportsPresent)
					return static_cast<std::uint32_t>(index);
			}
			return std::nullopt;
		}

		// Query for a queue family that supports graphics operations.
		// e.g vk::QueueFlagBits::eGraphics
		auto FindGraphicsQueueFamilyIndex(this const PhysicalDevice& self) 
			-> std::optional<std::uint32_t>
		{
			return self.FindQueueFamilyIndex(vk::QueueFlagBits::eGraphics);
		}

		auto SupportsExtensions(
			this const PhysicalDevice& self,
			const std::ranges::range auto& requiredExtensions
		) -> bool
		{
			if (not self)
				return false;

			auto extensions = std::vector<vk::ExtensionProperties>{ self.Device.enumerateDeviceExtensionProperties() };
			for (std::string_view required : requiredExtensions)
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
			this const PhysicalDevice& self,
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

		auto CreateLogicalDevice(
			this const PhysicalDevice& self,
			const vk::DeviceCreateInfo& createInfo
		) -> LogicalDevice
		{
			return LogicalDevice{ vk::raii::Device{ self.Device, createInfo } };
		}

		auto GetFeatures(this const PhysicalDevice& self) 
			-> vk::PhysicalDeviceFeatures
		{
			return self ? self.Device.getFeatures() : vk::PhysicalDeviceFeatures{};
		}
	};
}