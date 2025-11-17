export module helloworld:hellotriangleapplication;
import std;
import vulkan;

export namespace Build
{
	constexpr bool IsDebug =
#ifdef _DEBUG
		true;
#else
		false;
#endif
	constexpr bool IsRelease = not IsDebug;
}

//
//
// Various helper functions and structures for the Hello Triangle application
export namespace HelloTriangle
{
	struct QueueFamilyIndices
	{
		std::optional<std::uint32_t> graphicsFamily;
		auto IsComplete(this const auto& self) -> bool { return self.graphicsFamily.has_value(); }
	};

	auto FindQueueFamilies(Vulkan::VkPhysicalDevice device) -> QueueFamilyIndices
	{
		std::uint32_t queueFamilyCount = 0;
		Vulkan::vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<Vulkan::VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		Vulkan::vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		QueueFamilyIndices indices{};
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & Vulkan::VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
				indices = { .graphicsFamily = i };
			if (indices.IsComplete())
				break;
			i++;
		}

		return indices;
	}

	// Extension -- needs to be loaded manually
	auto CreateDebugUtilsMessengerEXT(
		Vulkan::VkInstance instance,
		const Vulkan::VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const Vulkan::VkAllocationCallbacks* pAllocator,
		Vulkan::VkDebugUtilsMessengerEXT* pDebugMessenger
	) -> Vulkan::VkResult
	{
		auto fn = reinterpret_cast<Vulkan::PFN_vkCreateDebugUtilsMessengerEXT>(
			Vulkan::vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
		);
		return fn
			? fn(instance, pCreateInfo, pAllocator, pDebugMessenger)
			: Vulkan::VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	// Extension -- needs to be loaded manually
	void DestroyDebugUtilsMessengerEXT(
		Vulkan::VkInstance instance,
		Vulkan::VkDebugUtilsMessengerEXT debugMessenger,
		const Vulkan::VkAllocationCallbacks* pAllocator
	)
	{
		auto fn = reinterpret_cast<Vulkan::PFN_vkDestroyDebugUtilsMessengerEXT>(
			Vulkan::vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
		);
		fn ? fn(instance, debugMessenger, pAllocator) : void();
	}
}

//
//
// Main application class for the Hello Triangle application
export namespace HelloTriangle
{
	struct Application
	{
		constexpr static int Width = 800;
		constexpr static int Height = 600;
		constexpr static bool EnableValidationLayers = Build::IsDebug;
		constexpr static std::array ValidationLayers{
			"VK_LAYER_KHRONOS_validation"
		};

		void Run(this auto& self)
		{
			self.InitWindow();
			self.InitVulkan();
			self.EnumerateExtensions();
			self.CheckValidationLayerSupport();
			self.MainLoop();
			self.Cleanup();
		}

	private:
		GLFW::GLFWwindow* window = nullptr;
		Vulkan::VkInstance instance;
		Vulkan::VkDebugUtilsMessengerEXT debugMessenger;
		// Destroyed automatically when the instance is destroyed.
		Vulkan::VkPhysicalDevice physicalDevice = nullptr;
		Vulkan::VkDevice device;
		// Implicitly destroyed when the device is destroyed.
		Vulkan::VkQueue graphicsQueue;

		auto CheckValidationLayerSupport(this const auto& self) -> bool
		{
			std::uint32_t layerCount = 0;
			auto result = Vulkan::vkEnumerateInstanceLayerProperties(
				&layerCount,
				nullptr
			);
			if (VkResult::VK_SUCCESS != result)
				throw std::runtime_error("Failed to enumerate instance layer properties count.");

			std::vector<Vulkan::VkLayerProperties> availableLayers(layerCount);
			result = Vulkan::vkEnumerateInstanceLayerProperties(
				&layerCount,
				availableLayers.data()
			);
			if (VkResult::VK_SUCCESS != result)
				throw std::runtime_error("Failed to enumerate instance layer properties count.");

			return std::ranges::all_of(
				ValidationLayers,
				[&availableLayers](std::string_view layerName)
				{
					bool found =
						std::any_of(
							availableLayers.begin(),
							availableLayers.end(),
							[layerName](const Vulkan::VkLayerProperties& layer)
							{
								return layerName == layer.layerName;
							}
						);
					return found
						? true
						: (std::println("Validation layer not found: {}", layerName), false);
				}
			);
		}
		
		void Cleanup(this auto& self)
		{
			Vulkan::vkDestroyDevice(self.device, nullptr);
			if (EnableValidationLayers)
				DestroyDebugUtilsMessengerEXT(self.instance, self.debugMessenger, nullptr);
			Vulkan::vkDestroyInstance(self.instance, nullptr);
			GLFW::glfwDestroyWindow(self.window);
			GLFW::glfwTerminate();
		}

		void CreateInstance(this auto& self)
		{
			if (EnableValidationLayers and not self.CheckValidationLayerSupport())
				throw std::runtime_error("Validation layers requested, but not available");

			Vulkan::VkApplicationInfo appInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName = "Hello Triangle",
				.applicationVersion = Vulkan::MakeVersion(1, 0, 0),
				.pEngineName = "No Engine",
				.engineVersion = Vulkan::MakeVersion(1, 0, 0),
				.apiVersion = Vulkan::ApiVersion1
			};

			auto extensions = self.GetRequiredExtensions();

			Vulkan::VkInstanceCreateInfo createInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pApplicationInfo = &appInfo,
				.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
				.ppEnabledExtensionNames = extensions.data(),
			};

			Vulkan::VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
			if (EnableValidationLayers)
			{
				self.PopulateDebugMessengerCreateInfo(debugCreateInfo);
				createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
				createInfo.ppEnabledLayerNames = ValidationLayers.data();
				createInfo.pNext =
					reinterpret_cast<Vulkan::VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
			}

