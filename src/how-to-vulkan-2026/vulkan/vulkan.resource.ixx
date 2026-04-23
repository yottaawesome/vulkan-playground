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
}
