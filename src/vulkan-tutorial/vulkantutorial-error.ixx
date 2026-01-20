export module vulkantutorial:error;
import std;

export namespace VulkanTutorial
{
	struct VulkanError : std::runtime_error
	{
		VulkanError(
			std::string_view message, 
			const std::source_location& loc = std::source_location::current()
		) : std::runtime_error(FormatMessage(message, loc))
		{}

		static auto FormatMessage(
			std::string_view message,
			const std::source_location& loc = std::source_location::current()
		) -> std::string
		{
			return std::format(
				"VulkanError: {}\n  at {}:{} in {}",
				message,
				loc.file_name(),
				loc.line(),
				loc.function_name()
			);
		}
	};
}