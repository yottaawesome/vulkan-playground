export module vulkantutorial:util;
import std;

export namespace VulkanTutorial::Util
{
	template<size_t VSize, typename TChar>
	struct FixedString
	{
		using View = std::basic_string_view<TChar>;
		using String = std::basic_string<TChar>;

		TChar Buffer[VSize]{};

		constexpr FixedString(const TChar (&str)[VSize]) noexcept
		{
			std::copy_n(str, VSize, Buffer);
		}

		constexpr auto ToView(this const FixedString& self) noexcept -> View
		{
			return View{ self.Buffer, VSize - 1 };
		}

		constexpr auto ToString(this const FixedString& self) noexcept -> String
		{
			return String{ self.Buffer, VSize - 1 };
		}

		constexpr auto Size(this const FixedString& self) noexcept -> size_t
		{
			return VSize - 1;
		}

		struct Iterator
		{
			const TChar* Ptr;
			constexpr auto operator*() const noexcept -> const TChar&
			{
				return *Ptr;
			}
			constexpr auto operator++(this Iterator& self) noexcept -> Iterator&
			{
				++self.Ptr;
				return self;
			}
			constexpr auto operator!=(this const Iterator& self, const Iterator& other) noexcept -> bool
			{
				return self.Ptr != other.Ptr;
			}
		};

		constexpr auto begin(this const FixedString& self) noexcept -> Iterator
		{
			return Iterator{ self.Buffer };
		}
		constexpr auto end(this const FixedString& self) noexcept -> Iterator
		{
			return Iterator{ self.Buffer + VSize - 1 };
		}

		constexpr auto operator<=>(this const FixedString& self, const FixedString& other) noexcept = default;
	};
	template<size_t VSize>
	FixedString(const char(&)[VSize]) -> FixedString<VSize, char>;
	template<size_t VSize>
	FixedString(const wchar_t(&)[VSize]) -> FixedString<VSize, wchar_t>;

	constexpr bool IsDebug = 
#ifdef _DEBUG	
		true;
#else
		false;
#endif
	constexpr bool IsRelease = not IsDebug;
	constexpr bool EnableValidationLayers = IsDebug;
}
