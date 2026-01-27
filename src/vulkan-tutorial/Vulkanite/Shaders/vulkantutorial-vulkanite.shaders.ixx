export module vulkantutorial:vulkanite.shaders;
import std;
import :libs;

export namespace VulkanTutorial::Vulkanite::Shaders
{
	class ShaderModule
	{
	public:
		ShaderModule(vk::raii::ShaderModule mod)
			: shaderModule(std::move(mod))
		{ }

		auto Get(this ShaderModule& self) -> vk::raii::ShaderModule&
		{
			return self.shaderModule;
		}

		auto operator->(this ShaderModule& self) -> vk::raii::ShaderModule*
		{
			return &self.shaderModule;
		}

		void Close()
		{
			shaderModule = nullptr;
		}

	private:
		vk::raii::ShaderModule shaderModule;
	};
}
