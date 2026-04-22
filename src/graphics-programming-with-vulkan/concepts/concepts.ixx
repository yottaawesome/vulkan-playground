export module vulkangfx:concepts;
import std;

export namespace Concepts
{
	template<typename T, typename... Ts>
	concept OneOf = (std::same_as<T, Ts> or ...);

	template<typename T, typename S>
	concept RangeOf = std::ranges::range<T> and std::same_as<std::ranges::range_value_t<T>, S>;
}
