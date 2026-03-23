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

		// Can be signaled and waited on by the GPU and the CPU.
		class TimelineSemaphore
		{
		public:
			static auto Create(vkr::VkDevice device, std::uint64_t initialValue = 0) -> TimelineSemaphore
			{
				if (not device)
					throw Error::RuntimeError{ "Device must not be null to create a timeline semaphore." };

				auto timelineCreateInfo = vkr::VkSemaphoreTypeCreateInfo{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
					.pNext = nullptr,
					.semaphoreType = vkr::VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE,
					.initialValue = initialValue
				};
				auto createInfo = vkr::VkSemaphoreCreateInfo{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
					.pNext = &timelineCreateInfo,
					.flags = 0
				};
				
				auto timeline = vkr::VkSemaphore{};
				auto result = vkr::VkResult{ vkr::vkCreateSemaphore(device, &createInfo, nullptr, &timeline) };
				if (not result)
					throw VulkanError{ result, "Failed to create timeline semaphore." };
				return TimelineSemaphore{ SemaphoreUniquePtr{ timeline, SemaphoreDeleter{ device }} };
			}

			auto Wait(
				this const TimelineSemaphore& self,
				std::uint64_t semaphoreValue,
				std::chrono::nanoseconds timeout = std::chrono::nanoseconds{ std::numeric_limits<std::uint64_t>::max() }
			) -> Vulkan::Result
			{
				auto sems = std::array{ self.handle.get() };
				auto semValues = std::array{ semaphoreValue };
				auto waitInfo = vkr::VkSemaphoreWaitInfo{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
					.pNext = nullptr,
					.flags = 0,
					.semaphoreCount = 1,
					.pSemaphores = sems.data(),
					.pValues = semValues.data()
				};
				return vkr::vkWaitSemaphores(self.handle.get_deleter().Device, &waitInfo, timeout.count());
			}

			auto Signal(this const TimelineSemaphore& self, std::uint64_t semaphoreValue) -> Result
			{
				auto signalInfo = vkr::VkSemaphoreSignalInfo{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
					.pNext = nullptr,
					.semaphore = self.handle.get(),
					.value = semaphoreValue
				};
				return vkr::vkSignalSemaphore(self.handle.get_deleter().Device, &signalInfo);
			}

			[[nodiscard]]
			auto GetHandle(this const TimelineSemaphore& self) noexcept -> vkr::VkSemaphore
			{
				return self.handle.get();
			}

			using QueryResult = std::expected<std::uint64_t, Result>;

			auto Query(this const TimelineSemaphore& self) -> QueryResult
			{
				auto value = std::uint64_t{};
				auto result = vkr::vkGetSemaphoreCounterValue(self.handle.get_deleter().Device, self.handle.get(), &value);
				if (not result)
					return std::unexpected{ result };
				return value;
			}

		private:
			TimelineSemaphore(SemaphoreUniquePtr handleIn)
				: handle(std::move(handleIn))
			{ }

			SemaphoreUniquePtr handle;
		};

		// Waited on and signalled by the GPU only
		class BinarySemaphore
		{
		public:
			[[nodiscard]]
			static auto Create(vkr::VkDevice device) -> BinarySemaphore
			{
				if (not device)
					throw Error::RuntimeError{ "Device must not be null to create a semaphore." };

				auto createInfo = vkr::VkSemaphoreCreateInfo{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
				};
				auto semaphore = vkr::VkSemaphore{};
				auto result = Result{ vkr::vkCreateSemaphore(device, &createInfo, nullptr, &semaphore) };
				if (not result)
					throw VulkanError{ result, "Failed to create semaphore." };

				return BinarySemaphore{ SemaphoreUniquePtr{ semaphore, SemaphoreDeleter{ device } } };
			}

			[[nodiscard]]
			auto GetHandle(this const BinarySemaphore& self) noexcept -> vkr::VkSemaphore
			{
				return self.handle.get();
			}

		private:
			BinarySemaphore(SemaphoreUniquePtr handleIn)
				: handle(std::move(handleIn))
			{ }

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

		class Fence
		{
		public:
			[[nodiscard]]
			static auto Create(vkr::VkDevice device, bool signaled = false) -> Fence
			{
				if (not device)
					throw Error::RuntimeError{ "Device must not be null to create a fence." };
				auto createInfo = vkr::VkFenceCreateInfo{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
					.flags = signaled ? vkr::VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT : 0u
				};
				auto fence = vkr::VkFence{};
				auto result = Result{ vkr::vkCreateFence(device, &createInfo, nullptr, &fence) };
				if (not result)
					throw VulkanError{ result, "Failed to create fence." };

				return Fence{ FenceUniquePtr{ fence, FenceDeleter{ device } } };
			}

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
				auto fences = std::array{ self.handle.get() };
				return { vkr::vkWaitForFences(self.handle.get_deleter().Device, static_cast<std::uint32_t>(fences.size()), fences.data(), true, timeout.count())};
			}
			auto Reset(this const Fence& self) -> Result
			{
				auto fences = std::array{ self.handle.get() };
				return { vkr::vkResetFences(self.handle.get_deleter().Device, static_cast<std::uint32_t>(fences.size()), fences.data()) };
			}
			[[nodiscard]]
			auto GetStatus(this const Fence& self) -> Result
			{
				return vkr::vkGetFenceStatus(self.handle.get_deleter().Device, self.handle.get());
			}

		private:
			Fence(FenceUniquePtr handleIn)
				: handle(std::move(handleIn))
			{}

		private:
			FenceUniquePtr handle;
		};
	}
}
