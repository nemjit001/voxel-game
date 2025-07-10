#pragma once

#include <cstdint>
#include <vector>
#include <webgpu/webgpu.h>

namespace gfx
{
	/// @brief Texture dimensions for interpreting extent variables.
	enum class TextureDimensions
	{
		Dim1D,
		Dim2D,
		Dim3D,
	};

	/// @brief Texture extent in 3 directions.
	struct TextureExtent
	{
		uint32_t width;
		uint32_t height;
		uint32_t depthOrArrayLayers;
	};

	/// @brief The Texture class stores host-side and device-side texture data.
	class Texture
	{
	public:
		/// @brief Create a new texture object.
		/// @param dimensions Texture dimensions, used to interpret extent values.
		/// @param extent Texture extent in x/y/z directions, supports layered texturees.
		/// @param components Number of color channels in this texture.
		/// @param pTextureData Contiguous texture data, this will be copied to an internal buffer.
		Texture(TextureDimensions dimensions, TextureExtent const& extent, uint8_t components, void* pTextureData);
		~Texture();

		Texture(Texture const&) = delete;
		Texture& operator=(Texture const&) = delete;

		/// @brief Check if this mesh is dirty, i.e. its host-side buffers have been updated.
		/// @return 
		bool isDirty() const { return m_dirty; }

		/// @brief Clear the mesh dirty flag to indicate host and device buffers are in sync.
		void clearDirtyFlag() { m_dirty = false; }

		/// @brief Get the texture dimensionality of this texture.
		/// @return 
		TextureDimensions dimensions() const { return m_dimensions; }

		/// @brief Get the extent of this texture.
		/// @return 
		TextureExtent extent() const { return m_extent; }

		/// @brief Get the number of color channels in this texture, in range [0, 4].
		/// @return 
		uint8_t components() const { return m_components; }

		/// @brief Get the host-side texture data stored in this texture as bytes.
		/// @return 
		uint8_t const* data() const { return m_data.data(); }

		/// @brief Set the device-side texture handle. Takes ownership of this texture.
		/// @param texture 
		void setTexture(WGPUTexture texture);

		/// @brief Get the device-side texture handle.
		/// @return 
		WGPUTexture getTexture() const { return m_texture; }

		/// @brief Get the device-side texture view handle.
		/// @return 
		WGPUTextureView getTextureView() const { return m_textureView; }

	private:
		bool					m_dirty			= true;
		TextureDimensions		m_dimensions	= TextureDimensions::Dim1D;
		TextureExtent			m_extent		= {};
		uint8_t					m_components	= 0;	// Color components
		std::vector<uint8_t>	m_data			= {};	// Texture data stored as byte array.
		WGPUTexture				m_texture		= nullptr;
		WGPUTextureView			m_textureView	= nullptr;
	};
} // namespace gfx
