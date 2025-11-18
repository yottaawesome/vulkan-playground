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
		std::optional<std::uint32_t> GraphicsFamily;
		std::optional<std::uint32_t> PresentFamily;
		auto IsComplete(this const auto& self) noexcept -> bool 
		{ 
			return self.GraphicsFamily.has_value()
				and self.PresentFamily.has_value(); 
		}
	};

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

	struct SwapChainSupportDetails
	{
		Vulkan::VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<Vulkan::VkSurfaceFormatKHR> Formats;
		std::vector<Vulkan::VkPresentModeKHR> PresentModes;
	};
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
		constexpr static std::array DeviceExtensions{
			Vulkan::VkHkrSwapchainExtensionName
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
		Vulkan::VkSurfaceKHR surface;
		Vulkan::VkQueue presentQueue;
		Vulkan::VkSwapchainKHR swapChain;
		std::vector<Vulkan::VkImage> swapChainImages;
		Vulkan::VkFormat swapChainImageFormat;
		Vulkan::VkExtent2D swapChainExtent;
		std::vector<Vulkan::VkImageView> swapChainImageViews;

		auto CheckDeviceExtensionSupport(
			this const auto& self, 
			Vulkan::VkPhysicalDevice device
		) noexcept -> bool
		{
			std::uint32_t extensionCount = 0;
			Vulkan::vkEnumerateDeviceExtensionProperties(
				device,
				nullptr,
				&extensionCount,
				nullptr
			);
			std::vector<Vulkan::VkExtensionProperties> availableExtensions(extensionCount);
			Vulkan::vkEnumerateDeviceExtensionProperties(
				device,
				nullptr,
				&extensionCount,
				availableExtensions.data()
			);

			std::set<std::string> requiredExtensions{
				DeviceExtensions.begin(), 
				DeviceExtensions.end()
			};
			for (const auto& extension : availableExtensions)
				requiredExtensions.erase(extension.extensionName);

			return requiredExtensions.empty();
		}

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
		
		auto ChooseSwapExtent(
			this const auto& self,
			const Vulkan::VkSurfaceCapabilitiesKHR& capabilities
		) -> Vulkan::VkExtent2D
		{
			if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
				return capabilities.currentExtent;

			int width; 
			int height;
			GLFW::glfwGetFramebufferSize(self.window, &width, &height);
			Vulkan::VkExtent2D actualExtent{
				.width = static_cast<std::uint32_t>(width),
				.height = static_cast<std::uint32_t>(height)
			};
			actualExtent.width = 
				std::clamp(
					actualExtent.width,
					capabilities.minImageExtent.width,
					capabilities.maxImageExtent.width
				);
			actualExtent.height =
				std::clamp(
					actualExtent.height,
					capabilities.minImageExtent.height,
					capabilities.maxImageExtent.height
				);
			return actualExtent;
		}

		auto ChooseSwapPresentMode(
			this const auto& self,
			const std::vector<Vulkan::VkPresentModeKHR>& availablePresentModes
		) -> Vulkan::VkPresentModeKHR
		{
			for (auto&& mode : availablePresentModes)
				if (mode == Vulkan::VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
					return mode;
			return Vulkan::VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
		}

		auto ChooseSwapSurfaceFormat(
			this const auto& self,
			const std::vector<Vulkan::VkSurfaceFormatKHR>& availableFormats
		) -> Vulkan::VkSurfaceFormatKHR
		{
			for (const auto& availableFormat : availableFormats)
				if (availableFormat.format == Vulkan::VkFormat::VK_FORMAT_B8G8R8A8_SRGB
					and availableFormat.colorSpace == Vulkan::VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
				) return availableFormat;
			return availableFormats[0];
		}

		void Cleanup(this auto& self)
		{
			for (auto imageView : self.swapChainImageViews)
				Vulkan::vkDestroyImageView(self.device, imageView, nullptr);
			Vulkan::vkDestroySwapchainKHR(self.device, self.swapChain, nullptr);
			Vulkan::vkDestroyDevice(self.device, nullptr);
			if (EnableValidationLayers)
				DestroyDebugUtilsMessengerEXT(self.instance, self.debugMessenger, nullptr);
			Vulkan::vkDestroySurfaceKHR(self.instance, self.surface, nullptr);
			Vulkan::vkDestroyInstance(self.instance, nullptr);
			GLFW::glfwDestroyWindow(self.window);
			GLFW::glfwTerminate();
		}

		void CreateImageViews(this auto& self)
		{
			self.swapChainImageViews.resize(self.swapChainImages.size());
			for (size_t i = 0; i < self.swapChainImages.size(); i++)
			{
				Vulkan::VkImageViewCreateInfo createInfo{
					.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.image = self.swapChainImages[i],
					.viewType = Vulkan::VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
					.format = self.swapChainImageFormat,
					.components{
						.r = Vulkan::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
						.g = Vulkan::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
						.b = Vulkan::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
						.a = Vulkan::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY
					},
					.subresourceRange{
						.aspectMask = Vulkan::VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1
					}
				};
				auto result = Vulkan::vkCreateImageView(
					self.device,
					&createInfo,
					nullptr,
					&self.swapChainImageViews[i]
				);
				if (result != Vulkan::VkResult::VK_SUCCESS)
					throw std::runtime_error("Failed to create image views.");
			}
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
			QueueFamilyIndices indices = self.FindQueueFamilies(self.physicalDevice);

			std::vector<Vulkan::VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<std::uint32_t> uniqueQueueFamilies{ 
				indices.GraphicsFamily.value(), 
				indices.PresentFamily.value() 
			};
			float queuePriority = 1.0f;
			for (std::uint32_t queueFamily : uniqueQueueFamilies) 
			{
				Vulkan::VkDeviceQueueCreateInfo queueCreateInfo{};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			Vulkan::VkDeviceQueueCreateInfo queueCreateInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = indices.GraphicsFamily.value(),
				.queueCount = 1
			};
			queueCreateInfo.pQueuePriorities = &queuePriority;

			Vulkan::VkPhysicalDeviceFeatures deviceFeatures{};
			Vulkan::VkDeviceCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size()),
				.pQueueCreateInfos = queueCreateInfos.data(),
				.enabledExtensionCount = static_cast<std::uint32_t>(DeviceExtensions.size()),
				.ppEnabledExtensionNames = DeviceExtensions.data(),
				.pEnabledFeatures = &deviceFeatures,
			};
			if (EnableValidationLayers)
			{
				createInfo.enabledLayerCount = static_cast<std::uint32_t>(ValidationLayers.size());
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
				indices.GraphicsFamily.value(),
				0,
				&self.graphicsQueue
			);
			Vulkan::vkGetDeviceQueue(
				self.device, 
				indices.PresentFamily.value(), 
				0, 
				&self.presentQueue
			);
		}

		void CreateSurface(this auto& self)
		{
			Vulkan::VkWin32SurfaceCreateInfoKHR createInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
				.hinstance = Win32::GetModuleHandleW(nullptr),
				.hwnd = GLFW::glfwGetWin32Window(self.window),
			};

			auto result = Vulkan::vkCreateWin32SurfaceKHR(
				self.instance,
				&createInfo,
				nullptr,
				&self.surface
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create window surface.");
		}

		void CreateSwapChain(this auto& self)
		{
			SwapChainSupportDetails swapChainSupport = self.QuerySwapChainSupport(self.physicalDevice);

			Vulkan::VkSurfaceFormatKHR surfaceFormat =
				self.ChooseSwapSurfaceFormat(swapChainSupport.Formats);
			Vulkan::VkPresentModeKHR presentMode =
				self.ChooseSwapPresentMode(swapChainSupport.PresentModes);
			Vulkan::VkExtent2D extent = self.ChooseSwapExtent(swapChainSupport.Capabilities);

			std::uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
			if (
				swapChainSupport.Capabilities.maxImageCount > 0
				and imageCount > swapChainSupport.Capabilities.maxImageCount
			) imageCount = swapChainSupport.Capabilities.maxImageCount;

			Vulkan::VkSwapchainCreateInfoKHR createInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.surface = self.surface,
				.minImageCount = imageCount,
				.imageFormat = surfaceFormat.format,
				.imageColorSpace = surfaceFormat.colorSpace,
				.imageExtent = extent,
				.imageArrayLayers = 1,
				.imageUsage = Vulkan::VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			};

			QueueFamilyIndices indices = self.FindQueueFamilies(self.physicalDevice);
			std::uint32_t queueFamilyIndices[]{
				indices.GraphicsFamily.value(),
				indices.PresentFamily.value()
			};

			if (indices.GraphicsFamily != indices.PresentFamily)
			{
				createInfo.imageSharingMode = Vulkan::VkSharingMode::VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
			{
				createInfo.imageSharingMode = Vulkan::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 0;
				createInfo.pQueueFamilyIndices = nullptr;
			}
			createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
			createInfo.compositeAlpha = Vulkan::VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = true;
			createInfo.oldSwapchain = nullptr;

			auto result = Vulkan::vkCreateSwapchainKHR(
				self.device,
				&createInfo,
				nullptr,
				&self.swapChain
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create swap chain.");

			std::uint32_t actualImageCount;
			Vulkan::vkGetSwapchainImagesKHR(
				self.device, 
				self.swapChain, 
				&actualImageCount,
				nullptr
			);
			self.swapChainImages.resize(actualImageCount);
			Vulkan::vkGetSwapchainImagesKHR(
				self.device, 
				self.swapChain, 
				&actualImageCount,
				self.swapChainImages.data()
			);

			self.swapChainImageFormat = surfaceFormat.format;
			self.swapChainExtent = extent;
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

		auto FindQueueFamilies(this auto& self, Vulkan::VkPhysicalDevice device) -> QueueFamilyIndices
		{
			std::uint32_t queueFamilyCount = 0;
			Vulkan::vkGetPhysicalDeviceQueueFamilyProperties(
				device, 
				&queueFamilyCount, 
				nullptr
			);

			std::vector<Vulkan::VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			Vulkan::vkGetPhysicalDeviceQueueFamilyProperties(
				device, 
				&queueFamilyCount, 
				queueFamilies.data()
			);

			int i = 0;
			QueueFamilyIndices indices{};
			for (const auto& queueFamily : queueFamilies)
			{
				if (queueFamily.queueFlags & Vulkan::VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
					indices = { .GraphicsFamily = i };

				VkBool32 presentSupport = false;
				Vulkan::vkGetPhysicalDeviceSurfaceSupportKHR(
					device, 
					i, 
					self.surface, 
					&presentSupport
				);
				if (presentSupport)
					indices.PresentFamily = i;

				if (indices.IsComplete())
					break;
				i++;
			}

			return indices;
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
			self.CreateSurface();
			self.PickPhysicalDevice();
			self.CreateLogicalDevice();
			self.CreateSwapChain();
			self.CreateImageViews();
		}

		void InitWindow(this auto& self)
		{
			GLFW::glfwInit();
			GLFW::glfwWindowHint(GLFW::ClientApi, GLFW::NoApi);
			GLFW::glfwWindowHint(GLFW::Resizable, false);
			self.window =
				GLFW::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
		}

		auto IsDeviceSuitable(
			this const auto& self, 
			Vulkan::VkPhysicalDevice device
		) -> bool
		{
			QueueFamilyIndices indices = self.FindQueueFamilies(device);
			bool extensionsSupported = self.CheckDeviceExtensionSupport(device);
			bool swapChainAdequate = 
				[&] -> bool
				{
					if (not extensionsSupported)
						return false;
					SwapChainSupportDetails swapChainSupport =
						self.QuerySwapChainSupport(device);
					return not swapChainSupport.Formats.empty()
						and not swapChainSupport.PresentModes.empty();
				}();
			
			return indices.IsComplete()
				and extensionsSupported 
				and swapChainAdequate;
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

		void PopulateDebugMessengerCreateInfo(
			this auto& self, 
			Vulkan::VkDebugUtilsMessengerCreateInfoEXT& createInfo
		)
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

		auto QuerySwapChainSupport(
			this const auto& self,
			Vulkan::VkPhysicalDevice device
		) -> SwapChainSupportDetails
		{
			SwapChainSupportDetails details;
			Vulkan::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				device,
				self.surface,
				&details.Capabilities
			);

			std::uint32_t formatCount;
			Vulkan::vkGetPhysicalDeviceSurfaceFormatsKHR(
				device,
				self.surface,
				&formatCount,
				details.Formats.data()
			);
			if (formatCount != 0)
			{
				details.Formats.resize(formatCount);
				Vulkan::vkGetPhysicalDeviceSurfaceFormatsKHR(
					device,
					self.surface,
					&formatCount,
					details.Formats.data()
				);
			}

			std::uint32_t presentModeCount;
			Vulkan::vkGetPhysicalDeviceSurfacePresentModesKHR(
				device, 
				self.surface, 
				&presentModeCount, 
				nullptr
			);
			if (presentModeCount != 0)
			{
				details.PresentModes.resize(presentModeCount);
				Vulkan::vkGetPhysicalDeviceSurfacePresentModesKHR(
					device,
					self.surface,
					&presentModeCount,
					details.PresentModes.data()
				);
			}

			return details;
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
