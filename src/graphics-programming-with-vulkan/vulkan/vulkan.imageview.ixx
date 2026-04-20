export module vulkangfx:vulkan.imageview;
import std;
import :error;
import :vulkan.exports;
import :vulkan.error;

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
	using VkImageViewUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkImageView>, ImageViewDeleter>;

	class ImageView
	{
	public:
		struct Factory;

		constexpr ImageView() = default;
		constexpr ImageView(ImageView&&) = default;
		constexpr auto operator=(ImageView&&) -> ImageView & = default;

		ImageView(VkImageViewUniquePtr handleIn)
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

	private:
		VkImageViewUniquePtr Handle;
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

		auto operator()(this const ImageView::Factory& self) -> VkImageViewUniquePtr
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

			return VkImageViewUniquePtr{ imageView, ImageViewDeleter{ self.Device } };
		}
	};
}
