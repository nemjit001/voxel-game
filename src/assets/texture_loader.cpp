#include "texture_loader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <cassert>
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <stb_image_write.h>

namespace assets
{
	std::shared_ptr<gfx::Texture> TextureLoader::load(std::string const& path)
	{
		SPDLOG_INFO("Loading texture file {}", path);

		// Load texture file from disk
		int w, h, c; // width, height, channels
		stbi_uc* pImageData = stbi_load(path.c_str(), &w, &h, &c, 0);

		// XXX(nemjit001): This is kinda gross, but we reload the texture with 4 channels forced because WebGPU does not support RGB textures
		if (pImageData && c == 3)
		{
			STBI_FREE(pImageData);		
			pImageData = stbi_load(path.c_str(), &w, &h, nullptr, 4);
			c = 4;
		}

		if (!pImageData)
		{
			SPDLOG_ERROR("Failed to load texture file (does it exist?)");
			return {};
		}

		// Check if texture extent is actually valid
		if (w <= 0 || h <= 0 || c < 0 || c > 4)
		{
			SPDLOG_ERROR("Texture file has invalid width/height/component values ({}x{}x{})", w, h, c);
			STBI_FREE(pImageData);
			return {};
		}

		// Set extent & create texture object
		gfx::TextureExtent const extent{ static_cast<uint32_t>(w), static_cast<uint32_t>(h), 1 };
		std::shared_ptr<gfx::Texture> texture = std::make_shared<gfx::Texture>(
			gfx::TextureDimensions::Dim2D,
			extent,
			static_cast<uint8_t>(c),
			pImageData
		);

		STBI_FREE(pImageData);
		SPDLOG_INFO("Loaded texture file!");
		return texture;
	}
} // namespace assets
