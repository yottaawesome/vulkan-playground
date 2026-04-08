export module vulkangfx:vulkan.texturesampler;
import std;
import :vulkan.exports;
import :vulkan.error;
import :error;

export namespace Vulkan
{
	struct TextureSamplerDeleter
	{
		vkr::VkDevice Device = nullptr;
		TextureSamplerDeleter() = default;
		TextureSamplerDeleter(vkr::VkDevice device) : Device(device) 
		{
			if (not device)
				throw Error::RuntimeError("Device must be valid to create TextureSamplerDeleter.");
		}
		void operator()(this auto&& self, vkr::VkSampler sampler) noexcept
		{
			vkr::vkDestroySampler(self.Device, sampler, nullptr);
		}
	};
	using TextureSamplerUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkSampler>, TextureSamplerDeleter>;

	class TextureSampler
	{
	public:
		struct Factory
		{
			vkr::VkDevice Device = nullptr;
			vkr::VkSamplerCreateInfo Info{};
			auto operator()(this auto&& self) -> TextureSampler
			{
				if (not self.Device)
					throw Error::RuntimeError{"Device must be valid to create TextureSampler."};

				self.Info.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

				auto generatedSampler = vkr::VkSampler{};
				auto result = Result{vkr::vkCreateSampler(self.Device, &self.Info, nullptr, &generatedSampler)};
				if (not result)
					throw VulkanError(result, "Failed to create texture sampler.");
				return TextureSampler{ TextureSamplerUniquePtr(generatedSampler, TextureSamplerDeleter(self.Device)) };
			}
		};

		TextureSampler(TextureSamplerUniquePtr samplerIn) noexcept 
			: sampler(std::move(samplerIn)) 
		{
			if (not sampler)
				throw Error::RuntimeError{ "Texture sampler cannot be nullptr." };
		}

		constexpr auto GetHandle(this auto&& self) noexcept -> vkr::VkSampler
		{
			return self.sampler.get();
		}

		constexpr auto Destroy(this auto&& self) noexcept
		{
			self.sampler.reset();
		}

	private:
		TextureSamplerUniquePtr sampler;
	};
}