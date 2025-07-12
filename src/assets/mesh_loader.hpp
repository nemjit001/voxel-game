#pragma once

#include <memory>
#include <string>

#include "rendering/mesh.hpp"

namespace assets
{
	/// @brief The MeshLoader class handles mesh file I/O.
	/// NOTE: only supports glTF2.0 binary files.
	class MeshLoader
	{
	public:
		/// @brief Load a mesh from a file on disk.
		/// @param path File path to load mesh from.
		/// @return A mesh pointer or nullptr on error.
		std::shared_ptr<gfx::Mesh> load(std::string const& path);
	};
} // namespace assets
