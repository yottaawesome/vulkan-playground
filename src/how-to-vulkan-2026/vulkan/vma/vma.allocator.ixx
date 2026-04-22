export module vulkan26:vulkan.vma.allocator;
import std;
import :vulkan.exports;
import :vulkan.error;

export namespace vma
{
	class Allocator
	{
	public:
		~Allocator()
		{
			if (allocator)
			{
				vmaDestroyAllocator(allocator);
				allocator = nullptr;
			}
		}
		constexpr Allocator(VmaAllocator allocatorIn)
			: allocator(allocatorIn)
		{
			if (not allocator)
				throw vk::Error{ VkResult::VK_ERROR_INITIALIZATION_FAILED };
		}
		constexpr auto Get(this const auto& self) noexcept -> VmaAllocator
		{
			return self.allocator;
		}
	private:
		VmaAllocator allocator = nullptr;
	};
}