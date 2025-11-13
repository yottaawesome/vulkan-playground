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
    Vulkan::VkInstance instance;

    void EnumerateExtensions()
    {
		std::uint32_t extensionsCount = 0;
        auto result = 
            Vulkan::vkEnumerateInstanceExtensionProperties(
                nullptr, 
                &extensionsCount, 
                nullptr
            );
		if (result != Vulkan::VkResult::VK_SUCCESS) 
			throw std::runtime_error("Failed to enumerate extensions count.");

		std::vector<Vulkan::VkExtensionProperties> extensions(extensionsCount);
        result = Vulkan::vkEnumerateInstanceExtensionProperties(
            nullptr,
            &extensionsCount,
            extensions.data()
        );
        if (result != Vulkan::VkResult::VK_SUCCESS)
            throw std::runtime_error("Failed to enumerate extensions.");

        for (const auto& extension : extensions)
            std::println("Extension: {}", extension.extensionName);
    }

    void CreateInstance()
    {
		Vulkan::VkApplicationInfo appInfo{
			.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Hello Triangle",
			.applicationVersion = Vulkan::MakeVersion(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = Vulkan::MakeVersion(1, 0, 0),
			.apiVersion = Vulkan::ApiVersion1
        };

        std::uint32_t extensionCount = 0;
        const char** extensions =
            GLFW::glfwGetRequiredInstanceExtensions(&extensionCount);
        Vulkan::VkInstanceCreateInfo createInfo{
            .sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
			.enabledExtensionCount = extensionCount,
			.ppEnabledExtensionNames = extensions
        };
        Vulkan::VkResult result = 
            Vulkan::vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != Vulkan::VkResult::VK_SUCCESS) 
            throw std::runtime_error("Failed to create instance!");
    }

    void InitWindow()
    {
        GLFW::glfwInit();
        GLFW::glfwWindowHint(GLFW::ClientApi, GLFW::NoApi);
		GLFW::glfwWindowHint(GLFW::Resizable, false);
		window = GLFW::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
    }

    void InitVulkan() 
    {
        CreateInstance();
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
		Vulkan::vkDestroyInstance(instance, nullptr);
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
