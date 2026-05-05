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

	void Destroy(ktxTexture* text)
	{
		ktxTexture_Destroy(text);
	}

	using Texture = ktxTexture;
	using Texture1 = ktxTexture1;
	using TextureCreateFlagBits = ktxTextureCreateFlagBits;
	using ErrorCode = ktx_error_code_e;
}

export using
	::ktxTexture,
	::ktxTexture1,
	::ktxTextureCreateFlagBits,
	::ktx_error_code_e,
	::ktxTexture_GetVkFormat,
	::ktxTexture_CreateFromNamedFile
;