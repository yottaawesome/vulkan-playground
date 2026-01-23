export module vulkantutorial:app;
import std;
import :libs;
import :error;
import :util;
import :vulkanite;

export namespace VulkanTutorial::App
{
	constexpr auto Width = std::uint32_t{ 800 };
	constexpr auto Height = std::uint32_t{ 600 };

	class MainApp
	{
	public:
		constexpr MainApp() = default;
		// Not copyable or movable
		MainApp(const MainApp&) = delete;
		auto operator=(this MainApp&, const MainApp&) -> MainApp& = delete;

		void Run(this MainApp& self)
		{
			self.InitWindow()
				.InitVulkan()
				.MainLoop()
				.Cleanup();
		}

	private: // Private fields
		glfw::GLFWwindow* window;
		vk::raii::Context context;
		vk::raii::Instance instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
		Vulkanite::Device::PhysicalDevice physicalDevice;
		Vulkanite::Device::LogicalDevice device = nullptr;
		vk::raii::SurfaceKHR surface = nullptr;
		constexpr static auto deviceExtensions = std::array{ vk::KHRSwapchainExtensionName };
		vk::raii::Queue presentQueue = nullptr;
		vk::raii::Queue graphicsQueue = nullptr;

	private: // Core internal initialisation methods.
		// The first step is to initialise the GLFW window.
		auto InitWindow(this MainApp& self) -> MainApp&
		{
			glfw::glfwInit();
			glfw::glfwWindowHint(glfw::ClientApi, glfw::NoApi);
			glfw::glfwWindowHint(glfw::Resizable, false);
			self.window = glfw::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
			return self;
		}

		// We then need to initialise our connection 
		// to Vulkan by creating a Vulkan instance.
		auto InitVulkan(this MainApp& self) -> MainApp&
		{
			self.CreateInstance();
			self.SetupDebugMessenger();
			self.CreateSurface();
			self.PickPhysicalDevice();
			self.CreateLogicalDevice();
			return self;
		}

		auto MainLoop(this MainApp& self) -> MainApp&
		{
			while (not glfw::glfwWindowShouldClose(self.window))
			{
				glfw::glfwPollEvents();
			}
			return self;
		}

	private: // Internal methods.
		void CreateSurface(this MainApp& self)
		{
			auto surface = Vulkan::VkSurfaceKHR{};
			auto result = vk::Result{ glfw::glfwCreateWindowSurface(*self.instance, self.window, nullptr, &surface) };
			if (result != vk::Result::eSuccess)
				throw Error::VulkanError("Failed to create window surface.");
			self.surface = vk::raii::SurfaceKHR{ self.instance, surface };
		}

		void CreateLogicalDevice(this MainApp& self)
		{
			auto graphicsIndex = std::optional{ self.physicalDevice.FindGraphicsQueueFamilyIndex() };
			if (not graphicsIndex)
				throw Error::VulkanError("Failed to find graphics queue family index.");
			
			auto presentIndex = std::optional{ self.physicalDevice.FindPresentQueueFamilyIndexForSurface(self.surface) };
			if (not presentIndex)
				throw Error::VulkanError("Failed to find present queue family index.");

			// The tutorial is weird and uses a structure chain in
			// one page https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/04_Logical_device_and_queues.html
			// and then forgets about it in a subsequent link: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/00_Window_surface.html.
			using StructureChain = vk::StructureChain<
				vk::PhysicalDeviceFeatures2,
				vk::PhysicalDeviceVulkan13Features,
				vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
			>;
			auto featureChain = StructureChain{
				{},
				{.dynamicRendering = true},
				{.extendedDynamicState = true}
			};

			auto queuePriority = float{ 0.5f };
			auto deviceQueueCreateInfo = vk::DeviceQueueCreateInfo{
				.queueFamilyIndex = *graphicsIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			};
			// Previous versions of Vulkan made a distinction between
			// instance and device validation layers, but this is no 
			// longer the case, and the associated members of the 
			// struct are now ignored by newer implementations.
			auto deviceCreateInfo = vk::DeviceCreateInfo{
				.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &deviceQueueCreateInfo,
				.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size()),
				.ppEnabledExtensionNames = deviceExtensions.data()
			};
			self.device = 
				self.physicalDevice.CreateLogicalDevice(deviceCreateInfo);
			self.graphicsQueue = self.device.GetQueue(*graphicsIndex, 0);
			self.presentQueue = self.device.GetQueue(*presentIndex, 0);
		}

		void PickPhysicalDevice(this MainApp& self)
		{
			auto physicalDevices = self.instance.enumeratePhysicalDevices();
			if (physicalDevices.empty())
				throw Error::VulkanError("Failed to find GPUs with Vulkan support.");

			auto deviceList = Vulkanite::Device::PhysicalDeviceList{ physicalDevices };
			deviceList.FilterUnsupportedDevices();
			std::println("{}", deviceList);
			
			if (std::optional supported = deviceList.FirstSupportedDevice(); 
				supported)
			{
				auto bestDevice = Vulkanite::Device::ScoredPhysicalDevice{ *std::move(supported) };
				std::println("Selected physical device: {}", bestDevice);
				self.physicalDevice = std::move(bestDevice).ToGraphicsProcessingUnit();
			}
			else
			{
				throw Error::VulkanError("Failed to find a suitable GPU.");
			}
		}

		void Cleanup(this MainApp& self)
		{
			glfw::glfwDestroyWindow(self.window);
			glfw::glfwTerminate();
		}

