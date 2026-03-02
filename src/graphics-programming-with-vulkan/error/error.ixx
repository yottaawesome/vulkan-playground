export module vulkangfx:error;
import std;

export namespace Error
{
	template<typename...TArgs>
	struct FormatString
	{
	public:
		consteval FormatString(std::convertible_to<std::format_string<TArgs...>> auto&& fmt, TArgs&&... args, const std::source_location& loc = std::source_location::current())
			: Message(Format(fmt, loc, std::forward<TArgs>(args)...)), Location(loc)
		{ }

		std::string Message;
		std::source_location Location;

	private:
		static auto Format(std::format_string<TArgs...> fmt, const std::source_location& loc, TArgs&&...args) -> std::string
		{
			return std::format("{} ({}:{}:{})", std::format(fmt, std::forward<TArgs>(args)...), loc.function_name(), loc.file_name(), loc.line());
		}
	};
	template<typename... Ts>
	FormatString(std::format_string<Ts...>, Ts&&...) -> FormatString<Ts...>;

	struct RuntimeError : std::runtime_error
	{
	public:
		template<typename...TArgs>
		RuntimeError(FormatString<TArgs...> formatString)
			: std::runtime_error(std::move(formatString.Message)), Location(formatString.Location)
		{ }

		RuntimeError(std::string_view message, const std::source_location& loc = std::source_location::current())
			: std::runtime_error(FormatLocation(message, loc)), Location(loc)
		{ }

		const std::source_location Location;

	private:
		static auto FormatLocation(std::string_view message, const std::source_location& loc) -> std::string
		{
			return std::format("{} ({}:{}:{})", message, loc.function_name(), loc.file_name(), loc.line());
		}
	};

	struct IndexOutOfRangeError : RuntimeError
	{
		IndexOutOfRangeError(std::string_view message, auto index, auto max, const std::source_location& loc = std::source_location::current())
			: RuntimeError(std::format("{} (index: {}, max: {})", message, index, max), loc)
		{ }
	};
}