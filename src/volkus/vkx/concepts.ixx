export module volkus:vkx.concepts;
import std;
import :vkx.exports;

export namespace Volkus::vkx
{
	template<typename T, typename M>
	concept RangeOf = std::ranges::range<T> and std::same_as<std::ranges::range_value_t<T>, M>;

	template<typename T, typename M>
	concept ResourceLike = requires(T instance)
	{
		{ instance.Get() } -> std::same_as<M>;
	};

	template<typename T>
	concept InstanceLike = 
		ResourceLike<T, VkInstance>
		and requires(T t)
		{
			{t.GetLayers()} -> std::ranges::range;
			{t.GetExtensions()} -> std::ranges::range;
			{t.GetApplicationInfo()} -> std::convertible_to<VkApplicationInfo>;
			{t.GetFlags()} -> std::convertible_to<VkInstanceCreateFlags>;
		};

	template<typename T, typename M>
	concept UniquePtrLike = requires (T instance)
	{
		{ instance.get() } -> std::same_as<M>;
		{ instance.reset() } -> std::same_as<void>;
	};
}
