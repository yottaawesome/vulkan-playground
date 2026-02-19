export module vulkangfx:glfw.window;
import std;
import :glfw.exports;
import :glfw.raii;
import :glfw.error;

namespace glfw
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
			glfw::glfwInit();
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

		operator GlfwWindowUniquePtr(this const WindowFactory& self)
		{
			return self.Create();
		}
	};
}
