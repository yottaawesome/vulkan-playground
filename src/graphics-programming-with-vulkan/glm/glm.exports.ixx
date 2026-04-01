module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module vulkangfx:glm.exports;

export namespace glm
{
	using
		::glm::vec2,
		::glm::vec3,
		::glm::vec4,
		::glm::mat2,
		::glm::mat3,
		::glm::mat4,
		::glm::ivec1,
		::glm::ivec2,
		::glm::ivec3,
		::glm::ivec4,
		::glm::rotate,
		::glm::radians,
		::glm::perspective,
		::glm::translate,
		::glm::lookAt
		;
}