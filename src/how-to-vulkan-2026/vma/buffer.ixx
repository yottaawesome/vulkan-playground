export module vulkan26:vma.buffer;
import std;
import vulkanlib;
import :error;

export namespace vma
{
	class VmaBufferDeleter
	{
	public:
		constexpr VmaBufferDeleter(
			VmaAllocator allocatorIn,
			VmaAllocation allocationIn
		) : allocator(allocatorIn), allocation(allocationIn)
		{
			if (not allocator)
				throw ::Error::RuntimeError{ "allocator cannot be null" };
			if (not allocation)
				throw ::Error::RuntimeError{ "allocation cannot be null" };
		}
		auto operator()(VkBuffer buffer) noexcept
		{
			vmaDestroyBuffer(allocator, buffer, allocation);
		}
		constexpr auto GetAllocator(this const VmaBufferDeleter& self) noexcept -> VmaAllocator
		{
			return self.allocator;
		}
		constexpr auto GetAllocation(this const VmaBufferDeleter& self) noexcept -> VmaAllocation
		{
			return self.allocation;
		}
	private:
		VmaAllocator allocator = nullptr;
		VmaAllocation allocation = nullptr;
	};
	using VmaBufferUniquePtr = std::unique_ptr<std::remove_pointer_t<VkBuffer>, VmaBufferDeleter>;

	class VmaBuffer [[nodiscard]]
	{
	public:
		VmaBuffer(VmaBufferUniquePtr bufferIn)
			: buffer(std::move(bufferIn))
		{ }
		constexpr auto GetBuffer(this const VmaBuffer& self) -> VkBuffer
		{
			return self.buffer.get();
		}
		constexpr auto GetAllocation(this const VmaBuffer& self) -> VmaAllocation
		{
			return self.buffer.get_deleter().GetAllocation();
		}
		constexpr auto GetAllocator(this const VmaBuffer& self) -> VmaAllocator
		{
			return self.buffer.get_deleter().GetAllocator();
		}
	private:
		VmaBufferUniquePtr buffer;
	};
}