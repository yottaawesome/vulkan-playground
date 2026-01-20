export module vulkantutorial:mainapp;
import std;
import :libs;
import :error;
import :util;

export namespace VulkanTutorial
{
    constexpr auto Width = std::uint32_t{ 800 };
    constexpr auto Height = std::uint32_t{ 600 };

    struct MainApp
    {
        constexpr MainApp() = default;
        MainApp(const MainApp&) = delete;
        auto operator=(this MainApp&, const MainApp&) -> MainApp& = delete;

        void Run(this MainApp& self)
        {
            self.InitWindow()
                .InitVulkan()
                .MainLoop()
                .Cleanup();
        }

    private:
        glfw::GLFWwindow* window;
        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
        vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

        void Cleanup(this MainApp& self)
        {
            glfw::glfwDestroyWindow(self.window);
            glfw::glfwTerminate();
        }

        void PrintSupportedExtensions(this const MainApp& self)
        {
            auto availableExtensions =
                std::vector<vk::ExtensionProperties>{ self.context.enumerateInstanceExtensionProperties() };
            std::println("Available Vulkan extensions ({}):", availableExtensions.size());
            for (const auto& ext : availableExtensions)
                std::println(" -> {}", std::string_view{ ext.extensionName });
        }

        auto GetRequiredVulkanExtensions(this const MainApp& self) -> std::vector<std::string>
        {
            // We need glfw to tell us what extensions to use.
            auto glfwExtensionCount = std::uint32_t{ 0 };
            auto glfwExtensions = glfw::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            // Is the extension requested by glfw supported by vulkan?
            auto supportedExtensions = self.context.enumerateInstanceExtensionProperties();
            // Treat argv as a span of const char*
            auto requiredExtensions =
                std::span<const char* const>{ glfwExtensions, glfwExtensionCount }
            | std::ranges::to<std::vector<std::string>>();
            if (EnableValidationLayers)
                requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
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
            return requiredExtensions;
        }

        static auto DebugCallback(
            vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
            vk::DebugUtilsMessageTypeFlagsEXT type,
            const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void*
        ) -> vk::Bool32
        {
            if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
                std::println("Validation layer: {}", pCallbackData->pMessage);
            return false;
        }

        auto GetRequiredVulkanLayers(this MainApp& self) -> std::vector<std::string>
        {
            auto requiredLayers = std::vector<std::string>{
                //"VK_LAYER_LUNARG_api_dump" 
            };
            if constexpr (EnableValidationLayers)
            {
                requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
                //layers.push_back("VK_LAYER_LUNARG_api_dump");
            }

            auto layerProperties = self.context.enumerateInstanceLayerProperties();
            for (std::string_view requiredLayer : requiredLayers)
            {
                bool supported = std::ranges::any_of(
                    layerProperties,
                    [requiredLayer](const vk::LayerProperties& layerProp)
                    {
                        return requiredLayer == layerProp.layerName;
                    }
                );
                if (not supported)
                    throw VulkanError(std::format("Required layer not supported: {}", requiredLayer));
            }

            return requiredLayers;
        }

        void CreateInstance(this MainApp& self)
        {
            self.PrintSupportedExtensions();

            // Check required layers are supported.
            auto requiredLayers = self.GetRequiredVulkanLayers();
            auto layersList = requiredLayers
                | std::ranges::views::transform([](const std::string& layer) { return layer.c_str(); })
                | std::ranges::to<std::vector<const char*>>();
            auto requiredExtensions = self.GetRequiredVulkanExtensions();
            auto extensionList = requiredExtensions
                | std::ranges::views::transform([](auto&& s) { return s.c_str(); })
                | std::ranges::to<std::vector<const char*>>();

            //Create the Vulkan instance
            constexpr auto appInfo = vk::ApplicationInfo{
                .pApplicationName = "Hello Triangle",
                .applicationVersion = vk::MakeVersion(1, 0, 0),
                .pEngineName = "No Engine",
                .engineVersion = vk::MakeVersion(1, 0, 0),
                .apiVersion = vk::ApiVersion14
            };
            auto createInfo = vk::InstanceCreateInfo{
                .pApplicationInfo = &appInfo,
                .enabledLayerCount = static_cast<std::uint32_t>(layersList.size()),
                .ppEnabledLayerNames = layersList.data(),
                .enabledExtensionCount = static_cast<std::uint32_t>(extensionList.size()),
                .ppEnabledExtensionNames = extensionList.data(),
            };
            self.instance = vk::raii::Instance(self.context, createInfo);
        }

        auto InitVulkan(this MainApp& self) -> MainApp&
        {
            self.CreateInstance();
            self.SetupDebugMessenger();
            return self;
        }

        void SetupDebugMessenger(this MainApp& self)
        {
            if (not EnableValidationLayers)
                return;

            constexpr auto ToUint32 =
                [](auto...v)static constexpr noexcept->std::uint32_t
                {
                    return (static_cast<std::uint32_t>(v) | ...);
                };
            using Severities = vk::DebugUtilsMessageSeverityFlagBitsEXT;
            auto severities = 
                ToUint32(Severities::eVerbose, Severities::eWarning, Severities::eError);
            auto severityFlags = vk::DebugUtilsMessageSeverityFlagsEXT(severities);

            using MessageTypes = vk::DebugUtilsMessageTypeFlagBitsEXT;
            auto messageTypes = ToUint32(
                MessageTypes::eGeneral,
                MessageTypes::ePerformance,
                MessageTypes::eValidation
            );
            auto messageTypeFlags = vk::DebugUtilsMessageTypeFlagsEXT{ messageTypes };

            auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT{
                .messageSeverity = severityFlags,
                .messageType = messageTypeFlags,
				.pfnUserCallback = &MainApp::DebugCallback,
            };
            self.debugMessenger = self.instance.createDebugUtilsMessengerEXT(createInfo);
        }

		auto InitWindow(this MainApp& self) -> MainApp&
        {
            glfw::glfwInit();
            glfw::glfwWindowHint(glfw::ClientApi, glfw::NoApi);
            glfw::glfwWindowHint(glfw::Resizable, false);
            self.window = glfw::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
            return self;
        }

        auto MainLoop(this MainApp& self) -> MainApp&
        {
            while (not glfw::glfwWindowShouldClose(self.window))
            {
                glfw::glfwPollEvents();
            }
            return self;
        }
    };
}
