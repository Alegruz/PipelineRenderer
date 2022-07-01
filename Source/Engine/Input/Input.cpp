#include "pch.h"

#include "Input/Input.h"

namespace pr
{
	BOOL KeyboardInput::IsButtonPressed(BYTE button) const noexcept
	{	
		assert(VK_LBUTTON <= button && button <= VK_OEM_CLEAR);

		return (m_KeyboardInput[button / 64] & (1ull << ((button - 1ull) % 64)));
	}

	void KeyboardInput::ClearButton(BYTE button) noexcept
	{
		m_KeyboardInput[button / 64] &= ~(1ull << ((button - 1ull) % 64));
	}

	void KeyboardInput::SetButton(BYTE button) noexcept
	{
		m_KeyboardInput[button / 64] = (1ull << ((button - 1ull) % 64)) | m_KeyboardInput[button / 64];
	}

	void KeyboardInput::ToggleButton(BYTE button) noexcept
	{
		m_KeyboardInput[button / 64] ^= 1ull << ((button - 1ull) % 64);
	}

	void KeyboardInput::PrintKeyboardInputBinary() const noexcept
	{
		WCHAR szBinary[0xfe + 4] = { L'\0', };
		szBinary[0] = L'0';
		szBinary[1] = L'x';

		for (size_t i = 2; i < 0xfe + 2; ++i)
		{
			szBinary[i] = L'0' + ((m_KeyboardInput[i / 64] & (1ull << ((i % 64) - 1ull))) >> ((i % 64) - 1ull));
		}
		szBinary[0xfe + 2] = L'\0';
		szBinary[0xfe + 3] = L'\n';
		OutputDebugString(szBinary);
		OutputDebugString(L"\n");
	}
}
