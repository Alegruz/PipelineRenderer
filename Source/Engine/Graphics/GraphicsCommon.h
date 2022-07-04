#pragma once

#include "pch.h"

namespace pr
{
	HRESULT CreateAdapter(_Out_ ComPtr<IDXGIAdapter4>& pOutAdapter, _In_ IDXGIFactory4* pDxgiFactory, _In_ BOOL bUseWarp) noexcept;
	HRESULT CreateCommandAllocator(_Out_ ComPtr<ID3D12CommandAllocator>& pOutCommandAllocator, _In_ ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type) noexcept;
	HRESULT CreateCommandList(_Out_ ComPtr<ID3D12GraphicsCommandList>& pOutCommandList, _In_ ID3D12Device2* pDevice, _In_ ID3D12CommandAllocator* pCommandAllocator, D3D12_COMMAND_LIST_TYPE type) noexcept;
	HRESULT CreateCommandQueue(_Out_ ComPtr<ID3D12CommandQueue>& pOutCommandQueue, _In_ ID3D12Device2* pDevice, _In_ D3D12_COMMAND_LIST_TYPE type) noexcept;
	HRESULT CreateDescriptorHeap(_Out_ ComPtr<ID3D12DescriptorHeap>& pOutDescriptorHeap, _In_ ID3D12Device2* pDevice, _In_ D3D12_DESCRIPTOR_HEAP_TYPE type, _In_ UINT uNumDescriptors) noexcept;
	HRESULT CreateDevice(_Out_ ComPtr<ID3D12Device2>& pOutDevice, _In_ IDXGIAdapter4* pDxgiAdapter) noexcept;
	HRESULT CreateEventHandle(_Out_ HANDLE& outFenceEvent);
	HRESULT CreateFence(_Out_ ComPtr<ID3D12Fence>& pOutFence, _In_ ID3D12Device2* pDevice) noexcept;
	HRESULT CreateSwapChain(_Out_ ComPtr<IDXGISwapChain4>& pOutSwapChain, _In_ HWND hWnd, _In_ IDXGIFactory4* pDxgiFactory, _In_ ID3D12CommandQueue* pCommandQueue, _In_ UINT uWidth, _In_ UINT uHeight, _In_ UINT uNumBuffers, _In_ BOOL bIsTearingSupported) noexcept;
	HRESULT Flush(_Inout_ UINT64& uFenceValue, _In_ ID3D12CommandQueue* pCommandQueue, _In_ ID3D12Fence* pFence, HANDLE fenceEvent) noexcept;
	HRESULT Signal(_Out_ UINT64& uOutFenceValue, _In_ ID3D12CommandQueue* pCommandQueue, _In_ ID3D12Fence* pFence) noexcept;
	HRESULT UpdateRenderTargetViews(_Out_ ComPtr<ID3D12Resource>* ppOutBackBuffers, _In_ UINT uNumBackBuffers, _In_ ID3D12Device2* pDevice, _In_ IDXGISwapChain4* pSwapChain, _In_ ID3D12DescriptorHeap* pDescriptorHeap) noexcept;
	HRESULT WaitForFenceValue(_In_ ID3D12Fence* pFence, _In_ UINT64 uFenceValue, _In_ HANDLE fenceEvent, _In_opt_ DWORD dwMilliseconds) noexcept;
	HRESULT WaitForFenceValue(_In_ ID3D12Fence* pFence, _In_ UINT64 uFenceValue, _In_ HANDLE fenceEvent) noexcept;
	HRESULT UpdateBufferResource(
		_Out_ ID3D12Resource** ppOutDestinationResource,
		_Out_ ID3D12Resource** ppOutIntermediateResource,
		_In_ ID3D12Device* pDevice, 
		_In_ ID3D12GraphicsCommandList2* pCommandList, 
		_In_ size_t numElements, 
		_In_ size_t elementSize,
		_In_ const void* pBufferData,
		_In_ D3D12_RESOURCE_FLAGS flags
	) noexcept;
	HRESULT UpdateBufferResource(
		_Out_ ID3D12Resource** ppOutDestinationResource,
		_Out_ ID3D12Resource** ppOutIntermediateResource,
		_In_ ID3D12Device* pDevice, 
		_In_ ID3D12GraphicsCommandList2* pCommandList, 
		_In_ size_t numElements,
		_In_ size_t elementSize,
		_In_ const void* pBufferData
	) noexcept;
}
