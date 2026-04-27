export module volkus:vkx.concepts;
import std;
import :vkx.exports;

export namespace Volkus::vkx
{
	template<typename T, typename M>
	concept ResourceLike = requires(T instance)
	{
		{ instance.Get() } -> std::same_as<M>;
	};

	template<typename T>
	concept InstanceLike = ResourceLike<T, VkInstance>;

	template<typename T, typename M>
	concept UniquePtrLike = requires (T instance)
	{
		{ instance.get() } -> std::same_as<M>;
		{ instance.reset() } -> std::same_as<void>;
	};
}
