export module vulkan26:vulkan.command;
import std;
import vulkanlib;
import :error;
import :vulkan.error;

export namespace vk
{
	class CommandBuffer
	{
	public:
		~CommandBuffer()
		{
			if (buffer)
			{
				vkFreeCommandBuffers(device, commandPool, 1, &buffer);
				buffer = nullptr;
			}
		}

		CommandBuffer(VkCommandBuffer buffer, VkDevice device, VkCommandPool commandPool)
			: buffer(buffer), device(device), commandPool(commandPool)
		{}

		CommandBuffer(const CommandBuffer&) = delete;
		auto operator=(const CommandBuffer&) = delete;

		CommandBuffer(CommandBuffer&& other) noexcept
			: buffer(std::exchange(other.buffer, nullptr))
		{}

		auto operator=(CommandBuffer&& other) noexcept -> CommandBuffer&
		{
			if (this != &other)
			{
				buffer = std::exchange(other.buffer, nullptr);
				device = std::exchange(other.device, nullptr);
				commandPool = std::exchange(other.commandPool, nullptr);
			}
			return *this;
		}

		auto Begin(this auto& self, VkCommandBufferUsageFlags flags = 0) -> void
		{
			if (not self.buffer)
				throw ::Error::RuntimeError{ "Invalid command buffer" };
			auto beginInfo = VkCommandBufferBeginInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = flags
			};
			auto result = vk::Result{ vkBeginCommandBuffer(self.buffer, &beginInfo) };
			if (not result)
				throw ::Error::RuntimeError{ std::format("Failed to begin command buffer: {}", result) };
		}

		auto End(this auto& self) -> void
		{
			if (not self.buffer)
				throw ::Error::RuntimeError{ "Invalid command buffer" };
			auto result = vk::Result{ vkEndCommandBuffer(self.buffer) };
			if (not result)
				throw ::Error::RuntimeError{ std::format("Failed to end command buffer: {}", result) };
		}

		auto Record(this auto& self, auto&& recordFunc, VkCommandBufferUsageFlags flags = 0) -> void
		{
			self.Begin(flags);
			std::invoke(recordFunc, self);
			self.End();
		}

