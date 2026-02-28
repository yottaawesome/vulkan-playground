export module vulkangfx:vulkan.raii;
import std;
import :vulkan.exports;
import :raii;

export namespace Vulkan
{
	using VkInstanceUniquePtr = Raii::IndirectUniquePtr<vkr::VkInstance, vkr::vkDestroyInstance, nullptr>;
}
