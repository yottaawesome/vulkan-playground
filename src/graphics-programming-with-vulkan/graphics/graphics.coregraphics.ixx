export module vulkangfx:graphics.coregraphics;
import std;
import :vulkan;
import :glfw;

export namespace Graphics
{
	class CoreVulkan
	{
	public:
		CoreVulkan(glfw::Window* window)
			: Window(window)
		{
			if (not window)
				throw std::runtime_error("GLFW window pointer cannot be null.");
		}

		void Initialize(this CoreVulkan& self)
		{
			self.InitialiseInstance();
		}

	private:
		auto InitialiseInstance(this CoreVulkan& self) -> decltype(self)
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

		auto CreateSurface(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto CreateSynchronizationPrimitives(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto DescribeGraphicsPipeline(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto DrawFrame(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto FlushCommands(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto CreateCommandBuffers(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto CreateSwapChain(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

	private:
		vulkan::VkInstance Instance = nullptr;
		glfw::Window* Window = nullptr;
	};
}