export module vulkangfx:vulkan.buffer;
import std;
import :vulkan.exports;
import :vulkan.error;
import :error;

export namespace Vulkan
{
	auto FindMemoryType(
		vkr::VkPhysicalDevice physicalDevice,
		std::uint32_t typeFilter,
		vkr::VkMemoryPropertyFlags properties
	) -> std::uint32_t
	{
		auto memProperties = vkr::VkPhysicalDeviceMemoryProperties{};
		vkr::vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		for (std::uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			bool passesTypeFilter = typeFilter & (1 << i);
			bool hasRequiredProperties = (memProperties.memoryTypes[i].propertyFlags & properties) == properties;
			if (passesTypeFilter and hasRequiredProperties)
				return i;
		}
		throw Error::RuntimeError("Failed to find suitable memory type.");
	}

	struct BufferHandle
	{
		vkr::VkBuffer Buffer = nullptr;
		vkr::VkDeviceMemory Memory = nullptr;

		auto Destroy(this auto&& self, vkr::VkDevice device) noexcept
		{
			if (self.Buffer)
				vkr::vkDestroyBuffer(device, self.Buffer, nullptr);
			if (self.Memory)
				vkr::vkFreeMemory(device, self.Memory, nullptr);
			(self.Buffer = nullptr, self.Memory = nullptr);
		}

		struct Factory
		{
			vkr::VkDevice Device = nullptr;
			vkr::VkPhysicalDevice PhysicalDevice = nullptr;
			vkr::VkBufferCreateInfo bufferInfo{};
			vkr::VkMemoryPropertyFlags MemoryProperties = 0;

			[[nodiscard]]
			auto operator()(this auto&& self) -> BufferHandle
			{
				if (not self.Device)
					throw Error::RuntimeError("BufferFactory requires a valid VkDevice.");
				if (not self.PhysicalDevice)
					throw Error::RuntimeError("BufferFactory requires a valid VkPhysicalDevice.");

				self.bufferInfo.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				auto bufferHandle = BufferHandle{};
				auto result = Vulkan::Result{ vkr::vkCreateBuffer(self.Device, &self.bufferInfo, nullptr, &bufferHandle.Buffer) };
				if (not result)
					throw VulkanError{ result, "Failed to create buffer." };

				auto memoryRequirements = vkr::VkMemoryRequirements{};
				vkr::vkGetBufferMemoryRequirements(self.Device, bufferHandle.Buffer, &memoryRequirements);
				auto chosenMemoryType = Vulkan::FindMemoryType(self.PhysicalDevice, memoryRequirements.memoryTypeBits, self.MemoryProperties);
				auto allocInfo = vkr::VkMemoryAllocateInfo{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
					.pNext = nullptr,
					.allocationSize = memoryRequirements.size,
					.memoryTypeIndex = chosenMemoryType
				};
				result = Vulkan::Result{ vkr::vkAllocateMemory(self.Device, &allocInfo, nullptr, &bufferHandle.Memory) };
				if (not result)
					throw VulkanError{ result, "Failed to allocate buffer memory." };
				vkr::vkBindBufferMemory(self.Device, bufferHandle.Buffer, bufferHandle.Memory, 0);
				return bufferHandle;
			}
		};
	};

	struct BufferDeleter
	{
		vkr::VkDevice Device = nullptr;
		BufferDeleter() = default;
		BufferDeleter(vkr::VkDevice device) : Device(device)
		{
			if (not Device)
				throw Error::RuntimeError("BufferDeleter requires a valid VkDevice.");
		}
		void operator()(this const BufferDeleter& self, vkr::VkBuffer bufferHandle) noexcept
		{
			vkr::vkDestroyBuffer(self.Device, bufferHandle, nullptr);
		}
	};
	using BufferUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkBuffer>, BufferDeleter>;
	struct MemoryDeleter
	{
		vkr::VkDevice Device = nullptr;
		MemoryDeleter() = default;
		MemoryDeleter(vkr::VkDevice device) : Device(device)
		{
			if (not Device)
				throw Error::RuntimeError("MemoryDeleter requires a valid VkDevice.");
		}
		void operator()(this const MemoryDeleter& self, vkr::VkDeviceMemory memoryHandle) noexcept
		{
			vkr::vkFreeMemory(self.Device, memoryHandle, nullptr);
		}
	};
	using MemoryUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkDeviceMemory>, MemoryDeleter>;

	template<typename TVertex>
	class VertexBuffer
	{
	public:
		~VertexBuffer()
		{
			Destroy();
		}