	private:
		VkDevice device{};
		VkCommandPool commandPool{};
		VkCommandBuffer buffer{};
	};

	struct CommandBufferView
	{
		VkCommandBuffer Buffer{};
		auto Begin(this auto& self, VkCommandBufferUsageFlags flags = 0) -> void
		{
			if (not self.Buffer)
				throw ::Error::RuntimeError{ "Invalid command buffer" };
			auto beginInfo = VkCommandBufferBeginInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = flags
			};
			auto result = vk::Result{ vkBeginCommandBuffer(self.Buffer, &beginInfo) };
			if (not result)
				throw ::Error::RuntimeError{ std::format("Failed to begin command buffer: {}", result) };
		}

		auto End(this auto& self) -> void
		{
			if (not self.Buffer)
				throw ::Error::RuntimeError{ "Invalid command buffer" };
			auto result = vk::Result{ vkEndCommandBuffer(self.Buffer) };
			if (not result)
				throw ::Error::RuntimeError{ std::format("Failed to end command buffer: {}", result) };
		}
	};

	class CommandBuffers 
	{
	public:
		~CommandBuffers()
		{
			if (not buffers.empty())
			{
				vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(buffers.size()), buffers.data());
				buffers.clear();
			}
		}

		CommandBuffers(VkDevice device, VkCommandPool commandPool, std::vector<VkCommandBuffer> buffers)
			: device(device), commandPool(commandPool), buffers(buffers)
		{}

		CommandBuffers(const CommandBuffers&) = delete;
		auto operator=(const CommandBuffers&) = delete;

		CommandBuffers(CommandBuffers&& other) noexcept
			: device(std::exchange(other.device, nullptr))
			, commandPool(std::exchange(other.commandPool, nullptr))
			, buffers(std::exchange(other.buffers, {}))
		{}
		CommandBuffers& operator=(CommandBuffers&& other) noexcept
		{
			if (this != &other)
			{
				if (not buffers.empty())
					vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(buffers.size()), buffers.data());
				device = std::exchange(other.device, nullptr);
				commandPool = std::exchange(other.commandPool, nullptr);
				buffers = std::exchange(other.buffers, {});
			}
			return *this;
		}

	protected:
		VkDevice device{};
		VkCommandPool commandPool{};
		std::vector<VkCommandBuffer> buffers;
	};

	template<size_t N>
	class CommandBufferArray
	{
	public:
		~CommandBufferArray()
		{
			if (not buffers.empty())
			{
				vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(buffers.size()), buffers.data());
				buffers.fill(nullptr);
			}
		}
		CommandBufferArray(VkDevice device, VkCommandPool commandPool, std::array<VkCommandBuffer, N> buffers)
			: device(device), commandPool(commandPool), buffers(buffers)
		{}

		CommandBufferArray(const CommandBufferArray&) = delete;
		auto operator=(const CommandBufferArray&) = delete;
		
		CommandBufferArray(CommandBufferArray&& other) noexcept
			: device(std::exchange(other.device, nullptr))
			, commandPool(std::exchange(other.commandPool, nullptr))
			, buffers(std::exchange(other.buffers, {}))
		{}
		CommandBufferArray& operator=(CommandBufferArray&& other) noexcept
		{
			if (this != &other)
			{
				if (not buffers.empty())
					vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(buffers.size()), buffers.data());
				device = std::exchange(other.device, nullptr);
				commandPool = std::exchange(other.commandPool, nullptr);
				buffers = std::exchange(other.buffers, {});
			}
			return *this;
		}

		constexpr auto operator[](size_t index) noexcept -> VkCommandBuffer
		{
			if (index >= N)
				throw std::out_of_range{ "Index out of range" };
			return buffers[index];
		}
	protected:
		VkDevice device{};
		VkCommandPool commandPool{};
		std::array<VkCommandBuffer, N> buffers{};
	};

	class CommandPool
	{
	public:
		~CommandPool()
		{
			if (commandPool)
			{
				vkDestroyCommandPool(device, commandPool, nullptr);
				commandPool = nullptr;
			}
		}
		constexpr CommandPool(VkDevice deviceIn, VkCommandPool commandPoolIn)
			: device(deviceIn), commandPool(commandPoolIn)
		{
			if (not device)
				throw ::Error::RuntimeError{ "Invalid device" };
			if (not commandPool)
				throw ::Error::RuntimeError{ "Invalid command pool" };
		}

		CommandPool(VkDevice device, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex)
			: device(device)
		{
			if (not device)
				throw ::Error::RuntimeError{ "Invalid device" };
			auto createInfo = VkCommandPoolCreateInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags = flags,
				.queueFamilyIndex = queueFamilyIndex
			};
			auto commandPool = VkCommandPool{};
			auto result = vk::Result{ vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) };
			if (not result)
				throw vk::Error{ result, "Failed to create command pool." };
			this->commandPool = commandPool;
		}


		CommandPool(CommandPool const&) = delete;
		auto operator=(CommandPool const&) = delete;

		// Copyable
		CommandPool(CommandPool&& other) noexcept
			: device(std::exchange(other.device, nullptr)), commandPool(std::exchange(other.commandPool, nullptr))
		{}
		auto operator=(CommandPool&& other) noexcept -> CommandPool&
		{
			if (this != &other)
				return *this;
			if (device and commandPool)
				vkDestroyCommandPool(device, commandPool, nullptr);
			device = std::exchange(other.device, nullptr);
			commandPool = std::exchange(other.commandPool, nullptr);
			return *this;
		}

		constexpr auto Get(this const auto& self) noexcept -> VkCommandPool
		{
			return self.commandPool;
		}

		template<size_t N>
		auto CreateArray(VkCommandBufferLevel level) -> CommandBufferArray<N>
		{
			if (not device)
				throw ::Error::RuntimeError{ "Invalid device" };
			if (not commandPool)
				throw ::Error::RuntimeError{ "Invalid command pool" };

			auto commandBuffers = std::array<VkCommandBuffer, N>{};
			auto allocInfo = VkCommandBufferAllocateInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = commandPool,
				.level = level,
				.commandBufferCount = static_cast<std::uint32_t>(N)
			};
			auto result = vk::Result{ vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) };
			if (not result)
				throw Error{ result, "Failed to allocate command buffers" };

			return CommandBufferArray<N>{ device, commandPool, commandBuffers };
		}

		auto CreateCommandBuffer(this auto& self, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) -> CommandBuffer
		{
			if (not device)
				throw ::Error::RuntimeError{ "Invalid device" };
			if (not commandPool)
				throw ::Error::RuntimeError{ "Invalid command pool" };
			auto allocInfo = VkCommandBufferAllocateInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = commandPool,
				.level = level,
				.commandBufferCount = 1
			};
			auto commandBuffer = VkCommandBuffer{};
			auto result = vk::Result{ vkAllocateCommandBuffers(self.device, &allocInfo, &commandBuffer) };
			if (not result)
				throw ::Error::RuntimeError{ std::format("Failed to allocate command buffer: {}", result) };
			return CommandBuffer{ commandBuffer, device, commandPool };
		}
	private:
		VkDevice device{};
		VkCommandPool commandPool{};
	};
}