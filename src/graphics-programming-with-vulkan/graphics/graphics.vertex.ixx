export module vulkangfx:graphics.vertex;
import std;
import :vulkan;
import :glm;
import :util;

export namespace Graphics
{
	struct Vertex
	{
		glm::vec3 position{};
		glm::vec2 uv{};

		constexpr Vertex() = default;
		constexpr Vertex(glm::vec3 position, glm::vec2 uv) : position(position), uv(uv) {}

		static auto GetBindingDescription() -> vkr::VkVertexInputBindingDescription
		{
			return {
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = vkr::VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX
			};
		}

		static auto GetAttributeDescription() -> auto
		{
			return std::array{
				vkr::VkVertexInputAttributeDescription{
					.location = 0,
					.binding = 0,
					.format = vkr::VkFormat::VK_FORMAT_R32G32B32_SFLOAT,
					.offset = static_cast<std::uint32_t>(Util::OffsetOf(&Vertex::position))
				},
				vkr::VkVertexInputAttributeDescription{
					.location = 1,
					.binding = 0,
					.format = vkr::VkFormat::VK_FORMAT_R32G32_SFLOAT,
					.offset = static_cast<std::uint32_t>(Util::OffsetOf(&Vertex::uv))
				}
			};
		}
	};
}
