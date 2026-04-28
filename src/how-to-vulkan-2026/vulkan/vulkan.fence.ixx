export module vulkan26:vulkan.fence;
import std;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan26
{
	class FenceDeleter
	{
	public:
		FenceDeleter(vk::VkDevice device)
			: device(device)
		{
			if (not device)
				throw std::runtime_error{ "Device handle cannot be null for FenceDeleter" };
		}
		void operator()(this auto&& self, vk::VkFence fence) noexcept
		{
			vk::vkDestroyFence(self.device, fence, nullptr);
		}
		constexpr auto GetDevice() const noexcept -> vk::VkDevice
		{
			return device;
		}
	private:
		vk::VkDevice device{};
	};
	using FenceUniquePtr = std::unique_ptr<std::remove_pointer_t<vk::VkFence>, FenceDeleter>;

	auto CreateFenceUniquePtr(vk::VkDevice device, vk::VkFenceCreateFlags flags = 0)
	{
		 auto fenceCI = vk::VkFenceCreateInfo{ 
			 .sType = vk::VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, 
			 .flags = flags 
		 };
		 auto fence = vk::VkFence{};
		 auto result = vk::Result{vk::vkCreateFence(device, &fenceCI, nullptr, &fence)};
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
		auto GetHandle() const noexcept -> vk::VkFence
		{
			return handle.get();
		}

	private:
		FenceUniquePtr handle;
	};
}