export module vulkangfx:vulkan.texture;
import std;
import :vulkan.exports;
import :vulkan.error;
import :vulkan.buffer;
import :vulkan.error;
import :glm;

export namespace Vulkan
{
	class Texture
	{
	public:
		struct Factory;

		~Texture() { Destroy(); }

		Texture(
			vkr::VkDevice device,
			vkr::VkImage image,
			vkr::VkImageView imageView,
			vkr::VkDeviceMemory memory,
			vkr::VkDescriptorSet descriptorSet
		) : device(device), image(image), imageView(imageView), memory(memory), descriptorSet(descriptorSet) 
		{
			if (not image)
				throw ::Error::RuntimeError{ "Texture image cannot be nullptr." };
			//if (not imageView)
				//throw ::Error::RuntimeError{ "Texture image view cannot be nullptr." };
			if (not memory)
				throw ::Error::RuntimeError{ "Texture memory cannot be nullptr." };
			//if (not descriptorSet)
				//throw ::Error::RuntimeError{ "Texture descriptor set cannot be nullptr." };
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
		constexpr auto GetImageHandleAddress(this auto&& self) noexcept -> vkr::VkImage*
		{
			return &self.image;
		}
		constexpr auto GetImageViewHandle(this auto&& self) noexcept -> vkr::VkImageView
		{
			return self.imageView;
		}
		constexpr auto GetMemoryHandle(this auto&& self) noexcept -> vkr::VkDeviceMemory
		{
			return self.memory;
		}
		constexpr auto GetMemoryHandleAddress(this auto&& self) noexcept -> vkr::VkDeviceMemory*
		{
			return &self.memory;
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
	};

	auto CreateImage(
		glm::ivec2 size, 
		vkr::VkBufferUsageFlags usage, 
		vkr::VkMemoryPropertyFlags properties,
		vkr::VkDevice device,
		vkr::VkPhysicalDevice physicalDevice
	) -> Texture
	{
		auto imageCreateInfo = vkr::VkImageCreateInfo{
			.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = vkr::VkImageType::VK_IMAGE_TYPE_2D,
			.format = vkr::VkFormat::VK_FORMAT_R8G8B8A8_SRGB,
			.extent = {
				static_cast<std::uint32_t>(size.x),
				static_cast<std::uint32_t>(size.y),
				1
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = vkr::VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
			.tiling = vkr::VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
			.usage = usage,
			.sharingMode = vkr::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = vkr::VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
		};
		vkr::VkImage image = nullptr;
		auto result = Result{ vkr::vkCreateImage(device, &imageCreateInfo, nullptr, &image) };
		if (not result)
			throw VulkanError{ result, "Failed to create image." };

		auto memoryRequirements = vkr::VkMemoryRequirements{};
		vkr::vkGetImageMemoryRequirements(device, image, &memoryRequirements);
		auto chosen_memory_type = std::uint32_t{ FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties) };

		auto allocationInfo = vkr::VkMemoryAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memoryRequirements.size,
			.memoryTypeIndex = chosen_memory_type
		};
		vkr::VkDeviceMemory memory = nullptr;
		result = Result{
			vkr::vkAllocateMemory(device, &allocationInfo, nullptr, &memory)
		};
		if (not result)
			throw std::runtime_error("Failed to allocate image memory!");

		vkr::vkBindImageMemory(device, image, memory, 0);

		return Texture{ device, image, nullptr, memory, nullptr };
	}
}