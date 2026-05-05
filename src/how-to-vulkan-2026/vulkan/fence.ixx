export module vulkan26:vulkan.fence;
import std;
import vulkanlib;
import :vulkan.error;

export namespace Vulkan26
{
	class FenceDeleter
	{
	public:
		FenceDeleter(VkDevice device)
			: device(device)
		{
			if (not device)
				throw std::runtime_error{ "Device handle cannot be null for FenceDeleter" };
		}
		void operator()(this auto&& self, VkFence fence) noexcept
		{
			vkDestroyFence(self.device, fence, nullptr);
		}
		constexpr auto GetDevice() const noexcept -> VkDevice
		{
			return device;
		}
	private:
		VkDevice device{};
	};
	using FenceUniquePtr = std::unique_ptr<std::remove_pointer_t<VkFence>, FenceDeleter>;

	auto CreateFenceUniquePtr(VkDevice device, VkFenceCreateFlags flags = 0)
	{
		 auto fenceCI = VkFenceCreateInfo{ 
			 .sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, 
			 .flags = flags 
		 };
		 auto fence = VkFence{};
		 auto result = vk::Result{vkCreateFence(device, &fenceCI, nullptr, &fence)};
		 if (not result)
			 throw vk::Error{ result, "Failed creating fence "};
		 return FenceUniquePtr{ fence, FenceDeleter{ device } };
	}

	class Fence
	{
	public:
		Fence(FenceUniquePtr handleIn)
			: handle(std::move(handleIn))
		{}
		auto GetHandle() const noexcept -> VkFence
		{
			return handle.get();
		}

	private:
		FenceUniquePtr handle;
	};
}