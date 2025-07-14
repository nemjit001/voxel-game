#include "mesh_loader.hpp"

#include <cassert>
#include <vector>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <tiny_gltf.h>

namespace assets
{
	template<typename AccessorType>
	std::vector<AccessorType> readBufferContents(tinygltf::Accessor const& accessor, tinygltf::BufferView const& view, tinygltf::Buffer const& buffer)
	{
		assert(accessor.ByteStride(view) != -1);

		std::vector<AccessorType> contents{};
		for (size_t i = 0; i < accessor.count; i++)
		{
			size_t const offset = view.byteOffset + i * accessor.ByteStride(view);
			contents.push_back(*reinterpret_cast<AccessorType const*>(&buffer.data[offset]));
		}

		return contents;
	}

	std::shared_ptr<gfx::Mesh> MeshLoader::load(std::string const& path)
	{
		SPDLOG_INFO("Loading mesh file {}", path);

		// Load model file using tinygltf, assuming glb file
		std::string warning{};
		std::string error{};
		tinygltf::Model model{};
		tinygltf::TinyGLTF loader{};
		if (!loader.LoadBinaryFromFile(&model, &error, &warning, path)) {
			if (!warning.empty()) {
				SPDLOG_WARN("{}", warning);
			}

			if (!error.empty()) {
				SPDLOG_ERROR("{}", error);
			}

			return {};
		}

		if (!warning.empty()) {
			SPDLOG_WARN("{}", warning);
		}

		// Parse file contents into single mesh
		std::vector<gfx::Vertex> vertices{};
		std::vector<gfx::IndexType> indices{};

		for (auto const& mesh : model.meshes) {
			SPDLOG_TRACE("Found mesh: {}", mesh.name);

			for (auto const& primitive : mesh.primitives)
			{
				// Skip non triangle, non-indexed meshes for now
				if (primitive.mode != TINYGLTF_MODE_TRIANGLES || primitive.indices < 0) {
					continue;
				}

				// Load index data
				std::vector<gfx::IndexType> subMeshIndices{};
				auto const& indicesAccessor = model.accessors[primitive.indices];
				auto const& indicesView = model.bufferViews[indicesAccessor.bufferView];
				auto const& indicesBuffer = model.buffers[indicesView.buffer];
				if (indicesAccessor.type == TINYGLTF_TYPE_SCALAR && indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
					std::vector<uint16_t> contents = readBufferContents<uint16_t>(indicesAccessor, indicesView, indicesBuffer);
					for (auto const& c : contents) {
						subMeshIndices.push_back(static_cast<gfx::IndexType>(c));
					}
				}
				else if (indicesAccessor.type == TINYGLTF_TYPE_SCALAR && indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
					std::vector<uint32_t> contents = readBufferContents<uint32_t>(indicesAccessor, indicesView, indicesBuffer);
					for (auto const& c : contents) {
						subMeshIndices.push_back(static_cast<gfx::IndexType>(c));
					}
				}
				else {
					throw std::runtime_error("Unsupported index type encountered in GLTF file");
				}

				// Load vertex attributes
				std::vector<glm::vec3> positions{};
				std::vector<glm::vec3> normals{};
				std::vector<glm::vec3> tangents{};
				std::vector<glm::vec2> texcoords{};
				for (auto const& attr : primitive.attributes)
				{
					// Fetch attribute accessor/view/buffer etc.
					SPDLOG_TRACE("Attribute {}:{}", attr.first, attr.second);
					auto const& accessor = model.accessors[attr.second];
					auto const& view = model.bufferViews[accessor.bufferView];
					auto const& buffer = model.buffers[view.buffer];

					/// Read vec3 data from source
					auto const readVec3Data = [&](std::vector<glm::vec3>& out)
					{
						if (accessor.type == TINYGLTF_TYPE_VEC3 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
						{
							out = readBufferContents<glm::vec3>(accessor, view, buffer);
						}
						else if (accessor.type == TINYGLTF_TYPE_VEC4 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
						{
							std::vector<glm::vec4> const contents = readBufferContents<glm::vec4>(accessor, view, buffer);

							for (auto const& c : contents) {
								out.push_back(glm::vec3(c) / c.w); // Works for tangent handedness & heterogenous coord convert to vec3 :)
							}
						}
						else
						{
							throw std::runtime_error("Unsupported vec3-like data type encountered in GLTF file");
						}
					};

					// Read vec2 data from source
					auto const readVec2Data = [&](std::vector<glm::vec2>& out)
					{
						if (accessor.type == TINYGLTF_TYPE_VEC2 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
						{
							out = readBufferContents<glm::vec2>(accessor, view, buffer);
						}
						else
						{
							throw std::runtime_error("Unsupported vec2-like data type encountered in GLTF file");
						}
					};

					// Actually load data using util functions
					if (attr.first == "POSITION") {
						readVec3Data(positions);
					}
					else if (attr.first == "NORMAL") {
						readVec3Data(normals);
					}
					else if (attr.first == "TANGENT") {
						readVec3Data(tangents);
					}
					else if (attr.first == "TEXCOORD_0") { // Only support for 1 texture channel (why should we need more?)
						readVec2Data(texcoords);
					}
				}

				// Tightly pack mesh data into mesh buffers
				assert(!positions.empty() && !normals.empty() && !tangents.empty() && !texcoords.empty());
				assert(positions.size() == normals.size() && positions.size() == tangents.size() && tangents.size() == texcoords.size());

				size_t const vertexCount = positions.size();
				for (size_t i = 0; i < vertexCount; i++) {
					vertices.push_back(gfx::Vertex{ positions[i], normals[i], tangents[i], texcoords[i] });
				}

				uint32_t const indexOffset = static_cast<uint32_t>(indices.size());
				for (auto const& idx : subMeshIndices) {
					indices.push_back(indexOffset + idx);
				}
			}
		}

		// Done!
		SPDLOG_INFO("Loaded mesh file {}", path);
		return std::make_shared<gfx::Mesh>(vertices, indices);
	}
} // namespace assets
