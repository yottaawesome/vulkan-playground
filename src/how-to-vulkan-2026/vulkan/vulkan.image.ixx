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
			// According to the VMA header, vmaDestroyImage() is shorthand for vkDestroyImage() and vmaFreeMemory().
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
	using Image = TypedResource<ImageUniquePtr>;
	using VmaImage = TypedResource<VmaImageUniquePtr>;

	class DepthImage
	{
	public:
		constexpr DepthImage(VmaImage depthImageIn, ImageView depthImageViewIn)
			: depthImage(std::move(depthImageIn)), depthImageView(std::move(depthImageViewIn))
		{ 
			if (not depthImage)
				throw ::Error::RuntimeError{ "Depth image cannot be null" };
			if (not depthImageView)
				throw ::Error::RuntimeError{ "Depth image view cannot be null" };
		}
		constexpr auto GetImage(this const DepthImage& self) noexcept -> vk::VkImage
		{
			return *self.depthImage;
		}
		constexpr auto GetView(this const DepthImage& self) noexcept -> vk::VkImageView
		{
			return *self.depthImageView;
		}
		constexpr auto Destroy(this auto& self) noexcept
		{
			self.depthImage.Destroy();
			self.depthImageView.Destroy();
		}
	private:
		VmaImage depthImage;
		ImageView depthImageView;
	};
}
