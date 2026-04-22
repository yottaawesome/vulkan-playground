export module vulkangfx:vulkan.buffer;
import std;
import :error;
import :vulkan.exports;
import :vulkan.error;
import :vulkan.memory;

export namespace Vulkan
{
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

	class VulkanBuffer
	{
	public:
		static auto CreateBuffer(
			std::uint64_t size,
			vkr::VkDevice device,
			vkr::VkBufferUsageFlagBits additionalUsageFlags,
			vkr::VkSharingMode sharingMode
		) -> BufferUniquePtr
		{
			auto bufferInfo = vkr::VkBufferCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size = size,
				.usage = static_cast<vkr::VkBufferUsageFlags>(additionalUsageFlags),
				.sharingMode = sharingMode
			};
			auto bufferHandle = vkr::VkBuffer{};
			auto result = Vulkan::Result{ vkr::vkCreateBuffer(device, &bufferInfo, nullptr, &bufferHandle) };
			if (not result)
				throw VulkanError{ result, "Failed to create buffer." };
			return BufferUniquePtr{ bufferHandle, BufferDeleter{device} };
		}
	protected:
		BufferUniquePtr buffer;
	};

	class GenericBuffer
	{
	public:
		GenericBuffer() = default;

		GenericBuffer(
			std::uint64_t size,
			vkr::VkDevice device,
			vkr::VkPhysicalDevice physicalDevice
		) : size(size), device(device)
		{
			if (not device)
				throw Error::RuntimeError("BufferFactory requires a valid VkDevice.");
			if (not physicalDevice)
				throw Error::RuntimeError("BufferFactory requires a valid VkPhysicalDevice.");
		}

		GenericBuffer(const GenericBuffer&) = delete;
		auto operator=(const GenericBuffer&) -> GenericBuffer & = delete;

		GenericBuffer(GenericBuffer&&) = default;
		auto operator=(GenericBuffer&&) -> GenericBuffer & = default;

		auto ToBufferHandle(this auto&& self) -> BufferHandle
		{
			return BufferHandle{ self.buffer.get(), self.memory.get() };
		}

		constexpr auto GetSize(this auto&& self) noexcept -> std::size_t
		{
			return self.size;
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

		auto Map(this auto&& self) -> void*
		{
			void* mapped;
			vkr::vkMapMemory(self.device, self.memory.get(), 0, self.GetSize(), 0, &mapped);
			return mapped;
		}

		auto Unmap(this auto&& self) -> void
		{
			vkr::vkUnmapMemory(self.device, self.memory.get());
		}

		auto MapMemory(this auto&& self, auto&& fn) -> void*
		try
		{
			void* mapped;
			vkr::vkMapMemory(self.device, self.memory.get(), 0, self.GetSize(), 0, &mapped);
			std::invoke(fn, mapped);
			vkr::vkUnmapMemory(self.device, self.memory.get());
			return mapped;
		}
		catch (...)
		{
			vkr::vkUnmapMemory(self.device, self.memory.get());
			throw;
		}

	protected:
		std::uint64_t size = 0;
		vkr::VkDevice device = nullptr;
		BufferUniquePtr buffer;
		MemoryUniquePtr memory;
	};

	template<typename TVertex>
	class VertexBuffer : public GenericBuffer
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

	protected:
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

			self.buffer = VulkanBuffer::CreateBuffer(self.size, self.device, static_cast<vkr::VkBufferUsageFlagBits>(vkr::VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | additionalUsageFlags), sharingMode);
			self.memory = DeviceMemory::CreateMemory(self.device, self.buffer.get(), physicalDevice, memoryProperties);

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

	struct NoInitT {} constexpr NoInit;

	class IndexBuffer : public GenericBuffer
	{
	public:
		~IndexBuffer()
		{
			Destroy();
		}

		IndexBuffer(
			std::uint64_t size,
			vkr::VkDevice device,
			vkr::VkPhysicalDevice physicalDevice,
			vkr::VkSharingMode sharingMode,
			vkr::VkBufferUsageFlagBits additionalUsageFlags,
			vkr::VkMemoryPropertyFlags memoryProperties
		) : GenericBuffer(size, device, physicalDevice)
		{
			Create(physicalDevice, sharingMode, additionalUsageFlags, memoryProperties);
		}

		IndexBuffer(const IndexBuffer&) = delete;
		auto operator=(const IndexBuffer&) -> IndexBuffer & = delete;

		IndexBuffer(IndexBuffer&&) = default;
		auto operator=(IndexBuffer&&) -> IndexBuffer& = default;

	protected:
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

			self.buffer = VulkanBuffer::CreateBuffer(self.size, self.device, static_cast<vkr::VkBufferUsageFlagBits>(vkr::VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT | additionalUsageFlags), sharingMode);
			self.memory = CreateMemory(self.device, self.buffer.get(), physicalDevice, memoryProperties);
			vkr::vkBindBufferMemory(self.device, self.buffer.get(), self.memory.get(), 0);
		}
	};
}
