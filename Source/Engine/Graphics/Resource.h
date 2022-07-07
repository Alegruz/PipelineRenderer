#pragma once

#include "pch.h"

namespace pr
{
	class Resource
	{
	public:
		explicit Resource() noexcept;
		explicit Resource(const std::wstring& szName) noexcept;
		explicit Resource(
			ID3D12Device2* pDevice,
			const D3D12_RESOURCE_DESC& desc,
			const D3D12_CLEAR_VALUE* pClearValue,
			const std::wstring& szName
		) noexcept;
		explicit Resource(
			ID3D12Device2* pDevice,
			const D3D12_RESOURCE_DESC& desc,
			const D3D12_CLEAR_VALUE* pClearValue
		) noexcept;
		explicit Resource(
			ID3D12Device2* pDevice,
			const D3D12_RESOURCE_DESC& desc
		) noexcept;
		explicit Resource(
			ComPtr<ID3D12Resource>& pResource,
			const std::wstring& szName
		) noexcept;
		explicit Resource(
			ComPtr<ID3D12Resource>& pResource
		) noexcept;
		explicit Resource(const Resource& other) noexcept;
		explicit Resource(Resource&& other) noexcept = default;
		Resource& operator=(const Resource& other) noexcept;
		Resource& operator=(Resource&& other) noexcept;
		virtual ~Resource() noexcept = default;

		BOOL IsValid() const noexcept;
		ComPtr<ID3D12Resource>& GetD3D12Resource() noexcept;
		const ComPtr<ID3D12Resource>& GetD3D12Resource() const noexcept;
		D3D12_RESOURCE_DESC GetD3D12ResourceDesc() const noexcept;
		virtual void SetD3D12Resource(ComPtr<ID3D12Resource>& pResource, const D3D12_CLEAR_VALUE* pClearValue) noexcept;
		virtual void SetD3D12Resource(ComPtr<ID3D12Resource>& pResource) noexcept;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept  = 0;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetSrv() const noexcept;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept  = 0;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUav() const noexcept;
		void SetName(const std::wstring& szName) noexcept;
		virtual void Reset() noexcept;

	protected:
		ComPtr<ID3D12Resource> m_pResource;
		std::unique_ptr<D3D12_CLEAR_VALUE> m_pClearValue;
		std::wstring m_szResourceName;
	};
}