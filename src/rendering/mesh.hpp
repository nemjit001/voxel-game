#pragma once

#include <cstdint>
#include <vector>
#include <webgpu/webgpu.h>

#include "vertex_layout.hpp"

namespace gfx
{
	using IndexType = uint32_t;

	/// @brief The Mesh class stores host-side and device-side mesh data.
	class Mesh
	{
	public:
		/// @brief Defaulted constructor.
		Mesh() = default;

		/// @brief Create a new Mesh.
		/// @param vertices 
		/// @param indices 
		Mesh(std::vector<Vertex> const& vertices, std::vector<IndexType> const& indices);

		/// @brief Destructor.
		~Mesh();

		Mesh(Mesh const&) = delete;
		Mesh& operator=(Mesh const&) = delete;

		/// @brief Set the host-side vertex and index buffers for this mesh.
		/// @param vertices Buffer containing vertex data.
		/// @param indices Buffer containing indices for mesh triangles.
		void setBuffers(std::vector<Vertex> const& vertices, std::vector<IndexType> const& indices);

		/// @brief Retrieve the stored host-side mesh buffers.
		/// @param vertices Buffer containing vertex data.
		/// @param indices Buffer containing indices for mesh triangles.
		void getBuffers(std::vector<Vertex>& vertices, std::vector<IndexType>& indices);

		/// @brief Check if this mesh is dirty, i.e. its host-side buffers have been updated.
		/// @return 
		bool isDirty() const { return m_dirty; }

		/// @brief Clear the mesh dirty flag to indicate host and device buffers are in sync.
		void clearDirtyFlag() { m_dirty = false; }

		/// @brief Retrieve the number of vertices of this mesh.
		/// @return 
		size_t vertexCount() const { return m_vertices.size(); }

		/// @brief Retrieve the number of indices of this mesh.
		/// @return 
		size_t indexCount() const { return m_indices.size(); }

		/// @brief Set the device-side vertex buffer for this mesh. Takes ownership of this buffer.
		/// @param buffer 
		void setVertexBuffer(WGPUBuffer buffer);

		/// @brief Set the device-side index buffer for this mesh. Takes ownership of this buffer.
		/// @param buffer 
		void setIndexBuffer(WGPUBuffer buffer);

		/// @brief Get the device-side vertex buffer for this mesh.
		/// @return 
		WGPUBuffer getVertexBuffer() const { return m_vertexBuffer; }

		/// @brief Get the device-side index buffer for this mesh.
		/// @return 
		WGPUBuffer getIndexBuffer() const { return m_indexBuffer; }

	private:
		bool					m_dirty			= true;
		std::vector<Vertex>		m_vertices		= {};
		std::vector<IndexType>	m_indices		= {};
		WGPUBuffer				m_vertexBuffer	= nullptr;
		WGPUBuffer				m_indexBuffer	= nullptr;
	};
} // namespace gfx
