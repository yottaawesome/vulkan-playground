export module volkus:vkx.instance;
import std;
import vulkanlib;
import :vkx.concepts;
import :vkx.error;
import :vkx.vulkanresource;

// Raw Vulkan instance and handle management.
namespace Volkus::vkx
{
	struct InstanceDeleter
	{
		static void operator()(VkInstance instance) noexcept
		{
			vkDestroyInstance(instance, nullptr);
		}
	};
	using InstanceUniquePtr = std::unique_ptr<std::remove_pointer_t<VkInstance>, InstanceDeleter>;
	template<typename T>
	concept InstanceUniquePtrLike = UniquePtrLike<T, VkInstance>;
}

export namespace Volkus::vkx
{
	auto CreateVkInstance(const VkInstanceCreateInfo& createInfo, bool initializeVolkAndLoadInstance) -> InstanceUniquePtr
	{
		if (initializeVolkAndLoadInstance)
			volkInitialize();
		auto instance = VkInstance{};
		auto result = Volkus::vkx::Result{ vkCreateInstance(&createInfo, nullptr, &instance) };
		if (not result)
			throw VulkanError{ result, "Failed to create Vulkan instance" };
		if (initializeVolkAndLoadInstance)
			volkLoadInstance(instance);
		return InstanceUniquePtr{ instance };
	}

	class Instance : public VulkanResource<InstanceUniquePtr>
	{
	public:
		constexpr Instance() = default;

		constexpr Instance(InstanceUniquePtr instanceIn)
			: VulkanResource<InstanceUniquePtr>(std::move(instanceIn))
		{}

		auto EnumeratePhysicalDevices(this auto&& self) -> std::vector<VkPhysicalDevice>
		{
			auto deviceCount = std::uint32_t{};
			auto result = Volkus::vkx::Result{ vkEnumeratePhysicalDevices(self.Get(), &deviceCount, nullptr) };
			if (not result)
				throw VulkanError{ result, "Failed to enumerate physical devices (count)" };
			auto devices = std::vector<VkPhysicalDevice>(deviceCount);
			result = Result{ vkEnumeratePhysicalDevices(self.Get(), &deviceCount, devices.data()) };
			if (not result)
				throw VulkanError{ result, "Failed to enumerate physical devices (data)" };
			return devices;
		}
	};

	template<typename T>
	concept InstanceFactoryLike = requires(T t)
	{
		{ t.GetLayers() } -> std::ranges::range;
		{ t.GetExtensions() } -> std::ranges::range;
		{ t.GetApplicationInfo() } -> std::convertible_to<VkApplicationInfo>;
		{ t.GetFlags() } -> std::convertible_to<VkInstanceCreateFlags>;
	};

	struct InstanceFactory
	{
		[[nodiscard]]
		auto operator()(this InstanceFactoryLike auto&& self, bool initializeVolkAndLoadInstance) -> Instance
		{
			auto instance = VkInstance{};
			auto layers = self.GetLayers();
			auto extensions = self.GetExtensions();
			auto applicationInfo = self.GetApplicationInfo();
			auto createInfo = VkInstanceCreateInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.flags = self.GetFlags(),
				.pApplicationInfo = &applicationInfo,
				.enabledLayerCount = static_cast<std::uint32_t>(layers.size()),
				.ppEnabledLayerNames = layers.data(),
				.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
				.ppEnabledExtensionNames = extensions.data()
			};
			return { CreateVkInstance(createInfo, initializeVolkAndLoadInstance) };
		}
	};
}
