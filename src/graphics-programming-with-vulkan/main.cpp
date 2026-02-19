import  std;
import vulkangfx;

auto wWinMain(
	Win32::HINSTANCE,
	Win32::HINSTANCE,
	Win32::LPWSTR,
	Win32::UINT
) -> int
try
{
	auto context = glfw::Context{};
	auto window = glfw::Window{ glfw::WindowFactory{}() };

	auto primaryMonitor = glfw::Monitor{};

	auto [monitorWidth, monitorHeight] = primaryMonitor.GetScreenDimensions();
	auto [windowWidth, windowHeight] = window.GetContentAreaDimensions();
	window.SetPosition((monitorWidth - windowWidth) / 2, (monitorHeight - windowHeight) / 2);

	auto coreVulkan = Graphics::CoreVulkan{ &window };
	coreVulkan.Initialise();

	while (window.StillOpen())
	{
		glfw::glfwPollEvents();
	}

	return 0;
}
catch (const std::exception& ex)
{
	Win32::MessageBoxA(nullptr, ex.what(), "Error", Win32::MessageBoxFlags::Critical | Win32::MessageBoxFlags::Ok);
	return -1;
}