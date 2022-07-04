#include "pch.h"

#include "Event/EventManager.h"
#include "Utility/Utility.h"

namespace pr
{
	HRESULT EventManager::GetEvent(EventMessage& event) noexcept
	{
		HRESULT hr = S_OK;

		if (GetSize() == 0)
		{
			hr = E_FAIL;
			CHECK_AND_RETURN_HRESULT(hr, L"EventManager::GetEvent >> No Events");
		}

		event = m_aEvents[m_NextEventIndex - 1];
		--m_NextEventIndex;

		return hr;
	}

	EventMessage* EventManager::GetEvents() noexcept
	{
		m_NextEventIndex = 0;

		return m_aEvents;
	}

	size_t EventManager::GetSize() const noexcept
	{
		return m_NextEventIndex;
	}
	
	HRESULT EventManager::AddEvent(EventMessage event) noexcept
	{
		HRESULT hr = S_OK;

		if (m_NextEventIndex >= MAX_NUM_EVENTS)
		{
			hr = E_FAIL;
			CHECK_AND_RETURN_HRESULT(hr, L"EventManager::AddEvent >> Event buffer full");
		}

		m_aEvents[m_NextEventIndex++] = event;

		return hr;
	}
}