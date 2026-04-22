export module vulkangfx:vulkan.depthtesting;
import std;
import :error;
import :glm;
import :concepts;
import :vulkan.exports;
import :vulkan.error;
import :vulkan.image;
import :vulkan.memory;

export namespace Vulkan
{
	class DepthImage
	{
	public:
		struct Factory;

		constexpr DepthImage() = default;
		constexpr DepthImage(DepthImage&&) = default;
		constexpr auto operator=(DepthImage&&) -> DepthImage& = default;

		DepthImage(vkr::VkFormat formatIn, Image imageIn, DeviceMemory memoryIn, ImageView viewIn)
			: format(formatIn)
			, image(std::move(imageIn))
			, memory(std::move(memoryIn))
			, view(std::move(viewIn))
		{
		}

		static auto PickDepthFormat(vkr::VkPhysicalDevice physicalDevice, Concepts::RangeOf<vkr::VkFormat> auto&& candidateFormats) -> vkr::VkFormat
		{
			if (not physicalDevice)
				throw ::Error::RuntimeError{ "Physical device cannot be nullptr." };

			auto predicate =
				[physicalDevice](vkr::VkFormat format) -> bool
				{
					auto formatProperties = vkr::VkFormatProperties{};
					vkr::vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
					return (formatProperties.optimalTilingFeatures & vkr::VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
				};

			auto it = std::ranges::find_if(candidateFormats, predicate);
			if (it == std::ranges::end(candidateFormats))
				throw ::Error::RuntimeError{ "Failed to find a supported depth format." };
			return *it;
		}

		static constexpr auto HasStencilComponent(vkr::VkFormat format) noexcept -> bool
		{
			return format == vkr::VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT
				or format == vkr::VkFormat::VK_FORMAT_D24_UNORM_S8_UINT
				or format == vkr::VkFormat::VK_FORMAT_D16_UNORM_S8_UINT;
		}

		constexpr auto GetImage(this const DepthImage& self) noexcept -> vkr::VkImage
		{
			return self.image.GetHandle();
		}
		constexpr auto GetImageView(this const DepthImage& self) noexcept -> vkr::VkImageView
		{
			return self.view.GetHandle();
		}
		constexpr auto GetFormat(this const DepthImage& self) noexcept -> vkr::VkFormat
		{
			return self.format;
		}

	private:
		vkr::VkFormat format{};
		Image image;
		DeviceMemory memory;
		ImageView view;
	};

	struct DepthImage::Factory
	{
		vkr::VkDevice Device{};
		vkr::VkPhysicalDevice PhysicalDevice{};
		vkr::VkExtent2D Extent{};
		std::span<const vkr::VkFormat> CandidateFormats{};

		auto operator()(this const DepthImage::Factory& self) -> DepthImage
		{
			if (not self.Device)
				throw ::Error::RuntimeError{ "Device cannot be nullptr." };
			if (not self.PhysicalDevice)
				throw ::Error::RuntimeError{ "Physical device cannot be nullptr." };
			if (self.CandidateFormats.empty())
				throw ::Error::RuntimeError{ "At least one candidate depth format must be provided." };

			auto format = DepthImage::PickDepthFormat(self.PhysicalDevice, self.CandidateFormats);

			auto imagePtr = ImageFactory{
				.Device = self.Device,
				.ImageCreateInfo = {
					.imageType = vkr::VkImageType::VK_IMAGE_TYPE_2D,
					.format = format,
					.extent = { self.Extent.width, self.Extent.height, 1 },
					.mipLevels = 1,
					.arrayLayers = 1,
					.samples = vkr::VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
					.tiling = vkr::VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
					.usage = vkr::VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
					.sharingMode = vkr::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
					.initialLayout = vkr::VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
				}
			}.CreateImage();
			auto imageHandle = imagePtr.get();

			auto memoryPtr = CreateMemory(
				self.Device,
				imageHandle,
				self.PhysicalDevice,
				vkr::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);

			auto memory = DeviceMemory{ std::move(memoryPtr) };
			memory.BindToImage(self.Device, imageHandle);

			using Aspect = vkr::VkImageAspectFlagBits;
			auto aspectMask = Aspect::VK_IMAGE_ASPECT_DEPTH_BIT;
			if (DepthImage::HasStencilComponent(format))
				aspectMask = static_cast<Aspect>(aspectMask | Aspect::VK_IMAGE_ASPECT_STENCIL_BIT);

			auto viewPtr = ImageView::Factory{
				.Device = self.Device,
				.Image = imageHandle,
				.ViewType = vkr::VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
				.Format = format,
				.SubresourceRange = {
					.aspectMask = static_cast<std::uint32_t>(aspectMask),
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			}();

			return DepthImage{
				format,
				Image{ std::move(imagePtr) },
				std::move(memory),
				ImageView{ std::move(viewPtr) }
			};
		}
	};
}
