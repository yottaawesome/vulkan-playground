export module vulkantutorial:vulkanite.device.scoredphysicaldevice;
import std;
import :libs;
import :vulkanite.device.physicaldevice;

export namespace VulkanTutorial::Vulkanite::Device
{
	constexpr auto RequiredDeviceExtensions =
		std::array{
			vk::KHRSwapchainExtensionName,
			vk::KHRSpirv14ExtensionName,
			vk::KHRSynchronization2ExtensionName,
			vk::KHRCreateRenderpass2ExtensionName
	};

	// A wrapper around vk::raii::PhysicalDevice that adds scoring and feature checks.
	struct ScoredPhysicalDevice
	{
		PhysicalDevice Gpu;

		explicit ScoredPhysicalDevice(PhysicalDevice device)
			: Gpu(std::move(device))
		{
		}

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

			auto queueFamilyProperties = std::vector{ self.Gpu.Device.getQueueFamilyProperties() };
			if (not self.Gpu.SupportsGraphicsQueue())
			{
				std::println("Queue family with graphics support not found on device {}.", self.Gpu.GetName());
				return false;
			}

			if (not self.Gpu.SupportsExtensions(RequiredDeviceExtensions))
				return false;

			return true;
		}

		auto GetDevice(this auto&& self) -> decltype(auto)
		{
			return std::forward_list<decltype(self)>(self.Device);
		}

		auto ToGraphicsProcessingUnit(this auto&& self) -> decltype(auto)
		{
			return std::forward_like<decltype(self)>(self.Gpu);
		}
	};
}