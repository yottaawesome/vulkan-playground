export module vulkan26:tinyobj.functions;
import std;
import :tinyobj.exports;

export namespace tinyobj
{
	struct FileData
	{
		std::vector<attrib_t> Attribs;
		std::vector<shape_t> Shapes;
		std::vector<material_t> Materials;
		std::string Warning;
		std::string Error;

		static auto From(const std::filesystem::path& path) -> FileData
		{
			auto attrib = attrib_t{};
			auto shapes = std::vector<shape_t>{};
			auto materials = std::vector<material_t>{};
			auto warn = std::string{};
			auto err = std::string{};
			if (not LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str()))
				throw std::runtime_error{ "Failed to load OBJ file: " + err };
			return { 
				.Attribs = { std::move(attrib) }, 
				.Shapes = std::move(shapes), 
				.Materials = std::move(materials),
				.Warning = std::move(warn),
				.Error = std::move(err),
			};
		}
	};
}