			Vulkan::VkResult result =
				Vulkan::vkCreateInstance(&createInfo, nullptr, &self.instance);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create instance!");
		}

		void CreateLogicalDevice(this auto& self)
		{
			QueueFamilyIndices indices = FindQueueFamilies(self.physicalDevice);

			Vulkan::VkDeviceQueueCreateInfo queueCreateInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = indices.graphicsFamily.value(),
				.queueCount = 1
			};

			float queuePriority = 1.0f;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			Vulkan::VkPhysicalDeviceFeatures deviceFeatures{};
			Vulkan::VkDeviceCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &queueCreateInfo,
				.enabledExtensionCount = 0,
				.pEnabledFeatures = &deviceFeatures
			};
			if (EnableValidationLayers)
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
				createInfo.ppEnabledLayerNames = ValidationLayers.data();
			}

			auto result = Vulkan::vkCreateDevice(
				self.physicalDevice,
				&createInfo,
				nullptr,
				&self.device
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create logical device.");

			Vulkan::vkGetDeviceQueue(
				self.device,
				indices.graphicsFamily.value(),
				0,
				&self.graphicsQueue
			);
		}

		// stdcall on Windows, but this is ignored on x64.
		static auto DebugCallback(
			Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			Vulkan::VkDebugUtilsMessageTypeFlagsEXT messageType,
			const Vulkan::VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		) -> Vulkan::VkBool32
		{
			if (messageSeverity >= Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			{
				std::println("Validation layer: {}", pCallbackData->pMessage);
			}

			return false;
		}

		void EnumerateExtensions(this const auto& self)
		{
			std::uint32_t extensionsCount = 0;
			auto result =
				Vulkan::vkEnumerateInstanceExtensionProperties(
					nullptr,
					&extensionsCount,
					nullptr
				);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to enumerate extensions count.");

			std::vector<Vulkan::VkExtensionProperties> extensions(extensionsCount);
			result = Vulkan::vkEnumerateInstanceExtensionProperties(
				nullptr,
				&extensionsCount,
				extensions.data()
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to enumerate extensions.");

			for (const auto& extension : extensions)
				std::println("Extension: {}", extension.extensionName);
		}

		auto GetRequiredExtensions(this const auto& self) -> std::vector<const char*>
		{
			std::uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions =
				GLFW::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			std::vector<const char*> extensions(
				glfwExtensions,
				glfwExtensions + glfwExtensionCount
			);
			if (EnableValidationLayers)
				extensions.push_back(Vulkan::DebugUtilExtensionName);
			return extensions;
		}

		void InitVulkan(this auto& self)
		{
			self.CreateInstance();
			self.SetupDebugMessenger();
			self.PickPhysicalDevice();
			self.CreateLogicalDevice();
		}

		void InitWindow(this auto& self)
		{
			GLFW::glfwInit();
			GLFW::glfwWindowHint(GLFW::ClientApi, GLFW::NoApi);
			GLFW::glfwWindowHint(GLFW::Resizable, false);
			self.window =
				GLFW::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
		}

		auto IsDeviceSuitable(this const auto& self, Vulkan::VkPhysicalDevice device) -> bool
		{
			QueueFamilyIndices indices = FindQueueFamilies(device);

			return indices.graphicsFamily.has_value();

			/*Vulkan::VkPhysicalDeviceProperties deviceProperties;
			Vulkan::vkGetPhysicalDeviceProperties(device, &deviceProperties);
			Vulkan::VkPhysicalDeviceFeatures deviceFeatures;
			Vulkan::vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
			return
				deviceProperties.deviceType == Vulkan::VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
				and deviceFeatures.geometryShader;*/
		}

		void MainLoop(this auto& self)
		{
			while (not GLFW::glfwWindowShouldClose(self.window))
			{
				GLFW::glfwPollEvents();
			}
		}

		auto PickPhysicalDevice(this auto& self) -> Vulkan::VkPhysicalDevice
		{
			std::uint32_t deviceCount = 0;
			Vulkan::vkEnumeratePhysicalDevices(self.instance, &deviceCount, nullptr);
			if (deviceCount == 0)
				throw std::runtime_error("Failed to find GPUs with Vulkan support.");

			std::vector<Vulkan::VkPhysicalDevice> devices(deviceCount);
			Vulkan::vkEnumeratePhysicalDevices(self.instance, &deviceCount, devices.data());

			for (const auto& device : devices)
				if (self.IsDeviceSuitable(device))
					return (self.physicalDevice = device, device);
			throw std::runtime_error("Failed to find a suitable GPU.");
		}

		void PopulateDebugMessengerCreateInfo(this auto& self, Vulkan::VkDebugUtilsMessengerCreateInfoEXT& createInfo)
		{
			createInfo = Vulkan::VkDebugUtilsMessengerCreateInfoEXT{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.messageSeverity = // filter messages based on severity
					Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
					| Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
					| Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				.messageType = // filter messages based on type
					Vulkan::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
					| Vulkan::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
					| Vulkan::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				.pfnUserCallback = DebugCallback,
				.pUserData = nullptr // Optional
			};
		}

		void SetupDebugMessenger(this auto& self)
		{
			if (not EnableValidationLayers)
				return;
			Vulkan::VkDebugUtilsMessengerCreateInfoEXT createInfo{};

			self.PopulateDebugMessengerCreateInfo(createInfo);

			auto result = CreateDebugUtilsMessengerEXT(
				self.instance,
				&createInfo,
				nullptr,
				&self.debugMessenger
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to set up debug messenger");
		}
	};
}
