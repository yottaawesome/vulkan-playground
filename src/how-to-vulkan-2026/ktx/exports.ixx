module;

#include <volk.h>
#include <ktx.h>
#include <ktxvulkan.h>

export module vulkan26:ktx.exports;

export namespace ktx
{
	auto GetImageOffset(ktxTexture* tex, uint32_t level, uint32_t layer, uint32_t faceSice) -> ktx_size_t
	{
		ktx_size_t mipOffset{ 0 };
		ktxTexture_GetImageOffset(tex, level, layer, faceSice, &mipOffset);
		return mipOffset;
	}

	auto Destroy(ktxTexture* text) -> void
	{
		ktxTexture_Destroy(text);
	}

	using 
		::ktxTexture,
		::ktxTexture1,
		::ktxTextureCreateFlagBits,
		::ktx_error_code_e,
		::ktxTexture_GetVkFormat,
		::ktxTexture_CreateFromNamedFile
		;
}