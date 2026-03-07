export module vulkan26:sdl3.misc;
import :sdl3.exports;
import std;

export namespace sdl3
{
	struct Init
	{
		~Init()
		{
			sdl3::SDL_Quit();
		}

		Init(std::uint32_t flags)
		{
			if(not sdl3::SDL_Init(flags))
				throw std::runtime_error("Failed to initialize SDL");
		}
	};
}
