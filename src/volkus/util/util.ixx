export module volkus:util;
import std;

export namespace Volkus::Util
{
	template<typename T>
	struct AlwaysFalse : std::false_type {};
}
