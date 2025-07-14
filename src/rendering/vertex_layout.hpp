#pragma once

#include <glm/glm.hpp>

namespace gfx
{
	/// @brief Simple layout for static vertices.
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec2 texcoord;
	};
} // namespace gfx
