#include "files.hpp"

#include <filesystem>
#include <fstream>

#include "macros.hpp"

#if		GAME_PLATFORM_WINDOWS
#include <windows.h>
#endif	// GAME_PLATFORM_WINDOWS

namespace core::fs
{
	std::string getProgramDirectory()
	{
		std::string programPath{}; // Assume empty

#if		GAME_PLATFORM_WINDOWS
		char path[MAX_PATH]{};
		GetModuleFileNameA(nullptr /* this executable */, path, sizeof(path) - 1);
		programPath = std::string(path);
#endif	// GAME_PLATFORM_WINDOWS

		std::filesystem::path const programDir = std::filesystem::path(programPath).parent_path();
		return std::filesystem::weakly_canonical(programDir).string(); // Weakly canonical since we can assume the path exists because the program is running :)
	}

	std::string getFullAssetPath(std::string const& relative)
	{
		std::filesystem::path const programDir(getProgramDirectory());
		std::filesystem::path const absolutePath = programDir / relative;

		return std::filesystem::weakly_canonical(absolutePath).string(); // Weakly canonical since it is not our job to verify the file exists
	}

	std::vector<uint8_t> readBinaryFile(std::string const& path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file) {
			return {};
		}

		auto const& bytes = file.tellg();
		std::vector<uint8_t> contents(bytes, 0);

		file.seekg(std::ios::beg);
		file.read(reinterpret_cast<char*>(contents.data()), bytes);
		file.close();

		return contents;
	}
} // namespace core::fs
