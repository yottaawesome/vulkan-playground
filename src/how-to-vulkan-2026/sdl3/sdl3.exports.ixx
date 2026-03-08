module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
export module vulkan26:sdl3.exports;
import std;

export namespace sdl3
{
	using 
		::SDL_Window,
		::SDL_Init,
		::SDL_Quit,
		::SDL_CreateWindow,
		::SDL_DestroyWindow,
		::SDL_PumpEvents,
		::SDL_PollEvent,
		::SDL_GetError
		;
}

export namespace sdl3::InitFlags
{
	enum : std::uint32_t
	{
		Audio = SDL_INIT_AUDIO,
		Video = SDL_INIT_VIDEO,
		Joystick = SDL_INIT_JOYSTICK,
		Haptic = SDL_INIT_HAPTIC,
		Events = SDL_INIT_EVENTS
	};
}

export namespace sdl3::vk
{
	using 
		::SDL_Vulkan_LoadLibrary,
		::SDL_Vulkan_CreateSurface,
		::SDL_Vulkan_GetInstanceExtensions,
		::SDL_Vulkan_GetPresentationSupport
		;
}
