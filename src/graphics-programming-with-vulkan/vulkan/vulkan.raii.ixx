export module vulkangfx:vulkan.raii;
import std;
import :vulkan.exports;
import :raii;

export namespace Vulkan
{
	using VkInstanceUniquePtr = Raii::IndirectUniquePtr<vkr::VkInstance, vkr::vkDestroyInstance, nullptr>;

	struct SurfaceDeleter
	{
		SurfaceDeleter(vkr::VkInstance instance)
			: instance(instance)
		{
			if (not instance)
				throw std::invalid_argument("Instance pointer cannot be null.");
		}
		void operator()(this const SurfaceDeleter& self, vkr::VkSurfaceKHR surface) noexcept
		{
			if (surface)
				vkr::vkDestroySurfaceKHR(self.instance, surface, nullptr);
		}
		vkr::VkInstance instance;
	};
	using VkSurfaceUniquePtr = std::unique_ptr<vkr::VkSurfaceKHR, SurfaceDeleter>;
}
