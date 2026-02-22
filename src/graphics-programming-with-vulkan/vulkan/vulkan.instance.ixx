export module vulkangfx:vulkan.instance;
import std;
import :vulkan.exports;
import :vulkan.error;
import :vulkan.raii;

export namespace Vulkan::Instance
{
	struct AppInfo
	{
		std::string_view ApplicationName = "Graphics Programming with Vulkan and C++";
		std::uint32_t ApplicationVersion = vkr::MakeVersion(1, 0, 0);
		std::string_view EngineName = "Vulkangeance";
		std::uint32_t EngineVersion = vkr::MakeVersion(1, 0, 0);
		std::uint32_t ApiVersion = static_cast<std::uint32_t>(vkr::Versions::Vulkan14);

		auto ToVkApplicationInfo(this const AppInfo& self) noexcept -> vkr::VkApplicationInfo
		{
			return vkr::VkApplicationInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pNext = nullptr,
				.pApplicationName = self.ApplicationName.data(),
				.applicationVersion = self.ApplicationVersion,
				.pEngineName = self.EngineName.data(),
				.engineVersion = self.EngineVersion,
				.apiVersion = self.ApiVersion
			};
		}
	};

	struct InstanceInfo
	{
		vkr::VkInstanceCreateFlags Flags = 0;
		std::vector<const char*> EnabledLayerNames;
		std::vector<const char*> EnabledExtensionNames;
		
		auto ToVkInstanceCreateInfo(
			this const InstanceInfo& self, 
			const vkr::VkApplicationInfo& appInfo
		) noexcept -> vkr::VkInstanceCreateInfo
		{
			return vkr::VkInstanceCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = self.Flags,
				.pApplicationInfo = &appInfo,
				.enabledLayerCount = static_cast<std::uint32_t>(self.EnabledLayerNames.size()),
				.ppEnabledLayerNames = self.EnabledLayerNames.data(),
				.enabledExtensionCount = static_cast<std::uint32_t>(self.EnabledExtensionNames.size()),
				.ppEnabledExtensionNames = self.EnabledExtensionNames.data()
			};
		}
	};

	struct Factory
	{
		AppInfo ApplicationInfo{
			.ApplicationName = "Graphics Programming with Vulkan and C++",
			.ApplicationVersion = vkr::MakeVersion(1, 0, 0),
			.EngineName = "Vulkangeance",
			.EngineVersion = vkr::MakeVersion(1, 0, 0),
			.ApiVersion = static_cast<std::uint32_t>(vkr::Versions::Vulkan14)
		};
		InstanceInfo InstanceInfo{
			.Flags = 0,
		};

		auto operator()(this Factory& self) -> Vulkan::VkInstanceUniquePtr
		{
			// Re-seat required fields to protect against accidental modification.
			auto appInfo = self.ApplicationInfo.ToVkApplicationInfo();
			auto instanceCreateInfo = self.InstanceInfo.ToVkInstanceCreateInfo(appInfo);

			auto instance = vkr::VkInstance{};
			auto result = Vulkan::Result{
				vkr::vkCreateInstance(&instanceCreateInfo, nullptr, &instance)
			};
			if (not result)
				throw Vulkan::VulkanError{ result, "Failed to create Vulkan instance." };
			return Vulkan::VkInstanceUniquePtr{ instance };
		}
	};

	auto EnumerateSupportedExtensions(
		std::string_view layerName = {}
	) -> std::vector<vkr::VkExtensionProperties>
	{
		auto count = std::uint32_t{};
		auto result = Vulkan::Result{
			vkr::vkEnumerateInstanceExtensionProperties(
				layerName.empty() ? nullptr : layerName.data(),
				&count,
				nullptr
			)};
		if (not result)
			throw Vulkan::VulkanError{ result, "Failed to enumerate instance extension properties." };

		auto properties = std::vector<vkr::VkExtensionProperties>{ count };
		result = Vulkan::Result{
			vkr::vkEnumerateInstanceExtensionProperties(
				layerName.empty() ? nullptr : layerName.data(),
				&count,
				properties.data()
			)};
		if (not result)
			throw Vulkan::VulkanError{ result, "Failed to enumerate instance extension properties." };
		return properties;
	}

	struct UnsupportedExtensions
	{
		std::vector<std::string_view> Names;
		auto HasUnsupported(
			this const UnsupportedExtensions& self
		) noexcept -> bool
		{
			return not self.Names.empty();
		}
	};

	auto EvaluateExtensionSupport(
		std::vector<const char*> requiredExtensions
	) -> UnsupportedExtensions
	{
		auto required = requiredExtensions
			| std::ranges::views::transform(
				[](const char* ext) { return std::string_view{ ext }; })
			| std::ranges::to<std::vector>();

		auto supportedExtensions = EnumerateSupportedExtensions();

		auto unsupported = std::vector<std::string_view>{};
		for (auto& extension : required)
		{
			bool supported = std::ranges::any_of(
				supportedExtensions,
				[&extension](const auto& prop)
				{
					return extension == std::string_view{ prop.extensionName };
				}
			);
			if (not supported)
				unsupported.push_back(extension);
		}
		return { .Names = unsupported };
	}

	auto EnumerateInstanceLayers() -> std::vector<vkr::VkLayerProperties>
	{
		auto count = std::uint32_t{};
		auto result = Vulkan::Result{
			vkr::vkEnumerateInstanceLayerProperties(&count, nullptr)
		};
		if (not result)
			throw Vulkan::VulkanError{ result, "Failed to enumerate instance layer properties." };
		auto properties = std::vector<vkr::VkLayerProperties>{ count };
		result = Vulkan::Result{
			vkr::vkEnumerateInstanceLayerProperties(&count, properties.data())
		};
		if (not result)
			throw Vulkan::VulkanError{ result, "Failed to enumerate instance layer properties." };
		return properties;
	}

	auto EvaluateLayerSupport(
		std::vector<const char*> requiredLayers
	) -> std::vector<std::string_view>
	{
		auto supportedLayers = EnumerateInstanceLayers();
		auto unsupported = std::vector<std::string_view>{};
		for (const char* layer : requiredLayers)
		{
			bool supported = std::ranges::any_of(
				supportedLayers,
				[layer](const auto& prop)
				{
					return layer == std::string_view{ prop.layerName };
				}
			);
			if (not supported)
				unsupported.push_back(layer);
		}
		return unsupported;
	}

	using DebugMessengerCallback = auto(
		vkr::VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		vkr::VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const vkr::VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)->vkr::VkBool32;

	class MainInstance
	{
	public:
		MainInstance() = default;

		MainInstance(VkInstanceUniquePtr Handle) : Handle{ std::move(Handle) }
		{}

		template<typename TSignature>
		auto GetInstanceProcAddr(
			this const MainInstance& self,
			const char* pName
		) -> TSignature
		{
			auto fn = reinterpret_cast<TSignature>(vkGetInstanceProcAddr(self.Handle.get(), pName));
			if (not fn)
				throw Vulkan::VulkanError{
					vkr::VkResult::VK_ERROR_EXTENSION_NOT_PRESENT,
					std::format("Failed to load function: {}", pName)
				};
			return fn;
		}

		void DestroyDebugUtilsMessengerEXT(
			this const MainInstance& self,
			vkr::VkDebugUtilsMessengerEXT debugMessenger
		)
		{
			auto fn = self.GetInstanceProcAddr<vkr::PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT");
			fn(self.Handle.get(), debugMessenger, nullptr);
		}

		auto SetupDebugMessenger(
			this MainInstance& self, 
			int severity,
			int types,
			void* userData,
			DebugMessengerCallback callback
		) -> vkr::VkDebugUtilsMessengerEXT
		{
			if (not callback)
				throw std::invalid_argument("Debug messenger callback cannot be null.");

			auto fn = self.GetInstanceProcAddr<vkr::PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT");
			auto debugCreateInfo =
				vkr::VkDebugUtilsMessengerCreateInfoEXT
				{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
					.messageSeverity = static_cast<vkr::VkDebugUtilsMessageSeverityFlagsEXT>(severity),
					.messageType = static_cast<vkr::VkDebugUtilsMessageTypeFlagsEXT>(types),
					.pfnUserCallback = callback,
					.pUserData = userData
				};
			
			auto messenger = vkr::VkDebugUtilsMessengerEXT{};
			auto result = Vulkan::Result{fn(self.Handle.get(), &debugCreateInfo, nullptr, &messenger)};
			if (not result)
				throw Vulkan::VulkanError{ result, "Failed to create debug utils messenger." };
			return messenger;
		}

		auto GetHandle(this const MainInstance& self) noexcept -> VkInstance
		{
			return self.Handle.get();
		}

	private:
		VkInstanceUniquePtr Handle;
	};
}