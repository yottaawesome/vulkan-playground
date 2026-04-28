export module vulkan26:vulkan.semaphore;
import std;
import :vulkan.exports;

export namespace Vulkan26
{
	class SemaphoreDeleter
	{
	public:
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

	class Semaphore
	{
	public:
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
