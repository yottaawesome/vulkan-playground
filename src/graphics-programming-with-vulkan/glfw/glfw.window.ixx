export module vulkangfx:glfw.window;
import std;
import :glfw.exports;
import :glfw.raii;

namespace glfw
{
	struct WindowFactory
	{
		std::string Title = "Vulkan";
		std::uint32_t ClientApi = glfw::WindowHints::NoApi;
		bool Resizable = false;
		std::uint32_t Width = 800;
		std::uint32_t Height = 600;

		auto Create(this const WindowFactory& self) -> GlfwWindowUniquePtr
		{
			glfw::glfwInit();
			glfw::glfwWindowHint(glfw::WindowHints::ClientApi, self.ClientApi);
			glfw::glfwWindowHint(glfw::WindowHints::Resizable, self.Resizable);
			auto window = glfw::glfwCreateWindow(self.Width, self.Height, self.Title.c_str(), nullptr, nullptr);
			if (not window)
				throw std::runtime_error("Failed to create GLFW window");
			return GlfwWindowUniquePtr{ window };
		}

		auto operator()(this const WindowFactory& self) -> GlfwWindowUniquePtr
		{
			return self.Create();
		}

		operator GlfwWindowUniquePtr(this const WindowFactory& self)
		{
			return self.Create();
		}
	};
}
