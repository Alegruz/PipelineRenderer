#pragma once

#include "pch.h"

namespace pr
{
	/*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
	  Class:    MouseRelativeMovement
	  Summary:  Data structure that stores mouse relative movement data
	S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
	struct MouseInput
	{
		explicit constexpr MouseInput() noexcept
			: lRelativeX(0)
			, lRelativeY(0)
			, hZDelta(0)
		{
		}

		LONG lRelativeX;
		LONG lRelativeY;
		SHORT hZDelta;
	};

	class KeyboardInput final
	{
	public:
		explicit constexpr KeyboardInput() noexcept = default;
		explicit constexpr KeyboardInput(const KeyboardInput& other) noexcept = default;
		explicit constexpr KeyboardInput(KeyboardInput&& other) noexcept = default;
		constexpr KeyboardInput& operator=(const KeyboardInput& other) noexcept = default;
		constexpr KeyboardInput& operator=(KeyboardInput&& other) noexcept = default;
		~KeyboardInput() noexcept = default;

		BOOL IsButtonPressed(BYTE button) const noexcept;
		BOOL IsButtonPressing(BYTE button) const noexcept;
		
		void ClearButton(BYTE button) noexcept;
		void ProcessedButton(BYTE button) noexcept;
		void SetButton(BYTE button) noexcept;
		//void ToggleButton(BYTE button) noexcept;

		void PrintKeyboardInputBinary() const noexcept;

	private:
		UINT64 m_KeyboardInputPersistent[4];
		UINT64 m_KeyboardInput[4];
	};
	static_assert(sizeof(KeyboardInput) == 64);
}