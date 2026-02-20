export module vulkangfx:glfw.raii;
import std;
import :raii;
import :glfw.exports;
import :glfw.error;

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
			if (not glfw::glfwInit())
				throw glfw::Error("Failed to initialise GLFW.");
		}
	};
}
