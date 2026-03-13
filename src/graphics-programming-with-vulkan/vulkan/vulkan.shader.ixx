export module vulkangfx:vulkan.shaders;
import std;
import :error;
import :file;
import :stlhelpers;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan
{
	struct Deleter
	{
		vkr::VkDevice device = nullptr;
		Deleter(vkr::VkDevice deviceIn)
			: device(deviceIn)
		{ 
			if (not device)
				throw Error::RuntimeError("Device must not be null.");
		}

		constexpr void operator()(this auto&& self, vkr::VkShaderModule shaderModule) noexcept
		{
			vkr::vkDestroyShaderModule(self.device, shaderModule, nullptr);
		}
	};
	using ShaderModuleUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkShaderModule>, Deleter>;

	struct ShaderModuleFactory
	{
		vkr::VkDevice Device = nullptr;
		std::filesystem::path FilePath;

		auto operator()(this const auto& self) -> ShaderModuleUniquePtr
		{
			auto code = File::ReadFileBytes(self.FilePath);
			vkr::VkShaderModuleCreateInfo createInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = code.size(),
				.pCode = reinterpret_cast<const std::uint32_t*>(code.data())
			};
			auto shaderModule = vkr::VkShaderModule{};
			auto result = Result{ vkr::vkCreateShaderModule(self.Device, &createInfo, nullptr, &shaderModule) };
			if (not result)
				throw VulkanError{ result, "Failed to create shader module." };
			return ShaderModuleUniquePtr{ shaderModule, Deleter{ self.Device } };
		}
	};

	class ShaderModule
	{
	public:
		ShaderModule(ShaderModuleUniquePtr handleIn)
			: handle(std::move(handleIn))
		{}

		constexpr auto GetHandle(this const ShaderModule& self) noexcept -> vkr::VkShaderModule
		{
			return self.handle.get();
		}
	private:
		ShaderModuleUniquePtr handle;
	};
}