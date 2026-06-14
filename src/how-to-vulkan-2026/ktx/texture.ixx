export module vulkan26:ktx.loadedtexture;
import std;
import :ktx.exports;
import :vulkan;

export namespace ktx
{
	class LoadedTexture
	{
	public:
		~LoadedTexture()
		{
			ktx::Destroy(ktxTexture);
		}

		LoadedTexture(const LoadedTexture&) = delete;
		auto operator=(const LoadedTexture&) -> LoadedTexture& = delete;

		LoadedTexture(std::filesystem::path filename)
		{
			ktxTexture_CreateFromNamedFile(filename.string().c_str(), ktxTextureCreateFlagBits::KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
		}

		auto GetFormat() const -> VkFormat
		{
			return ktxTexture_GetVkFormat(ktxTexture);
		}

		auto GetWidth() const -> std::uint32_t
		{
			return ktxTexture->baseWidth;
		}

		auto GetHeight() const -> std::uint32_t
		{
			return ktxTexture->baseHeight;
		}

		auto GetNumLevels() const -> std::uint32_t
		{
			return ktxTexture->numLevels;
		}

		auto GetDataSize() const -> ktx_size_t
		{
			return ktxTexture->dataSize;
		}

		auto GetData() const -> const ktx_uint8_t*
		{
			return ktxTexture->pData;
		}

	private:
		ktxTexture* ktxTexture{ nullptr };
	};
};