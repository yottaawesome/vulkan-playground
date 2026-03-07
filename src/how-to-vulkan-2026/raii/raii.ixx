export module vulkan26:raii;
import std;

export namespace Raii
{
	template <auto VDeleteFn, auto...VArgs>
	struct Deleter
	{
		static constexpr void operator()(auto&& args) noexcept
		{
			VDeleteFn(std::forward<decltype(args)>(args), std::forward<decltype(VArgs)>(VArgs)...);
		}
	};

	template <typename T, auto VDeleteFn, auto...VArgs>
	using DirectUniquePtr = std::unique_ptr<T, Deleter<VDeleteFn, VArgs...>>;
	template <typename T, auto VDeleteFn, auto...VArgs>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleteFn, VArgs...>>;
}