		VertexBuffer(
			std::vector<TVertex> verticesIn, 
			vkr::VkDevice device,
			vkr::VkPhysicalDevice physicalDevice,
			vkr::VkSharingMode sharingMode, 
			vkr::VkBufferUsageFlagBits additionalUsageFlags,
			vkr::VkMemoryPropertyFlags memoryProperties
		) : vertices(std::move(verticesIn)), device(device)
		{
			if (not device)
				throw Error::RuntimeError("BufferFactory requires a valid VkDevice.");
			if (not physicalDevice)
				throw Error::RuntimeError("BufferFactory requires a valid VkPhysicalDevice.");

			Create(physicalDevice, sharingMode, additionalUsageFlags, memoryProperties);
		}

		auto ToBufferHandle(this auto&& self) -> BufferHandle
		{
			return BufferHandle{ self.buffer.get(), self.memory.get() };
		}

		VertexBuffer(const VertexBuffer&) = delete;
		auto operator=(const VertexBuffer&) -> VertexBuffer& = delete;

		constexpr auto GetVertexCount(this auto&& self) noexcept -> std::size_t
		{
			return self.vertices.size();
		}

		constexpr auto GetSize(this auto&& self) noexcept -> std::size_t
		{
			return sizeof(TVertex) * self.vertices.size();
		}

		auto GetBuffer(this auto&& self) noexcept -> vkr::VkBuffer
		{
			return self.buffer;
		}

		auto GetMemory(this auto&& self) noexcept -> vkr::VkDeviceMemory
		{
			return self.memory;
		}

		auto Destroy(this auto&& self)
		{
			self.buffer.reset();
			self.memory.reset();
		}
	private:
		void Create(
			this VertexBuffer& self,
			vkr::VkPhysicalDevice physicalDevice,
			vkr::VkSharingMode sharingMode,
			vkr::VkBufferUsageFlagBits additionalUsageFlags,
			vkr::VkMemoryPropertyFlags memoryProperties
		)
		{
			if (not self.device)
				throw Error::RuntimeError("BufferFactory requires a valid VkDevice.");
			if (not physicalDevice)
				throw Error::RuntimeError("BufferFactory requires a valid VkPhysicalDevice.");

			self.buffer =
				[sharingMode, additionalUsageFlags, &self]
				{
					auto bufferInfo = vkr::VkBufferCreateInfo{
						.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
						.size = sizeof(TVertex) * self.vertices.size(),
						.usage = static_cast<vkr::VkBufferUsageFlags>(vkr::VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | additionalUsageFlags),
						.sharingMode = sharingMode
					};
					auto bufferHandle = vkr::VkBuffer{};
					auto result = Vulkan::Result{ vkr::vkCreateBuffer(self.device, &bufferInfo, nullptr, &bufferHandle) };
					if (not result)
						throw VulkanError{ result, "Failed to create buffer." };
					return BufferUniquePtr{ bufferHandle, BufferDeleter{self.device} };
				}();
			
			self.memory = 
				[&self, physicalDevice, memoryProperties] -> MemoryUniquePtr
				{
					auto memoryRequirements = vkr::VkMemoryRequirements{};
					vkr::vkGetBufferMemoryRequirements(self.device, self.buffer.get(), &memoryRequirements);
					auto chosenMemoryType = Vulkan::FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, memoryProperties);
					auto allocInfo = vkr::VkMemoryAllocateInfo{
						.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
						.pNext = nullptr,
						.allocationSize = memoryRequirements.size,
						.memoryTypeIndex = chosenMemoryType
					};
					auto memoryHandle = vkr::VkDeviceMemory{};
					auto result = Vulkan::Result{ vkr::vkAllocateMemory(self.device, &allocInfo, nullptr, &memoryHandle) };
					if (not result)
						throw VulkanError{ result, "Failed to allocate buffer memory." };
					return MemoryUniquePtr{ memoryHandle, MemoryDeleter(self.device) };
				}();


			vkr::vkBindBufferMemory(self.device, self.buffer.get(), self.memory.get(), 0);

			void* data;
			vkr::vkMapMemory(self.device, self.memory.get(), 0, self.GetSize(), 0, &data);
			std::memcpy(data, self.vertices.data(), static_cast<std::size_t>(self.GetSize()));
			vkr::vkUnmapMemory(self.device, self.memory.get());
		}

