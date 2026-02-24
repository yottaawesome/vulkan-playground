export module vulkangfx:vulkan.physicaldevice;
import std;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan
{
	struct LogicalDevice
	{
		LogicalDevice(
			vkr::VkDeviceQueueCreateInfo* deviceQueueCreateInfos = nullptr,
			std::vector<const char*> enabledExtensions = {}
		)
		{ }
	};

	struct PhysicalDevice
	{
		vkr::VkPhysicalDevice Handle;
		auto CreateLogicalDevice(this const PhysicalDevice& self) -> LogicalDevice
		{
			return LogicalDevice{};
		}

		auto SupportsGraphicsQueue(this const PhysicalDevice& self) -> bool
		{
			return self.SupportsQueueFamily(vkr::VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
		}

		auto SupportsQueueFamily(this const PhysicalDevice& self, vkr::VkQueueFlagBits requested) -> bool
		{
			auto queueFamilies = std::vector{ self.GetQueueFamilyProperties() };
			return std::ranges::any_of(queueFamilies, [requested](const vkr::VkQueueFamilyProperties& qfp)
			{
				return qfp.queueFlags & requested;
			});
		}

		auto GetQueueFamilyProperties(
			this const PhysicalDevice& self
		) -> std::vector<vkr::VkQueueFamilyProperties>
		{
			auto count = std::uint32_t{};
			vkr::vkGetPhysicalDeviceQueueFamilyProperties(self.Handle, &count, nullptr);
			auto properties = std::vector<vkr::VkQueueFamilyProperties>{ count };
			vkr::vkGetPhysicalDeviceQueueFamilyProperties(self.Handle, &count, properties.data());
			return properties;
		}

		auto GetProperties(this const PhysicalDevice& self) -> vkr::VkPhysicalDeviceProperties
		{
			auto properties = vkr::VkPhysicalDeviceProperties{};
			vkr::vkGetPhysicalDeviceProperties(self.Handle, &properties);
			return properties;
		}

		auto GetType(this const PhysicalDevice& self) -> vkr::VkPhysicalDeviceType
		{
			return self.GetProperties().deviceType;
		}
	};

	struct PhysicalDeviceList
	{
		PhysicalDeviceList(const std::vector<PhysicalDevice>& devices)
			: Devices(devices)
		{ }

		PhysicalDeviceList(vkr::VkInstance instanceToQuery)
		{
			if (not instanceToQuery)
				throw std::invalid_argument("Instance pointer cannot be null.");

			auto deviceCount = std::uint32_t{};
			auto result = Result{ vkr::vkEnumeratePhysicalDevices(instanceToQuery, &deviceCount, nullptr) };
			if (not Result{ result })
				throw VulkanError{ result, "Failed to enumerate physical devices." };
			
			auto deviceHandles = std::vector<vkr::VkPhysicalDevice>{ deviceCount };
			result = Result{ vkr::vkEnumeratePhysicalDevices(instanceToQuery, &deviceCount, deviceHandles.data()) };
			if (not result)
				throw VulkanError{ result, "Failed to enumerate physical devices." };

			Devices = deviceHandles 
				| std::ranges::views::transform([](vkr::VkPhysicalDevice handle) { return PhysicalDevice{ handle }; })
				| std::ranges::to<std::vector<PhysicalDevice>>();
		}

		constexpr auto begin(this auto&& self) noexcept -> std::vector<PhysicalDevice>::iterator
		{
			return self.Devices.begin();
		}

		constexpr auto end(this auto&& self) noexcept -> std::vector<PhysicalDevice>::iterator
		{
			return self.Devices.end();
		}

		auto FilterByGraphicsSupport(this const PhysicalDeviceList& self) -> PhysicalDeviceList
		{
			auto filtered = self.Devices
				| std::ranges::views::filter([](const PhysicalDevice& device) { return device.SupportsGraphicsQueue(); })
				| std::ranges::to<std::vector<PhysicalDevice>>();
			return PhysicalDeviceList{ std::move(filtered) };
		}

		auto FilterByPhysicalDeviceType(this const PhysicalDeviceList& self, vkr::VkPhysicalDeviceType type) -> PhysicalDeviceList
		{
			auto filtered = self.Devices
				| std::ranges::views::filter(
					[type](const PhysicalDevice& device)
					{
						return device.GetType() == type;
					})
				| std::ranges::to<std::vector<PhysicalDevice>>();
			return PhysicalDeviceList{ std::move(filtered) };
		}

		std::vector<PhysicalDevice> Devices;
	};
}
