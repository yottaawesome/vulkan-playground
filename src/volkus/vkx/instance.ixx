export module volkus:vkx.instance;
import std;
import :vkx.exports;
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

	template<typename T>
	class InstanceType : public VulkanResource<T>
	{
	public:
		constexpr InstanceType() = default;

		constexpr InstanceType(T instanceIn) 
			: VulkanResource<T>(std::move(instanceIn))
		{ }

		static auto CreateRaw(const VkInstanceCreateInfo& createInfo, bool initializeVolkAndLoadInstance) -> T
		{
			if (initializeVolkAndLoadInstance)
				volkInitialize();
			auto instance = VkInstance{};
			auto result = Volkus::vkx::Result{ vkCreateInstance(&createInfo, nullptr, &instance) };
			if (not result)
				throw VulkanError{ result, "Failed to create Vulkan instance" };
			if (initializeVolkAndLoadInstance)
				volkLoadInstance(instance);
			return T{ instance };
		}

		// Create a Vulkan instance, optionally initializing volk and loading the instance-level entry points.
		static auto Create(const VkInstanceCreateInfo& createInfo, bool initializeVolkAndLoadInstance) -> InstanceType
		{
			if (initializeVolkAndLoadInstance)
				volkInitialize();
			auto instance = VkInstance{};
			auto result = Volkus::vkx::Result{ vkCreateInstance(&createInfo, nullptr, &instance) };
			if (not result)
				throw VulkanError{ result, "Failed to create Vulkan instance" };
			if(initializeVolkAndLoadInstance)
				volkLoadInstance(instance);
			return InstanceType{ InstanceUniquePtr{ instance } };
		}

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
}

export namespace Volkus::vkx
{
	using Instance = InstanceType<InstanceUniquePtr>;
}

namespace
{
	static_assert(
		Volkus::vkx::InstanceLike<Volkus::vkx::InstanceType<Volkus::vkx::InstanceUniquePtr>>, 
		"InstanceType<InstanceUniquePtr> does not satisfy InstanceLike");
}
