export module vulkantutorial:concepts;
import std;

export namespace VulkanTutorial::Concepts
{
	template<typename T>
	concept Printable = requires(T t)
	{
		{ t.ToString() } -> std::convertible_to<std::string>;
	};

	template<typename TRange, typename TTarget>
	concept RangeOf = 
		std::ranges::range<TRange>
		and std::same_as<std::ranges::range_value_t<TRange>, TTarget>;
}
