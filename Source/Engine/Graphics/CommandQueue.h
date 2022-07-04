#pragma once

#include "pch.h"

namespace pr
{
	class CommandQueue
	{
	public:
		explicit CommandQueue() noexcept = delete;
		explicit CommandQueue(_In_ const ComPtr<ID3D12Device2>& pDevice, _In_ D3D12_COMMAND_LIST_TYPE type) noexcept;
		CommandQueue(_In_ const CommandQueue& other) = delete;
		CommandQueue(_In_ CommandQueue&& other) = delete;
		CommandQueue& operator=(_In_ const CommandQueue& other) = delete;
		CommandQueue& operator=(_In_ CommandQueue&& other) = delete;
		virtual ~CommandQueue() noexcept = default;

		HRESULT Initialize() noexcept;

		HRESULT GetCommandList(_Out_ ComPtr<ID3D12GraphicsCommandList2>& pOutCommandList) noexcept;
		HRESULT ExecuteCommandList(_Out_ UINT64& uOutFenceValue, _In_ ID3D12GraphicsCommandList2* pCommandList) noexcept;

		UINT64 Signal() noexcept;
		BOOL IsFenceComplete(_In_ UINT64 uFenceValue) noexcept;
		void WaitForFenceValue(_In_ UINT64 uFenceValue) noexcept;
		void Flush() noexcept;

		const ComPtr<ID3D12CommandQueue>& GetD3D12CommandQueue() const noexcept;

	protected:
		HRESULT createCommandAllocator(_Out_ ComPtr<ID3D12CommandAllocator>& pOutCommandAllocator) noexcept;
		HRESULT createCommandList(_Out_ ComPtr<ID3D12GraphicsCommandList2>& pOutCommandList, _In_ ID3D12CommandAllocator* pCommandAllocator) noexcept;

	private:
		struct CommandAllocatorEntry
		{
			UINT64 uFenceValue;
			ComPtr<ID3D12CommandAllocator> pCommandAllocator;
		};

		using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
		using CommandListQueue = std::queue<ComPtr<ID3D12GraphicsCommandList2>>;

		ComPtr<ID3D12Device2> m_pDevice;
		ComPtr<ID3D12CommandQueue> m_pCommandQueue;
		ComPtr<ID3D12Fence> m_pFence;
		HANDLE m_FenceEvent;
		UINT64 m_uFenceValue;
		D3D12_COMMAND_LIST_TYPE m_CommandListType;

		CommandAllocatorQueue m_CommandAllocatorQueue;
		CommandListQueue m_CommandListQueue;
	};
	static_assert(sizeof(CommandQueue) == 136);
}