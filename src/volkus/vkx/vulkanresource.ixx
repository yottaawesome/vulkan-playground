export module volkus:vkx.vulkanresource;
import std;
import :vkx.error;
import :util;

namespace Volkus::vkx
{
	template<typename T>
	class VulkanResource
	{
	public:
		constexpr VulkanResource() = default;
		constexpr VulkanResource(T resource) : m_resource(std::move(resource))
		{ 
			if (not this->m_resource)
				throw VolkusError{ "Failed to create Vulkan resource" };
		}

		VulkanResource(const VulkanResource&) = delete;
		VulkanResource& operator=(const VulkanResource&) = delete;

		constexpr VulkanResource(VulkanResource&&) noexcept = default;
		constexpr VulkanResource& operator=(VulkanResource&&) noexcept = default;

		constexpr auto Get(this auto&& self) noexcept
		{
			if constexpr (std::is_pointer_v<T>)
			{
				return self.m_resource;
			}
			else if constexpr (requires { self.m_resource.get(); })
			{
				return self.m_resource.get();
			}
			else
			{
				static_assert(Util::AlwaysFalse<T>::value, "T must be a pointer type or a unique_ptr-like type");
			}
		}

		constexpr auto operator*(this auto&& self) noexcept
		{
			return self.Get();
		}
	protected:
		T m_resource{};
	};
}

namespace
{
	static_assert(
		[] {
			auto instance = Volkus::vkx::VulkanResource<std::unique_ptr<int>>{ std::make_unique<int>(30) };
			if (not instance.Get())
				throw "Instance::Get() returned nullptr";
			if (*instance.Get() != 30)
				throw "Instance::Get() does not return the correct value";
			if (not std::same_as<decltype(instance.Get()), int*>)
				throw "Instance::Get() does not return the correct type";
			return true;
		}(), "Instance failed to pass static checks.");
}