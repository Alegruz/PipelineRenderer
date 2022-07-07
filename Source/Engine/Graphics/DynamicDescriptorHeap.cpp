#include "pch.h"

#include "Graphics/CommandList.h"
#include "Graphics/DynamicDescriptorHeap.h"
#include "Graphics/RootSignature.h"
#include "Utility/Utility.h"

namespace pr
{
	DynamicDescriptorHeap::DynamicDescriptorHeap(ID3D12Device2* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE heapType, size_t numDescriptorsPerHeap) noexcept
		: m_Type(heapType)
		, m_NumDescriptorsPerHeap(numDescriptorsPerHeap)
		, m_DescriptorHandleIncrementSize(pDevice->GetDescriptorHandleIncrementSize(heapType))
		, m_pahDescriptorHandleCache(std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(m_NumDescriptorsPerHeap))
		, m_uDescriptorTableBitMask(0)
		, m_uStaleDescriptorTableBitMask(0)
		, m_DescriptorHeapPool()
		, m_AvailableDescriptorHeaps()
		, m_pCurrentDescriptorHeap()
		, m_hCurrentGpuDescriptorHandle(D3D12_DEFAULT)
		, m_hCurrentCpuDescriptorHandle(D3D12_DEFAULT)
		, m_NumFreeHandles(0)
	{
	}

	DynamicDescriptorHeap::DynamicDescriptorHeap(ID3D12Device2* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE heapType) noexcept
		: DynamicDescriptorHeap(pDevice, heapType, 1024)
	{
	}

