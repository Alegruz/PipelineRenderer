#pragma once

#include <cstdint>

namespace pr
{
	enum class eTextureUsage : uint8_t
	{
		ALBEDO,
		DIFFUSE = ALBEDO,
		HEIGHT_MAP,
		DEPTH = HEIGHT_MAP,
		NORMAL_MAP,
		RENDER_TARGET,
		COUNT,
	};
}