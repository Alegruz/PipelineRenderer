#pragma once

#include "pch.h"

namespace pr
{
	class KeyboardInput final
	{
	public:
		explicit constexpr KeyboardInput() noexcept = default;
		explicit constexpr KeyboardInput(const KeyboardInput& Other) noexcept = default;
		explicit constexpr KeyboardInput(KeyboardInput&& Other) noexcept = default;
		constexpr KeyboardInput& operator=(const KeyboardInput& Other) noexcept = default;
		constexpr KeyboardInput& operator=(KeyboardInput&& Other) noexcept = default;
		~KeyboardInput() noexcept = default;

		constexpr BOOL IsButtonPressed(BYTE Button) noexcept;
		
		constexpr void ClearButton(BYTE Button) noexcept;
		constexpr void SetButton(BYTE Button) noexcept;
		constexpr void ToggleButton(BYTE Button) noexcept;

	private:
		BYTE m_KeyboardInput = 0x00;
	};
}