		void PrintSupportedExtensions(this const MainApp& self)
		{
			auto availableExtensions =
				std::vector<vk::ExtensionProperties>{ self.context.enumerateInstanceExtensionProperties() };
			std::println("Available Vulkan extensions ({}):", availableExtensions.size());
			for (const auto& ext : availableExtensions)
				std::println(" -> {}", std::string_view{ ext.extensionName });
		}

		auto GetRequiredVulkanExtensions(this const MainApp& self) -> std::vector<std::string>
		{
			// We need glfw to tell us what extensions to use.
			auto glfwExtensionCount = std::uint32_t{ 0 };
			auto glfwExtensions = glfw::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			// Is the extension requested by glfw supported by vulkan?
			auto supportedExtensions = self.context.enumerateInstanceExtensionProperties();
			// Treat argv as a span of const char*
			auto requiredExtensions =
				std::span<const char* const>{ glfwExtensions, glfwExtensionCount }
			| std::ranges::to<std::vector<std::string>>();
			if (Util::EnableValidationLayers)
				requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
			for (const std::string_view extName : requiredExtensions)
			{
				bool supported = std::ranges::any_of(
					supportedExtensions,
					[extName](const vk::ExtensionProperties& extProp)
					{
						return extName == extProp.extensionName;
					}
				);
				if (not supported)
					throw Error::VulkanError(std::format("Required extension not supported: {}", extName));
			}
			return requiredExtensions;
		}

		static auto DebugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
			vk::DebugUtilsMessageTypeFlagsEXT type,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void*
		) -> vk::Bool32
		{
			std::println("A");
			if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
				std::println("Validation layer: {}", pCallbackData->pMessage);
			return false;
		}

		auto GetRequiredVulkanLayers(this MainApp& self) -> std::vector<std::string>
		{
			auto requiredLayers = std::vector<std::string>{
				//"VK_LAYER_LUNARG_api_dump" 
			};
			if constexpr (Util::EnableValidationLayers)
			{
				requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
				//layers.push_back("VK_LAYER_LUNARG_api_dump");
			}

			auto layerProperties = self.context.enumerateInstanceLayerProperties();
			for (std::string_view requiredLayer : requiredLayers)
			{
				bool supported = std::ranges::any_of(
					layerProperties,
					[requiredLayer](const vk::LayerProperties& layerProp)
					{
						return requiredLayer == layerProp.layerName;
					}
				);
				if (not supported)
					throw Error::VulkanError(std::format("Required layer not supported: {}", requiredLayer));
			}

			return requiredLayers;
		}

		void CreateInstance(this MainApp& self)
		{
			self.PrintSupportedExtensions();

			// Check required layers are supported.
			auto requiredLayers = self.GetRequiredVulkanLayers();
			auto requiredExtensions = self.GetRequiredVulkanExtensions();
			// Transform required layers and extensions to const char* arrays
			// for InstanceCreateInfo().
			auto layersList = requiredLayers
				| std::ranges::views::transform([](const std::string& layer) { return layer.c_str(); })
				| std::ranges::to<std::vector<const char*>>();
			auto extensionList = requiredExtensions
				| std::ranges::views::transform([](auto&& s) { return s.c_str(); })
				| std::ranges::to<std::vector<const char*>>();

			// Create the Vulkan instance
			constexpr auto appInfo = vk::ApplicationInfo{
				.pApplicationName = "Hello Triangle",
				.applicationVersion = vk::MakeVersion(1, 0, 0),
				.pEngineName = "No Engine",
				.engineVersion = vk::MakeVersion(1, 0, 0),
				.apiVersion = vk::ApiVersion14
			};
			auto createInfo = vk::InstanceCreateInfo{
				.pApplicationInfo = &appInfo,
				.enabledLayerCount = static_cast<std::uint32_t>(layersList.size()),
				.ppEnabledLayerNames = layersList.data(),
				.enabledExtensionCount = static_cast<std::uint32_t>(extensionList.size()),
				.ppEnabledExtensionNames = extensionList.data(),
			};
			self.instance = vk::raii::Instance(self.context, createInfo);
		}

		void SetupDebugMessenger(this MainApp& self)
		{
			if (not Util::EnableValidationLayers)
				return;

			constexpr auto ToUint32 =
				[](auto...v) static constexpr noexcept -> std::uint32_t
				{
					return (static_cast<std::uint32_t>(v) | ...);
				};
			using Severities = vk::DebugUtilsMessageSeverityFlagBitsEXT;
			auto severities = 
				ToUint32(Severities::eVerbose, Severities::eWarning, Severities::eError);
			auto severityFlags = vk::DebugUtilsMessageSeverityFlagsEXT(severities);

			using MessageTypes = vk::DebugUtilsMessageTypeFlagBitsEXT;
			auto messageTypes = 
				ToUint32(MessageTypes::eGeneral, MessageTypes::ePerformance, MessageTypes::eValidation);
			auto messageTypeFlags = vk::DebugUtilsMessageTypeFlagsEXT{ messageTypes };

			constexpr auto LambdaDebugCallback = 
				[](
					vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
					vk::DebugUtilsMessageTypeFlagsEXT type,
					const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
					void* userData
				) static -> vk::Bool32
				{
					if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
						std::println("Validation layer: {}", callbackData->pMessage);
					return false;
				};

			auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT{
				.messageSeverity = severityFlags,
				.messageType = messageTypeFlags,
				.pfnUserCallback = &self.DebugCallback,
				.pUserData = reinterpret_cast<void**>(&self)
			};
			self.debugMessenger = self.instance.createDebugUtilsMessengerEXT(createInfo);
		}
	};
}
