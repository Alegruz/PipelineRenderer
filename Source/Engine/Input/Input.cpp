#include "pch.h"

#include "Input/Input.h"

namespace pr
{
	constexpr BOOL KeyboardInput::IsButtonPressed(BYTE Button) noexcept
	{
		return (m_KeyboardInput & (0x01 << (Button - 0x01)));
	}

	constexpr void KeyboardInput::ClearButton(BYTE Button) noexcept
	{
		m_KeyboardInput &= ~(0x01 << (Button - 0x01));
	}

	constexpr void KeyboardInput::SetButton(BYTE Button) noexcept
	{
		m_KeyboardInput = (0x01 << Button) | m_KeyboardInput;
	}

	constexpr void KeyboardInput::ToggleButton(BYTE Button) noexcept
	{
		m_KeyboardInput ^= 0x01 << (Button - 0x01);
	}
}
