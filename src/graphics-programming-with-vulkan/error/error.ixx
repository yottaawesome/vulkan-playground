export module vulkangfx:error;
import std;

export namespace Error
{
	template<typename...TArgs>
	struct FormatString
	{
		constexpr FormatString(std::format_string<TArgs...> fmt, TArgs&&... args, const std::source_location& loc = std::source_location::current())
			: Message(Format(fmt, loc, std::forward<TArgs>(args)...)), Location(loc)
		{ }

		std::string Message;
		std::source_location Location;
		
		template<typename...TArgs>
		static constexpr auto Format(std::format_string<TArgs...> fmt, const std::source_location& loc, TArgs&&...args) -> std::string
		{
			return std::format("{} ({}:{}:{})", std::format(fmt, std::forward<TArgs>(args)...), loc.function_name(), loc.file_name(), loc.line());
		}
	};
	template<typename... Ts>
	FormatString(std::format_string<Ts...>, Ts&&...) -> FormatString<Ts...>;

	struct RuntimeError : std::runtime_error
	{
		template<typename...TArgs>
		explicit RuntimeError(FormatString<TArgs...>&& formatString)
			: std::runtime_error(std::forward<decltype(formatString)>(formatString).Message), Location(formatString.Location)
		{ }

		RuntimeError(std::string_view message, const std::source_location& loc = std::source_location::current())
			: std::runtime_error(Format(message, loc))
		{ }

		static auto Format(std::string_view message, const std::source_location& loc) -> std::string
		{
			return std::format("{} ({}:{}:{})", message, loc.file_name(), loc.line(), loc.column());
		}

		const std::source_location Location;
	};
}