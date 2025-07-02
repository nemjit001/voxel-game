#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace core::fs
{
	/// @brief Get the current program directory.
	/// @return 
	std::string getProgramDirectory();

	/// @brief Get a full asset path from a relative asset directory.
	/// @param relative 
	/// @return The full asset path based on the current program directory.
	std::string getFullAssetPath(std::string const& relative);

	/// @brief Read a binary file from disk.
	/// @param path File path.
	/// @return The binary file contents or an empty vector on failure.
	std::vector<uint8_t> readBinaryFile(std::string const& path);
} // namespace core::fs
