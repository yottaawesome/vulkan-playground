export module volkus:vkx.debug;
import std;
import vulkanlib;
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

	auto CreateDebugMessengerPtr(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator
	) -> DebugMessengerUniquePtr
	{
		if (not instance)
			throw std::runtime_error{ "Instance handle cannot be null for CreateDebugUtilsMessengerEXT" };
		auto fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (not fn)
			throw std::runtime_error("Failed to load vkCreateDebugUtilsMessengerEXT function pointer. Check if the extension VK_EXT_debug_utils is enabled.");

		auto debugMessenger = VkDebugUtilsMessengerEXT{};
		auto result = Result{ fn(instance, pCreateInfo, pAllocator, &debugMessenger) };
		if (not result)
			throw VulkanError{ result, "Failed to create debug utils messenger" };
		return DebugMessengerUniquePtr{ debugMessenger, DebugMessengerDeleter{instance} };
	}

	class DebugMessenger : public VulkanResource<DebugMessengerUniquePtr>
	{
	public:
		DebugMessenger(DebugMessengerUniquePtr debugMessenger)
			: VulkanResource(std::move(debugMessenger))
		{ }

		static auto DefaultCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		) -> VkBool32
		{
			if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
				std::println("Validation layer: {}", pCallbackData->pMessage);
			return VkFalse;
		}
	};

	// Debug callback is of the following signature:
	using DebugCallbackSignature = auto(*)(
		VkDebugUtilsMessageSeverityFlagBitsEXT,
		VkDebugUtilsMessageTypeFlagsEXT,
		const VkDebugUtilsMessengerCallbackDataEXT*,
		void*
	)->VkBool32;

	template<typename T>
	concept DebugMessengerFactoryLike = requires(T t)
	{
		{ t.GetSeverity() } -> std::convertible_to<VkDebugUtilsMessageTypeFlagsEXT>;
		{ t.GetMessageType() } -> std::convertible_to<VkDebugUtilsMessageTypeFlagsEXT>;
		{ t.GetCallback() } -> std::convertible_to<DebugCallbackSignature>;
	};

	struct DebugMessengerFactory
	{
		[[nodiscard]]
		auto operator()(this  auto&& self, VkInstance instance) -> DebugMessenger
		{
			auto severity = VkDebugUtilsMessageSeverityFlagsEXT{ self.GetSeverity() };
			auto types = VkDebugUtilsMessageTypeFlagsEXT{ self.GetMessageType() };
			auto callback = self.GetCallback();
			auto debugInfo =
				VkDebugUtilsMessengerCreateInfoEXT{
					.sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
					.pNext = nullptr,
					.flags = 0,
					.messageSeverity = severity,
					.messageType = types,
					.pfnUserCallback = callback,
					.pUserData = nullptr
				};
			return DebugMessenger{ CreateDebugMessengerPtr(instance, &debugInfo, nullptr) };
		}
	};
}
