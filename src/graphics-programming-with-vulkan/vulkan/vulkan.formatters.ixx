export module vulkangfx:vulkan.formatters;
import std;
import :vulkan.exports;

export namespace std
{
	template<>
	struct formatter<vkr::VkResult>
	{
		constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
		{
			return ctx.begin();
		}
		auto format(vkr::VkResult result, format_context& ctx) const -> decltype(ctx.out())
		{
			return format_to(ctx.out(), "{}", vkr::VkResultToString(result));
		}
	};

	template<>
	struct formatter<vkr::VkPhysicalDeviceType>
	{
		constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
		{
			return ctx.begin();
		}
		auto format(vkr::VkPhysicalDeviceType type, format_context& ctx) const -> decltype(ctx.out())
		{
			return format_to(ctx.out(), "{}", vkr::PhysicalDeviceTypeToString(type));
		}
	};
}