	HRESULT DynamicDescriptorHeap::StageDescriptors(size_t rootParameterIndex, size_t offset, size_t numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE hSrcDescriptors) noexcept
	{
		HRESULT hr = S_OK;
		if (numDescriptors > m_NumDescriptorsPerHeap || rootParameterIndex >= MAX_DESCRIPTOR_TABLES)
		{
			hr = E_INVALIDARG;
			CHECK_AND_RETURN_HRESULT(
				hr, 
				L"DynamicDescriptorHeap::StageDescriptors >> Number of descriptors exceeds number of descriptors per heap or root parameter index exceeds maximum number of descriptor tables"
			);
		}

		DescriptorTableCache& descriptorTableCache = m_aDescriptorTableCache[rootParameterIndex];

		if ((offset + numDescriptors) > descriptorTableCache.NumDescriptors)
		{
			hr = E_INVALIDARG;
			CHECK_AND_RETURN_HRESULT(
				hr,
				L"DynamicDescriptorHeap::StageDescriptors >> Number of descriptors exceeds number of descriptors in the descriptor tables"
			);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE* pDstDescriptor = (descriptorTableCache.phBaseDescriptor + offset);
		for (size_t i = 0; i < numDescriptors; ++i)
		{
			pDstDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(hSrcDescriptors, i, m_DescriptorHandleIncrementSize);
		}

		m_uStaleDescriptorTableBitMask |= (1 << rootParameterIndex);

		return hr;
	}

	HRESULT DynamicDescriptorHeap::CommitStagedDescriptors(CommandList& commandList, ID3D12Device2* pDevice, std::function<void(ID3D12GraphicsCommandList2*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc) noexcept
	{
		HRESULT hr = S_OK;
		size_t numDescriptorsToCommit = computeStaleDescriptorCount();

		if (numDescriptorsToCommit > 0)
		{
			ID3D12GraphicsCommandList2* pCommandList = commandList.GetCommandList().Get();
			if (pCommandList != nullptr)
			{
				hr = E_INVALIDARG;
				CHECK_AND_RETURN_HRESULT(hr, L"DynamicDescriptorHeap::CommitStagedDescriptors >> Command list is nullptr");
			}

			if (!m_pCurrentDescriptorHeap || m_NumFreeHandles < numDescriptorsToCommit)
			{
				hr = requestDescriptorHeap(m_pCurrentDescriptorHeap, pDevice);
				CHECK_AND_RETURN_HRESULT(hr, L"DynamicDescriptorHeap::CommitStagedDescriptors >> Request descriptor heap");
				m_hCurrentCpuDescriptorHandle = m_pCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
				m_hCurrentGpuDescriptorHandle = m_pCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
				m_NumFreeHandles = m_NumDescriptorsPerHeap;

				commandList.SetDescriptorHeap(m_Type, m_pCurrentDescriptorHeap.Get());
				m_uStaleDescriptorTableBitMask = m_uDescriptorTableBitMask;
			}

			DWORD dwRootIndex = 0;

			while (_BitScanForward(&dwRootIndex, m_uStaleDescriptorTableBitMask))
			{
				UINT uNumSrcDescriptors = m_aDescriptorTableCache[dwRootIndex].NumDescriptors;
				D3D12_CPU_DESCRIPTOR_HANDLE* phSrcDescriptorHandles = m_aDescriptorTableCache[dwRootIndex].phBaseDescriptor;

				D3D12_CPU_DESCRIPTOR_HANDLE ahDstDescriptorRangeStarts[] =
				{
					m_hCurrentCpuDescriptorHandle,
				};
				UINT auDstDescriptorRangeSizes[] =
				{
					uNumSrcDescriptors,
				};

				pDevice->CopyDescriptors(1, ahDstDescriptorRangeStarts, auDstDescriptorRangeSizes, uNumSrcDescriptors, phSrcDescriptorHandles, nullptr, m_Type);
				setFunc(pCommandList, dwRootIndex, m_hCurrentGpuDescriptorHandle);

				m_hCurrentCpuDescriptorHandle.Offset(uNumSrcDescriptors, m_DescriptorHandleIncrementSize);
				m_hCurrentGpuDescriptorHandle.Offset(uNumSrcDescriptors, m_DescriptorHandleIncrementSize);
				m_NumFreeHandles -= uNumSrcDescriptors;

				m_uStaleDescriptorTableBitMask ^= (1 << dwRootIndex);
			}
		}
	}

	HRESULT DynamicDescriptorHeap::CommitStagedDescriptorsForDraw(CommandList& commandList, ID3D12Device2* pDevice) noexcept
	{
		return CommitStagedDescriptors(commandList, pDevice, &ID3D12GraphicsCommandList2::SetGraphicsRootDescriptorTable);
	}

	HRESULT DynamicDescriptorHeap::CommitStagedDescriptorsForDispatch(CommandList& commandList, ID3D12Device2* pDevice) noexcept
	{
		return CommitStagedDescriptors(commandList, pDevice, &ID3D12GraphicsCommandList2::SetComputeRootDescriptorTable);
	}

	HRESULT DynamicDescriptorHeap::CopyDescriptor(D3D12_GPU_DESCRIPTOR_HANDLE& hOutDescriptor, CommandList& commandList, ID3D12Device2* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor) noexcept
	{
		HRESULT hr = S_OK;

		if (!m_pCurrentDescriptorHeap || m_NumFreeHandles < 1)
		{
			hr = requestDescriptorHeap(m_pCurrentDescriptorHeap, pDevice);
			CHECK_AND_RETURN_HRESULT(hr, L"DynamicDescriptorHeap::CopyDescriptor >> Request descriptor heap");
			m_hCurrentCpuDescriptorHandle = m_pCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			m_hCurrentGpuDescriptorHandle = m_pCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			m_NumFreeHandles = m_NumDescriptorsPerHeap;

			commandList.SetDescriptorHeap(m_Type, m_pCurrentDescriptorHeap.Get());

			m_uStaleDescriptorTableBitMask = m_uDescriptorTableBitMask;
		}

		hOutDescriptor = m_hCurrentGpuDescriptorHandle;
		pDevice->CopyDescriptorsSimple(1, m_hCurrentCpuDescriptorHandle, hCpuDescriptor, m_Type);

		m_hCurrentCpuDescriptorHandle.Offset(1, m_DescriptorHandleIncrementSize);
		m_hCurrentGpuDescriptorHandle.Offset(1, m_DescriptorHandleIncrementSize);
		m_NumFreeHandles -= 1;

		return hr;
	}

	void DynamicDescriptorHeap::ParseRootSignature(const RootSignature& rootSignature) noexcept
	{
		m_uStaleDescriptorTableBitMask = 0;

		const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc = rootSignature.GetRootSignatureDesc();

		m_uDescriptorTableBitMask = rootSignature.GetDescriptorTableBitMask(m_Type);
		UINT uDescriptorTableBitMask = m_uDescriptorTableBitMask;
		size_t currentOffset = 0;
		DWORD dwRootIndex = 0;
		while (_BitScanForward(&dwRootIndex, uDescriptorTableBitMask) && dwRootIndex < rootSignatureDesc.NumParameters)
		{
			UINT uNumDescriptors = rootSignature.GetNumDescriptors(dwRootIndex);

			DescriptorTableCache& descriptorTableCache = m_aDescriptorTableCache[dwRootIndex];
			descriptorTableCache.NumDescriptors = uNumDescriptors;
			descriptorTableCache.phBaseDescriptor = m_pahDescriptorHandleCache.get() + currentOffset;

			currentOffset += uNumDescriptors;

			uDescriptorTableBitMask ^= (1 << dwRootIndex);
		}

		assert(currentOffset <= m_NumDescriptorsPerHeap && "The root signature requires more than the maximum number of descriptors per descriptor heap. Consider increasing the maximum number of descriptors per descriptor heap.");
	}

	void DynamicDescriptorHeap::Reset() noexcept
	{
		m_AvailableDescriptorHeaps = m_DescriptorHeapPool;
		m_pCurrentDescriptorHeap.Reset();
		m_hCurrentCpuDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
		m_hCurrentGpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
		m_NumFreeHandles = 0;
		m_uDescriptorTableBitMask = 0;
		m_uStaleDescriptorTableBitMask = 0;

		for (size_t i = 0; i < MAX_DESCRIPTOR_TABLES; ++i)
		{
			m_aDescriptorTableCache[i].Reset();
		}
	}

	HRESULT DynamicDescriptorHeap::requestDescriptorHeap(ComPtr<ID3D12DescriptorHeap>& pOutDescriptorHeap, ID3D12Device2* pDevice) noexcept
	{
		HRESULT hr = S_OK;

		if (!m_AvailableDescriptorHeaps.empty())
		{
			pOutDescriptorHeap = m_AvailableDescriptorHeaps.front();
			m_AvailableDescriptorHeaps.pop();
		}
		else
		{
			hr = createDescriptorHeap(pOutDescriptorHeap, pDevice);
			CHECK_AND_RETURN_HRESULT(hr, L"DynamicDescriptorHeap::requestDescriptorHeap >> creating descriptor heap");
			m_DescriptorHeapPool.push(pOutDescriptorHeap);
		}

		return hr;
	}

	HRESULT DynamicDescriptorHeap::createDescriptorHeap(ComPtr<ID3D12DescriptorHeap>& pOutDescriptorHeap, ID3D12Device2* pDevice) noexcept
	{
		HRESULT hr = S_OK;

		D3D12_DESCRIPTOR_HEAP_DESC desc =
		{
			.Type = m_Type,
			.NumDescriptors = m_NumDescriptorsPerHeap,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		};

		hr = pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pOutDescriptorHeap));
		CHECK_AND_RETURN_HRESULT(hr, L"DynamicDescriptorHeap::createDescriptorHeap >> Create descriptor heap");

		return hr;
	}

	size_t DynamicDescriptorHeap::computeStaleDescriptorCount() const noexcept
	{
		size_t numStaleDescriptors = 0;
		DWORD dwIndex = 0;
		DWORD dwStaleDescriptorsBitMask = m_uStaleDescriptorTableBitMask;

		while (_BitScanForward(&dwIndex, dwStaleDescriptorsBitMask))
		{
			numStaleDescriptors += m_aDescriptorTableCache[dwIndex].NumDescriptors;
			dwStaleDescriptorsBitMask ^= (1 << dwIndex);
		}

		return numStaleDescriptors;
	}

	DynamicDescriptorHeap::DescriptorTableCache::DescriptorTableCache() noexcept
		: NumDescriptors(0)
		, phBaseDescriptor(nullptr)
	{
	}

	void DynamicDescriptorHeap::DescriptorTableCache::Reset()
	{
		NumDescriptors = 0;
		phBaseDescriptor = nullptr;
	}
}