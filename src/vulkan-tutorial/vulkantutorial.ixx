export module vulkantutorial;
import std;
export import :libs;
export import :error;

namespace VulkanTutorial
{
    constexpr std::uint32_t Width = 800;
    constexpr std::uint32_t Height = 600;

    struct Init{} constexpr Init{};

    struct MainApp
    {
		constexpr MainApp() = default;

        void Run(this MainApp& self)
        {
            self.InitWindow();
            self.InitVulkan();
            self.MainLoop();
            self.Cleanup();
        }

    private:
        glfw::GLFWwindow* window;
        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;

        void Cleanup(this MainApp& self)
        {
            glfw::glfwDestroyWindow(self.window);
            glfw::glfwTerminate();
        }

        void CreateInstance(this MainApp& self)
        {
            constexpr auto appInfo = vk::ApplicationInfo{
                .pApplicationName = "Hello Triangle",
                .applicationVersion = vk::MakeVersion(1, 0, 0),
                .pEngineName = "No Engine",
                .engineVersion = vk::MakeVersion(1, 0, 0),
                .apiVersion = vk::ApiVersion14 
            };
        }

        void PrintSupportedExtensions(this const MainApp& self)
        {
            auto availableExtensions =
                std::vector<vk::ExtensionProperties>{ self.context.enumerateInstanceExtensionProperties() };
            std::println("Available Vulkan extensions ({}):", availableExtensions.size());
            for (const auto& ext : availableExtensions)
                std::println(" -> {}", std::string_view{ ext.extensionName });
		}

        void InitVulkan(this MainApp& self)
        {
            self.PrintSupportedExtensions();

            constexpr auto appInfo = vk::ApplicationInfo{
                .applicationVersion = vk::MakeVersion(1, 0, 0),
                .pEngineName = "No Engine",
                .engineVersion = vk::MakeVersion(1, 0, 0),
                .apiVersion = vk::ApiVersion14
            };

			// We need glfw to tell us what extensions to use.
            auto glfwExtensionCount = std::uint32_t{ 0 };
            auto glfwExtensions = glfw::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			// Is the extension requested by glfw supported by vulkan?
            auto supportedExtensions = self.context.enumerateInstanceExtensionProperties();
            // Treat argv as a span of const char*
            auto requiredExtensions = std::span<const char* const>{ glfwExtensions, glfwExtensionCount };
            
			// Check that all required glfw extensions are supported
            for (const std::string_view extName : requiredExtensions)
            {
                bool supported = std::ranges::any_of(
                    supportedExtensions,
                    [extName](const vk::ExtensionProperties& extProp)
                    {
                        return extName == extProp.extensionName;
                    }
                );
                if (not supported)
                    throw VulkanError(std::format("Required extension not supported: {}", extName));
            }
			//Create the Vulkan instance
            vk::InstanceCreateInfo createInfo{
				.pApplicationInfo = &appInfo,
                .enabledExtensionCount = glfwExtensionCount,
				.ppEnabledExtensionNames = glfwExtensions
            };
            self.instance = vk::raii::Instance(self.context, createInfo);
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

template<typename T>
struct value_category {
    // Or can be an integral or enum value
    static constexpr auto value = "prvalue";
};

template<typename T>
struct value_category<T&> {
    static constexpr auto value = "lvalue";
};

template<typename T>
struct value_category<T&&> {
    static constexpr auto value = "xvalue";
};

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