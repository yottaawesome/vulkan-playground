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
		struct Factory
		{
			vkr::VkDevice Device = nullptr;
			vkr::VkCommandPool CommandPool = nullptr;
			vkr::VkCommandBufferLevel Level = vkr::VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			[[nodiscard]]
			auto operator()(this auto&& self) -> CommandBuffer
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
					throw VulkanError{ result, "Failed to allocate command buffer." };
				return CommandBuffer{CommandBufferUniquePtr(commandBufferHandle, CommandBufferDeleter(self.Device, self.CommandPool))};
			}
		};

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

		auto Begin(
			this const CommandBuffer& self, 
			const vkr::VkCommandBufferBeginInfo& beginInfo = vkr::VkCommandBufferBeginInfo{ .sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, }
		) -> decltype(self)
		{
			auto result = vkr::vkBeginCommandBuffer(self.commandBuffer.get(), &beginInfo);
			if (result != vkr::VkResult::VK_SUCCESS)
				throw VulkanError{result, "Failed to begin recording command buffer."};
			return decltype(self)(self);
		}

		auto End(this const CommandBuffer& self) -> decltype(self)
		{
			auto result = vkr::vkEndCommandBuffer(self.commandBuffer.get());
			if (result != vkr::VkResult::VK_SUCCESS)
				throw VulkanError{result, "Failed to end recording command buffer."};
			return decltype(self)(self);
		}

		auto SetViewport(this const CommandBuffer& self, const vkr::VkViewport& viewport) -> decltype(self)
		{
			vkr::vkCmdSetViewport(self.commandBuffer.get(), 0, 1, &viewport);
			return decltype(self)(self);
		}

		auto SetScissor(this const CommandBuffer& self, const vkr::VkRect2D& scissor) -> decltype(self)
		{
			vkr::vkCmdSetScissor(self.commandBuffer.get(), 0, 1, &scissor);
			return decltype(self)(self);
		}

		auto BeginRendering(this const CommandBuffer& self, const vkr::VkRenderingInfo& renderingInfo) -> decltype(self)
		{
			vkr::vkCmdBeginRendering(self.commandBuffer.get(), &renderingInfo);
			return decltype(self)(self);
		}

		// Convenience overload that builds VkRenderingInfo internally. The pointer
		// in renderingInfo.pColorAttachments points to the parameter, which is safe
		// because vkCmdBeginRendering consumes the data immediately during recording.
		// This pattern would be unsafe in a deferred/batched command builder where
		// the struct outlives the call.
		auto BeginRendering(
			this const CommandBuffer& self, 
			const vkr::VkRect2D& renderArea,
			const vkr::VkRenderingAttachmentInfo& colorAttachment
		) -> decltype(self)
		{
			auto renderingInfo = vkr::VkRenderingInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_RENDERING_INFO,
				.renderArea = renderArea,
				.layerCount = 1,
				.colorAttachmentCount = 1,
				.pColorAttachments = &colorAttachment
			};
			vkr::vkCmdBeginRendering(self.commandBuffer.get(), &renderingInfo);
			return decltype(self)(self);
		}

		auto EndRendering(this const CommandBuffer& self) -> decltype(self)
		{
			vkr::vkCmdEndRendering(self.commandBuffer.get());
			return decltype(self)(self);
		}

		auto BindAndDrawVertexBuffer(
			this const CommandBuffer& self,
			vkr::VkBuffer& buffer,
			std::uint32_t vertexCount,
			std::uint32_t instanceCount,
			std::uint32_t firstVertex,
			std::uint32_t firstInstance
		) -> decltype(self)
		{
			auto offset = vkr::VkDeviceSize{};
			vkr::vkCmdBindVertexBuffers(
				self.commandBuffer.get(),
				0,
				1,
				&buffer,
				&offset
			);
			vkr::vkCmdDraw(self.commandBuffer.get(), vertexCount, instanceCount, firstVertex, firstInstance);
			return decltype(self)(self);
		}

		auto BindVertexBuffers(
			this const CommandBuffer& self, 
			vkr::VkBuffer& buffer
		) -> decltype(self)
		{
			auto offset = vkr::VkDeviceSize{};
			vkr::vkCmdBindVertexBuffers(
				self.commandBuffer.get(), 
				0, 
				1, 
				&buffer, 
				&offset
			);
			return decltype(self)(self);
		}

		auto Draw(
			this const CommandBuffer& self, 
			std::uint32_t vertexCount, 
			std::uint32_t instanceCount, 
			std::uint32_t firstVertex, 
			std::uint32_t firstInstance
		) -> decltype(self)
		{
			vkr::vkCmdDraw(self.commandBuffer.get(), vertexCount, instanceCount, firstVertex, firstInstance);
			return decltype(self)(self);
		}

		auto BindPipeline(this const CommandBuffer& self, vkr::VkPipelineBindPoint pipelineBindPoint, vkr::VkPipeline pipeline) -> decltype(self)
		{
			vkr::vkCmdBindPipeline(self.commandBuffer.get(), pipelineBindPoint, pipeline);
			return decltype(self)(self);
		}

		auto PipelineBarrier2(
			this const CommandBuffer& self,
			const vkr::VkDependencyInfo& dependencyInfo
		) -> decltype(self)
		{
			vkr::vkCmdPipelineBarrier2(self.commandBuffer.get(), &dependencyInfo);
			return decltype(self)(self);
		}

		auto PipelineBarrier2Ex(
			this const CommandBuffer& self,
			const vkr::VkImageMemoryBarrier2& barrier
		) -> decltype(self)
		{
			auto dependencyInfo = vkr::VkDependencyInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &barrier
			};
			vkr::vkCmdPipelineBarrier2(self.commandBuffer.get(), &dependencyInfo);
			return decltype(self)(self);
		}

	private:
		CommandBufferUniquePtr commandBuffer;
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

	class CommandPool
	{
	public:
		struct Factory
		{
			vkr::VkDevice Device = nullptr;
			vkr::VkCommandPoolCreateInfo CreateInfo{};

			[[nodiscard]]
			auto operator()(this auto&& self) -> CommandPool
			{
				if (not self.Device)
					throw Error::RuntimeError("CommandPoolFactory requires a valid VkDevice.");
				self.CreateInfo.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

				auto commandPool = vkr::VkCommandPool{};
				auto result = vkr::vkCreateCommandPool(self.Device, &self.CreateInfo, nullptr, &commandPool);
				if (result != vkr::VkResult::VK_SUCCESS)
					throw VulkanError{ result, "Failed to create command pool." };
				return { CommandPoolUniquePtr{ commandPool, CommandPoolDeleter(self.Device) } };
			}
		};

		CommandPool(CommandPoolUniquePtr poolIn) :pool(std::move(poolIn)) {};

		auto CreateCommandBuffer(this auto&& self, vkr::VkCommandBufferLevel level) -> CommandBuffer
		{
			return CommandBuffer::Factory{ self.pool.get_deleter().Device, self.pool.get(), level}();
		}

		auto CreatePrimaryCommandBuffer(this auto&& self) -> CommandBuffer
		{
			return self.CreateCommandBuffer(vkr::VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		}

		auto CreateSecondaryCommandBuffer(this auto&& self) -> CommandBuffer
		{
			return self.CreateCommandBuffer(vkr::VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		}

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