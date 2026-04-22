export module vulkangfx:vulkan.memory;
import std;
import :error;
import :concepts;
import :vulkan.exports;
import :vulkan.error;

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

	struct DeviceMemoryDeleter
	{
		vkr::VkDevice Device = nullptr;
		DeviceMemoryDeleter() = default;
		DeviceMemoryDeleter(vkr::VkDevice device) : Device(device)
		{
			if (not Device)
				throw Error::RuntimeError("DeviceMemoryDeleter requires a valid VkDevice.");
		}
		void operator()(this const DeviceMemoryDeleter& self, vkr::VkDeviceMemory memoryHandle) noexcept
		{
			vkr::vkFreeMemory(self.Device, memoryHandle, nullptr);
		}
	};
	using MemoryUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkDeviceMemory>, DeviceMemoryDeleter>;

	auto CreateMemory(
		vkr::VkDevice device,
		Concepts::OneOf<vkr::VkBuffer, vkr::VkImage> auto buffer,
		vkr::VkPhysicalDevice physicalDevice,
		vkr::VkMemoryPropertyFlags memoryProperties
	) -> MemoryUniquePtr
	{
		auto memoryRequirements = vkr::VkMemoryRequirements{};

		if constexpr (std::same_as<decltype(buffer), vkr::VkBuffer>)
			vkr::vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
		else if constexpr (std::same_as<decltype(buffer), vkr::VkImage>)
			vkr::vkGetImageMemoryRequirements(device, buffer, &memoryRequirements);

		auto chosenMemoryType = Vulkan::FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, memoryProperties);
		auto allocInfo = vkr::VkMemoryAllocateInfo{
			.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = memoryRequirements.size,
			.memoryTypeIndex = chosenMemoryType
		};
		auto memoryHandle = vkr::VkDeviceMemory{};
		auto result = Vulkan::Result{ vkr::vkAllocateMemory(device, &allocInfo, nullptr, &memoryHandle) };
		if (not result)
			throw VulkanError{ result, "Failed to allocate buffer memory." };
		return MemoryUniquePtr{ memoryHandle, DeviceMemoryDeleter(device) };
	}

	class DeviceMemory
	{
	public:
		constexpr DeviceMemory() = default;
		constexpr DeviceMemory(MemoryUniquePtr memoryIn)
			: memory(std::move(memoryIn))
		{
			if (memory)
				throw Error::RuntimeError{ "Memory handle must not be null." };
		}

		auto BindToBuffer(this const DeviceMemory& self, vkr::VkDevice device, vkr::VkBuffer buffer, vkr::VkDeviceSize size = 0)
		{
			auto result = Vulkan::Result{ vkr::vkBindBufferMemory(device, buffer, self.memory.get(), size) };
			if (not result)
				throw VulkanError{ result, "Failed to bind buffer memory." };
		}

		auto BindToImage(this const DeviceMemory& self, vkr::VkDevice device, vkr::VkImage image, vkr::VkDeviceSize size = 0)
		{
			auto result = Vulkan::Result{ vkr::vkBindImageMemory(device, image, self.memory.get(), size) };
			if (not result)
				throw VulkanError{ result, "Failed to bind image memory." };
		}

		auto Map(this auto&& self) -> void*
		{
			if (not self.mapped)
				vkr::vkMapMemory(self.device, self.memory.get(), 0, self.GetSize(), 0, &self.mapped);
			return self.mapped;
		}

		void Unmap(this auto&& self)
		{
			if (self.mapped)
				vkr::vkUnmapMemory(self.device, self.memory.get());
		}
		
	protected:
		MemoryUniquePtr memory;
		void* mapped = nullptr;
	};
}