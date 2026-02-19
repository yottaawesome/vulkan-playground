export module vulkangfx:glfw.raii;
import std;
import :raii;
import :glfw.exports;

export namespace glfw
{
	using GlfwWindowUniquePtr = Raii::DirectUniquePtr<glfw::GLFWwindow, glfw::glfwDestroyWindow>;

	struct Context
	{
		~Context()
		{
			glfw::glfwTerminate();
		}
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
		
		Context()
		{
			glfw::glfwInit();
		}
	};
}
