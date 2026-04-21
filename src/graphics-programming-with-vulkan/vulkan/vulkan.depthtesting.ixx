export module vulkangfx:vulkan.depthtesting;
import std;
import :error;
import :vulkan.exports;
import :vulkan.error;
import :glm;

export namespace Vulkan
{
	class ImageDeleter
	{
	public:
		constexpr ImageDeleter() = default;
		ImageDeleter(vkr::VkDevice device) 
			: device(device) 
		{
			if(not device)
				throw ::Error::RuntimeError{ "Device cannot be nullptr." };
		}
		void operator()(this const ImageDeleter& self, vkr::VkImage image)
		{
			if (image)
				vkr::vkDestroyImage(self.device, image, nullptr);
		}
		constexpr auto GetDevice(this const ImageDeleter& self) noexcept -> vkr::VkDevice
		{
			return self.device;
		}
	private:
		vkr::VkDevice device = nullptr;;
	};
	using UniqueImagePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkImage>, ImageDeleter>;

	struct ImageFactory
	{
		glm::ivec2 size{};
		vkr::VkFormat format{};
		vkr::VkImageUsageFlags usage{};
		vkr::VkMemoryPropertyFlags properties{};
		vkr::VkDevice device{};
		vkr::VkPhysicalDevice physicalDevice{};
		vkr::VkImageCreateInfo imageCreateInfo{};

		auto CreateImage() -> UniqueImagePtr
		{
			return {};
		}
	};

	class DepthImage
	{
	public:
		class Factory;
	};

	class DepthImage::Factory
	{
	};
}
