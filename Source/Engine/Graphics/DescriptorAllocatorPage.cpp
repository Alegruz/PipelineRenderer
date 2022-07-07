#include "pch.h"

#include "Graphics/DescriptorAllocatorPage.h"
#include "Utility/Utility.h"

namespace pr
{
	DescriptorAllocatorPage::FreeBlockInfo::FreeBlockInfo(SizeType size) noexcept
		: Size(size)
		, FreeListBySizeIter()
	{
	}

	DescriptorAllocatorPage::StaleDescriptorInfo::StaleDescriptorInfo(OffsetType offset, SizeType size, size_t frameNumber) noexcept
		: Offset(offset)
		, Size(size)
		, FrameNumber(frameNumber)
	{
	}

	DescriptorAllocatorPage::DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, size_t numDescriptors) noexcept
		: m_FreeListByOffset()
		, m_FreeListBySize()
		, m_StaleDescriptors()
		, m_pDescriptorHeap()
		, m_HeapType(type)
		, m_hBaseDescriptor()
		, m_DescriptorHandleIncrementSize()
		, m_NumDescriptorsInHeap(numDescriptors)
		, m_NumFreeHandles()
		, m_AllocationMutex()
	{
	}

	HRESULT DescriptorAllocatorPage::Initialize(ID3D12Device2* pDevice) noexcept
	{
		HRESULT hr = S_OK;

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
		{
			.Type = m_HeapType,
			.NumDescriptors = m_NumDescriptorsInHeap,
		};

		hr = pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pDescriptorHeap));
		CHECK_AND_RETURN_HRESULT(hr, L"DescriptorAllocatorPage::Initialize >> Creating descriptor heap");

		m_hBaseDescriptor = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_DescriptorHandleIncrementSize = pDevice->GetDescriptorHandleIncrementSize(m_HeapType);
		m_NumFreeHandles = m_NumDescriptorsInHeap;

		addNewBlock(0, m_NumFreeHandles);

		return hr;
	}
	D3D12_DESCRIPTOR_HEAP_TYPE DescriptorAllocatorPage::GetHeapType() const noexcept
	{
		return m_HeapType;
	}

	BOOL DescriptorAllocatorPage::HasSpace(size_t numDescriptors) const noexcept
	{
		return m_FreeListBySize.lower_bound(numDescriptors) != m_FreeListBySize.end();
	}

	size_t DescriptorAllocatorPage::GetNumFreeHandles() const noexcept
	{
		return m_NumFreeHandles;
	}

	HRESULT DescriptorAllocatorPage::Allocate(DescriptorAllocation& outAllocation, size_t numDescriptors) noexcept
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);
		HRESULT hr = S_OK;

		if (numDescriptors > m_NumFreeHandles)
		{
			outAllocation = DescriptorAllocation();
			return hr;
		}

		auto smallestBlockIter = m_FreeListBySize.lower_bound(numDescriptors);
		if (smallestBlockIter == m_FreeListBySize.end())
		{
			outAllocation = DescriptorAllocation();
			return hr;
		}

		SizeType blockSize = smallestBlockIter->first;
		auto offsetIter = smallestBlockIter->second;
		OffsetType offset = offsetIter->first;

		m_FreeListBySize.erase(smallestBlockIter);
		m_FreeListByOffset.erase(offsetIter);

		OffsetType newOffset = offset + numDescriptors;
		SizeType newSize = blockSize - numDescriptors;

		if (newSize > 0)
		{
			addNewBlock(newOffset, newSize);
		}

		m_NumFreeHandles -= numDescriptors;

		outAllocation = DescriptorAllocation(
			CD3DX12_CPU_DESCRIPTOR_HANDLE(m_hBaseDescriptor, offset, m_DescriptorHandleIncrementSize),
			numDescriptors,
			m_DescriptorHandleIncrementSize,
			shared_from_this()
		);

		return hr;
	}

	void DescriptorAllocatorPage::Free(DescriptorAllocation&& descriptor, size_t frameNumber) noexcept
	{
		OffsetType offset = computeOffset(descriptor.GetDescriptorHandle());

		std::lock_guard<std::mutex> lock(m_AllocationMutex);

		m_StaleDescriptors.emplace(offset, descriptor.GetNumHandles(), frameNumber);
	}

	void DescriptorAllocatorPage::ReleaseStaleDescriptors(size_t frameNumber)
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);

		while (!m_StaleDescriptors.empty() && m_StaleDescriptors.front().FrameNumber <= frameNumber)
		{
			StaleDescriptorInfo& staleDescriptor = m_StaleDescriptors.front();

			OffsetType offset = staleDescriptor.Offset;
			SizeType numDescriptors = staleDescriptor.Size;

			freeBlock(offset, numDescriptors);

			m_StaleDescriptors.pop();
		}
	}

	size_t DescriptorAllocatorPage::computeOffset(D3D12_CPU_DESCRIPTOR_HANDLE hHandle) noexcept
	{
		return (hHandle.ptr - m_hBaseDescriptor.ptr) / m_DescriptorHandleIncrementSize;
	}

	void DescriptorAllocatorPage::addNewBlock(size_t offset, size_t numDescriptors) noexcept
	{
		auto offsetIter = m_FreeListByOffset.emplace(offset, numDescriptors);
		auto sizeIter = m_FreeListBySize.emplace(numDescriptors, offsetIter.first);
		offsetIter.first->second.FreeListBySizeIter = sizeIter;
	}
	
	void DescriptorAllocatorPage::freeBlock(size_t offset, size_t numDescriptors) noexcept
	{
		auto nextBlockIter = m_FreeListByOffset.upper_bound(offset);
		auto prevBlockIter = nextBlockIter;

		if (prevBlockIter != m_FreeListByOffset.begin())
		{
			--prevBlockIter;
		}
		else
		{
			prevBlockIter = m_FreeListByOffset.end();
		}

		m_NumFreeHandles += numDescriptors;

		if (prevBlockIter != m_FreeListByOffset.end() && offset == prevBlockIter->first + prevBlockIter->second.Size)
		{
			// The previous block is exactly behind the block that is to be freed.
			//
			// PrevBlock.Offset           Offset
			// |                          |
			// |<-----PrevBlock.Size----->|<------Size-------->|
			//

			offset = prevBlockIter->first;
			numDescriptors += prevBlockIter->second.Size;

			m_FreeListBySize.erase(prevBlockIter->second.FreeListBySizeIter);
			m_FreeListByOffset.erase(prevBlockIter);
		}

		if (nextBlockIter != m_FreeListByOffset.end() && offset + numDescriptors == nextBlockIter->first)
		{
			// The next block is exactly in front of the block that is to be freed.
			//
			// Offset               NextBlock.Offset 
			// |                    |
			// |<------Size-------->|<-----NextBlock.Size----->|
			//

			numDescriptors += nextBlockIter->second.Size;

			m_FreeListBySize.erase(nextBlockIter->second.FreeListBySizeIter);
			m_FreeListByOffset.erase(nextBlockIter);
		}

		addNewBlock(offset, numDescriptors);
	}
}