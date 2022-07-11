/*+===================================================================
  File:      TEXTURE.H

  Summary:   Texture header file contains declaration of class
			 Texture used to abstract texture data.

  Classes:  Texture

  ?2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "pch.h"

#include "Graphics/Resource.h"
#include "Graphics/DescriptorAllocation.h"
#include "Texture/TextureUsage.h"

namespace pr
{
	class DescriptorAllocator;

	class Texture : public Resource
	{
	public:
		static BOOL CheckSrvSupport(D3D12_FORMAT_SUPPORT1 formatSupport) noexcept;
		static BOOL CheckRtvSupport(D3D12_FORMAT_SUPPORT1 formatSupport) noexcept;
		static BOOL CheckUavSupport(D3D12_FORMAT_SUPPORT1 formatSupport) noexcept;
		static BOOL CheckDsvSupport(D3D12_FORMAT_SUPPORT1 formatSupport) noexcept;
		static BOOL IsUavCompatibleFormat(DXGI_FORMAT format) noexcept;
		static BOOL IsSRgbFormat(DXGI_FORMAT format) noexcept;
		static BOOL IsBgrFormat(DXGI_FORMAT format) noexcept;
		static BOOL IsDepthFormat(DXGI_FORMAT format) noexcept;
		static DXGI_FORMAT GetTypelessFormat(DXGI_FORMAT format) noexcept;
			
	public:
		explicit Texture() noexcept;
		explicit Texture(_In_ eTextureUsage textureUsage) noexcept;
		explicit Texture(_In_ eTextureUsage textureUsage, _In_ const std::wstring& szName) noexcept;
		explicit Texture(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc) noexcept;
		explicit Texture(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pClearValue) noexcept;
		explicit Texture(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pClearValue, eTextureUsage textureUsage) noexcept;
		explicit Texture(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, const D3D12_CLEAR_VALUE* pClearValue, eTextureUsage textureUsage, const std::wstring& szName) noexcept;
		explicit Texture(ComPtr<ID3D12Resource> resource) noexcept;
		explicit Texture(ComPtr<ID3D12Resource> resource, eTextureUsage textureUsage) noexcept;
		explicit Texture(ComPtr<ID3D12Resource> resource, eTextureUsage textureUsage, const std::wstring& szName) noexcept;
		explicit Texture(const Texture& other) noexcept;
		explicit Texture(Texture&& other) noexcept;
		Texture& operator=(const Texture& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;
		virtual ~Texture() = default;

		// Should be called once to load the texture
		virtual HRESULT Initialize(_In_ ID3D12Device2* pDevice, DescriptorAllocator& allocator);

		eTextureUsage GetTextureUsage() const noexcept;
		void SetTextureUsage(eTextureUsage textureUsage) noexcept;

		HRESULT Resize(ID3D12Device2* pDevice, UINT uWidth, UINT uHeight, UINT uDepthOrArraySize) noexcept;
		HRESULT Resize(ID3D12Device2* pDevice, UINT uWidth, UINT uHeight) noexcept;

		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept override;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUavDesc) const noexcept override;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetRtv() const noexcept;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetDsv() const noexcept;

	private:
		HRESULT createSrv(_Out_ DescriptorAllocation& srv, ID3D12Device2* pDevice, DescriptorAllocator& allocator, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept;
		HRESULT createUav(_Out_ DescriptorAllocation& uav, ID3D12Device2* pDevice, DescriptorAllocator& allocator, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUavDesc) const noexcept;

	private:
		mutable std::unordered_map<size_t, DescriptorAllocation> m_Srvs;
		mutable std::unordered_map<size_t, DescriptorAllocation> m_Uavs;

		mutable std::mutex m_SrvMutex;
		mutable std::mutex m_UavMutex;

		DescriptorAllocation m_Rtv;
		DescriptorAllocation m_Dsv;

		eTextureUsage m_TextureUsage;
	};
}