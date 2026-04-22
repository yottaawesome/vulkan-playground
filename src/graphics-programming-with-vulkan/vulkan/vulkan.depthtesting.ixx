export module vulkangfx:vulkan.depthtesting;
import std;
import :error;
import :glm;
import :concepts;
import :vulkan.exports;
import :vulkan.error;
import :vulkan.image;

export namespace Vulkan
{
	class DepthImage
	{
	public:
		class Factory;
		DepthImage(vkr::VkFormat formatIn, ImageUniquePtr imageIn, ImageViewUniquePtr imageViewIn)
			: format(formatIn), image(std::move(imageIn)), imageView(std::move(imageViewIn))
		{
			if (not image)
				throw ::Error::RuntimeError{ "Image cannot be nullptr." };
			if (not imageView)
				throw ::Error::RuntimeError{ "Image view cannot be nullptr." };
		}

		static auto PickDepthFormat(vkr::VkPhysicalDevice physicalDevice, Concepts::RangeOf<vkr::VkFormat> auto&& candidateFormats) -> vkr::VkFormat
		{
			if (not physicalDevice)
				throw ::Error::RuntimeError{ "Physical device cannot be nullptr." };

			auto predicate = 
				[physicalDevice = physicalDevice](vkr::VkFormat format) -> bool
				{
					auto formatProperties = vkr::VkFormatProperties{};
					vkr::vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
					return formatProperties.optimalTilingFeatures& vkr::VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
				};

			auto filter = candidateFormats | std::ranges::views::filter(predicate);
			return filter.empty() 
				? throw VulkanError{ "Failed to find a supported depth format." } 
				: filter.front();
		}

		constexpr auto GetImage(this const DepthImage& self) noexcept -> vkr::VkImage
		{
			return self.image.get();
		}
		constexpr auto GetImageView(this const DepthImage& self) noexcept -> vkr::VkImageView
		{
			return self.imageView.get();
		}
		constexpr auto GetFormat(this const DepthImage& self) noexcept -> vkr::VkFormat
		{
			return self.format;
		}

	private:
		vkr::VkFormat format{};
		ImageUniquePtr image;
		ImageViewUniquePtr imageView;
	};

	class DepthImage::Factory
	{
	};
}
