#include "pch.h"
#include "Graphics/IndexBuffer.h"

namespace pr
{
	IndexBuffer::IndexBuffer() noexcept
		: IndexBuffer(L"")
	{
	}

	IndexBuffer::IndexBuffer(const std::wstring& szName) noexcept
		: Buffer(szName)
		, m_NumIndices()
		, m_View{ .Format = DXGI_FORMAT_UNKNOWN }
	{
	}

	void IndexBuffer::CreateViews(ID3D12Device2* pDevice, size_t numElements, size_t elementSize) noexcept
	{
		assert(elementSize == 2 || elementSize == 4 && "Indices must be 16, or 32-bit integeres");

		m_NumIndices = numElements;

		m_View.BufferLocation = m_pResource->GetGPUVirtualAddress();
		m_View.SizeInBytes = static_cast<UINT>(numElements * elementSize);
		m_View.Format = (elementSize == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;;
	}

	size_t IndexBuffer::GetNumIndices() const noexcept
	{
		return m_NumIndices;
	}

	DXGI_FORMAT IndexBuffer::GetFormat() const noexcept
	{
		return m_View.Format;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBuffer::GetView() const noexcept
	{
		return m_View;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE IndexBuffer::GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc) const noexcept
	{
		assert(false);
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE IndexBuffer::GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc) const noexcept
	{
		assert(false);
		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}
}