export module vulkantutorial:concepts;
import std;

export namespace VulkanTutorial
{
	template<typename T>
	concept Printable = requires(T t)
	{
		{ t.ToString() } -> std::convertible_to<std::string>;
	};
}
