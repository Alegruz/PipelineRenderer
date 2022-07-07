#pragma once

#include "pch.h"

#include "Graphics/DescriptorAllocation.h"

namespace pr
{
	class DescriptorAllocatorPage;

	class DescriptorAllocator final
	{
	public:
		DescriptorAllocator() = delete;
		explicit DescriptorAllocator(_In_ D3D12_DESCRIPTOR_HEAP_TYPE type, _In_opt_ size_t numDescriptorsPerHeap) noexcept;
		explicit DescriptorAllocator(_In_ D3D12_DESCRIPTOR_HEAP_TYPE type) noexcept;
		explicit DescriptorAllocator(const DescriptorAllocator& other) noexcept = default;
		explicit DescriptorAllocator(DescriptorAllocator&& other) noexcept = default;
		DescriptorAllocator& operator=(const DescriptorAllocator& other) noexcept = default;
		DescriptorAllocator& operator=(DescriptorAllocator&& other) noexcept = default;
		~DescriptorAllocator() noexcept = default;

		HRESULT Allocate(_Out_ DescriptorAllocation& outAllocation, _In_ ID3D12Device2* pDevice, _In_opt_ size_t numDescriptors) noexcept;
		HRESULT Allocate(_Out_ DescriptorAllocation& outAllocation, _In_ ID3D12Device2* pDevice) noexcept;
		void ReleaseStaleDescriptors(size_t frameNumber) noexcept;

	private:
		using DescriptorHeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;

	private:
		HRESULT createAllocatorPage(_Out_ std::shared_ptr<DescriptorAllocatorPage>& pOutPage, _In_ ID3D12Device2* pDevice) noexcept;

	private:
		D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
		size_t m_NumDescriptorsPerHeap;
		DescriptorHeapPool m_HeapPool;
		std::set<size_t> m_AvailableHeaps;
		std::mutex m_AllocationMutex;
	};
}