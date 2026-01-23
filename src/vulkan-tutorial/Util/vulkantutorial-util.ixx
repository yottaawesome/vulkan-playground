export module vulkantutorial:util;
import std;

export namespace VulkanTutorial::Util
{
	template<size_t VSize, typename TChar>
	struct FixedString
	{
		TChar Buffer[VSize]{};

		constexpr FixedString(const TChar (&str)[VSize]) noexcept
		{
			std::copy_n(str, VSize, Buffer);
		}

		constexpr auto ToView() const noexcept -> std::basic_string_view<TChar>
		{
			return std::basic_string_view<TChar>{ Buffer, VSize - 1 };
		}

		constexpr auto ToString() const noexcept -> std::basic_string<TChar>
		{
			return std::basic_string<TChar>{ Buffer, VSize - 1 };
		}
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
