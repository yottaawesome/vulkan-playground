export module vulkantutorial:physicaldevice;
import std;
import :libs;
import :formatters;

export namespace VulkanTutorial
{
	struct PhysicalDeviceScore
	{
		vk::raii::PhysicalDevice Device = nullptr;

		PhysicalDeviceScore(vk::raii::PhysicalDevice device)
			: Device(std::move(device))
		{
		}

		unsigned Score =
			[device = Device]
			{
				auto properties = device.getProperties();
				auto score = unsigned{};
				if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
					return score += 1000;
				return score;
			}();

		auto ToString(this const PhysicalDeviceScore& self) -> std::string
		{
			auto properties = self.Device.getProperties();
			return std::format(
				"Name: {}, Type: {}, Score: {}",
				std::string_view{ properties.deviceName },
				properties.deviceType,
				self.Score
			);
		}

		auto SupportsRequiredFeatures(this const PhysicalDeviceScore& self) -> bool
		{
			auto features = self.Device.getFeatures();
			return features.geometryShader;
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
		std::vector<PhysicalDeviceScore> Devices;

		PhysicalDeviceList(VectorConvertible<PhysicalDeviceScore> auto&& v)
			: Devices(v | std::ranges::to<std::vector<PhysicalDeviceScore>>())
		{
			SortAscendingByScore(Devices);
		}

		static void SortAscendingByScore(std::ranges::range auto&& devices) noexcept
		{
			std::sort(
				devices.begin(),
				devices.end(),
				[](const auto& a, const auto& b) static noexcept
				{
					return a.Device.getProperties().deviceType > b.Device.getProperties().deviceType;
				});
		}

		auto ToString(this const PhysicalDeviceList& self) -> std::string
		{
			auto result = std::format("Found {} devices:\n", self.Devices.size());
			for (const PhysicalDeviceScore& value : self.Devices)
			{
				result += std::format(" -> Device: {}\n", value);
			}
			return result;
		}

		auto empty(this const PhysicalDeviceList& self) noexcept -> bool
		{
			return self.Devices.empty();
		}

		auto FirstSupportedDevice(this const PhysicalDeviceList& self) -> std::optional<PhysicalDeviceScore>
		{
			auto supportedDevices = self.Devices
				| std::ranges::views::filter([](const PhysicalDeviceScore& deviceScore) { return deviceScore.SupportsRequiredFeatures(); })
				| std::ranges::to<std::vector<PhysicalDeviceScore>>();
			if (supportedDevices.empty())
				return std::nullopt;

			SortAscendingByScore(supportedDevices);
			return std::move(supportedDevices.front());
		}
	};
}