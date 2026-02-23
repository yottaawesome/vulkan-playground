export module vulkangfx:vulkan.surface;
import std;
import :vulkan.raii;
import :vulkan.error;
import :vulkan.exports;

export namespace Vulkan
{
	// Movable but not copyable, because it owns a Vulkan surface handle, which is a non-owning handle that must be explicitly destroyed.
	struct Surface
	{
	public:
		~Surface()
		{
			Destroy();
		}

		Surface(const Surface&) = delete;
		Surface& operator=(this Surface&, const Surface&) = delete;

		Surface(Surface&& other) noexcept
			: surface{ std::exchange(other.surface, nullptr) }, instance{ std::exchange(other.instance, nullptr) }
		{ }

		auto operator=(this Surface& self, Surface&& other) noexcept
		{
			if (&self != &other)
			{
				self.Destroy();
				self.surface = std::exchange(other.surface, nullptr);
				self.instance = std::exchange(other.instance, nullptr);
			}
			return std::move(self);
		}

		Surface(vkr::VkSurfaceKHR surface, vkr::VkInstance instance) 
			: surface{ std::move(surface) }, instance{ std::move(instance) }
		{ }
		
		auto GetHandle(this const Surface& self) noexcept -> vkr::VkSurfaceKHR
		{
			return self.surface;
		}

		void Destroy(this Surface& self)
		{
			if (not self.surface)
				return;
			vkr::vkDestroySurfaceKHR(self.instance, self.surface, nullptr);
			self.instance = nullptr;
			self.surface = nullptr;
		}

	private:
		vkr::VkSurfaceKHR surface;
		vkr::VkInstance instance; // should this be a shared_ptr, specifically a weak_ptr?
	};
}
