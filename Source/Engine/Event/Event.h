#pragma once

#include <cstdint>

namespace pr
{
	enum class eEventType : uint8_t
	{
		RESIZE,
		FULLSCREEN,
		COUNT,
	};
}