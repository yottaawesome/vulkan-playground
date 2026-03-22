export module vulkangfx:vulkan.sync;
import std;
import :error;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan::Sync
{
	inline namespace Semaphores
	{
		struct SemaphoreDeleter
		{
			vkr::VkDevice Device = nullptr;
			SemaphoreDeleter(vkr::VkDevice device) : Device(device) 
			{
				if (not Device)
					throw Error::RuntimeError{ "SemaphoreDeleter requires a valid device." };
			}
			auto operator()(this auto&& self, vkr::VkSemaphore semaphore) noexcept -> void
			{
				vkr::vkDestroySemaphore(self.Device, semaphore, nullptr);
			}
		};
		using SemaphoreUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkSemaphore>, SemaphoreDeleter>;

		struct SemaphoreFactory
		{
			vkr::VkDevice Device = nullptr;
			vkr::VkSemaphoreCreateInfo CreateInfo{};
			[[nodiscard]]
			auto operator()(this auto&& self) -> SemaphoreUniquePtr
			{
				if (not self.Device)
					throw Error::RuntimeError{ "SemaphoreFactory requires a valid device." };
			
				self.CreateInfo.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				auto semaphore = vkr::VkSemaphore{};
				auto result = Result{vkr::vkCreateSemaphore(self.Device, &self.CreateInfo, nullptr, &semaphore)};
				if (not result)
					throw VulkanError{ result, "Failed to create semaphore." };
			
				return SemaphoreUniquePtr{ semaphore, SemaphoreDeleter{ self.Device } };
			}
		};

		class Semaphore
		{
		public:
			Semaphore(SemaphoreUniquePtr handleIn)
				: handle(std::move(handleIn))
			{ 
				if (not handle)
					throw Error::RuntimeError{ "Semaphore must have a valid handle." };
			}
			[[nodiscard]]
			auto GetHandle(this const Semaphore& self) noexcept -> vkr::VkSemaphore
			{
				return self.handle.get();
			}
		private:
			SemaphoreUniquePtr handle;
		};
	}

	inline namespace Fences
	{
		struct FenceDeleter
		{
			vkr::VkDevice Device = nullptr;
			FenceDeleter(vkr::VkDevice device) : Device(device)
			{
				if (not Device)
					throw Error::RuntimeError{ "FenceDeleter requires a valid device." };
			}
			auto operator()(this auto&& self, vkr::VkFence fence) noexcept -> void
			{
				vkr::vkDestroyFence(self.Device, fence, nullptr);
			}
		};
		using FenceUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkFence>, FenceDeleter>;

		struct FenceFactory
		{
			vkr::VkDevice Device = nullptr;
			vkr::VkFenceCreateInfo CreateInfo{};
			[[nodiscard]]
			auto operator()(this auto&& self) -> FenceUniquePtr
			{
				if (not self.Device)
					throw Error::RuntimeError{ "FenceFactory requires a valid device." };

				self.CreateInfo.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				auto fence = vkr::VkFence{};
				auto result = Result{ vkr::vkCreateFence(self.Device, &self.CreateInfo, nullptr, &fence) };
				if (not result)
					throw VulkanError{ result, "Failed to create fence." };

				return FenceUniquePtr{ fence, FenceDeleter{ self.Device } };
			}
		};

		class Fence
		{
		public:
			Fence(FenceUniquePtr handleIn)
				: handle(std::move(handleIn))
			{}
			[[nodiscard]]
			auto GetHandle(this const Fence& self) noexcept -> vkr::VkFence
			{
				return self.handle.get();
			}
			auto Wait(
				this const Fence& self, 
				std::chrono::nanoseconds timeout = std::chrono::nanoseconds{ std::numeric_limits<std::uint64_t>::max() }
			) -> Result
			{
				auto sems = std::array{ self.handle.get() };
				return { vkr::vkWaitForFences(self.handle.get_deleter().Device, 1, sems.data(), false, timeout.count())};
			}
		private:
			FenceUniquePtr handle;
		};
	}
}
