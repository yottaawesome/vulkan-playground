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

	struct ExtensionSupport
	{
		std::string_view ExtensionName;
		bool Supported = false;
	};

	struct AllExtensionSupport
	{
		std::vector<ExtensionSupport> Extensions;

		auto AllSupported(
			this const AllExtensionSupport& self
		) noexcept -> bool
		{
			return std::ranges::all_of(self.Extensions, [](const auto& ext) { return ext.Supported; });
		}

		auto ListUnsupportedExtensions(
			this const AllExtensionSupport& self
		) noexcept -> std::vector<std::string_view>
		{
			auto unsupported = std::vector<std::string_view>{};
			for (const auto& ext : self.Extensions)
				if (not ext.Supported)
					unsupported.push_back(ext.ExtensionName);
			return unsupported;
		}
	};

	auto EvaluateExtensionSupport(
		std::vector<const char*> requiredExtensions
	) -> AllExtensionSupport
	{
		auto ext = requiredExtensions
			| std::ranges::views::transform(
				[](const char* ext) { return ExtensionSupport{ std::string_view{ ext }, false }; })
			| std::ranges::to<std::vector>();

		auto supportedExtensions = EnumerateSupportedExtensions();
		for (auto& extSupport : ext)
		{
			extSupport.Supported = std::ranges::any_of(
				supportedExtensions,
				[&extSupport](const auto& prop)
				{
					return extSupport.ExtensionName == std::string_view{ prop.extensionName };
				}
			);
		}
		return { .Extensions=ext };
	}
}