export module vulkantutorial:vulkanite.sync;
import std;
import :libs;

export namespace VulkanTutorial::Vulkanite::Sync
{
	struct Semaphore
	{
		vk::raii::Semaphore Mutex;
		auto operator->(this Semaphore& self) noexcept -> vk::raii::Semaphore*
		{
			return &self.Mutex;
		}
		operator bool(this const Semaphore& self) noexcept
		{
			return *self.Mutex != nullptr;
		}
		operator vk::raii::Semaphore& (this Semaphore& self) noexcept
		{
			return self.Mutex;
		}
	};
}
