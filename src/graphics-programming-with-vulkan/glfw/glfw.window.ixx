export module vulkangfx:glfw.window;
import std;
import :glfw.exports;
import :glfw.raii;
import :glfw.error;
import :gsl;

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
	struct WindowFactory
	{
		std::string Title = "Vulkan";
		std::int32_t ClientApi = glfw::WindowHints::NoApi;
		std::int32_t Width = 800;
		std::int32_t Height = 600;
		bool Resizable = false;

		auto Create(this const WindowFactory& self) -> GlfwWindowUniquePtr
		{
			glfw::glfwWindowHint(glfw::WindowHints::ClientApi, self.ClientApi);
			glfw::glfwWindowHint(glfw::WindowHints::Resizable, self.Resizable);
			auto window = glfw::glfwCreateWindow(self.Width, self.Height, self.Title.c_str(), nullptr, nullptr);
			if (not window)
				throw Error("Failed to create GLFW window");
			return GlfwWindowUniquePtr{ window };
		}

		auto operator()(this const WindowFactory& self) -> GlfwWindowUniquePtr
		{
			return self.Create();
		}

		explicit operator GlfwWindowUniquePtr(this const WindowFactory& self)
		{
			return self.Create();
		}
	};

	class Window
	{
	public:
		Window(GlfwWindowUniquePtr window) 
			: window(std::move(window))
		{
			if (not this->window)
				throw std::runtime_error("Window pointer cannot be null.");
		}

		auto GetContentAreaDimensions(this const Window& self) -> std::pair<int, int>
		{
			int width, height;
			glfw::glfwGetWindowSize(self.window.get(), &width, &height);
			return { width, height };
		}

		auto GetScreenPosition(this const Window& self) -> std::pair<int, int>
		{
			int xPos, yPos;
			glfw::glfwGetWindowPos(self.window.get(), &xPos, &yPos);
			return { xPos, yPos };
		}

		auto GetFramebufferDimensions(this const Window& self) -> std::pair<int, int>
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

		

	private:
		glfw::GlfwWindowUniquePtr window = nullptr;
	};
}
