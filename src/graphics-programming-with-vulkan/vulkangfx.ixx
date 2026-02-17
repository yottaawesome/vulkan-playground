export module vulkangfx;
import  std;
export import :raii;
export import :win32;
export import :glfw;

export extern "C++" auto wWinMain(
    Win32::HINSTANCE, 
    Win32::HINSTANCE, 
    Win32::LPWSTR, 
    Win32::UINT
) -> int
{
	auto context = glfw::Context{};
	auto window = glfw::GlfwWindowUniquePtr{ glfw::WindowFactory{}.Create() };
		
	while (not glfw::glfwWindowShouldClose(window.get()))
	{
		glfw::glfwPollEvents();
	}

    return 0;
}
