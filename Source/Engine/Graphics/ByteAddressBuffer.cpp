#include "pch.h"
#include "Graphics/ByteAddressBuffer.h"
#include "Graphics/DescriptorAllocator.h"
#include "Utility/Math.h"
#include "Utility/Utility.h"

namespace pr
{
	ByteAddressBuffer::ByteAddressBuffer(const std::wstring& szName, ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept
		: Buffer(szName)
		, m_BufferSize()
		, m_Srv()
		, m_Uav()
	{
		HRESULT hr = allocator.Allocate(m_Srv, pDevice);
		AssertHresult(hr, L"ByteAddressBuffer Constructor >> Allocating shader resource view");

		hr = allocator.Allocate(m_Uav, pDevice);
		AssertHresult(hr, L"ByteAddressBuffer Constructor >> Allocating unordered access view");
	}
	
	ByteAddressBuffer::ByteAddressBuffer(ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept
		: ByteAddressBuffer(L"", pDevice, allocator)
	{
	}

	ByteAddressBuffer::ByteAddressBuffer(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, size_t numElements, size_t elementSize, const std::wstring& szName) noexcept
		: Buffer(pDevice, desc, numElements, elementSize, szName)
		, m_BufferSize()
		, m_Srv()
		, m_Uav()
	{
	}

	ByteAddressBuffer::ByteAddressBuffer(ID3D12Device2* pDevice, const D3D12_RESOURCE_DESC& desc, size_t numElements, size_t elementSize) noexcept
		: ByteAddressBuffer(pDevice, desc, numElements, elementSize, L"")
	{
	}

	void ByteAddressBuffer::CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept
	{
		m_BufferSize = AlignUp(numElements * elementSize, 4);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
		{
			.Format = DXGI_FORMAT_R32_TYPELESS,
			.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Buffer = D3D12_BUFFER_SRV{ .NumElements = static_cast<UINT>(m_BufferSize / 4), .Flags = D3D12_BUFFER_SRV_FLAG_RAW },
		};
		pDevice->CreateShaderResourceView(m_pResource.Get(), &srvDesc, m_Srv.GetDescriptorHandle());

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc =
		{
			.Format = DXGI_FORMAT_R32_TYPELESS,
			.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
			.Buffer = D3D12_BUFFER_UAV{ .NumElements = static_cast<UINT>(m_BufferSize / 4), .Flags = D3D12_BUFFER_UAV_FLAG_RAW },
		};
		pDevice->CreateUnorderedAccessView(m_pResource.Get(), nullptr, &uavDesc, m_Uav.GetDescriptorHandle());
	}

	size_t ByteAddressBuffer::GetBufferSize() const noexcept
	{
		return m_BufferSize;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ByteAddressBuffer::GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept
	{
		UNREFERENCED_PARAMETER(pSrvDesc);
		return m_Srv.GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ByteAddressBuffer::GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept
	{
		UNREFERENCED_PARAMETER(pSrvDesc);
		return m_Uav.GetDescriptorHandle();
	}
}