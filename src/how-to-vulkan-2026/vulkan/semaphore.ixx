export module vulkan26:vulkan.semaphore;
import std;
import vulkanlib;
import :vulkan.error;

export namespace Vulkan26
{
	class SemaphoreDeleter
	{
	public:
		constexpr SemaphoreDeleter() = default;
		SemaphoreDeleter(VkDevice device)
			: device(device)
		{
			if (not device)
				throw std::runtime_error{ "Device handle cannot be null for SemaphoreDeleter" };
		}
		void operator()(this auto&& self, VkSemaphore semaphore) noexcept
		{
			vkDestroySemaphore(self.device, semaphore, nullptr);
		}
		constexpr auto GetDevice() const noexcept -> VkDevice
		{
			return device;
		}
	private:
		VkDevice device{};
	};
	using SemaphoreUniquePtr = std::unique_ptr<std::remove_pointer_t<VkSemaphore>, SemaphoreDeleter>;

	auto CreateSemaphoreUniquePtr(VkDevice device, VkSemaphoreCreateFlags flags = 0) -> SemaphoreUniquePtr
	{
		auto semaphoreCI = VkSemaphoreCreateInfo{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.flags = flags
		};
		auto semaphore = VkSemaphore{};
		auto result = vk::Result{ vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore) };
		if (not result)
			throw vk::Error{ result, "Failed creating semaphore " };
		return SemaphoreUniquePtr{ semaphore, SemaphoreDeleter{ device } };
	}

	class Semaphore
	{
	public:
		Semaphore() = default;
		Semaphore(SemaphoreUniquePtr handleIn)
			: handle(std::move(handleIn))
		{ }
		auto GetHandle() const noexcept -> VkSemaphore
		{
			return handle.get();
		}
	private:
		SemaphoreUniquePtr handle;
	};
}
