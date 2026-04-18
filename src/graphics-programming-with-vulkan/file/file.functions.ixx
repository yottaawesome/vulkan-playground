export module vulkangfx:file.functions;
import std;
import :error;

export namespace File
{
	[[nodiscard]]
	auto ReadFileBytes(const std::filesystem::path& filePath) -> std::vector<std::byte>
	{
		auto file = std::ifstream(filePath, std::ios::ate | std::ios::binary);
		if (not file.is_open())
			throw Error::RuntimeError(std::format("Failed to open file: {}", filePath.string()));
		auto fileSize = static_cast<std::size_t>(file.tellg());
		auto buffer = std::vector<std::byte>(fileSize);
		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		return buffer;
	}
}
