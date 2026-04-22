export module vulkangfx:vulkan.image;
import std;
import :glm;
import :error;
import :vulkan.exports;
import :vulkan.error;
import :vulkan.memory;

// Concepts
export namespace Vulkan
{
	template<typename T>
	concept ImageLike = requires(const T& t)
	{
		{ t.GetHandle() } noexcept -> std::same_as<vkr::VkImage>;
	};

	template<typename T>
	concept ImageViewLike = requires(const T& t)
	{
		{ t.GetHandle() } noexcept -> std::same_as<vkr::VkImageView>;
	};
}

// Image
export namespace Vulkan
{
	class ImageDeleter
	{
	public:
		constexpr ImageDeleter() = default;
		ImageDeleter(vkr::VkDevice device)
			: device(device)
		{
			if (not device)
				throw ::Error::RuntimeError{ "Device cannot be nullptr." };
		}
		void operator()(this const ImageDeleter& self, vkr::VkImage image)
		{
			vkr::vkDestroyImage(self.device, image, nullptr);
		}
		constexpr auto GetDevice(this const ImageDeleter& self) noexcept -> vkr::VkDevice
		{
			return self.device;
		}
	private:
		vkr::VkDevice device = nullptr;;
	};
	using ImageUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkImage>, ImageDeleter>;

	struct ImageFactory
	{
		vkr::VkDevice Device{};
		vkr::VkImageCreateInfo ImageCreateInfo{};

		auto CreateImage() -> ImageUniquePtr
		{
			ImageCreateInfo.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			auto image = vkr::VkImage{};
			auto result = Result{
				vkr::vkCreateImage(Device, &ImageCreateInfo, nullptr, &image) };
			if (not result)
				throw VulkanError{ result, "Failed to create image." };

			return ImageUniquePtr{ image, ImageDeleter{ Device } };
		}
	};

	class Image
	{
	public:
		constexpr Image() = default;
		constexpr Image(Image&&) = default;
		constexpr auto operator=(Image&&) -> Image & = default;
		Image(ImageUniquePtr imageIn)
			: image(std::move(imageIn))
		{
			if (not image)
				throw Error::RuntimeError{ "Image handle must not be null." };
		}
		[[nodiscard]]
		constexpr auto GetHandle(this const Image& self) noexcept -> vkr::VkImage
		{
			return self.image.get();
		}
		constexpr auto Destroy(this Image& self) noexcept
		{
			self.image.reset();
		}
	private:
		ImageUniquePtr image;
	};

	class BoundImageMemory
	{
	public:
		BoundImageMemory(vkr::VkDevice device, Image imageIn, DeviceMemory memoryIn)
			: image(std::move(imageIn)), memory(std::move(memoryIn))
		{ 
			memory.BindToImage(device, image.GetHandle());
		}

	private:
		Image image;
		DeviceMemory memory;
	};
}

// ImageView
export namespace Vulkan
{
	struct ImageViewDeleter
	{
		vkr::VkDevice Device = nullptr;
		constexpr ImageViewDeleter() = default;

		ImageViewDeleter(vkr::VkDevice device)
			: Device(device)
		{
			if (not Device)
				throw Error::RuntimeError{ "Device must not be null for ImageViewDeleter." };
		}
		void operator()(this ImageViewDeleter& self, vkr::VkImageView imageView) noexcept
		{
			if (imageView)
				vkr::vkDestroyImageView(self.Device, imageView, nullptr);
		}
	};
	using ImageViewUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkImageView>, ImageViewDeleter>;

	class ImageView
	{
	public:
		struct Factory;

		constexpr ImageView() = default;
		constexpr ImageView(ImageView&&) = default;
		constexpr auto operator=(ImageView&&) -> ImageView & = default;

		ImageView(ImageViewUniquePtr handleIn)
			: Handle(std::move(handleIn))
		{
			if (not Handle)
				throw Error::RuntimeError{ "Image view handle must not be null." };
		}

		[[nodiscard]]
		constexpr auto GetHandle(this const ImageView& self) noexcept -> vkr::VkImageView
		{
			return self.Handle.get();
		}

		constexpr auto Destroy(this auto&& self) noexcept
		{
			self.Handle.reset();
		}

		[[nodiscard]]
		constexpr auto Detach(this ImageView& self) noexcept -> ImageViewUniquePtr
		{
			return std::move(self.Handle);
		}

	private:
		ImageViewUniquePtr Handle;
	};

	struct ImageView::Factory
	{
		vkr::VkDevice					Device = nullptr;
		void*							Next = nullptr;
		vkr::VkImageViewCreateFlags     Flags = 0;
		vkr::VkImage                    Image = nullptr;
		vkr::VkImageViewType            ViewType = vkr::VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		vkr::VkFormat                   Format = vkr::VkFormat::VK_FORMAT_UNDEFINED;
		vkr::VkComponentMapping         Components = {};
		vkr::VkImageSubresourceRange	SubresourceRange = {};

		auto ToVulkanStruct(this const ImageView::Factory& self) noexcept -> vkr::VkImageViewCreateInfo
		{
			return vkr::VkImageViewCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = self.Next,
				.flags = self.Flags,
				.image = self.Image,
				.viewType = self.ViewType,
				.format = self.Format,
				.components = self.Components,
				.subresourceRange = self.SubresourceRange
			};
		}

		auto operator()(this const ImageView::Factory& self) -> ImageViewUniquePtr
		{
			if (not self.Device)
				throw Error::RuntimeError{ "Device must not be null to create an image view." };

			auto createInfo = vkr::VkImageViewCreateInfo{ self.ToVulkanStruct() };
			auto imageView = vkr::VkImageView{};
			auto result = Result{
				vkr::vkCreateImageView(
					self.Device,
					&createInfo,
					nullptr,
					&imageView
				) };
			if (not result)
				throw VulkanError{ result, "Failed to create image view." };

			return ImageViewUniquePtr{ imageView, ImageViewDeleter{ self.Device } };
		}
	};
}

// ImageWithMemory
export namespace Vulkan
{
	struct ImageWithMemory
	{
		vkr::VkDevice Device{};
		Image Image{};
		DeviceMemory Memory{};
	};

	auto CreateImage2(
		glm::ivec2 size,
		vkr::VkBufferUsageFlags usage,
		vkr::VkMemoryPropertyFlags properties,
		vkr::VkDevice device,
		vkr::VkPhysicalDevice physicalDevice
	) -> ImageWithMemory
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
		auto image = vkr::VkImage{};
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

		return ImageWithMemory{ 
			device, 
			Image{ ImageUniquePtr{ image, ImageDeleter{ device } } }, 
			DeviceMemory{ MemoryUniquePtr{ memory, DeviceMemoryDeleter{ device } } } 
		};
	}
}
