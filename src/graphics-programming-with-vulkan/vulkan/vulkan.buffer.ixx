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

		struct Factory
		{
			vkr::VkDevice Device = nullptr;
			vkr::VkPhysicalDevice PhysicalDevice = nullptr;
			vkr::VkBufferCreateInfo bufferInfo{};
			vkr::VkMemoryPropertyFlags MemoryProperties = 0;

			[[nodiscard]]
			auto operator()(this Factory& self) -> BufferHandle
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
}
