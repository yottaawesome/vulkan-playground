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
			auto applicationInfo = vkr::VkApplicationInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pNext = nullptr,
				.pApplicationName = "Graphics Programming with Vulkan and C++",
				.applicationVersion = vkr::MakeVersion(1, 0, 0),
				.pEngineName = "Vulkangeance",
				.engineVersion = vkr::MakeVersion(1, 0, 0),
				.apiVersion = static_cast<std::uint32_t>(vkr::Versions::Vulkan14)
			};

			auto extensions = gsl::span<gsl::czstring>{glfw::GetRequiredVulkanExtensions()};
			auto createInfo = vkr::VkInstanceCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.pApplicationInfo = &applicationInfo,
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
				.ppEnabledExtensionNames = extensions.data()
			};
			auto result = Vulkan::Result{
				vkr::vkCreateInstance(&createInfo, nullptr, std::out_ptr(self.instance))
			};
			if (not result)
				throw Vulkan::VulkanError{result, "Failed to create Vulkan instance."};

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