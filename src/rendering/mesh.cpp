#include "mesh.hpp"

#include <cassert>

namespace gfx
{
	Mesh::Mesh(std::vector<Vertex> const& vertices, std::vector<IndexType> const& indices)
		:
		m_vertices(vertices),
		m_indices(indices)
	{
		//
	}

	Mesh::~Mesh()
	{
		if (m_vertexBuffer) {
			wgpuBufferRelease(m_vertexBuffer);
		}

		if (m_indexBuffer) {
			wgpuBufferRelease(m_indexBuffer);
		}
	}

	void Mesh::setBuffers(std::vector<Vertex> const& vertices, std::vector<IndexType> const& indices)
	{
		m_dirty = true;
		m_vertices = vertices;
		m_indices = indices;
	}

	void Mesh::getBuffers(std::vector<Vertex>& vertices, std::vector<IndexType>& indices)
	{
		vertices = m_vertices;
		indices = m_indices;
	}

	void Mesh::setVertexBuffer(WGPUBuffer buffer)
	{
		assert(buffer != nullptr && "Buffer handle cannot be a nullptr");

		if (m_vertexBuffer) {
			wgpuBufferRelease(m_vertexBuffer);
		}

		m_vertexBuffer = buffer;
	}

	void Mesh::setIndexBuffer(WGPUBuffer buffer)
	{
		assert(buffer != nullptr && "Buffer handle cannot be a nullptr");

		if (m_indexBuffer) {
			wgpuBufferRelease(m_indexBuffer);
		}

		m_indexBuffer = buffer;
	}
} // namespace gfx