		std::vector<TVertex> vertices;
		vkr::VkDevice device = nullptr;
		BufferUniquePtr buffer;
		MemoryUniquePtr memory;
	};

	class IndexBuffer
	{
	public:
		~IndexBuffer()
		{
			Destroy();
		}

		IndexBuffer(
			std::vector<std::uint32_t> verticesIn,
			vkr::VkDevice device,
			vkr::VkPhysicalDevice physicalDevice,
			vkr::VkSharingMode sharingMode,
			vkr::VkBufferUsageFlagBits additionalUsageFlags,
			vkr::VkMemoryPropertyFlags memoryProperties
		) : vertices(std::move(verticesIn)), device(device)
		{
			if (not device)
				throw Error::RuntimeError("BufferFactory requires a valid VkDevice.");
			if (not physicalDevice)
				throw Error::RuntimeError("BufferFactory requires a valid VkPhysicalDevice.");

			Create(physicalDevice, sharingMode, additionalUsageFlags, memoryProperties);
		}

		auto ToBufferHandle(this auto&& self) -> BufferHandle
		{
			return BufferHandle{ self.buffer.get(), self.memory.get() };
		}

		IndexBuffer(const IndexBuffer&) = delete;
		auto operator=(const IndexBuffer&) -> IndexBuffer & = delete;

		IndexBuffer(IndexBuffer&&) = default;
		auto operator=(IndexBuffer&&) -> IndexBuffer& = default;

		constexpr auto GetVertexCount(this auto&& self) noexcept -> std::size_t
		{
			return self.vertices.size();
		}

		constexpr auto GetSize(this auto&& self) noexcept -> std::size_t
		{
			return sizeof(std::uint32_t) * self.vertices.size();
		}

		auto GetBuffer(this auto&& self) noexcept -> vkr::VkBuffer
		{
			return self.buffer.get();
		}

		auto GetMemory(this auto&& self) noexcept -> vkr::VkDeviceMemory
		{
			return self.memory.get();
		}

		auto Destroy(this auto&& self)
		{
			self.buffer.reset();
			self.memory.reset();
		}

		auto MapMemoryAndCopy(this IndexBuffer& self)
		{
			vkr::vkMapMemory(self.device, self.memory.get(), 0, self.GetSize(), 0, &self.mapped);
			std::memcpy(self.mapped, self.vertices.data(), static_cast<std::size_t>(self.GetSize()));
			vkr::vkUnmapMemory(self.device, self.memory.get());
		}

	private:
		void Create(
			this IndexBuffer& self,
			vkr::VkPhysicalDevice physicalDevice,
			vkr::VkSharingMode sharingMode,
			vkr::VkBufferUsageFlagBits additionalUsageFlags,
			vkr::VkMemoryPropertyFlags memoryProperties
		)
		{
			if (not self.device)
				throw Error::RuntimeError("BufferFactory requires a valid VkDevice.");
			if (not physicalDevice)
				throw Error::RuntimeError("BufferFactory requires a valid VkPhysicalDevice.");

			self.buffer =
				[sharingMode, additionalUsageFlags, &self]
				{
					auto bufferInfo = vkr::VkBufferCreateInfo{
						.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
						.size = sizeof(std::uint32_t) * self.vertices.size(),
						.usage = static_cast<vkr::VkBufferUsageFlags>(vkr::VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT | additionalUsageFlags),
						.sharingMode = sharingMode
					};
					auto bufferHandle = vkr::VkBuffer{};
					auto result = Vulkan::Result{ vkr::vkCreateBuffer(self.device, &bufferInfo, nullptr, &bufferHandle) };
					if (not result)
						throw VulkanError{ result, "Failed to create buffer." };
					return BufferUniquePtr{ bufferHandle, BufferDeleter{self.device} };
				}();

			self.memory =
				[&self, physicalDevice, memoryProperties] -> MemoryUniquePtr
				{
					auto memoryRequirements = vkr::VkMemoryRequirements{};
					vkr::vkGetBufferMemoryRequirements(self.device, self.buffer.get(), &memoryRequirements);
					auto chosenMemoryType = Vulkan::FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, memoryProperties);
					auto allocInfo = vkr::VkMemoryAllocateInfo{
						.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
						.pNext = nullptr,
						.allocationSize = memoryRequirements.size,
						.memoryTypeIndex = chosenMemoryType
					};
					auto memoryHandle = vkr::VkDeviceMemory{};
					auto result = Vulkan::Result{ vkr::vkAllocateMemory(self.device, &allocInfo, nullptr, &memoryHandle) };
					if (not result)
						throw VulkanError{ result, "Failed to allocate buffer memory." };
					return MemoryUniquePtr{ memoryHandle, MemoryDeleter(self.device) };
				}();
			vkr::vkBindBufferMemory(self.device, self.buffer.get(), self.memory.get(), 0);
		}

		std::vector<std::uint32_t> vertices;
		vkr::VkDevice device = nullptr;
		BufferUniquePtr buffer;
		MemoryUniquePtr memory;
		void* mapped = nullptr;
	};
}
