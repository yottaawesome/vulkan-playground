export module volkus:vkx.debug;
import std;
import :vkx.exports;
import :vkx.vulkanresource;
import :vkx.error;

namespace Volkus::vkx
{
	class DebugMessengerDeleter
	{
	public:
		DebugMessengerDeleter(VkInstance instance)
			: instance(instance)
		{
			if (not instance)
				throw std::runtime_error{ "Instance handle cannot be null for DebugMessengerDeleter" };
		}
		void operator()(this auto&& self, VkDebugUtilsMessengerEXT debugMessenger) noexcept
		{
			auto fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(self.instance, "vkDestroyDebugUtilsMessengerEXT"));
			// this should be terminal
			if (not fn)
				Fatal("Failed to load vkDestroyDebugUtilsMessengerEXT function pointer. Check if the extension VK_EXT_debug_utils is enabled.");
			fn(self.instance, debugMessenger, nullptr);
		}
	protected:
		VkInstance instance{};

	};
	using DebugMessengerUniquePtr = std::unique_ptr<std::remove_pointer_t<VkDebugUtilsMessengerEXT>, DebugMessengerDeleter>;
}

export namespace Volkus::vkx
{
	// Debug callback is of the following signature:
	/*auto DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) -> VkBool32*/

	class DebugMessenger : public VulkanResource<DebugMessengerUniquePtr>
	{
	public:
		DebugMessenger(DebugMessengerUniquePtr debugMessenger)
			: VulkanResource(std::move(debugMessenger))
		{ }

		static auto CreateDebugMessenger(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator
		) -> DebugMessenger
		{
			if (not instance)
				throw std::runtime_error{ "Instance handle cannot be null for CreateDebugUtilsMessengerEXT" };
			auto fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
			if (not fn)
				throw std::runtime_error("Failed to load vkCreateDebugUtilsMessengerEXT function pointer. Check if the extension VK_EXT_debug_utils is enabled.");

			auto debugMessenger = VkDebugUtilsMessengerEXT{};
			auto result = Result{fn(instance, pCreateInfo, pAllocator, &debugMessenger)};
			if (not result)
				throw VulkanError{ result, "Failed to create debug utils messenger" };

			return DebugMessenger{
				DebugMessengerUniquePtr{debugMessenger, DebugMessengerDeleter{instance}}
			};
		}
	};
}
