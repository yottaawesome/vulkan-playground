import std;
import vulkangfx;

auto Logger = Log::Logger<"Main">{};

auto wWinMain(
	Win32::HINSTANCE,
	Win32::HINSTANCE,
	Win32::LPWSTR,
	Win32::UINT
) -> int
try
{
	Logger.Info("Application started.");

	auto context = glfw::Context{};
	glfw::SetErrorCallback(
		[](int code, const char* description) noexcept
		{
			Logger.Error("GLFW error {}: {}", code, description ? description : "Unknown error");
		});
	auto window = glfw::Window{ glfw::Window::Factory{.Resizable = true}() };

	auto primaryMonitor = glfw::Monitor{};

	auto [monitorWidth, monitorHeight] = primaryMonitor.GetScreenDimensions();
	auto [windowWidth, windowHeight] = window.GetContentAreaDimensions();
	window.SetPosition((monitorWidth - windowWidth) / 2, (monitorHeight - windowHeight) / 2);

	auto gfx = Graphics::CoreVulkan{ &window };
	gfx.Initialise();

	auto vertices = std::array{
		Graphics::Vertex{ glm::vec3{ 0.0f, -0.5f, 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f } },
		Graphics::Vertex{ glm::vec3{ 0.5f, 0.5f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f } },
		Graphics::Vertex{ glm::vec3{ -0.5f, 0.5f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f } }
	};
	auto buffer = Vulkan::BufferHandle{gfx.CreateVertexBuffer(vertices)};

	// Recreate the swap chain if the framebuffer has been resized, usually when the user resizes the window. 
	// Note that the framebuffer size is in pixels.
	window.OnFramebufferResize = [&gfx](int, int) { gfx.RecreateSwapChain(); };
	while (window.StillOpen())
	{
		glfw::glfwPollEvents();
		gfx.DrawFrame(buffer);
	}
	window.OnFramebufferResize = [](int, int) {};

	gfx.WaitForDeviceIdle();
	gfx.DestroyVertexBuffer(buffer);

	return 0;
}
catch (const std::exception& ex)
{
	Win32::MessageBoxA(nullptr, ex.what(), "Error", Win32::MessageBoxFlags::Critical | Win32::MessageBoxFlags::Ok);
	return -1;
}
catch(...)
{
	Win32::MessageBoxA(nullptr, "An unknown error occurred.", "Error", Win32::MessageBoxFlags::Critical | Win32::MessageBoxFlags::Ok);
	return -1;
}