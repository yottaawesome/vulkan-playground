export module vulkan26:vulkan.surface;
import std;
import :error;
import :vulkan.exports;
import :vulkan.error;

export namespace vk
{
	class SurfaceDeleter
	{
	public:
		SurfaceDeleter(VkInstance instanceIn)
			: instance(instanceIn)
		{
			if (not instance)
				throw ::Error::RuntimeError{ "Instance must not be null." };
		}

		void operator()(this const auto& self, VkSurfaceKHR surface) noexcept
		{
			vkDestroySurfaceKHR(self.instance, surface, nullptr);
		}

		constexpr auto GetInstance(this const auto& self) noexcept -> VkInstance
		{
			return self.instance;
		}
	private:
		VkInstance instance = nullptr;
	};
	using SurfaceUniquePtr = std::unique_ptr<std::remove_pointer_t<VkSurfaceKHR>, SurfaceDeleter>;

	class Surface 
	{
	public:
		static auto Create(
			VkInstance instance, 
			VkPhysicalDevice physicalDevice, 
			VkSurfaceKHR surface
		) -> Surface
		{
			if (not instance)
				throw ::Error::RuntimeError{ "Instance must not be null." };
			if (not surface)
				throw ::Error::RuntimeError{ "Surface must not be null." };
			if (not physicalDevice)
				throw ::Error::RuntimeError{ "Physical device must not be null." };
			return Surface{ SurfaceUniquePtr{ surface, SurfaceDeleter{instance} }, physicalDevice };
		}

		constexpr auto Get(this const auto& self) noexcept -> VkSurfaceKHR
		{
			return self.surface.get();
		}

		auto GetSurfaceCapabilities(this const auto& self) -> VkSurfaceCapabilitiesKHR
		{
			auto surfaceCapabilities = VkSurfaceCapabilitiesKHR{};
			auto result = Result{ vkGetPhysicalDeviceSurfaceCapabilitiesKHR(self.physicalDevice, self.surface.get(), &surfaceCapabilities) };
			if (not result)
				throw Error{ result.result };
			return surfaceCapabilities;
		}

	private:
		Surface(SurfaceUniquePtr surfaceIn, VkPhysicalDevice physicalDeviceIn)
			: surface(std::move(surfaceIn))
			, physicalDevice(physicalDeviceIn)
		{}

		SurfaceUniquePtr surface;
		VkPhysicalDevice physicalDevice = nullptr;
	};
}