module;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

export module vulkangfx:stb.exports;

export namespace stb
{
	using
		::stbi_uc,
		::STBI_rgb_alpha,
		::STBI_default,
		::STBI_grey,
		::STBI_grey_alpha,
		::STBI_rgb,
		::STBI_rgb_alpha,
		::stbi_load_from_memory,
		::stbi_image_free
		;

	namespace Channels
	{
	}
}