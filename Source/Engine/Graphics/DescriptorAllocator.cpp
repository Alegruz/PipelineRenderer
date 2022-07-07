#include "pch.h"

#include "Graphics/DescriptorAllocator.h"
#include "Graphics/DescriptorAllocatorPage.h"
#include "Utility/Utility.h"

namespace pr
{
	DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, size_t numDescriptorsPerHeap) noexcept
		: m_HeapType(type)
		, m_NumDescriptorsPerHeap(numDescriptorsPerHeap)
	{
	}

	DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type) noexcept
		: DescriptorAllocator(type, 256)
	{
	}

	HRESULT DescriptorAllocator::Allocate(DescriptorAllocation& outAllocation, ID3D12Device2* pDevice, size_t numDescriptors) noexcept
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);
		HRESULT hr = S_OK;

		for (auto iter = m_AvailableHeaps.begin(); iter != m_AvailableHeaps.end(); ++iter)
		{
			std::shared_ptr<DescriptorAllocatorPage>& pAllocatorPage = m_HeapPool[*iter];

			pAllocatorPage->Allocate(outAllocation, numDescriptors);

			if (pAllocatorPage->GetNumFreeHandles() == 0)
			{
				iter = m_AvailableHeaps.erase(iter);
			}

			if (!outAllocation.IsNull())
			{
				break;
			}
		}

		if (outAllocation.IsNull())
		{
			m_NumDescriptorsPerHeap = std::max(m_NumDescriptorsPerHeap, numDescriptors);
			std::shared_ptr<DescriptorAllocatorPage> pNewPage; 
			hr = createAllocatorPage(pNewPage, pDevice);
			CHECK_AND_RETURN_HRESULT(hr, L"DescriptorAllocator::Allocate >> Creating allocator page");
			hr = pNewPage->Allocate(outAllocation, numDescriptors);
			CHECK_AND_RETURN_HRESULT(hr, L"DescriptorAllocator::Allocate >> Allocating new page");
		}

		return hr;
	}

	HRESULT DescriptorAllocator::Allocate(DescriptorAllocation& outAllocation, ID3D12Device2* pDevice) noexcept
	{
		return Allocate(outAllocation, pDevice, 1);
	}

	void DescriptorAllocator::ReleaseStaleDescriptors(size_t frameNumber) noexcept
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);

		for (size_t i = 0; i < m_HeapPool.size(); ++i)
		{
			std::shared_ptr<DescriptorAllocatorPage>& pPage = m_HeapPool[i];
			pPage->ReleaseStaleDescriptors(frameNumber);

			if (pPage->GetNumFreeHandles() > 0)
			{
				m_AvailableHeaps.insert(i);
			}
		}
	}

	HRESULT DescriptorAllocator::createAllocatorPage(std::shared_ptr<DescriptorAllocatorPage>& pOutPage, ID3D12Device2* pDevice) noexcept
	{
		HRESULT hr = S_OK;
		pOutPage = std::make_shared<DescriptorAllocatorPage>(m_HeapType, m_NumDescriptorsPerHeap);
		hr = pOutPage->Initialize(pDevice);
		CHECK_AND_RETURN_HRESULT(hr, L"DescriptorAllocator::createAllocatorPage >> Initializing descriptor allocator page");

		m_HeapPool.emplace_back(pOutPage);
		m_AvailableHeaps.insert(m_HeapPool.size() - 1);
		return hr;
	}
}