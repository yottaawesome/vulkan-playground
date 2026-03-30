export module vulkangfx:vulkan.buffer;
import std;
import :vulkan.exports;
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
			if (passesTypeFilter && hasRequiredProperties)
				return i;
		}
		throw Error::RuntimeError("Failed to find suitable memory type.");
	}

	struct BufferHandle
	{
		vkr::VkBuffer Buffer = nullptr;
		vkr::VkDeviceMemory Memory = nullptr;
	};
}
