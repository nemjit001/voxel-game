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
		if (!tinygltf::TinyGLTF().LoadBinaryFromFile(&model, &error, &warning, path)) {
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
		for (auto const& mesh : model.meshes) {
			SPDLOG_INFO("Found mesh: {}", mesh.name);

			for (auto const& primitive : mesh.primitives)
			{
				// Skip non triangle, non-indexed meshes for now
				if (primitive.mode != TINYGLTF_MODE_TRIANGLES || primitive.indices < 0) {
					continue;
				}

				// Load index data
				std::vector<gfx::IndexType> indices{};
				{
					auto const& indicesAccessor = model.accessors[primitive.indices];
					auto const& indicesView = model.bufferViews[indicesAccessor.bufferView];
					auto const& indicesBuffer = model.buffers[indicesView.buffer];
					if (indicesAccessor.type == TINYGLTF_TYPE_SCALAR && indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
						std::vector<uint16_t> contents = readBufferContents<uint16_t>(indicesAccessor, indicesView, indicesBuffer);
						for (auto const& c : contents) {
							indices.push_back(static_cast<gfx::IndexType>(c));
						}
					}
					else if (indicesAccessor.type == TINYGLTF_TYPE_SCALAR && indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
						std::vector<uint32_t> contents = readBufferContents<uint32_t>(indicesAccessor, indicesView, indicesBuffer);
						for (auto const& c : contents) {
							indices.push_back(static_cast<gfx::IndexType>(c));
						}
					}
					else {
						throw std::runtime_error("Unsupported index type encountered in GLTF file");
					}
				}

				// Load vertex attributes
				std::vector<glm::vec3> positions{};
				std::vector<glm::vec3> normals{};
				std::vector<glm::vec3> tangents{};
				std::vector<glm::vec2> texcoords{};
				for (auto const& attr : primitive.attributes)
				{
					SPDLOG_INFO("Attribute {}:{}", attr.first, attr.second);
					auto const& accessor = model.accessors[attr.second];
					auto const& view = model.bufferViews[accessor.bufferView];
					auto const& buffer = model.buffers[view.buffer];

					// Yucky if/else, pls fix w/ automatic type detection or loading into vectors
					if (attr.first == "POSITION")
					{
						if (accessor.type == TINYGLTF_TYPE_VEC3 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
							positions = readBufferContents<glm::vec3>(accessor, view, buffer);
						}
					}
					else if (attr.first == "NORMAL")
					{
						if (accessor.type == TINYGLTF_TYPE_VEC3 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
							normals = readBufferContents<glm::vec3>(accessor, view, buffer);
						}
						else if (accessor.type == TINYGLTF_TYPE_VEC4 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
							std::vector<glm::vec4> contents = readBufferContents<glm::vec4>(accessor, view, buffer);
							for (auto const& c : contents) {
								normals.push_back(glm::vec3(c));
							}
						}
					}
					else if (attr.first == "TANGENT")
					{
						if (accessor.type == TINYGLTF_TYPE_VEC3 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
							tangents = readBufferContents<glm::vec3>(accessor, view, buffer);
						}
						else if (accessor.type == TINYGLTF_TYPE_VEC4 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
							std::vector<glm::vec4> contents = readBufferContents<glm::vec4>(accessor, view, buffer);
							for (auto const& c : contents) {
								tangents.push_back(glm::vec3(c));
							}
						}
					}
					else if (attr.first == "TEXCOORD_0")
					{
						if (accessor.type == TINYGLTF_TYPE_VEC2 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
							texcoords = readBufferContents<glm::vec2>(accessor, view, buffer);
						}
					}
				}

				assert(!positions.empty() && !normals.empty() && !tangents.empty() && !texcoords.empty());

				// TODO(nemjit001): load index/vertex data into shared mesh buffers
			}
		}

		// Done!
		SPDLOG_INFO("Loaded mesh file {}", path);
		return std::make_shared<gfx::Mesh>();
	}
} // namespace assets
