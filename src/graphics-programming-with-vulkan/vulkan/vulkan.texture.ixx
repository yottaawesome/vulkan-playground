export module vulkangfx:vulkan.texture;
import std;
import :vulkan.exports;
import :vulkan.error;
import :vulkan.buffer;
import :vulkan.error;

export namespace Vulkan
{
	class Texture
	{
	public:
		struct Factory;

		~Texture() { Destroy(); }

		Texture() = default;

		Texture(
			vkr::VkImage image,
			vkr::VkImageView imageView,
			vkr::VkDeviceMemory memory,
			vkr::VkDescriptorSet descriptorSet
		) : image(image), imageView(imageView), memory(memory), descriptorSet(descriptorSet) 
		{
			if (not image)
				throw ::Error::RuntimeError{ "Texture image cannot be nullptr." };
			if (not imageView)
				throw ::Error::RuntimeError{ "Texture image view cannot be nullptr." };
			if (not memory)
				throw ::Error::RuntimeError{ "Texture memory cannot be nullptr." };
			if (not descriptorSet)
				throw ::Error::RuntimeError{ "Texture descriptor set cannot be nullptr." };
		}

		Texture(const Texture&) = delete;
		auto operator=(const Texture&) -> Texture & = delete;

		Texture(Texture&& other) noexcept
			: image(other.image), imageView(other.imageView), memory(other.memory), descriptorSet(other.descriptorSet)
		{
			other.image = nullptr;
			other.imageView = nullptr;
			other.memory = nullptr;
			other.descriptorSet = nullptr;
		}
		auto operator=(Texture&& other) noexcept -> Texture&
		{
			if (this == &other)
				return *this;
			this->~Texture(); // Clean up existing resources.
			image = other.image;
			imageView = other.imageView;
			memory = other.memory;
			descriptorSet = other.descriptorSet;
			other.image = nullptr;
			other.imageView = nullptr;
			other.memory = nullptr;
			other.descriptorSet = nullptr;
			return *this;
		}

		constexpr auto GetImageHandle(this auto&& self) noexcept -> vkr::VkImage
		{
			return self.image;
		}
		constexpr auto GetImageViewHandle(this auto&& self) noexcept -> vkr::VkImageView
		{
			return self.imageView;
		}
		constexpr auto GetMemoryHandle(this auto&& self) noexcept -> vkr::VkDeviceMemory
		{
			return self.memory;
		}
		constexpr auto GetDescriptorSetHandle(this auto&& self) noexcept -> vkr::VkDescriptorSet
		{
			return self.descriptorSet;
		}
		constexpr auto Destroy(this auto&& self) noexcept
		{
			if (self.descriptorSet)
			{
				// Descriptor sets are implicitly freed when the pool is reset or destroyed.
				self.descriptorSet = nullptr;
			}
			if (self.imageView)
			{
				vkr::vkDestroyImageView(self.device, self.imageView, nullptr);
				self.imageView = nullptr;
			}
			if (self.image)
			{
				vkr::vkDestroyImage(self.device, self.image, nullptr);
				self.image = nullptr;
			}
			if (self.memory)
			{
				vkr::vkFreeMemory(self.device, self.memory, nullptr);
				self.memory = nullptr;
			}
		}
	private:
		vkr::VkDevice device = nullptr;
		vkr::VkImage image = nullptr;
		vkr::VkImageView imageView = nullptr;
		vkr::VkDeviceMemory memory = nullptr;
		vkr::VkDescriptorSet descriptorSet = nullptr;
	};

	struct Texture::Factory
	{
		auto operator()(this auto&& self) -> Texture
		{
			return Texture{};
		}
	};
}