#include "pch.h"
#include "Graphics/StructuredBuffer.h"
#include "Graphics/DescriptorAllocator.h"
#include "Utility/Math.h"
#include "Utility/Utility.h"

namespace pr
{
	StructuredBuffer::StructuredBuffer(ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept
		: StructuredBuffer(L"", pDevice, allocator)
	{
	}

	StructuredBuffer::StructuredBuffer(const std::wstring& szName, ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept
		: Buffer(szName)
		, m_NumElements()
		, m_ElementSize()
		, m_Srv()
		, m_Uav()
		, m_CounterBuffer(pDevice, CD3DX12_RESOURCE_DESC::Buffer(4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), 1, 4, szName + L" Counter")
	{
		HRESULT hr = allocator.Allocate(m_Srv, pDevice);
		AssertHresult(hr, L"StructuredBuffer Constructor >> Allocating shader resource view");

		hr = allocator.Allocate(m_Uav, pDevice);
		AssertHresult(hr, L"StructuredBuffer Constructor >> Allocating unordered access view");
	}

	StructuredBuffer::StructuredBuffer(ID3D12Device2* pDevice, DescriptorAllocator& allocator, const D3D12_RESOURCE_DESC& desc, size_t numElements, size_t elementSize, const std::wstring& szName) noexcept
		: Buffer(pDevice, desc, numElements, elementSize,szName)
		, m_NumElements(numElements)
		, m_ElementSize(elementSize)
		, m_Srv()
		, m_Uav()
		, m_CounterBuffer(pDevice, CD3DX12_RESOURCE_DESC::Buffer(4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), 1, 4, szName + L" Counter")
	{
		HRESULT hr = allocator.Allocate(m_Srv, pDevice);
		AssertHresult(hr, L"StructuredBuffer Constructor >> Allocating shader resource view");

		hr = allocator.Allocate(m_Uav, pDevice);
		AssertHresult(hr, L"StructuredBuffer Constructor >> Allocating unordered access view");
	}

	StructuredBuffer::StructuredBuffer(ID3D12Device2* pDevice, DescriptorAllocator& allocator, const D3D12_RESOURCE_DESC& desc, size_t numElements, size_t elementSize) noexcept
		: StructuredBuffer(pDevice, allocator, desc, numElements, elementSize, L"")
	{
	}

	void StructuredBuffer::CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept
	{
		m_NumElements = numElements;
		m_ElementSize = elementSize;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
		{
			.Format = DXGI_FORMAT_UNKNOWN,
			.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Buffer = D3D12_BUFFER_SRV{ .NumElements = static_cast<UINT>(numElements), .StructureByteStride = static_cast<UINT>(m_ElementSize), .Flags = D3D12_BUFFER_SRV_FLAG_NONE },
		};
		pDevice->CreateShaderResourceView(m_pResource.Get(), &srvDesc, m_Srv.GetDescriptorHandle());

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc =
		{
			.Format = DXGI_FORMAT_UNKNOWN,
			.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
			.Buffer = D3D12_BUFFER_UAV{ 
				.NumElements = static_cast<UINT>(numElements), 
				.StructureByteStride = static_cast<UINT>(m_ElementSize), 
				.CounterOffsetInBytes = 0,
				.Flags = D3D12_BUFFER_UAV_FLAG_NONE,
				},
		};
		pDevice->CreateUnorderedAccessView(m_pResource.Get(), m_CounterBuffer.GetD3D12Resource().Get(), & uavDesc, m_Uav.GetDescriptorHandle());
	}

	size_t StructuredBuffer::GetNumElements() const noexcept
	{
		return m_NumElements;
	}

	size_t StructuredBuffer::GetElementSize() const noexcept
	{
		return m_ElementSize;
	}

	const ByteAddressBuffer& StructuredBuffer::GetCounterBuffer() const noexcept
	{
		return m_CounterBuffer;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE StructuredBuffer::GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept
	{
		return m_Srv.GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE StructuredBuffer::GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept
	{
		return m_Uav.GetDescriptorHandle();
	}
}