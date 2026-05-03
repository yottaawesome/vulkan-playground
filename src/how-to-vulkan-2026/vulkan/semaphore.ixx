export module vulkan26:vulkan.semaphore;
import std;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan26
{
	class SemaphoreDeleter
	{
	public:
		constexpr SemaphoreDeleter() = default;
		SemaphoreDeleter(vk::VkDevice device)
			: device(device)
		{
			if (not device)
				throw std::runtime_error{ "Device handle cannot be null for SemaphoreDeleter" };
		}
		void operator()(this auto&& self, vk::VkSemaphore semaphore) noexcept
		{
			vk::vkDestroySemaphore(self.device, semaphore, nullptr);
		}
		constexpr auto GetDevice() const noexcept -> vk::VkDevice
		{
			return device;
		}
	private:
		vk::VkDevice device{};
	};
	using SemaphoreUniquePtr = std::unique_ptr<std::remove_pointer_t<vk::VkSemaphore>, SemaphoreDeleter>;

	auto CreateSemaphoreUniquePtr(vk::VkDevice device, vk::VkSemaphoreCreateFlags flags = 0) -> SemaphoreUniquePtr
	{
		auto semaphoreCI = vk::VkSemaphoreCreateInfo{
			.sType = vk::VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.flags = flags
		};
		auto semaphore = vk::VkSemaphore{};
		auto result = vk::Result{ vk::vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore) };
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
		auto GetHandle() const noexcept -> vk::VkSemaphore
		{
			return handle.get();
		}
	private:
		SemaphoreUniquePtr handle;
	};
}
