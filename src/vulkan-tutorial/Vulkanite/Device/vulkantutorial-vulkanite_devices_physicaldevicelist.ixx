export module vulkantutorial:vulkanite_device_physicaldevicelist;
import std;
import :vulkanite_device_scoredphysicaldevice;

export namespace VulkanTutorial::Vulkanite::Device
{
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
				| std::ranges::views::transform(
					[](auto&& pd) static { return ScoredPhysicalDevice{ pd }; })
				| std::ranges::to<std::vector<ScoredPhysicalDevice>>();

			SortAscendingByScore(Devices);
		}

		void FilterUnsupportedDevices() noexcept
		{
			std::erase_if(
				Devices,
				[](const ScoredPhysicalDevice& deviceScore) static noexcept
				{
					auto supportsFeatures = bool{ deviceScore.SupportsRequiredFeatures() };
					if (not supportsFeatures)
						std::println("Device {} does not support required features.", deviceScore.Gpu.GetName());
					return not supportsFeatures;
				}
			);
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