// various string utilities, such as fixed-size strings.
export module vulkangfx:string;
import std;

export namespace String
{
	template<size_t N, typename TChar>
	struct FixedString
	{
		TChar Data[N]{};
		constexpr FixedString(const TChar(&str)[N]) noexcept
		{
			std::copy_n(str, N, Data);
		}

		constexpr auto ToView(this const FixedString& self) noexcept -> std::basic_string_view<TChar>
		{
			return std::basic_string_view<TChar>{ self.Data, N - 1 };
		}

		constexpr auto ToString(this const FixedString& self) noexcept -> std::basic_string_view<TChar>
		{
			return std::basic_string_view<TChar>{ self.Data, N - 1 };
		}

		constexpr auto begin(this const FixedString& self) noexcept -> const TChar*
		{
			return self.Data;
		}

		constexpr auto end(this const FixedString& self) noexcept -> const TChar*
		{
			return self.Data + N - 1;
		}

		constexpr auto size(this const FixedString&) noexcept -> size_t
		{
			return N - 1;
		}

		constexpr auto empty(this const FixedString&) noexcept -> bool
		{
			return N <= 1;
		}

		constexpr auto operator==(this const FixedString& self, const FixedString& other) noexcept -> bool
		{
			return self.ToView() == other.ToView();
		}

		constexpr auto operator==(this const FixedString& self, const TChar(&str)[N]) noexcept -> bool
		{
			return std::equal(self.Data, self.Data + N - 1, str, str + N - 1);
		}

		template<size_t M>
		constexpr auto operator==(this const FixedString&, const TChar(&)[M]) noexcept -> bool
		{
			return false;
		}
	};
	template<size_t N>
	FixedString(const char(&)[N]) -> FixedString<N, char>;
	template<size_t N>
	FixedString(const wchar_t(&)[N]) -> FixedString<N, wchar_t>;

	static_assert(FixedString("Hello").size() == 5);
	static_assert(FixedString("Hello").empty() == false);
	static_assert(FixedString("Hello") == FixedString("Hello"));
	static_assert(FixedString("Hello") == "Hello");
	static_assert((FixedString("Hello") == "Hello!") == false);
}
