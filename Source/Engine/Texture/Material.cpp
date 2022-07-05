#include "pch.h"

#include "Texture/Material.h"

namespace pr
{
	Material::Material(_In_ std::wstring szName)
		: pDiffuse()
		, pSpecularExponent()
		, pNormal()
		, m_szName(szName)
	{
	}

	HRESULT Material::Initialize(_In_ ID3D12Device* pDevice, _In_ ID3D12GraphicsCommandList2* pCommandList)
	{
		HRESULT hr = S_OK;

		if (pDiffuse)
		{
			hr = pDiffuse->Initialize(pDevice, pCommandList);
			if (FAILED(hr))
			{
				return hr;
			}
		}

		if (pSpecularExponent)
		{
			hr = pSpecularExponent->Initialize(pDevice, pCommandList);
			if (FAILED(hr))
			{
				return hr;
			}
		}

		if (pNormal)
		{
			hr = pNormal->Initialize(pDevice, pCommandList);
			if (FAILED(hr))
			{
				return hr;
			}
		}

		return hr;
	}

	std::wstring Material::GetName() const
	{
		return m_szName;
	}
}