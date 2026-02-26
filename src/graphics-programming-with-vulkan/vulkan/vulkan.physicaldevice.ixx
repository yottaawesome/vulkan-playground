export module vulkangfx:vulkan.physicaldevice;
import std;
import :raii;
import :vulkan.exports;
import :vulkan.error;
import :vulkan.formatters;

export namespace Vulkan
{
	struct PhysicalDevice
	{
		vkr::VkPhysicalDevice Handle;

		PhysicalDevice(vkr::VkPhysicalDevice handle)
			: Handle(handle)
		{ }

		auto SupportsGraphicsQueue(this const PhysicalDevice& self) -> bool
		{
			return self.SupportsQueueFamily(vkr::VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
		}

		auto SupportsComputeQueue(this const PhysicalDevice& self) -> bool
		{
			return self.SupportsQueueFamily(vkr::VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT);
		}

		auto SupportsTransferQueue(this const PhysicalDevice& self) -> bool
		{
			return self.SupportsQueueFamily(vkr::VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
		}

		auto SupportsSparseBindingQueue(this const PhysicalDevice& self) -> bool
		{
			return self.SupportsQueueFamily(vkr::VkQueueFlagBits::VK_QUEUE_SPARSE_BINDING_BIT);
		}

		auto SupportsQueues(
			this const PhysicalDevice& self, 
			std::convertible_to<vkr::VkQueueFlagBits> auto... args
		) -> bool
		{
			return (self.SupportsQueueFamily(args) and ...);
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

		auto Describe(this const PhysicalDevice& self) -> std::string
		{
			auto properties = vkr::VkPhysicalDeviceProperties{ self.GetProperties() };
			return std::format(
				"{} (type: {}, API version: {}), {} MB of memory",
				properties.deviceName,
				self.GetType(),
				vkr::VersionToString(properties.apiVersion),
				properties.limits.maxMemoryAllocationCount / (1024 * 1024)
			);
		}

		auto GetGraphicsQueueFamilyIndex(this const PhysicalDevice& self) -> std::optional<std::uint32_t>
		{
			auto queueFamilies = std::vector{ self.GetQueueFamilyProperties() };
			for (std::uint32_t i = 0; i < queueFamilies.size(); ++i)
				if (queueFamilies[i].queueFlags & vkr::VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
					return i;
			return std::nullopt;
		}
	};

	struct PhysicalDeviceList
	{
		PhysicalDeviceList(const std::vector<PhysicalDevice>& devices)
			: Devices(devices)
		{ }

		constexpr auto operator[](this auto&& self, size_t x) -> decltype(auto)
		{
			return std::forward_like<decltype(self)>(self.Devices[x]);
		}

		static auto Enumerate(vkr::VkInstance instance) -> PhysicalDeviceList
		{
			if (not instance)
				throw std::invalid_argument("Instance handle cannot be null.");

			auto deviceCount = std::uint32_t{};
			auto result = Result{ vkr::vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) };
			if (not result)
				throw VulkanError{ result, "Failed to enumerate physical devices." };

			auto deviceHandles = std::vector<vkr::VkPhysicalDevice>{ deviceCount };
			result = Result{ vkr::vkEnumeratePhysicalDevices(instance, &deviceCount, deviceHandles.data()) };
			if (not result)
				throw VulkanError{ result, "Failed to enumerate physical devices." };

			auto devices = deviceHandles 
				| std::ranges::views::transform([](vkr::VkPhysicalDevice handle) { return PhysicalDevice{ handle }; })
				| std::ranges::to<std::vector<PhysicalDevice>>();
			return PhysicalDeviceList{ std::move(devices) };
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

		auto FilterByQueueSupport(
			this const PhysicalDeviceList& self, 
			std::convertible_to<vkr::VkQueueFlagBits> auto... required
		) -> PhysicalDeviceList
		{
			auto filtered = self.Devices
				| std::ranges::views::filter(
					[...required = required](const PhysicalDevice& device)
					{
						return (device.SupportsQueues(required...));
					})
				| std::ranges::to<std::vector<PhysicalDevice>>();
			return PhysicalDeviceList{ std::move(filtered) };
		}

		std::vector<PhysicalDevice> Devices;
	};
}
