#pragma once

#include "pch.h"

#include "Graphics/DescriptorAllocation.h"

namespace pr
{
	class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage>
	{
	public:
		DescriptorAllocatorPage() = delete;
		explicit DescriptorAllocatorPage(_In_ D3D12_DESCRIPTOR_HEAP_TYPE type, _In_ size_t numDescriptors) noexcept;
		explicit DescriptorAllocatorPage(_In_ const DescriptorAllocatorPage& other) = default;
		explicit DescriptorAllocatorPage(_In_ DescriptorAllocatorPage&& other) = default;
		DescriptorAllocatorPage& operator=(_In_ const DescriptorAllocatorPage& other) = default;
		DescriptorAllocatorPage& operator=(_In_ DescriptorAllocatorPage&& other) = default;
		virtual ~DescriptorAllocatorPage() noexcept = default;

		HRESULT Initialize(_In_ ID3D12Device2* pDevice) noexcept;

		D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const noexcept;
		BOOL HasSpace(_In_ size_t numDescriptors) const noexcept;
		size_t GetNumFreeHandles() const noexcept;

		HRESULT Allocate(_Out_ DescriptorAllocation& outAllocation, _In_ size_t numDescriptors) noexcept;
		void Free(_In_ DescriptorAllocation&& descriptor, _In_ size_t frameNumber) noexcept;
		void ReleaseStaleDescriptors(size_t frameNumber);

	protected:
		size_t computeOffset(_In_ D3D12_CPU_DESCRIPTOR_HANDLE hHandle) noexcept;
		void addNewBlock(_In_ size_t offset, _In_ size_t numDescriptors) noexcept;
		void freeBlock(_In_ size_t offset, _In_ size_t numDescriptors) noexcept;

	private:
		using OffsetType = size_t;
		using SizeType = size_t;

		struct FreeBlockInfo;
		using FreeListByOffset = std::map<OffsetType, FreeBlockInfo>;
		using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;

		struct FreeBlockInfo final
		{
			FreeBlockInfo() = delete;
			explicit FreeBlockInfo(_In_ SizeType size) noexcept;
			explicit FreeBlockInfo(_In_ const FreeBlockInfo& other) noexcept = default;
			explicit FreeBlockInfo(_In_ FreeBlockInfo&& other) noexcept = default;
			FreeBlockInfo& operator=(_In_ const FreeBlockInfo& other) noexcept = default;
			FreeBlockInfo& operator=(_In_ FreeBlockInfo&& other) noexcept = default;
			~FreeBlockInfo() noexcept = default;

			SizeType Size;
			FreeListBySize::iterator FreeListBySizeIter;
		};
		
		struct StaleDescriptorInfo final
		{
			StaleDescriptorInfo() = delete;
			explicit StaleDescriptorInfo(_In_ OffsetType offset, _In_ SizeType size, _In_ size_t frameNumber) noexcept;
			explicit StaleDescriptorInfo(_In_ const StaleDescriptorInfo& other) noexcept = default;
			explicit StaleDescriptorInfo(_In_ StaleDescriptorInfo&& other) noexcept = default;
			StaleDescriptorInfo& operator=(_In_ const StaleDescriptorInfo& other) noexcept = default;
			StaleDescriptorInfo& operator=(_In_ StaleDescriptorInfo&& other) noexcept = default;
			~StaleDescriptorInfo() noexcept = default;

			OffsetType Offset;
			SizeType Size;
			size_t FrameNumber;
		};

		using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;

	private:
		FreeListByOffset m_FreeListByOffset;
		FreeListBySize m_FreeListBySize;
		StaleDescriptorQueue m_StaleDescriptors;

		ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
		D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_hBaseDescriptor;
		size_t m_DescriptorHandleIncrementSize;
		size_t m_NumDescriptorsInHeap;
		size_t m_NumFreeHandles;

		std::mutex m_AllocationMutex;
	};
}