#include "pch.h"

#include "Graphics/CommandQueue.h"

#include "Utility/Utility.h"

namespace pr
{
	CommandQueue::CommandQueue(const ComPtr<ID3D12Device2>& pDevice, D3D12_COMMAND_LIST_TYPE type) noexcept
		: m_pDevice(pDevice)
		, m_pCommandQueue()
		, m_pFence()
		, m_FenceEvent()
		, m_uFenceValue(0)
		, m_CommandListType(type)
		, m_CommandAllocatorQueue()
		, m_CommandListQueue()
	{
	}

	HRESULT CommandQueue::Initialize() noexcept
	{
		HRESULT hr = S_OK;

		// Describe and create the command queue
		D3D12_COMMAND_QUEUE_DESC queueDesc =
		{
			.Type = m_CommandListType,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0,
		};
		hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));
		CHECK_AND_RETURN_HRESULT(hr, L"CommandQueue::Initialize >> Command Queue Creation");

		hr = m_pDevice->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
		CHECK_AND_RETURN_HRESULT(hr, L"CommandQueue::Initialize >> Fence Creation");

		m_FenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!m_FenceEvent)
		{
			hr = E_FAIL;
		}

		return hr;
	}

	HRESULT CommandQueue::GetCommandList(_Out_ ComPtr<ID3D12GraphicsCommandList2>& pOutCommandList) noexcept
	{
		HRESULT hr = S_OK;

		ComPtr<ID3D12CommandAllocator> pCommandAllocator;

		if (!m_CommandAllocatorQueue.empty() && IsFenceComplete(m_CommandAllocatorQueue.front().uFenceValue))
		{
			pCommandAllocator = m_CommandAllocatorQueue.front().pCommandAllocator;
			m_CommandAllocatorQueue.pop();

			hr = pCommandAllocator->Reset();
			CHECK_AND_RETURN_HRESULT(hr, L"CommandQueue::GetCommandList >> Resetting command allocator");
		}
		else
		{
			hr = createCommandAllocator(pCommandAllocator);
			CHECK_AND_RETURN_HRESULT(hr, L"CommandQueue::GetCommandList >> Creating command allocator");
		}

		if (!m_CommandListQueue.empty())
		{
			pOutCommandList = m_CommandListQueue.front();
			m_CommandListQueue.pop();

			hr = pOutCommandList->Reset(pCommandAllocator.Get(), nullptr);
			CHECK_AND_RETURN_HRESULT(hr, L"CommandQueue::GetCommandList >> Resetting command list");
		}
		else
		{
			hr = createCommandList(pOutCommandList, pCommandAllocator.Get());
			CHECK_AND_RETURN_HRESULT(hr, L"CommandQueue::GetCommandList >> Creating command list");
		}

		hr = pOutCommandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), pCommandAllocator.Get());
		CHECK_AND_RETURN_HRESULT(hr, L"CommandQueue::GetCommandList >> Setting privaet data interface");

		return hr;
	}

	HRESULT CommandQueue::ExecuteCommandList(UINT64& uOutFenceValue, ID3D12GraphicsCommandList2* pCommandList) noexcept
	{
		HRESULT hr = S_OK;

		hr = pCommandList->Close();
		CHECK_AND_RETURN_HRESULT(hr, L"CommandQueue::ExecuteCommandList >> Closing command list");

		ComPtr<ID3D12CommandAllocator> pCommandAllocator;
		UINT uDataSize = sizeof(ID3D12CommandAllocator);
		hr = pCommandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &uDataSize, pCommandAllocator.GetAddressOf());
		CHECK_AND_RETURN_HRESULT(hr, L"CommandQueue::ExecuteCommandList >> Getting private data");

		ID3D12CommandList* const ppCommandLists[] =
		{
			pCommandList,
		};

		m_pCommandQueue->ExecuteCommandLists(1u, ppCommandLists);
		uOutFenceValue = Signal();

		m_CommandAllocatorQueue.emplace(CommandAllocatorEntry{ .uFenceValue = uOutFenceValue, .pCommandAllocator = pCommandAllocator });
		m_CommandListQueue.push(pCommandList);

		// The ownership of the command allocator has been transferred to the ComPtr
		// in the command allocator queue. It is safe to release the reference 
		// in this temporary COM pointer here.
		pCommandAllocator->Release();

		return hr;
	}

	UINT64 CommandQueue::Signal() noexcept
	{
		return UINT64();
	}

	BOOL CommandQueue::IsFenceComplete(UINT64 uFenceValue) noexcept
	{
		UNREFERENCED_PARAMETER(uFenceValue);
		return 0;
	}

	void CommandQueue::WaitForFenceValue(UINT64 uFenceValue) noexcept
	{
		UNREFERENCED_PARAMETER(uFenceValue);
	}

	void CommandQueue::Flush() noexcept
	{
	}

	const ComPtr<ID3D12CommandQueue>& CommandQueue::GetD3D12CommandQueue() const noexcept
	{
		// // O: insert return statement here
		return m_pCommandQueue;
	}

	HRESULT CommandQueue::createCommandAllocator(ComPtr<ID3D12CommandAllocator>& pOutCommandAllocator) noexcept
	{
		HRESULT hr = S_OK;

		hr = m_pDevice->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&pOutCommandAllocator));
		CHECK_AND_RETURN_HRESULT(hr, L"CommandQueue::createCommandAllocator >> Command Allocator Creation");

		return hr;
	}

	HRESULT CommandQueue::createCommandList(ComPtr<ID3D12GraphicsCommandList2>& pOutCommandList, ID3D12CommandAllocator* pCommandAllocator) noexcept
	{
		HRESULT hr = S_OK;

		hr = m_pDevice->CreateCommandList(0u, m_CommandListType, pCommandAllocator, nullptr, IID_PPV_ARGS(&pOutCommandList));
		CHECK_AND_RETURN_HRESULT(hr, L"Renderer::CreateCommandList >> Command List Creation");

		hr = pOutCommandList->Close();
		CHECK_AND_RETURN_HRESULT(hr, L"Renderer::CreateCommandList >> Command List Closing");

		return hr;
	}
}