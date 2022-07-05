#pragma once

#include <comdef.h>

namespace pr
{
	inline HRESULT CheckHresult(HRESULT hr, LPCWSTR pszMessage) noexcept
	{
		if (FAILED(hr))
		{
			OutputDebugString(pszMessage);
			static WCHAR szDebugMsg[256] = { L'\0', };
			_com_error err(hr);
			LPCTSTR errMsg = err.ErrorMessage();
			swprintf_s(szDebugMsg, L" %s HRESULT: 0x%08x\n", errMsg, hr);
			OutputDebugString(szDebugMsg);
			MessageBox(nullptr, szDebugMsg, LPSZ_ENGINE_TITLE, MB_OK | MB_ICONERROR);
		}

		return hr;
	}
	
	inline constexpr void AssertHresult(HRESULT hr, LPCWSTR pszMessage) noexcept
	{
		if (FAILED(hr))
		{
			OutputDebugString(pszMessage);
			WCHAR szDebugMsg[64] = { L'\0', };
			swprintf_s(szDebugMsg, L" HRESULT: 0x%08x\n", hr);
			OutputDebugString(szDebugMsg);
			assert(false);
		}
	}

#define CHECK_AND_RETURN_HRESULT(hr, pszMessage)	\
	pr::CheckHresult(hr, pszMessage);	\
	if (FAILED(hr))					\
	{								\
		return hr;					\
	}
}