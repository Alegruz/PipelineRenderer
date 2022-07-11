#include "pch.h"
#include "Graphics/ConstantBuffer.h"
#include "Graphics/DescriptorAllocator.h"
#include "Utility/Math.h"
#include "Utility/Utility.h"

namespace pr
{
	ConstantBuffer::ConstantBuffer(ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept
		: ConstantBuffer(L"", pDevice, allocator)
	{
	}

	ConstantBuffer::ConstantBuffer(const std::wstring& szName, ID3D12Device2* pDevice, DescriptorAllocator& allocator) noexcept
		: Buffer(szName)
		, m_SizeInBytes()
		, m_Cbv()
	{
		HRESULT hr = allocator.Allocate(m_Cbv, pDevice);
		AssertHresult(hr, L"ConstantBuffer Constructor >> Allocating constant buffer view");
	}

	void ConstantBuffer::CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept
	{
		m_SizeInBytes = numElements * elementSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC desc
		{
			.BufferLocation = m_pResource->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<UINT>(AlignUp(m_SizeInBytes, 16)),
		};

		pDevice->CreateConstantBufferView(&desc, m_Cbv.GetDescriptorHandle());
	}

	size_t ConstantBuffer::GetSizeInBytes() const noexcept
	{
		return m_SizeInBytes;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetView() const noexcept
	{
		return m_Cbv.GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept
	{
		assert(false);
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept
	{
		assert(false);
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}
}