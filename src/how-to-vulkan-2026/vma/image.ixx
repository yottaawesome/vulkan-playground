export module vulkan26:vma.image;
import std;
import :vulkan.exports;
import :vma.exports;
import :error;
import :raii;

export namespace vma
{
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
	
	using VmaImage = Raii::TypedResource<VmaImageUniquePtr>;
}
