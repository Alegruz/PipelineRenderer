#pragma once

namespace pr
{
	inline constexpr HRESULT CheckHresult(HRESULT hr, LPCWSTR pszMessage) noexcept
	{
		if (FAILED(hr))
		{
			OutputDebugString(pszMessage);
			WCHAR szDebugMsg[64] = { L'\0', };
			swprintf_s(szDebugMsg, L" HRESULT: 0x%08x\n", hr);
			OutputDebugString(szDebugMsg);
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