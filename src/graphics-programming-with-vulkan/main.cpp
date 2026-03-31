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
	auto vertexBuffer = Vulkan::BufferHandle{gfx.CreateVertexBuffer(vertices)};

	auto indices = std::array{ 0u, 1u, 2u };
	auto indexBuffer = Vulkan::BufferHandle{ gfx.CreateIndexBuffer(indices) };

	// Recreate the swap chain if the framebuffer has been resized, usually when the user resizes the window. 
	// Note that the framebuffer size is in pixels.
	window.OnFramebufferResize = [&gfx](int, int) { gfx.RecreateSwapChain(); };

	auto start = std::chrono::high_resolution_clock::now();
	auto angle = float{0.0f};
	while (window.StillOpen())
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto diff = now - start;
		start = now;
		angle += std::chrono::duration<float>(diff).count()*20;

		auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3{ 0.0f, 0.0f, 1.0f });

		glfw::glfwPollEvents();
		gfx.BeginDraw().and_then(
			[
				&gfx, 
				&vertexBuffer, 
				&indexBuffer, 
				&indices, 
				&rotation
			](Vulkan::Swapchain::NextSwapchainImage&& frameData) -> std::optional<Vulkan::Swapchain::NextSwapchainImage>
			{
				gfx.CurrentCommandBuffer().Begin(); // comment this out if using gfx.RecordCommandBuffer()  vvvvvvvvv
				gfx.SetModelMatrix(rotation);
				gfx.RecordCommandBufferBody(gfx.CurrentCommandBuffer(), frameData.ImageIndex, vertexBuffer, indexBuffer, static_cast<std::uint32_t>(indices.size()));
				//gfx.RecordCommandBuffer(gfx.CurrentCommandBuffer(), frameData.ImageIndex, vertexBuffer, indexBuffer, static_cast<std::uint32_t>(indices.size()));
				gfx.CurrentCommandBuffer().End(); // comment this out if using gfx.RecordCommandBuffer()    ^^^^^^^^^
				gfx.EndDraw(frameData);
				return frameData;
			});

		//gfx.DrawFrame(vertexBuffer, indexBuffer, static_cast<std::uint32_t>(indices.size()));
	}
	window.OnFramebufferResize = [](int, int) {};

	gfx.WaitForDeviceIdle();
	gfx.DestroyBuffer(indexBuffer);
	gfx.DestroyBuffer(vertexBuffer);

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