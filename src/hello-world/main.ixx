export module main;
import std;
import vulkan;

struct HelloTriangleApplication 
{
    constexpr static int Width = 800;
    constexpr static int Height = 600;

    void Run() 
    {
        InitWindow();
        InitVulkan();
        MainLoop();
        Cleanup();
    }

private:
	GLFW::GLFWwindow* window = nullptr;

    void InitWindow()
    {
        GLFW::glfwInit();
        GLFW::glfwWindowHint(GLFW::ClientApi, GLFW::NoApi);
		GLFW::glfwWindowHint(GLFW::Resizable, false);
		window = GLFW::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
    }

    void InitVulkan() 
    {

    }

    void MainLoop() 
    {
        while (not GLFW::glfwWindowShouldClose(window))
        {
            GLFW::glfwPollEvents();
        }
    }

    void Cleanup() 
    {
		GLFW::glfwDestroyWindow(window);
		GLFW::glfwTerminate();
    }
};

extern "C" int main()
try 
{
    HelloTriangleApplication app;
    app.Run();
    return 0;
}
catch (const std::exception& ex)
{
    std::println("{}", ex.what());
    return 1;
}
