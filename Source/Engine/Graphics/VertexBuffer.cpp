#include "pch.h"
#include "Graphics/VertexBuffer.h"

namespace pr
{
	VertexBuffer::VertexBuffer() noexcept
		: VertexBuffer(L"")
	{
	}

	VertexBuffer::VertexBuffer(const std::wstring& szName) noexcept
		: Buffer(szName)
		, m_NumVertices(0)
		, m_Stride(0)
		, m_View{}
	{
	}

	void VertexBuffer::CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept
	{
		m_NumVertices = numElements;
		m_Stride = elementSize;

		m_View.BufferLocation = m_pResource->GetGPUVirtualAddress();
		m_View.SizeInBytes = static_cast<UINT>(numElements * elementSize);
		m_View.StrideInBytes = static_cast<UINT>(m_Stride);
	}

	size_t VertexBuffer::GetNumVertices() const noexcept
	{
		return m_NumVertices;
	}

	size_t VertexBuffer::GetStride() const noexcept
	{
		return m_Stride;
	}

	D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetView() const noexcept
	{
		return m_View;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE VertexBuffer::GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept
	{
		assert(false);
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE VertexBuffer::GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept
	{
		assert(false);
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}
}