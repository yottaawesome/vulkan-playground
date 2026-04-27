export module modelviewer;
import std;
import volkus;

export namespace ModelViewer
{
	class VulkanInstance : public Volkus::vkx::Instance
	{
    public:
        VulkanInstance() : Volkus::vkx::Instance{Create()}
		{ }

        auto GetLayers(this auto&& self) noexcept
        {
            return std::array{ Vk::Layer::KhronosValidation };
        }

        auto GetExtensions(this auto&& self) noexcept
        {
            return std::array{
                Vk::InstanceExtension::DebugUtils,
                Vk::InstanceExtension::PlatformSurface
            };
		}

        auto GetApplicationInfo(this auto&& self) noexcept
        {
            return VkApplicationInfo{
                .sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = "Model Viewer",
                .applicationVersion = Vk::MakeApiVersion(0, 1, 0),
                .pEngineName = "Volkus Engine",
                .engineVersion = Vk::MakeApiVersion(0, 1, 0),
                .apiVersion = Vk::MakeApiVersion(1, 4, 0)
            };
		}

    private:
		auto Create(this auto&& self) -> Volkus::vkx::Instance
        {
            auto layers = self.GetLayers();
            auto extensions = self.GetExtensions();
            auto applicationInfo = self.GetApplicationInfo();

            auto createinfo = VkInstanceCreateInfo{
                .sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &applicationInfo,
                .enabledLayerCount = static_cast<std::uint32_t>(layers.size()),
                .ppEnabledLayerNames = layers.data(),
                .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data()
            };
            return Volkus::vkx::Instance::Create(createinfo, true);
        }
	};
}
