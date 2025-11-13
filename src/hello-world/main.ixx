export module main;
import std;
import vulkan;

namespace Build
{
    constexpr bool IsDebug =
#ifdef _DEBUG
        true;
#else
        false;
#endif
    constexpr bool IsRelease = not IsDebug;
}

// Extension -- needs to be loaded manually
auto CreateDebugUtilsMessengerEXT(
    Vulkan::VkInstance instance, 
    const Vulkan::VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const Vulkan::VkAllocationCallbacks* pAllocator, 
    Vulkan::VkDebugUtilsMessengerEXT* pDebugMessenger
) -> Vulkan::VkResult
{
    auto fn = reinterpret_cast<Vulkan::PFN_vkCreateDebugUtilsMessengerEXT>(
        Vulkan::vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
    );
    return fn
        ? fn(instance, pCreateInfo, pAllocator, pDebugMessenger)
        : Vulkan::VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
}

// Extension -- needs to be loaded manually
void DestroyDebugUtilsMessengerEXT(
    Vulkan::VkInstance instance, 
    Vulkan::VkDebugUtilsMessengerEXT debugMessenger,
    const Vulkan::VkAllocationCallbacks* pAllocator
) 
{
    auto func = reinterpret_cast<Vulkan::PFN_vkDestroyDebugUtilsMessengerEXT>(
        Vulkan::vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
    );
    if (func) 
        func(instance, debugMessenger, pAllocator);
}

struct HelloTriangleApplication 
{
    constexpr static int Width = 800;
    constexpr static int Height = 600;
    constexpr static bool EnableValidationLayers = Build::IsDebug;
    constexpr static std::array ValidationLayers{
        "VK_LAYER_KHRONOS_validation" 
    };

    void Run(this auto& self) 
    {
        self.InitWindow();
        self.InitVulkan();
        self.EnumerateExtensions();
        self.CheckValidationLayerSupport();
        self.MainLoop();
        self.Cleanup();
    }

private:
	GLFW::GLFWwindow* window = nullptr;
    Vulkan::VkInstance instance;
    Vulkan::VkDebugUtilsMessengerEXT debugMessenger;

    auto CheckValidationLayerSupport(this const auto& self) -> bool
    {
        std::uint32_t layerCount = 0;
		auto result = Vulkan::vkEnumerateInstanceLayerProperties(
            &layerCount, 
            nullptr
        );
		if (VkResult::VK_SUCCESS != result)
            throw std::runtime_error("Failed to enumerate instance layer properties count.");

		std::vector<Vulkan::VkLayerProperties> availableLayers(layerCount);
        result = Vulkan::vkEnumerateInstanceLayerProperties(
            &layerCount,
            availableLayers.data()
		);
        if (VkResult::VK_SUCCESS != result)
            throw std::runtime_error("Failed to enumerate instance layer properties count.");

        return std::ranges::all_of(
            ValidationLayers,
            [&availableLayers](std::string_view layerName)
            {
                bool found = 
                    std::any_of(
                        availableLayers.begin(),
                        availableLayers.end(),
                        [layerName](const Vulkan::VkLayerProperties& layer)
                        {
                            return layerName == layer.layerName;
                        }
                    );
                return found
                    ? true
                    : (std::println("Validation layer not found: {}", layerName), false);
            }
		);
    }

    void EnumerateExtensions(this const auto& self)
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

	auto GetRequiredExtensions(this const auto& self) -> std::vector<const char*>
    {
        std::uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = 
            GLFW::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(
            glfwExtensions, 
            glfwExtensions + glfwExtensionCount
        );
        if (EnableValidationLayers)
            extensions.push_back(Vulkan::DebugUtilExtensionName);
        return extensions;
    }

    void CreateInstance(this auto& self)
    {
		if (EnableValidationLayers and not self.CheckValidationLayerSupport())
            throw std::runtime_error("Validation layers requested, but not available");

		Vulkan::VkApplicationInfo appInfo{
			.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Hello Triangle",
			.applicationVersion = Vulkan::MakeVersion(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = Vulkan::MakeVersion(1, 0, 0),
			.apiVersion = Vulkan::ApiVersion1
        };

		auto extensions = self.GetRequiredExtensions();

        Vulkan::VkInstanceCreateInfo createInfo{
            .sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        Vulkan::VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (EnableValidationLayers)
        {
            self.PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
            createInfo.ppEnabledLayerNames = ValidationLayers.data();
            createInfo.pNext = 
                reinterpret_cast<Vulkan::VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
        }

        Vulkan::VkResult result = 
            Vulkan::vkCreateInstance(&createInfo, nullptr, &self.instance);
        if (result != Vulkan::VkResult::VK_SUCCESS) 
            throw std::runtime_error("Failed to create instance!");
    }

    void InitWindow(this auto& self)
    {
        GLFW::glfwInit();
        GLFW::glfwWindowHint(GLFW::ClientApi, GLFW::NoApi);
		GLFW::glfwWindowHint(GLFW::Resizable, false);
		self.window = 
            GLFW::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
    }

    void InitVulkan(this auto& self) 
    {
        self.CreateInstance();
		self.SetupDebugMessenger();
    }

    void PopulateDebugMessengerCreateInfo(this auto& self, Vulkan::VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = Vulkan::VkDebugUtilsMessengerCreateInfoEXT{
            .sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = // filter messages based on severity
                Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                | Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = // filter messages based on type
                Vulkan::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                | Vulkan::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                | Vulkan::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = DebugCallback,
            .pUserData = nullptr // Optional
        };
    }

    void SetupDebugMessenger(this auto& self)
    {
        if (not EnableValidationLayers)
			return;
        Vulkan::VkDebugUtilsMessengerCreateInfoEXT createInfo{};

        self.PopulateDebugMessengerCreateInfo(createInfo);

		auto result = CreateDebugUtilsMessengerEXT(
            self.instance, 
            &createInfo, 
            nullptr, 
            &self.debugMessenger
        );
		if (result != Vulkan::VkResult::VK_SUCCESS)
			throw std::runtime_error("Failed to set up debug messenger");
	}

    void MainLoop(this auto& self) 
    {
        while (not GLFW::glfwWindowShouldClose(self.window))
        {
            GLFW::glfwPollEvents();
        }
    }

    void Cleanup(this auto& self) 
    {
        if (EnableValidationLayers)
            DestroyDebugUtilsMessengerEXT(self.instance, self.debugMessenger, nullptr);
		Vulkan::vkDestroyInstance(self.instance, nullptr);
		GLFW::glfwDestroyWindow(self.window);
		GLFW::glfwTerminate();
    }

    // stdcall on Windows, but this is ignored on x64.
    static auto DebugCallback(
        Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        Vulkan::VkDebugUtilsMessageTypeFlagsEXT messageType,
        const Vulkan::VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
	) -> Vulkan::VkBool32
    {
        if (messageSeverity >= Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
			std::println("Validation layer: {}", pCallbackData->pMessage);
        }

        return false;
    }
};

extern "C" auto main() -> int
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
