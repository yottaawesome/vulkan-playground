export module modelviewer;
import std;
import volkus;

export namespace ModelViewer
{
	class VulkanInstance : public Volkus::vkx::Instance
	{
	public:
		VulkanInstance()
		{
			Create(true);
		}

		auto GetLayers(this auto&& self) noexcept -> std::ranges::range auto
		{
			return std::array{ Vk::Layer::KhronosValidation };
		}

		auto GetExtensions(this auto&& self) noexcept -> std::ranges::range auto
		{
			return std::array{
				Vk::InstanceExtension::DebugUtils,
				Vk::InstanceExtension::PlatformSurface
			};
		}

		auto GetFlags(this auto&& self) noexcept
		{
			return VkInstanceCreateFlags{ 0 };
		}

		auto GetApplicationInfo(this auto&& self) noexcept
		{
			return VkApplicationInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pNext = nullptr,
				.pApplicationName = "Model Viewer",
				.applicationVersion = Vk::MakeVersion(1, 0, 0),
				.pEngineName = "No Engine",
				.engineVersion = Vk::MakeVersion(1, 0, 0),
				.apiVersion = Vk::ApiVersion::V1_4
			};
		}
	};
}
