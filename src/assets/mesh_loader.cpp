#include "mesh_loader.hpp"

#include <unordered_map>
#include <spdlog/spdlog.h>
#include <tiny_gltf.h>

namespace assets
{
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

				// TODO(nemjit001): load index data into vector
				//

				for (auto const& attr : primitive.attributes)
				{
					SPDLOG_INFO("Attribute {}:{}", attr.first, attr.second);
					auto const& accessor = model.accessors[attr.second];

					// TODO(nemjit001): Load attribute data into vector
					//
				}
			}
		}

		// Done!
		SPDLOG_INFO("Loaded mesh file {}", path);
		return std::make_shared<gfx::Mesh>();
	}
} // namespace assets
