export module vulkangfx:vulkan.surface;
import std;
import :win32;
import :error;
import :vulkan.raii;
import :vulkan.error;
import :vulkan.exports;

export namespace Vulkan
{
	struct SurfaceDeleter
	{
	public:
		SurfaceDeleter(vkr::VkInstance instance)
			: instance{ instance }
		{ 
			if (not instance)
				throw Error::RuntimeError("Vulkan instance handle cannot be null.");
		}
		void operator()(this const SurfaceDeleter& self, vkr::VkSurfaceKHR surface) noexcept
		{
			if (not surface)
				return;
			vkr::vkDestroySurfaceKHR(self.instance, surface, nullptr);
		}
	private:
		vkr::VkInstance instance = nullptr;
	};
	using SurfaceUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkSurfaceKHR>, SurfaceDeleter>;

	struct SurfaceFactory
	{
		Win32::HINSTANCE appInstance = Win32::GetModuleHandleW(nullptr);
		Win32::HWND window = nullptr;
		vkr::VkInstance instance = nullptr;

		auto operator()(this const SurfaceFactory& self) -> SurfaceUniquePtr
		{
			if (not self.appInstance)
				throw Error::RuntimeError("Application instance handle cannot be null.");
			if (not self.window)
				throw Error::RuntimeError("Window handle cannot be null.");
			if (not self.instance)
				throw Error::RuntimeError("Vulkan instance handle cannot be null.");

			auto createInfo = vkr::VkWin32SurfaceCreateInfoKHR{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
				.hinstance = self.appInstance,
				.hwnd = self.window,
			};
			auto surface = vkr::VkSurfaceKHR{};
			auto result = Vulkan::Result{
				vkr::vkCreateWin32SurfaceKHR(self.instance, &createInfo, nullptr, &surface)
			};
			if (not result)
				throw Vulkan::VulkanError{ result, "Failed to create window surface." };
			return SurfaceUniquePtr{ surface, SurfaceDeleter{self.instance} };
		}
	};

	struct Surface
	{
	public:
		Surface(SurfaceUniquePtr surface, vkr::VkPhysicalDevice physicalDevice)
			: surface{ std::move(surface) }, physicalDevice{ physicalDevice }
		{ 
			if (not surface)
				throw Error::RuntimeError("Surface handle cannot be null.");
			if (not physicalDevice)
				throw Error::RuntimeError("Physical device handle cannot be null.");
		}

		auto SupportsPresentation(this const Surface& self, std::uint32_t queueFamilyIndex) -> bool
		{
			auto supported = VkBool32{};
			if (auto result = Result{ vkr::vkGetPhysicalDeviceSurfaceSupportKHR(self.physicalDevice, queueFamilyIndex, self.surface.get(), &supported) })
				throw Vulkan::VulkanError{ result, "Failed to query physical device surface support." };
			return supported;
		}

		auto GetSurfaceCapabilities(this const Surface& self) -> vkr::VkSurfaceCapabilitiesKHR
		{
			auto capabilities = vkr::VkSurfaceCapabilitiesKHR{};
			if (auto result = Result{ vkr::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(self.physicalDevice, self.surface.get(), &capabilities) })
				throw Vulkan::VulkanError{ result, "Failed to query surface capabilities." };
			return capabilities;
		}
		
		auto GetHandle(this const Surface& self) noexcept -> vkr::VkSurfaceKHR
		{
			return self.surface.get();
		}

	private:
		SurfaceUniquePtr surface;
		vkr::VkPhysicalDevice physicalDevice = nullptr;
	};
}
