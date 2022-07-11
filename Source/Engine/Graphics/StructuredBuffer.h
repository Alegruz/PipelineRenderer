#pragma once

#include "Graphics/Buffer.h"
#include "Graphics/ByteAddressBuffer.h"
#include "Graphics/DescriptorAllocation.h"

namespace pr
{
	class DescriptorAllocator;

	class StructuredBuffer : public Buffer
	{
	public:
		StructuredBuffer() = delete;
		explicit StructuredBuffer(ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept;
		explicit StructuredBuffer(const std::wstring& szName, ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept;
		explicit StructuredBuffer(
			ID3D12Device2* pDevice,
			DescriptorAllocator& allocator,
			const D3D12_RESOURCE_DESC& desc,
			size_t numElements,
			size_t elementSize,
			const std::wstring& szName
		) noexcept;
		explicit StructuredBuffer(
			ID3D12Device2* pDevice,
			DescriptorAllocator& allocator,
			const D3D12_RESOURCE_DESC& desc,
			size_t numElements,
			size_t elementSize
		) noexcept;
		explicit StructuredBuffer(const StructuredBuffer& other) noexcept = default;
		explicit StructuredBuffer(StructuredBuffer&& other) noexcept = default;
		StructuredBuffer& operator=(const StructuredBuffer& other) noexcept = default;
		StructuredBuffer& operator=(StructuredBuffer&& other) noexcept = default;
		virtual ~StructuredBuffer() noexcept = default;

		virtual void CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept override;

		size_t GetNumElements() const noexcept;
		size_t GetElementSize() const noexcept;
		const ByteAddressBuffer& GetCounterBuffer() const noexcept;

		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept override;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept override;

	private:
		size_t m_NumElements;
		size_t m_ElementSize;

		DescriptorAllocation m_Srv;
		DescriptorAllocation m_Uav;

		ByteAddressBuffer m_CounterBuffer;
	};
}