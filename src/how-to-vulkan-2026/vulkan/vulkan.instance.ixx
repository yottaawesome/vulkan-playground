export module vulkan26:vulkan.instance;
import std;
import :raii;
import :vulkan.error;
import :vulkan.exports;
import :vulkan.resource;

export namespace vk
{
	struct InstanceDeleter
	{
		static void operator()(vk::VkInstance instance)
		{
			vk::vkDestroyInstance(instance, nullptr);
		}
	};
	using InstanceUniquePtr = std::unique_ptr<std::remove_pointer_t<vk::VkInstance>, InstanceDeleter>;

	struct Instance : TypedResource<InstanceUniquePtr>
	{
		constexpr Instance(InstanceUniquePtr instanceIn)
			: TypedResource(std::move(instanceIn)) { }
	};
}
