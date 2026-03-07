export module vulkan26:vulkan.instance;
import std;
import :vulkan.error;
import :vulkan.exports;
import :raii;

export namespace Vk
{
	class Instance
	{
	public:
		~Instance()
		{
			if (instance)
				vkDestroyInstance(instance, nullptr);
		}
		constexpr Instance(VkInstance instanceIn)
			: instance(instanceIn)
		{
			if (not instance)
				throw Error{ VkResult::VK_ERROR_INITIALIZATION_FAILED };
		}

		Instance(Instance const&) = delete;
		Instance& operator=(Instance const&) = delete;

		constexpr auto Get(this const auto& self) noexcept -> VkInstance
		{
			return self.instance;
		}

	private:
		VkInstance instance = nullptr;
	};
}
