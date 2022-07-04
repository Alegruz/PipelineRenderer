#pragma once

#include "pch.h"

#include "Event/Event.h"

namespace pr
{
	struct EventMessage
	{
		eEventType Type;
		UINT64 uParam0;
		UINT64 uParam1;
		LPVOID pParam0;
	};
	static_assert(sizeof(EventMessage) == 32);

	class EventManager final
	{
	public:
		explicit EventManager() noexcept = default;
		explicit EventManager(_In_ const EventManager& other) noexcept = delete;
		explicit EventManager(_In_ EventManager&& other) noexcept = delete;
		EventManager& operator=(_In_ const EventManager& other) noexcept = delete;
		EventManager& operator=(_In_ EventManager&& other) noexcept = delete;
		~EventManager() noexcept = default;

		HRESULT GetEvent(_Out_ EventMessage& event) noexcept;
		EventMessage* GetEvents() noexcept;
		size_t GetSize() const noexcept;

		HRESULT AddEvent(_In_ EventMessage event) noexcept;

		static constexpr const size_t MAX_NUM_EVENTS = 256;
	private:
		EventMessage m_aEvents[MAX_NUM_EVENTS];
		size_t m_NextEventIndex;
	};
}