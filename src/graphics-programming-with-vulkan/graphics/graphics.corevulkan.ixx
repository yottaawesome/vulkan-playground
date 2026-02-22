export module vulkangfx:graphics.corevulkan;
import std;
import :vulkan;
import :glfw;
import :gsl;

export namespace Graphics
{
	class CoreVulkan
	{
	public:
		~CoreVulkan()
		{
			Teardown();
		}

		explicit CoreVulkan(gsl::not_null<glfw::Window*> window)	
			: window(window)
		{ }		

		void Initialise(this CoreVulkan& self)
		{
			self.CreateInstance()
				.AddDebugMessenger()
				.PickPhysicalDevice()
				.CreateSurface()
				.CreateLogicalDevice()
				.CreateSwapChain()
				.CreateImageViews()
				.CreateSyncObjects()
				.DescribeGraphicsPipeline()
				.CreateCommandPool()
				.CreateCommandBuffers();
		}

		// Order of initialisation.
	private:
		static auto GetRequiredExtensions() -> std::vector<const char*>
		{
			auto rawRequiredExtensions = gsl::span<gsl::czstring>{ glfw::GetRequiredVulkanExtensions() };
			auto vector = std::vector<const char*>{ rawRequiredExtensions.begin(), rawRequiredExtensions.end() };
			vector.push_back(vkr::Extensions::EXTDebugUtilsExtensionName);
			return vector;
		}

		static auto GetRequiredLayers() -> std::vector<const char*>
		{
			constexpr bool enableValidationLayers = true;
			auto layers = std::vector<const char*>{};
			if constexpr (enableValidationLayers)
				layers.push_back(vkr::Layers::KhronosValidationLayerName);
			return layers;
		}

		auto CreateInstance(this CoreVulkan& self) -> decltype(self)
		{
			auto requiredExtensions = std::vector{ self.GetRequiredExtensions() };
			auto requiredLayers = std::vector{ self.GetRequiredLayers() };

			auto extensionSupport = Vulkan::Instance::EvaluateExtensionSupport(requiredExtensions);
			if (extensionSupport.HasUnsupported())
			{
				auto message = std::format(
					"Not all required Vulkan extensions are supported. Unsupported extensions: {}",
					std::ranges::views::join_with(extensionSupport.Names, ", ") | std::ranges::to<std::string>()
				);
				throw std::runtime_error(message);
			}

			auto layerSupport = Vulkan::Instance::EvaluateLayerSupport(requiredLayers);
			if (not layerSupport.empty())
			{
				auto message = std::format(
					"Not all required Vulkan layers are supported. Unsupported layers: {}",
					std::ranges::views::join_with(layerSupport, ", ") | std::ranges::to<std::string>()
				);
				throw std::runtime_error(message);
			}

			auto instanceFactory = Vulkan::Instance::Factory{
				.ApplicationInfo = {
					.ApplicationName = "Graphics Programming with Vulkan and C++",
					.ApplicationVersion = vkr::MakeVersion(1, 0, 0),
					.EngineName = "Vulkangeance",
					.EngineVersion = vkr::MakeVersion(1, 0, 0),
					.ApiVersion = static_cast<std::uint32_t>(vkr::Versions::Vulkan14)
				},
				.InstanceInfo = {
					.Flags = 0,
					.EnabledLayerNames = requiredLayers,
					.EnabledExtensionNames = requiredExtensions,
				}
			};
			self.instance = instanceFactory();
			return self;
		}

		auto AddDebugMessenger(this CoreVulkan& self) -> decltype(self)
		{
			constexpr auto severity =
				vkr::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				vkr::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				vkr::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			constexpr auto types =
				vkr::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				vkr::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				vkr::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			constexpr auto callback =
				 [](
					vkr::VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
					vkr::VkDebugUtilsMessageTypeFlagsEXT messageTypes [[maybe_unused]],
					const vkr::VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
					void* pUserData [[maybe_unused]]
				) static -> vkr::VkBool32
				{
					if (messageSeverity >= vkr::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
						std::cerr << std::format("Validation layer: {}\n", pCallbackData->pMessage);
					return vkr::False;
				};
			self.debugMessenger = self.instance.SetupDebugMessenger(severity, types, &self, callback);
			return self;
		}

		auto CreateSurface(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto PickPhysicalDevice(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto CreateLogicalDevice(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto CreateSwapChain(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto CreateImageViews(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto CreateSyncObjects(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto DescribeGraphicsPipeline(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto CreateCommandPool(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto CreateCommandBuffers(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

	private:
		auto FlushCommands(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto DrawFrame(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto Teardown(this CoreVulkan& self) -> decltype(self)
		{
			self.instance.DestroyDebugUtilsMessengerEXT(self.debugMessenger);
			return self;
		}

	private:
		Vulkan::Instance::MainInstance instance;
		glfw::Window* window = nullptr;
		vkr::VkDebugUtilsMessengerEXT debugMessenger = nullptr;
	};
}