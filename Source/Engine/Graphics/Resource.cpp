#include "pch.h"

#include "Graphics/Resource.h"
#include "Graphics/ResourceStateTracker.h"
#include "Utility/Utility.h"

namespace pr
{
	Resource::Resource() noexcept
		: Resource(L"")
	{
	}

	Resource::Resource(const std::wstring& szName) noexcept
		: m_pResource()
		, m_pClearValue()
		, m_szResourceName(szName)
	{
	}

	Resource::Resource(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pClearValue, const std::wstring& szName) noexcept
	{
		if (pClearValue)
		{
			m_pClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*pClearValue);
		}

		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

		HRESULT hr = pDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			m_pClearValue.get(),
			IID_PPV_ARGS(&m_pResource)
		);
		AssertHresult(hr, L"Resource Constructor >> Creating committed resource");

		ResourceStateTracker::AddGlobalResourceState(m_pResource.Get(), D3D12_RESOURCE_STATE_COMMON);

		SetName(szName);
	}

	Resource::Resource(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pClearValue) noexcept
		: Resource(pDevice, desc, pClearValue, L"")
	{
	}

	Resource::Resource(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc) noexcept
		: Resource(pDevice, desc, nullptr, L"")
	{
	}

	Resource::Resource(ComPtr<ID3D12Resource>& pResource, const std::wstring& szName) noexcept
		: m_pResource(pResource)
		, m_pClearValue()
	{
		SetName(szName);
	}

	Resource::Resource(ComPtr<ID3D12Resource>& pResource) noexcept
		: Resource(pResource, L"")
	{
	}

	Resource::Resource(const Resource& other) noexcept
		: m_pResource(other.m_pResource)
		, m_pClearValue(std::make_unique<D3D12_CLEAR_VALUE>(*other.m_pClearValue))
		, m_szResourceName(other.m_szResourceName)
	{
	}

	Resource& Resource::operator=(const Resource& other) noexcept
	{
		if (this != &other)
		{
			m_pResource = other.m_pResource;
			m_szResourceName = other.m_szResourceName;

			if (other.m_pClearValue)
			{
				m_pClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*other.m_pClearValue);
			}
		}

		return *this;
	}

	Resource& Resource::operator=(Resource&& other) noexcept
	{
		if (this != &other)
		{
			m_pResource = other.m_pResource;
			m_pClearValue = std::move(other.m_pClearValue);
			m_szResourceName = other.m_szResourceName;

			other.m_pResource.Reset();
			other.m_szResourceName.clear();
		}

		return *this;
	}

	BOOL Resource::IsValid() const noexcept
	{
		return (m_pResource != nullptr);
	}

	ComPtr<ID3D12Resource>& Resource::GetD3D12Resource() noexcept
	{
		return m_pResource;
	}

	const ComPtr<ID3D12Resource>& Resource::GetD3D12Resource() const noexcept
	{
		return m_pResource;
	}

	D3D12_RESOURCE_DESC Resource::GetD3D12ResourceDesc() const noexcept
	{
		D3D12_RESOURCE_DESC desc = {};
		if (m_pResource)
		{
			desc = m_pResource->GetDesc();
		}

		return desc;
	}

	void Resource::SetD3D12Resource(ComPtr<ID3D12Resource>& pResource, const D3D12_CLEAR_VALUE* pClearValue) noexcept
	{
		m_pResource = pResource;
		if (m_pClearValue)
		{
			m_pClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*pClearValue);
		}
		else
		{
			m_pClearValue.reset();
		}
		SetName(m_szResourceName);
	}

	void Resource::SetD3D12Resource(ComPtr<ID3D12Resource>& pResource) noexcept
	{
		SetD3D12Resource(pResource, nullptr);
	}
	
	D3D12_CPU_DESCRIPTOR_HANDLE Resource::GetSrv() const noexcept
	{
		return GetSrv(nullptr);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Resource::GetUav() const noexcept
	{
		return GetUav(nullptr);
	}
	
	void Resource::SetName(const std::wstring& szName) noexcept
	{
		m_szResourceName = szName;
		if (m_pResource && !m_szResourceName.empty())
		{
			m_pResource->SetName(m_szResourceName.c_str());
		}
	}

	void Resource::Reset() noexcept
	{
		m_pResource.Reset();
		m_pClearValue.reset();
	}
}