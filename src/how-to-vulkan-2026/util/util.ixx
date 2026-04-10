export module vulkan26:util;
import std;

export namespace util
{
	template<typename T>
	struct FalseType : std::false_type {};
}
