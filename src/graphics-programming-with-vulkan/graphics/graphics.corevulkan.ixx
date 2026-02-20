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
		CoreVulkan(glfw::Window* window)
			: window(window ? window : throw std::runtime_error("GLFW window pointer cannot be null."))
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
		auto CreateInstance(this CoreVulkan& self) -> decltype(self)
		{
			auto rawRequiredExtensions = gsl::span<gsl::czstring>{ glfw::GetRequiredVulkanExtensions() };
			auto requiredExtensions = std::vector<const char*>{ rawRequiredExtensions.begin(), rawRequiredExtensions.end() };

			auto extensionSupport = Vulkan::Instance::EvaluateExtensionSupport(requiredExtensions);
			if (not extensionSupport.AllSupported())
			{
				auto unsupported = extensionSupport.ListUnsupportedExtensions();
				auto message = std::format(
					"Not all required Vulkan extensions are supported. Unsupported extensions: {}",
					std::ranges::views::join_with(unsupported, ", ") | std::ranges::to<std::string>()
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
					.EnabledExtensionNames = requiredExtensions,
				}
			};
			self.instance = instanceFactory();
			return self;
		}

		auto AddDebugMessenger(this CoreVulkan& self) -> decltype(self)
		{
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

	private:
		auto Cleanup(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

	private:
		Vulkan::VkInstanceUniquePtr instance;
		glfw::Window* window = nullptr;
	};
}