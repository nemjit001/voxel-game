#pragma once

#include <memory>
#include <string>

#include "rendering/texture.hpp"

namespace assets
{
	class TextureLoader
	{
	public:
		std::shared_ptr<gfx::Texture> load(std::string const& path);
	};
} // namespace assets
