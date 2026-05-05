export module vulkan26:vma.image;
import std;
import vulkanlib;
import :error;
import :raii;

export namespace vma
{
	class VmaImageDeleter
	{
	public:
		constexpr VmaImageDeleter() = default;
		constexpr VmaImageDeleter(VmaAllocator allocatorIn, VmaAllocation allocationIn)
			: allocator(allocatorIn), allocation(allocationIn)
		{
			if (not allocator)
				throw ::Error::RuntimeError{ "Invalid allocator" };
			if (not allocation)
				throw ::Error::RuntimeError{ "Invalid allocation" };
		}
		auto operator()(this const VmaImageDeleter& self, VkImage image)
		{
			// According to the VMA header, vmaDestroyImage() is shorthand for vkDestroyImage() and vmaFreeMemory().
			vmaDestroyImage(self.allocator, image, self.allocation);
		}
		constexpr auto GetAllocator(this const VmaImageDeleter& self) -> VmaAllocator
		{
			return self.allocator;
		}
		constexpr auto GetAllocation(this const VmaImageDeleter& self) -> VmaAllocation
		{
			return self.allocation;
		}
	private:
		VmaAllocator allocator{};
		VmaAllocation allocation{};
	};
	using VmaImageUniquePtr = std::unique_ptr<std::remove_pointer_t<VkImage>, VmaImageDeleter>;
	
	using VmaImage = Raii::TypedResource<VmaImageUniquePtr>;
}
