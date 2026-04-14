export module vulkan26:mesh;
import std;
import :glm;

export namespace Mesh
{
	struct Vertex
	{
		glm::vec3 Pos;
		glm::vec3 Normal;
		glm::vec2 Uv;
	};

	struct MeshData
	{
		std::vector<Mesh::Vertex> Vertices;
		std::vector<std::uint16_t> Indices;
	};
}
