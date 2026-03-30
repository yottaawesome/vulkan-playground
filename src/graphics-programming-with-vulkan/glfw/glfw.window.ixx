export module vulkangfx:glfw.window;
import std;
import :error;
import :gsl;
import :glfw.exports;
import :glfw.raii;
import :glfw.error;

// Note that glfw windows have two coordinate systems: virtual screen and content area.
// Both use the same units, which is screen coordinates, which is not necessarily
// pixels. The virtual screen is the entire area of the monitor(s), while the content 
// area is the area of the window that can be used for rendering. The positivbe y-axis 
// is from the top-left and extends downwards, while the x-axis is from the top-left 
// and extends rightwards.
// See https://www.glfw.org/docs/3.3/intro_guide.html#coordinate_systems.
// https://www.glfw.org/docs/3.3/window_guide.html#window_creation
export namespace glfw
{
	struct ScreenCoordinates
	{
		int X = 0;
		int Y = 0;
	};
	struct PixelCoordinates
	{
		int X = 0;
		int Y = 0;
	};
	struct ScreenDimensions
	{
		int Width = 0;
		int Height = 0;
	};

	template<typename T>
	struct IsPair : std::false_type {};
	template<typename A, typename B>
	struct IsPair<std::pair<A, B>> : std::true_type {};
	template<typename T>
	concept Pair = IsPair<T>::value;

	template<typename T>
	struct IsIntPair : std::false_type {};
	template<>
	struct IsIntPair<std::pair<int, int>> : std::true_type {};
	template<typename T>
	concept IntPair = IsIntPair<T>::value;

	class Window
	{
	public:
		struct Factory
		{
			std::string Title = "Vulkan";
			std::int32_t ClientApi = glfw::WindowHints::NoApi;
			std::int32_t Width = 800;
			std::int32_t Height = 600;
			bool Resizable = false;

			auto Create(this const Factory& self) -> Window
			{
				glfw::glfwWindowHint(glfw::WindowHints::ClientApi, self.ClientApi);
				glfw::glfwWindowHint(glfw::WindowHints::Resizable, self.Resizable);
				auto window = glfw::glfwCreateWindow(self.Width, self.Height, self.Title.c_str(), nullptr, nullptr);
				if (not window)
					throw Error("Failed to create GLFW window");
				return Window{GlfwWindowUniquePtr{ window }};
			}

			auto operator()(this const Factory& self) -> Window
			{
				return self.Create();
			}
		};

		using WindowHints = std::pair<std::int32_t, std::int32_t>;

		std::function<void(int width, int height)> OnFramebufferResize = [](int, int) {};

		Window(const std::string& title, int width, int height, bool resizable, IntPair auto&&... hints)
		{
			(static_cast<void>(glfw::glfwWindowHint(hints.first, hints.second)), ...);
			auto window = glfw::glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
			if (not window)
				throw Error("Failed to create GLFW window");
			window = GlfwWindowUniquePtr{ window };
		}

		Window(GlfwWindowUniquePtr windowIn) 
			: window(std::move(windowIn))
		{
			if (not window)
				throw ::Error::RuntimeError{ "Window pointer cannot be null." };
			glfw::glfwSetWindowUserPointer(window.get(), this);
			glfw::glfwSetFramebufferSizeCallback(window.get(), FramebufferResizeCallback);
		}

		auto GetWin32Window(this const Window& self) -> void*
		{
			return glfw::glfwGetWin32Window(self.window.get());
		}

		auto GetContentAreaDimensions(this const Window& self) -> ScreenDimensions
		{
			int width, height;
			glfw::glfwGetWindowSize(self.window.get(), &width, &height);
			return { width, height };
		}

		auto GetScreenPosition(this const Window& self) -> ScreenCoordinates
		{
			int xPos, yPos;
			glfw::glfwGetWindowPos(self.window.get(), &xPos, &yPos);
			return { xPos, yPos };
		}

		// This metric is in pixels.
		// https://www.glfw.org/docs/latest/window_guide.html#window_fbsize
		auto GetFramebufferDimensions(this const Window& self) -> PixelCoordinates
		{
			int width, height;
			glfw::glfwGetFramebufferSize(self.window.get(), &width, &height);
			return { width, height };
		}

		auto SetPosition(this const Window& self, int xPos, int yPos) -> void
		{
			glfw::glfwSetWindowPos(self.window.get(), xPos, yPos);
		}

		auto SetClientSize(this const Window& self, int width, int height) -> void
		{
			glfw::glfwSetWindowSize(self.window.get(), width, height);
		}

		auto Get() const -> GLFWwindow*
		{
			return window.get();
		}

		auto GetUniquePtr(this auto&& self) -> GlfwWindowUniquePtr
		{
			return std::forward_like<decltype(self)>(self.window);
		}

		auto ShouldClose(this const Window& self) -> bool
		{
			return glfw::glfwWindowShouldClose(self.window.get());
		}

		auto StillOpen(this const Window& self) -> bool
		{
			return not self.ShouldClose();
		}

		auto CreateVulkanSurface(this const Window& self, VulkanSupport::VkInstance instance) -> VulkanSupport::VkSurfaceKHR
		{
			if (not instance)
				throw ::Error::RuntimeError("Vulkan instance must not be null to create a surface.");
			auto surface = VulkanSupport::VkSurfaceKHR{};
			if (auto result = glfw::glfwCreateWindowSurface(instance, self.window.get(), nullptr, &surface); result != 0)
				throw VulkanSupportError{ result };
			return surface;
		}

	private:
		static auto FramebufferResizeCallback(glfw::GLFWwindow* window, int width, int height)
		{
			if (auto self = reinterpret_cast<Window*>(glfw::glfwGetWindowUserPointer(window)); self)
				self->OnFramebufferResize(width, height);
		}

		glfw::GlfwWindowUniquePtr window = nullptr;
	};
}
