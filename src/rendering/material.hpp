#pragma once

#include <memory>

#include "texture.hpp"

namespace gfx
{
	class Material
	{
	public:
		std::shared_ptr<Texture> albedoTexture = {};
		std::shared_ptr<Texture> normalTexture = {};
	};
} // namespace gfx
