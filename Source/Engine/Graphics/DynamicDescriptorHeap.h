#pragma once

#include "pch.h"

namespace pr
{
	class CommandList;
	class RootSignature;

	class DynamicDescriptorHeap
	{
	public:
		DynamicDescriptorHeap() = delete;
		explicit DynamicDescriptorHeap(_In_ ID3D12Device2* pDevice, _In_ D3D12_DESCRIPTOR_HEAP_TYPE heapType, _In_ size_t numDescriptorsPerHeap) noexcept;
		explicit DynamicDescriptorHeap(_In_ ID3D12Device2* pDevice, _In_ D3D12_DESCRIPTOR_HEAP_TYPE heapType) noexcept;
		explicit DynamicDescriptorHeap(_In_ const DynamicDescriptorHeap& other) noexcept;
		explicit DynamicDescriptorHeap(_In_ DynamicDescriptorHeap&& other) noexcept;
		DynamicDescriptorHeap& operator=(_In_ const DynamicDescriptorHeap& other) noexcept;
		DynamicDescriptorHeap& operator=(_In_ DynamicDescriptorHeap&& other) noexcept;
		virtual ~DynamicDescriptorHeap() noexcept = default;

		HRESULT StageDescriptors(_In_ size_t rootParameterIndex, _In_ size_t offset, _In_ size_t numDescriptors, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE hSrcDescriptors) noexcept;
		HRESULT CommitStagedDescriptors(_In_ CommandList& commandList, _In_ ID3D12Device2* pDevice, std::function<void(ID3D12GraphicsCommandList2*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc) noexcept;
		HRESULT CommitStagedDescriptorsForDraw(_In_ CommandList& commandList, _In_ ID3D12Device2* pDevice) noexcept;
		HRESULT CommitStagedDescriptorsForDispatch(_In_ CommandList& commandList, _In_ ID3D12Device2* pDevice) noexcept;

		HRESULT CopyDescriptor(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& hOutDescriptor, _In_ CommandList& commandList, _In_ ID3D12Device2* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor) noexcept;
		void ParseRootSignature(const RootSignature& rootSignature) noexcept;
		void Reset() noexcept;

	private:
		HRESULT requestDescriptorHeap(_Out_ ComPtr<ID3D12DescriptorHeap>& pOutDescriptorHeap, _In_ ID3D12Device2* pDevice) noexcept;
		HRESULT createDescriptorHeap(_Out_ ComPtr<ID3D12DescriptorHeap>& pOutDescriptorHeap, _In_ ID3D12Device2* pDevice) noexcept;
		size_t computeStaleDescriptorCount() const noexcept;

	private:
		static constexpr const size_t MAX_DESCRIPTOR_TABLES = sizeof(UINT) * 8;

		struct DescriptorTableCache final
		{
			explicit DescriptorTableCache() noexcept;
			explicit DescriptorTableCache(_In_ const DescriptorTableCache& other) noexcept = default;
			explicit DescriptorTableCache(_In_ DescriptorTableCache&& other) noexcept = default;
			DescriptorTableCache& operator=(_In_ const DescriptorTableCache& other) noexcept = default;
			DescriptorTableCache& operator=(_In_ DescriptorTableCache&& other) noexcept = default;
			~DescriptorTableCache() noexcept = default;

			void Reset();

			size_t NumDescriptors;
			D3D12_CPU_DESCRIPTOR_HANDLE* phBaseDescriptor;
		};

	private:
		D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
		size_t m_NumDescriptorsPerHeap;
		size_t m_DescriptorHandleIncrementSize;
		std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> m_pahDescriptorHandleCache;
		DescriptorTableCache m_aDescriptorTableCache[MAX_DESCRIPTOR_TABLES];
		UINT m_uDescriptorTableBitMask;
		UINT m_uStaleDescriptorTableBitMask;

		using DescriptorHeapPool = std::queue<ComPtr<ID3D12DescriptorHeap>>;

		DescriptorHeapPool m_DescriptorHeapPool;
		DescriptorHeapPool m_AvailableDescriptorHeaps;

		ComPtr<ID3D12DescriptorHeap> m_pCurrentDescriptorHeap;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_hCurrentGpuDescriptorHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_hCurrentCpuDescriptorHandle;

		size_t m_NumFreeHandles;
	};
}