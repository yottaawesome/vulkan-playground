export module vulkangfx:vulkan.swapchain;
import std;
import :error;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan
{
	struct SwapchainDeleter
	{
	public:
		SwapchainDeleter(const SwapchainDeleter&) = delete;
		SwapchainDeleter& operator=(const SwapchainDeleter&) = delete;

		SwapchainDeleter(SwapchainDeleter&&) = default;
		SwapchainDeleter& operator=(SwapchainDeleter&&) = default;

		SwapchainDeleter(vkr::VkDevice device)
			: Device(device) 
		{
			if (not Device)
				throw Error::RuntimeError{"Device must not be null for SwapchainDeleter."};
		}

		void operator()(this SwapchainDeleter& self, vkr::VkSwapchainKHR swapchain) noexcept
		{
			if (swapchain)
				vkr::vkDestroySwapchainKHR(self.Device, swapchain, nullptr);
		}
	private:
		vkr::VkDevice Device = nullptr;
	};
	using VkSwapchainKHRUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkSwapchainKHR>, SwapchainDeleter>;

	struct SwapchainFactory
	{
		vkr::VkDevice Device = nullptr;
		vkr::VkSwapchainCreateInfoKHR CreateInfo{};

		auto CreateSwapchainKHR(this SwapchainFactory& self) -> VkSwapchainKHRUniquePtr
		{
			if (not self.Device)
				throw Error::RuntimeError{"Device must not be null to create a swapchain."};

			self.CreateInfo.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

			auto swapchain = vkr::VkSwapchainKHR{ nullptr };
			if (auto result = Result{ vkr::vkCreateSwapchainKHR(self.Device, &self.CreateInfo, nullptr, &swapchain) })
				throw VulkanError{ result, "Failed to create swapchain." };
			return VkSwapchainKHRUniquePtr(swapchain, SwapchainDeleter(self.Device));
		}
	};

	class Swapchain
	{
	public:
		Swapchain(VkSwapchainKHRUniquePtr handle)
			: handle(std::move(handle))
		{
			if (not this->handle)
				throw Error::RuntimeError{"Swapchain handle must not be null."};
		}

		constexpr auto GetHandle(this const Swapchain& self) noexcept -> vkr::VkSwapchainKHR
		{
			return self.handle.get();
		}
	private:
		VkSwapchainKHRUniquePtr handle;
	};
}