#pragma once

#include <cassert>

namespace core
{
	/// @brief Align a memory address to a byte boundary.
	/// @param address Address to align.
	/// @param alignment Byte alignment boundary, MUST be multiple of 2.
	/// @return The aligned memory address.
	constexpr size_t alignAddress(size_t address, size_t alignment)
	{
		assert(alignment % 2 == 0 && "Alignment of memory address must be multiple of 2");
		return (address + (alignment - 1)) & ~(alignment - 1);
	}
} // namespace core
