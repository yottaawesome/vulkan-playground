module;

#include <cstddef>

export module vulkangfx:util;
import std;

export namespace Util
{
	template<typename T, typename M>
	constexpr auto OffsetOf(M T::* member) -> size_t
	{
		return reinterpret_cast<size_t>(&(((T*)0)->*member));
	}

	template<typename T>
	struct EnumValue
	{
		T Value{};
		constexpr operator T(this EnumValue self) noexcept { return self.Value; }
		constexpr auto operator==(this EnumValue self, T other) noexcept -> bool { return self.Value == other; }
		constexpr auto operator==(this EnumValue self, EnumValue other) noexcept -> bool { return self.Value == other.Value; }
		constexpr auto operator!=(this EnumValue self, T other) noexcept -> bool { return self.Value != other; }
		constexpr auto operator!=(this EnumValue self, EnumValue other) noexcept -> bool { return self.Value != other.Value; }
		constexpr auto ToUnderlying(this EnumValue self) noexcept -> std::underlying_type_t<T> { return std::to_underlying(self.Value); }
	};
}