export module vulkan26:vulkan.image;
import std;
import :vulkan.exports;
import :vulkan.error;
import :vulkan.resource;
import :error;

export namespace vk
{
	//
	//
	//
	class ImageViewDeleter
	{
	public:
		constexpr ImageViewDeleter(vk::VkDevice deviceIn)
			: device(deviceIn)
		{
			if (not device)
				throw ::Error::RuntimeError{"device cannot be nullptr"};
		}
		auto operator()(this const ImageViewDeleter& self, vk::VkImageView imageView)
		{
			vk::vkDestroyImageView(self.device, imageView, nullptr);
		}
	private:
		vk::VkDevice device = nullptr;
	};
	using ImageViewUniquePtr = std::unique_ptr<std::remove_pointer_t<vk::VkImageView>, ImageViewDeleter>;

	//
	//
	//
	class ImageView : public TypedResource<ImageViewUniquePtr>
	{
	public:
		constexpr ImageView(ImageViewUniquePtr imageViewIn)
			: TypedResource(std::move(imageViewIn))
		{ }
	};

	//
	//
	// ImageDeleter
	class ImageDeleter : public DeviceBasedDeleter
	{
	public:
		constexpr ImageDeleter() = default;
		constexpr ImageDeleter(vk::VkDevice deviceIn) 
			: DeviceBasedDeleter(deviceIn) 
		{ }
		constexpr auto operator()(this const ImageDeleter& self, vk::VkImage image) noexcept
		{
			vk::vkDestroyImage(self.device, image, nullptr);
		}
	};
	using ImageUniquePtr = std::unique_ptr<std::remove_pointer_t<vk::VkImage>, ImageDeleter>;

	// According to the VMA header, vmaDestroyImage() is shorthand for vkDestroyImage() and vmaFreeMemory().
	class VmaImageDeleter
	{
	public:
		constexpr VmaImageDeleter() = default;
		constexpr VmaImageDeleter(vma::VmaAllocator allocatorIn, vma::VmaAllocation allocationIn)
			: allocator(allocatorIn), allocation(allocationIn)
		{
			if (not allocator)
				throw ::Error::RuntimeError{ "Invalid allocator" };
			if (not allocation)
				throw ::Error::RuntimeError{ "Invalid allocation" };
		}
		auto operator()(this const VmaImageDeleter& self, vk::VkImage image)
		{
			vma::vmaDestroyImage(self.allocator, image, self.allocation);
		}
		constexpr auto GetAllocator(this const VmaImageDeleter& self) -> vma::VmaAllocator
		{
			return self.allocator;
		}
		constexpr auto GetAllocation(this const VmaImageDeleter& self) -> vma::VmaAllocation
		{
			return self.allocation;
		}
	private:
		vma::VmaAllocator allocator{};
		vma::VmaAllocation allocation{};
	};
	using VmaImageUniquePtr = std::unique_ptr<std::remove_pointer_t<vk::VkImage>, VmaImageDeleter>;

	//
	//
	//
	using Image = TypedResource<ImageDeleter>;
	using VmaImage = TypedResource<VmaImageDeleter>;
}
