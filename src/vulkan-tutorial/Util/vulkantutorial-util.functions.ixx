export module vulkantutorial:util.functions;
import std;

export namespace VulkanTutorial::Util
{
	auto ReadBinaryFile(const std::filesystem::path& filename) -> std::vector<std::byte>
	{
		auto file = std::ifstream{ filename.string(), std::ios::ate | std::ios::binary };
		if (not file.is_open())
			throw std::runtime_error("Failed to open file!");
		auto buffer = std::vector<std::byte>(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
		file.close();
		return buffer;
	}
}
