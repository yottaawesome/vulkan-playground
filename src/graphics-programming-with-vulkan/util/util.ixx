module;

#include <cstddef>

export module vulkangfx:util;

export namespace Util
{
	template<typename T, typename M>
	constexpr auto OffsetOf(M T::* member) -> size_t
	{
		return reinterpret_cast<size_t>(&(((T*)0)->*member));
	}
}