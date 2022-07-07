#pragma once

#include "pch.h"

namespace pr
{
	class RootSignature
	{
	public:
		explicit RootSignature() noexcept;
		explicit RootSignature(_In_ ID3D12Device2* pDevice, _In_ const D3D12_ROOT_SIGNATURE_DESC1& desc, _In_ D3D_ROOT_SIGNATURE_VERSION version) noexcept;
		explicit RootSignature(_In_ const RootSignature& other) = default;
		explicit RootSignature(_In_ RootSignature&& other) = default;
		RootSignature& operator=(_In_ const RootSignature& other) = default;
		RootSignature& operator=(_In_ RootSignature&& other) = default;
		virtual ~RootSignature() noexcept;

		void Destroy();

		ComPtr<ID3D12RootSignature>& GetRootSignature() noexcept;
		const ComPtr<ID3D12RootSignature>& GetRootSignature() const noexcept;
		const D3D12_ROOT_SIGNATURE_DESC1& GetRootSignatureDesc() const noexcept;
		UINT GetDescriptorTableBitMask(_In_ D3D12_DESCRIPTOR_HEAP_TYPE type) const noexcept;
		size_t GetNumDescriptors(_In_ size_t rootIndex) const noexcept;

		HRESULT SetRootSignatureDesc(_In_ ID3D12Device2* pDevice, _In_ const D3D12_ROOT_SIGNATURE_DESC1& desc, _In_ D3D_ROOT_SIGNATURE_VERSION version) noexcept;

		static constexpr const size_t NUM_ITEM = sizeof(UINT) * 8;
	private:
		D3D12_ROOT_SIGNATURE_DESC1 m_Desc;
		ComPtr<ID3D12RootSignature> m_RootSignature;
		size_t m_aNumDescriptorsPerTable[NUM_ITEM];
		UINT m_uSamplerTableBitMask;
		UINT m_uDescriptorTableBitMask;
	};
}