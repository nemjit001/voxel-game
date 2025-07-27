#pragma once

#include <memory>
#include <string>

#include "rendering/texture.hpp"

namespace assets
{
	/// @brief The TextureLoader class handles image file I/O.
	class TextureLoader
	{
	public:
		/// @brief Load a texture from disk.
		/// @param path File path to load texture from.
		/// @param mode Interpretation mode (color data indicates SRGB color space)
		/// @return 
		std::shared_ptr<gfx::Texture> load(std::string const& path, gfx::TextureMode mode);
	};
} // namespace assets
