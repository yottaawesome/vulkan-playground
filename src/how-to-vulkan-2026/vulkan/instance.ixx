export module vulkan26:vulkan.instance;
import std;
import vulkanlib;
import :raii;
import :vulkan.error;
import :vulkan.resource;

export namespace vk
{
	struct InstanceDeleter
	{
		static void operator()(VkInstance instance)
		{
			vkDestroyInstance(instance, nullptr);
		}
	};
	using InstanceUniquePtr = std::unique_ptr<std::remove_pointer_t<VkInstance>, InstanceDeleter>;

	struct Instance : Raii::TypedResource<InstanceUniquePtr>
	{
		constexpr Instance(InstanceUniquePtr instanceIn)
			: TypedResource(std::move(instanceIn)) { }
	};
}
