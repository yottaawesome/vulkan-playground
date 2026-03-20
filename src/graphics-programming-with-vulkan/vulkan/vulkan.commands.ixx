export module vulkangfx:vulkan.commands;
import std;
import :error;
import :vulkan.error;
import :vulkan.exports;

export namespace Vulkan
{
	struct CommandBufferDeleter
	{
		vkr::VkDevice Device = nullptr;
		vkr::VkCommandPool CommandPool = nullptr;
		CommandBufferDeleter(vkr::VkDevice device, vkr::VkCommandPool commandPool)
			: Device(device), CommandPool(commandPool)
		{
			if (not Device)
				throw Error::RuntimeError("CommandBufferDeleter requires a valid VkDevice.");
			if (not CommandPool)
				throw Error::RuntimeError("CommandBufferDeleter requires a valid VkCommandPool.");
		}
		void operator()(vkr::VkCommandBuffer commandBuffer) const noexcept
		{
			vkr::vkFreeCommandBuffers(Device, CommandPool, 1, &commandBuffer);
		}
	};
	using CommandBufferUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkCommandBuffer>, CommandBufferDeleter>;

	class CommandBuffer
	{
	public:
		CommandBuffer(CommandBufferUniquePtr commandBufferIn) 
			: commandBuffer(std::move(commandBufferIn)) 
		{}

		auto GetHandle(this auto&& self) noexcept -> vkr::VkCommandBuffer
		{
			return self.commandBuffer.get();
		}

		void Free(this auto&& self) // return to pool
		{
			self.commandBuffer.reset();
		}

		void Reset(this auto&& self, vkr::VkCommandBufferResetFlags flags = 0)
		{
			auto result = vkr::vkResetCommandBuffer(self.commandBuffer.get(), flags);
			if (result != vkr::VkResult::VK_SUCCESS)
				throw VulkanError{result, "Failed to reset command buffer."};
		}
	private:
		CommandBufferUniquePtr commandBuffer;
	};

	struct CommandBufferFactory
	{
		vkr::VkDevice Device = nullptr;
		vkr::VkCommandPool CommandPool = nullptr;
		vkr::VkCommandBufferLevel Level = vkr::VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		[[nodiscard]]
		auto operator()(this auto&& self) -> CommandBufferUniquePtr
		{
			if (not self.Device)
				throw Error::RuntimeError("CommandBufferFactory requires a valid VkDevice.");
			if (not self.CommandPool)
				throw Error::RuntimeError("CommandBufferFactory requires a valid VkCommandPool.");
			auto allocateInfo = vkr::VkCommandBufferAllocateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = self.CommandPool,
				.level = self.Level,
				.commandBufferCount = 1
			};
			auto commandBufferHandle = vkr::VkCommandBuffer{};
			auto result = Vulkan::Result{ vkr::vkAllocateCommandBuffers(self.Device, &allocateInfo, &commandBufferHandle) };
			if (not result)
				throw VulkanError{result, "Failed to allocate command buffer."};
			return CommandBufferUniquePtr(commandBufferHandle, CommandBufferDeleter(self.Device, self.CommandPool));
		}
	};

	struct CommandPoolDeleter
	{
		vkr::VkDevice Device = nullptr;
		CommandPoolDeleter(vkr::VkDevice device)
			: Device(device)
		{
			if (not Device)
				throw Error::RuntimeError("CommandPoolDeleter requires a valid VkDevice.");
		}

		void operator()(vkr::VkCommandPool commandPool) const noexcept
		{
			vkr::vkDestroyCommandPool(Device, commandPool, nullptr);
		}
	};
	using CommandPoolUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkCommandPool>, CommandPoolDeleter>;

	struct CommandPoolFactory
	{
		vkr::VkDevice Device = nullptr;
		vkr::VkCommandPoolCreateInfo CreateInfo{};

		[[nodiscard]]
		auto operator()(this auto&& self) -> CommandPoolUniquePtr
		{
			if (not self.Device)
				throw Error::RuntimeError("CommandPoolFactory requires a valid VkDevice.");
			self.CreateInfo.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

			auto commandPool = vkr::VkCommandPool{};
			auto result = vkr::vkCreateCommandPool(self.Device, &self.CreateInfo, nullptr, &commandPool);
			if (result != vkr::VkResult::VK_SUCCESS)
				throw VulkanError{result, "Failed to create command pool."};
			return CommandPoolUniquePtr(commandPool, CommandPoolDeleter(self.Device));
		}
	};

	class CommandPool
	{
	public:
		CommandPool(CommandPoolUniquePtr poolIn) :pool(std::move(poolIn)) {};

		auto GetHandle(this auto&& self) noexcept -> vkr::VkCommandPool
		{
			return self.pool.get();
		}

		void Destroy(this auto&& self)
		{
			self.pool.reset();
		}

		void Reset(this auto&& self, vkr::VkCommandPoolResetFlags flags = 0)
		{
			auto result = vkr::vkResetCommandPool(self.pool->Device, self.pool.get(), flags);
			if (result != vkr::VkResult::VK_SUCCESS)
				throw VulkanError{result, "Failed to reset command pool."};
		}
	private:
		CommandPoolUniquePtr pool;
	};
}