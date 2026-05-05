export module vulkan26:vulkan.image;
import std;
import vulkanlib;
import :vulkan.error;
import :vulkan.resource;
import :error;
import :raii;

export namespace vk
{
	//
	//
	//
	class ImageViewDeleter
	{
	public:
		constexpr ImageViewDeleter(VkDevice deviceIn)
			: device(deviceIn)
		{
			if (not device)
				throw ::Error::RuntimeError{"device cannot be nullptr"};
		}
		auto operator()(this const ImageViewDeleter& self, VkImageView imageView)
		{
			vkDestroyImageView(self.device, imageView, nullptr);
		}
	private:
		VkDevice device = nullptr;
	};
	using ImageViewUniquePtr = std::unique_ptr<std::remove_pointer_t<VkImageView>, ImageViewDeleter>;

	//
	//
	//
	class ImageView : public Raii::TypedResource<ImageViewUniquePtr>
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
		constexpr ImageDeleter(VkDevice deviceIn) 
			: DeviceBasedDeleter(deviceIn) 
		{ }
		constexpr auto operator()(this const ImageDeleter& self, VkImage image) noexcept
		{
			vkDestroyImage(self.device, image, nullptr);
		}
	};
	using ImageUniquePtr = std::unique_ptr<std::remove_pointer_t<VkImage>, ImageDeleter>;

	

	//
	//
	//
	using Image = Raii::TypedResource<ImageUniquePtr>;

	template<typename T>
	concept ImageLike = requires(T a) {
		{ *a } -> std::same_as<VkImage>;
		a.Destroy();
	};

	template<ImageLike T>
	class DepthImage
	{
	public:
		constexpr DepthImage(T depthImageIn, ImageView depthImageViewIn)
			: depthImage(std::move(depthImageIn)), depthImageView(std::move(depthImageViewIn))
		{ 
			if (not depthImage)
				throw ::Error::RuntimeError{ "Depth image cannot be null" };
			if (not depthImageView)
				throw ::Error::RuntimeError{ "Depth image view cannot be null" };
		}
		constexpr auto GetImage(this const DepthImage& self) noexcept -> VkImage
		{
			return *self.depthImage;
		}
		constexpr auto GetView(this const DepthImage& self) noexcept -> VkImageView
		{
			return *self.depthImageView;
		}
		constexpr auto Destroy(this auto& self) noexcept
		{
			self.depthImage.Destroy();
			self.depthImageView.Destroy();
		}
	private:
		T depthImage;
		ImageView depthImageView;
	};
}
