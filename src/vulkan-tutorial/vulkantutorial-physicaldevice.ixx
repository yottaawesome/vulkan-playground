export module vulkantutorial:physicaldevice;
import std;
import :libs;
import :formatters;

export namespace VulkanTutorial
{
	constexpr auto RequiredDeviceExtensions = 
		std::array{
			vk::KHRSwapchainExtensionName,
			vk::KHRSpirv14ExtensionName,
			vk::KHRSynchronization2ExtensionName,
			vk::KHRCreateRenderpass2ExtensionName
		};

	// A wrapper around vk::raii::PhysicalDevice that adds scoring and feature checks.
	struct GraphicsProcessingUnit
	{
		vk::raii::PhysicalDevice Device = nullptr;

		GraphicsProcessingUnit(vk::raii::PhysicalDevice device)
			: Device(std::move(device))
		{ }

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

	struct ScoredPhysicalDevice
	{
		GraphicsProcessingUnit Gpu;

		explicit ScoredPhysicalDevice(GraphicsProcessingUnit device)
			: Gpu(std::move(device))
		{ }

		auto Score(this const ScoredPhysicalDevice& self) -> unsigned
		{
			auto properties = vk::PhysicalDeviceProperties{ self.Gpu.Device.getProperties() };
			auto score = unsigned{};
			if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
				return score += 1000;
			return score;
		};

		auto ToString(this const ScoredPhysicalDevice& self) -> std::string
		{
			auto properties = self.Gpu.Device.getProperties();
			return std::format(
				"Name: {}, Type: {}, Score: {}",
				self.Gpu.GetName(),
				properties.deviceType,
				self.Score()
			);
		}

		auto SupportsRequiredFeatures(this const ScoredPhysicalDevice& self) -> bool
		{
			if (not self.Gpu)
				return false;
			auto features = vk::PhysicalDeviceFeatures{ self.Gpu.Device.getFeatures() };
			if (not features.geometryShader)
			{
				std::println("Device {} does not support geometry shaders.", self.Gpu.GetName());
				return false;
			}

			auto queueFamilyProperties = std::vector{self.Gpu.Device.getQueueFamilyProperties()};
			if (not self.Gpu.SupportsGraphicsQueue())
			{
				std::println("Queue family with graphics support not found on device {}.", self.Gpu.GetName());
				return false;
			}

			if (not self.Gpu.SupportsRequiredExtensions())
				return false;

			return true;
		}

		auto GetDevice(this auto&& self) -> decltype(auto)
		{
			return std::forward_list<decltype(self)>(self.Device);
		}
	};

	template<typename TVectorFrom, typename TTypeTo>
	concept VectorConvertible = 
		requires(std::remove_cvref_t<TVectorFrom>::value_type from)
		{
			TTypeTo(from);
		};

	struct PhysicalDeviceList
	{
		std::vector<ScoredPhysicalDevice> Devices;

		PhysicalDeviceList(const std::vector<vk::raii::PhysicalDevice>& pd)
		{
			Devices = pd 
				| std::ranges::views::transform([](auto&& pd) { return ScoredPhysicalDevice{ pd }; }) 
				| std::ranges::to<std::vector<ScoredPhysicalDevice>>();

			SortAscendingByScore(Devices);
		}

		static void SortAscendingByScore(std::ranges::range auto&& devices) noexcept
		{
			std::sort(
				devices.begin(),
				devices.end(),
				[](const auto& a, const auto& b) static noexcept
				{
					return a.Gpu.Device.getProperties().deviceType > b.Gpu.Device.getProperties().deviceType;
				});
		}

		auto ToString(this const PhysicalDeviceList& self) -> std::string
		{
			auto result = std::format("Found {} devices:\n", self.Devices.size());
			for (const ScoredPhysicalDevice& value : self.Devices)
			{
				result += std::format(" -> Device: {}\n", value);
			}
			return result;
		}

		auto empty(this const PhysicalDeviceList& self) noexcept -> bool
		{
			return self.Devices.empty();
		}

		auto FirstSupportedDevice(this const PhysicalDeviceList& self) 
			-> std::optional<ScoredPhysicalDevice>
		{
			auto supportedDevices = self.Devices
				| std::ranges::views::filter([](const ScoredPhysicalDevice& deviceScore) { return deviceScore.SupportsRequiredFeatures(); })
				| std::ranges::to<std::vector<ScoredPhysicalDevice>>();
			if (supportedDevices.empty())
				return std::nullopt;
			return supportedDevices.front();
		}
	};
}