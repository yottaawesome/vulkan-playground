export module vulkantutorial:vulkanite.device.logicaldevice;
import std;
import :libs;
import :vulkanite.shaders;
import :util;

export namespace VulkanTutorial::Vulkanite::Device
{
	struct LogicalDevice
	{
		vk::raii::Device Device;

		auto operator->(this LogicalDevice& self) noexcept -> vk::raii::Device*
		{
			return &self.Device;
		}

		operator bool(this const LogicalDevice& self) noexcept
		{
			return *self.Device != nullptr;
		}

		operator vk::raii::Device&(this LogicalDevice& self) noexcept
		{
			return self.Device;
		}

		auto CreateShaderModule(
			this const LogicalDevice& self,
			const std::filesystem::path& filename
		) -> vk::raii::ShaderModule
		{
			auto code = Util::ReadBinaryFile(filename);
			auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo{
				.codeSize = code.size(),
				.pCode = reinterpret_cast<const uint32_t*>(code.data())
			};
			return vk::raii::ShaderModule(self.Device, shaderModuleCreateInfo);
		}

		auto GetQueue(
			this const LogicalDevice& self, 
			std::uint32_t familyIndex, 
			std::uint32_t queueIndex
		) -> vk::raii::Queue
		{
			return vk::raii::Queue{ self.Device, familyIndex, queueIndex };
		}
	};
}
