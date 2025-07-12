#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "texture.hpp"

namespace gfx
{
	class Material
	{
	public:
		glm::vec3					albedoColor		= { 0.5F, 0.5F, 0.5F };
		std::shared_ptr<Texture>	albedoTexture	= {};
		std::shared_ptr<Texture>	normalTexture	= {};
	};
} // namespace gfx
