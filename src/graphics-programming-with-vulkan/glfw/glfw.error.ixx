export module vulkangfx:glfw.error;
import std;
import :glfw.exports;

export namespace glfw
{
	struct SimpleError
	{
		int Code = 0;
		std::string Description;

		SimpleError()
		{
			Code = 0;
			const char* description;
			Code = glfw::glfwGetError(&description);
			Description = description ? description : "Unknown error";
		}

		SimpleError(int code, const char* description)
			: Code(code), Description(description ? description : "Unknown error")
		{
		}
	};

	// https://www.glfw.org/docs/3.3/intro_guide.html#error_handling
	class Error : public std::runtime_error
	{
		struct ErrorInfo
		{
			int Code = 0;
			std::string Description;
			std::string FormattedMessage;
		};

	public:
		Error(std::string_view message, const std::source_location &loc = std::source_location::current())
			: Error(QueryError(message, loc))
		{}

	private:
		Error(ErrorInfo info)
			: std::runtime_error(std::move(info.FormattedMessage))
			, errorCode(info.Code)
			, errorDescription(std::move(info.Description))
		{}

		static auto QueryError(
			std::string_view message,
			const std::source_location& loc
		) -> ErrorInfo
		{
			const char* description;
			auto code = glfwGetError(&description);
			auto desc = std::string{ description ? description : "Unknown error" };
			auto formatted = std::format("{} -- {} at {}:{}:{}",
				message, desc, loc.file_name(), loc.line(), loc.column());
			return { code, std::move(desc), std::move(formatted) };
		}

		int errorCode = 0;
		std::string errorDescription;
	};
}
