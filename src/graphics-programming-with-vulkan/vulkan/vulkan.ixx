module;

#include <vulkan/vulkan.hpp>

export module vulkangfx:vulkan;
export import :vulkan.exports;

// Raw vulkan types and functions, not vk:: types.
export namespace vulkan
{
	using
		::VkInstance
	;
}