export module vulkan26:sdl3.error;
import std;
import :sdl3.exports;

export namespace sdl3::Error
{
	auto Get() -> std::string
	{
		return sdl3::SDL_GetError();
	}

	struct Error : std::runtime_error
	{
		Error(
			std::string_view error = SDL_GetError(), 
			const std::source_location& loc = std::source_location::current()
		) : std::runtime_error(Format(loc))
		{ }
		static auto Format(const std::source_location& loc) -> std::string
		{
			return std::format("SDL error: {} at {}:{}", sdl3::Error::Get(), loc.file_name(), loc.line());
		}
	};
}