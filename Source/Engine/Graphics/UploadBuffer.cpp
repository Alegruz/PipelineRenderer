#include "pch.h"
#include "Graphics/UploadBuffer.h"
#include "Utility/Math.h"
#include "Utility/Utility.h"

namespace pr
{
	UploadBuffer::UploadBuffer() noexcept
		: UploadBuffer(_2MB)
	{
	}

	UploadBuffer::UploadBuffer(size_t pageSize) noexcept
		: m_PagePool()
		, m_AvailablePages()
		, m_pCurrentPage()
		, m_PageSize(pageSize)
	{
	}

	size_t UploadBuffer::GetPageSize() const
	{
		return m_PageSize;
	}

	HRESULT UploadBuffer::Allocate(Allocation& outAllocation, ID3D12Device2* pDevice, size_t sizeInBytes, size_t alignment) noexcept
	{
		HRESULT hr = S_OK;
		if (sizeInBytes > m_PageSize)
		{
			hr = E_INVALIDARG;
			CHECK_AND_RETURN_HRESULT(hr, L"UploadBuffer::Allocate >> Size exceeds page size");
		}

		if (!m_pCurrentPage || !m_pCurrentPage->HasSpace(sizeInBytes, alignment))
		{
			hr = requestPage(m_pCurrentPage, pDevice);
			CHECK_AND_RETURN_HRESULT(hr, L"UploadBuffer::Allocate >> Page request");
		}
		hr = m_pCurrentPage->Allocate(outAllocation, sizeInBytes, alignment);
		CHECK_AND_RETURN_HRESULT(hr, L"UploadBuffer::Allocate >> Page allocation");

		return hr;
	}

	void UploadBuffer::Reset()
	{
		m_pCurrentPage->Reset();
		m_AvailablePages = m_PagePool;

		for (std::shared_ptr<Page>& pPage : m_AvailablePages)
		{
			pPage->Reset();
		}
	}

	HRESULT UploadBuffer::requestPage(std::shared_ptr<Page>& pOutPage, ID3D12Device2* pDevice) noexcept
	{
		HRESULT hr = S_OK;

		if (!m_AvailablePages.empty())
		{
			pOutPage = m_AvailablePages.front();
			m_AvailablePages.pop_front();
		}
		else
		{
			pOutPage = std::make_shared<Page>(m_PageSize);
			hr = pOutPage->Initialize(pDevice);
			CHECK_AND_RETURN_HRESULT(hr, L"UploadBuffer::requestPage >> Initializing page");
			m_PagePool.push_back(pOutPage);
		}

		return hr;
	}

	UploadBuffer::Page::Page(size_t sizeInBytes) noexcept
		: m_pResource()
		, m_pCpuPtr(nullptr)
		, m_GpuPtr(D3D12_GPU_VIRTUAL_ADDRESS(0))
		, m_PageSize(sizeInBytes)
		, m_Offset(0)
	{
	}

	UploadBuffer::Page::~Page() noexcept
	{
		HRESULT hr = Destroy();
		AssertHresult(hr, L"UploadBuffer::Page::~Page >> Destroying page");
	}

	HRESULT UploadBuffer::Page::Initialize(ID3D12Device2* pDevice) noexcept
	{
		HRESULT hr = S_OK;

		const CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_PageSize);

		hr = pDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pResource)
		);
		CHECK_AND_RETURN_HRESULT(hr, L"UploadBuffer::Page::Initialize >> Create committed resource");

		m_GpuPtr = m_pResource->GetGPUVirtualAddress();
		hr = m_pResource->Map(0, nullptr, &m_pCpuPtr);
		CHECK_AND_RETURN_HRESULT(hr, L"UploadBuffer::Page::Initialize >> Mapping cpu data to gpu resource");

		return hr;
	}

	HRESULT UploadBuffer::Page::Destroy() noexcept
	{
		HRESULT hr = S_OK;

		m_pResource->Unmap(0, nullptr);
		m_pCpuPtr = nullptr;
		m_GpuPtr = D3D12_GPU_VIRTUAL_ADDRESS(0);

		return hr;
	}

	BOOL UploadBuffer::Page::HasSpace(size_t sizeInBytes, size_t alignment) const noexcept
	{
		size_t alignedSize = AlignUp(sizeInBytes, alignment);
		size_t alignedOffset = AlignUp(m_Offset, alignment);

		return alignedOffset + alignedSize <= m_PageSize;
	}

	HRESULT UploadBuffer::Page::Allocate(Allocation& outAllocation, size_t sizeInBytes, size_t alignment) noexcept
	{
		HRESULT hr = S_OK;
		if (!HasSpace(sizeInBytes, alignment))
		{
			hr = E_INVALIDARG;
			CHECK_AND_RETURN_HRESULT(hr, L"UploadBuffer::Page::Allocate >> Can't allocate space from page");
		}

		size_t alignedSize = AlignUp(sizeInBytes, alignment);
		m_Offset = AlignUp(m_Offset, alignment);

		outAllocation =
		{
			.pCpu = static_cast<UINT8*>(m_pCpuPtr) + m_Offset,
			.Gpu = m_GpuPtr + m_Offset,
		};

		m_Offset += alignedSize;

		return hr;
	}

	void UploadBuffer::Page::Reset() noexcept
	{
		m_Offset = 0;
	}
}