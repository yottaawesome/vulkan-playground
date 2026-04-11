export module vulkan26:vulkan.resource;
import std;
import :vulkan.exports;
import :error;
import :util;

export namespace vk
{
	class DeviceBasedDeleter
	{
	public:
		constexpr DeviceBasedDeleter() = default;
		constexpr DeviceBasedDeleter(vk::VkDevice deviceIn)
			: device(deviceIn)
		{
			if (not device)
				throw ::Error::RuntimeError{ "Device cannot be nullptr" };
		}
		constexpr auto operator()(this const auto& self, auto image) noexcept
		{
			static_assert(util::FalseType<decltype(self)>::value, "This method must be implemented.");
		}
		constexpr auto GetDevice(this const DeviceBasedDeleter& self) noexcept -> vk::VkDevice
		{
			return self.device;
		}
	protected:
		vk::VkDevice device = nullptr;
	};

	template<typename T>
	class TypedResource
	{
	public:
		using pointer = typename T::pointer;
		constexpr TypedResource(T resourceIn)
			: resource(std::move(resourceIn))
		{
			if (not resource)
				throw ::Error::RuntimeError{ "Cannot initialise a resource with a null pointer" };
		}
		constexpr auto Get(this const auto& self) noexcept -> pointer
		{
			return self.resource.get();
		}
		constexpr void Destroy(this auto& self) noexcept
		{
			self.resource.reset();
		}
		constexpr auto operator*(this const auto& self) noexcept -> pointer
		{
			return self.resource.get();
		}
		explicit constexpr operator bool(this const auto& self) noexcept
		{
			return static_cast<bool>(self.resource);
		}
	protected:
		T resource;
	};

	static_assert(
		[] consteval -> bool
		{
			auto image = TypedResource<std::unique_ptr<int>>{ std::make_unique<int>(1) };
			if (*image.Get() != 1)
				throw "Unexpected Get() value.";
			if (**image != 1)
				throw "Unexpected operator* value.";
			image.Destroy();
			if (image.Get() != nullptr)
				throw "Expected Get() to return nullptr after resource is destroyed.";
			return true;
		}());
}
