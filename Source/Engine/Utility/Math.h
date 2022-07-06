#pragma once

namespace pr
{
	inline constexpr size_t AlignUpWithMask(size_t value, size_t mask) noexcept
	{
		return value & ~mask;
	}

	inline constexpr size_t AlignUp(size_t value, size_t alignment) noexcept
	{
		return AlignUpWithMask(value, alignment - 1);
	}
}
