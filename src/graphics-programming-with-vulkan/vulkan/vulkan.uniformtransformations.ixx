export module vulkangfx:vulkan.uniformtransformations;
import std;
import :glm;

export namespace Vulkan
{
	struct UniformTransformations
	{
		glm::mat4 view;
		glm::mat4 projection;
	};
}
