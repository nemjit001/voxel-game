#pragma once

#include <string>

namespace fs
{
	/// @brief Get the current program directory.
	/// @return 
	std::string getProgramDirectory();

	/// @brief Get a full asset path from a relative asset directory.
	/// @param relative 
	/// @return The full asset path based on the current program directory.
	std::string getFullAssetPath(std::string const& relative);
} // namespace fs
