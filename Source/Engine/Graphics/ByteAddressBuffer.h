#pragma once

#include "Graphics/Buffer.h"
#include "Graphics/DescriptorAllocation.h"

namespace pr
{
	class DescriptorAllocator;

	class ByteAddressBuffer : public Buffer
	{
	public:
		ByteAddressBuffer() = delete;
		explicit ByteAddressBuffer(ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept;
		explicit ByteAddressBuffer(const std::wstring& szName, ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept;
		explicit ByteAddressBuffer(
			ID3D12Device2* pDevice,
			const D3D12_RESOURCE_DESC& desc,
			size_t numElements,
			size_t elementSize,
			const std::wstring& szName
		) noexcept;
		explicit ByteAddressBuffer(
			ID3D12Device2* pDevice,
			const D3D12_RESOURCE_DESC& desc,
			size_t numElements,
			size_t elementSize
		) noexcept;
		explicit ByteAddressBuffer(const ByteAddressBuffer& other) noexcept = default;
		explicit ByteAddressBuffer(ByteAddressBuffer&& other) noexcept = default;
		ByteAddressBuffer& operator=(const ByteAddressBuffer& other) noexcept = default;
		ByteAddressBuffer& operator=(ByteAddressBuffer&& other) noexcept = default;
		virtual ~ByteAddressBuffer() noexcept = default;

		virtual void CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept override;

		size_t GetBufferSize() const noexcept;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept override;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept override;

	private:
		size_t m_BufferSize;
		DescriptorAllocation m_Srv;
		DescriptorAllocation m_Uav;
	};
}