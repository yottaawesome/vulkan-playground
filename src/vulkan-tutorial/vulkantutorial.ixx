export module vulkantutorial;
import std;
export import :libs;

namespace VulkanTutorial
{
    constexpr std::uint32_t Width = 800;
    constexpr std::uint32_t Height = 600;

    struct MainApp
    {
        void Run(this MainApp& self)
        {
            self.InitWindow();
            self.InitVulkan();
            self.MainLoop();
            self.Cleanup();
        }

    private:
        glfw::GLFWwindow* window;

        void Cleanup(this MainApp& self)
        {
            glfw::glfwDestroyWindow(self.window);
            glfw::glfwTerminate();
        }

        void CreateInstance(this MainApp& self)
        {
            self.CreateInstance();
        }

        void InitVulkan(this MainApp& self)
        {
        }

        void InitWindow(this MainApp& self)
        {
            glfw::glfwInit();
            glfw::glfwWindowHint(glfw::ClientApi, glfw::NoApi);
            glfw::glfwWindowHint(glfw::Resizable, false);
            self.window = glfw::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
        }

        void MainLoop(this MainApp& self)
        {
            while (not glfw::glfwWindowShouldClose(self.window)) 
            {
                glfw::glfwPollEvents();
            }
        }
    };
}

export auto main(int argc, char* argv[]) -> int
try
{
    VulkanTutorial::MainApp app;
    app.Run();
    return 0;
}
catch (const std::exception& ex)
{
    std::cerr << ex.what() << std::endl;
    return 1;
}