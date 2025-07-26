#include "texture.hpp"

#include <cassert>

namespace gfx
{
	Texture::Texture(TextureDimensions dimensions, TextureExtent const& extent, uint8_t components, void* pTextureData)
		:
		m_dimensions(dimensions),
		m_extent(extent),
		m_components(components)
	{
		assert(extent.width > 0 && extent.height > 0 && extent.depthOrArrayLayers > 0 && "Texture extent cannot be 0 in any direction");
		assert(components > 0 && components <= 4 && "Components must be between 0 and 4");
		assert(pTextureData != nullptr && "Texture data cannot be a nullptr");

		// Set internal buffer size
		size_t const size = extent.width * extent.height * extent.depthOrArrayLayers * components;
		m_data.resize(size);

		// Take ownership of passed raw image buffer
		memcpy(m_data.data(), pTextureData, size);
	}

	Texture::~Texture()
	{
		if (m_texture) {
			wgpuTextureRelease(m_texture);
		}

		if (m_textureView) {
			wgpuTextureViewRelease(m_textureView);
		}
	}

	void Texture::setTexture(WGPUTexture texture)
	{
		assert(texture != nullptr && "Texture handle cannot be a nullptr");

		if (m_texture && m_textureView) {
			wgpuTextureRelease(m_texture);
			wgpuTextureViewRelease(m_textureView);
		}

		m_texture = texture;
		m_textureView = wgpuTextureCreateView(m_texture, nullptr /* assume default view */);
	}
} // namespace gfx
