export module vulkangfx:glfw.error;
import std;
import :glfw.exports;

export namespace glfw
{
	// https://www.glfw.org/docs/3.3/intro_guide.html#error_handling
	class Error : public std::runtime_error
	{
	public:
		Error(std::string_view message, const std::source_location &loc = std::source_location::current())
			: std::runtime_error(Format(message, loc)) 
		{}

		auto Format(
			this Error& self, 
			std::string_view message, 
			const std::source_location& loc
		) -> std::string
		{
			const char* description;
			self.errorCode = glfwGetError(&description);
			self.errorDescription = description 
				? description : "Unknown error";
			return std::format("{} -- {} at {}:{}:{}",
				message, 
				self.errorDescription, 
				loc.file_name(), 
				loc.line(), 
				loc.column()
			);
		}
	private:
		int errorCode = 0;
		std::string errorDescription;
	};
}
