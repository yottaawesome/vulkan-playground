export module vulkan26:vulkan.buffer;
import std;
import :vulkan.exports;
import :error;

export namespace vma
{
	class VmaBufferDeleter
	{
	public:
		constexpr VmaBufferDeleter(
			vma::VmaAllocator allocatorIn,
			vma::VmaAllocation allocationIn
		) : allocator(allocatorIn), allocation(allocationIn)
		{
			if (not allocator)
				throw ::Error::RuntimeError{ "allocator cannot be null" };
			if (not allocation)
				throw ::Error::RuntimeError{ "allocation cannot be null" };
		}
		auto operator()(VkBuffer buffer) noexcept
		{
			vma::vmaDestroyBuffer(allocator, buffer, allocation);
		}
		constexpr auto GetAllocator(this const VmaBufferDeleter& self) noexcept -> vma::VmaAllocator
		{
			return self.allocator;
		}
		constexpr auto GetAllocation(this const VmaBufferDeleter& self) noexcept -> vma::VmaAllocation
		{
			return self.allocation;
		}
	private:
		vma::VmaAllocator allocator = nullptr;
		vma::VmaAllocation allocation = nullptr;
	};
	using VmaBufferUniquePtr = std::unique_ptr<std::remove_pointer_t<VkBuffer>, VmaBufferDeleter>;

	class VmaBuffer 
	{
	public:
		VmaBuffer(VmaBufferUniquePtr bufferIn)
			: buffer(std::move(bufferIn))
		{ }
		constexpr auto GetBuffer(this const VmaBuffer& self) -> VkBuffer
		{
			return self.buffer.get();
		}
		constexpr auto GetAllocation(this const VmaBuffer& self) -> vma::VmaAllocation
		{
			return self.buffer.get_deleter().GetAllocation();
		}
		constexpr auto GetAllocator(this const VmaBuffer& self) -> vma::VmaAllocator
		{
			return self.buffer.get_deleter().GetAllocator();
		}
	private:
		VmaBufferUniquePtr buffer;
	};
}