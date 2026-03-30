export module vulkangfx:vulkan.devicequeue;
import std;
import :error;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan
{
	// DeviceQueues are implicitly created as part of the logical device creation process, and are not explicitly destroyed. 
	// They are implicitly destroyed when the logical device is destroyed.
	class DeviceQueue
	{
	public:
		DeviceQueue(vkr::VkPhysicalDevice physicalDevice, vkr::VkDevice device, std::uint32_t queueFamilyIndex, std::uint32_t queueIndex)
			: physicalDevice(physicalDevice), queueFamilyIndex(queueFamilyIndex), queueIndex(queueIndex)
		{ 
			if (not physicalDevice)
				throw Error::RuntimeError("Invalid physical device handle.");
			if (not device)
				throw Error::RuntimeError("Invalid device handle.");
			vkr::vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &queue);
		}

		constexpr auto GetQueue(this const DeviceQueue& self) noexcept -> vkr::VkQueue { return self.queue; }

		auto GetQueueFamilyProperties(this DeviceQueue& self) noexcept -> vkr::VkQueueFamilyProperties 
		{ 
			auto queueFamilyProperties = vkr::VkQueueFamilyProperties{};
			vkr::vkGetPhysicalDeviceQueueFamilyProperties(self.physicalDevice, &self.queueFamilyIndex, &queueFamilyProperties);
			return queueFamilyProperties;
		}

		auto WaitIdle(this const DeviceQueue& self) -> decltype(self)
		{
			auto result = Vulkan::Result{ vkr::vkQueueWaitIdle(self.queue) };
			if (not result)
				throw VulkanError{ result, "Failed to wait for device queue to become idle." };
			return self;
		}

		auto SubmitBuffer(
			this DeviceQueue& self, 
			vkr::VkCommandBuffer commandBuffer
		)
		{
			auto commandBuffers = std::array{ commandBuffer };
			auto submitInfo = vkr::VkSubmitInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount = 1,
				.pCommandBuffers = commandBuffers.data(),
			};
			auto result = Vulkan::Result{ vkr::vkQueueSubmit(self.queue, 1, &submitInfo, nullptr) };
			if (not result)
				throw VulkanError{ result, "Failed to submit command buffer to device queue." };
			return self;
		}

		auto SubmitBuffers(
			this const DeviceQueue& self,
			std::span<const vkr::VkCommandBuffer>& commandBuffers,
			std::span<const vkr::VkSemaphore>& waitSemaphores,
			std::span<const vkr::VkSemaphore>& signalSemaphores,
			vkr::VkFence fence = nullptr
		) -> decltype(self)
		{
			auto submitInfo = vkr::VkSubmitInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.waitSemaphoreCount = static_cast<std::uint32_t>(waitSemaphores.size()),
				.pWaitSemaphores = waitSemaphores.data(),
				.pWaitDstStageMask = nullptr,
				.commandBufferCount = static_cast<std::uint32_t>(commandBuffers.size()),
				.pCommandBuffers = commandBuffers.data(),
				.signalSemaphoreCount = static_cast<std::uint32_t>(signalSemaphores.size()),
				.pSignalSemaphores = signalSemaphores.data()
			};
			auto result = Vulkan::Result{ vkr::vkQueueSubmit(self.queue, 1, &submitInfo, fence) };
			if (not result)
				throw VulkanError{ result, "Failed to submit command buffers to device queue." };
			return self;
		}

	private:
		vkr::VkPhysicalDevice physicalDevice{};
		std::uint32_t queueFamilyIndex = 0;
		std::uint32_t queueIndex = 0;
		vkr::VkQueue queue = nullptr;
	};
}