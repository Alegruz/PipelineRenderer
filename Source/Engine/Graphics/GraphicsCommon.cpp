#include "pch.h"

#include "Graphics/GraphicsCommon.h"
#include "Utility/Utility.h"

namespace pr
{
    HRESULT CreateAdapter(ComPtr<IDXGIAdapter4>& pOutAdapter, _In_ IDXGIFactory4* pDxgiFactory, BOOL bUseWarp) noexcept
    {
        HRESULT hr = S_OK;

        ComPtr<IDXGIAdapter4> pDxgiAdapter;
        if (!bUseWarp)
        {
            ComPtr<IDXGIAdapter1> pHardwareAdapter;

            SIZE_T maxDedicatedVideoMemory = 0;
            for (UINT i = 0; pDxgiFactory->EnumAdapters1(i, &pHardwareAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
                pHardwareAdapter->GetDesc1(&dxgiAdapterDesc);

                // Check to see if the adapter can create a D3D12 device without actually 
                // creating it. The adapter with the largest dedicated video memory
                // is favored.
                if ((dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                    SUCCEEDED(D3D12CreateDevice(
                        pHardwareAdapter.Get(),
                        D3D_FEATURE_LEVEL_11_0,
                        __uuidof(ID3D12Device),
                        nullptr
                    )) &&
                    dxgiAdapterDesc.DedicatedVideoMemory > maxDedicatedVideoMemory)
                {
                    maxDedicatedVideoMemory = dxgiAdapterDesc.DedicatedVideoMemory;
                }
            }
            if (pHardwareAdapter.Get())
            {
                hr = pHardwareAdapter.As(&pDxgiAdapter);
                CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateAdapter >> Hardware DXGI Adapter 1 to 4");
            }

            return hr;
        }

        {
            ComPtr<IDXGIAdapter> pWarpAdapter;
            hr = pDxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));
            CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateAdapter >> Warp Adapter Creation");

            hr = pWarpAdapter.As(&pDxgiAdapter);

            CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateAdapter >> Warp DXGI Adapter 1 to 4");
        }
        pOutAdapter = pDxgiAdapter;
        return hr;
    }

    HRESULT CreateCommandAllocator(ComPtr<ID3D12CommandAllocator>& pOutCommandAllocator, ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type) noexcept
    {
        HRESULT hr = S_OK;

        hr = pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&pOutCommandAllocator));
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateCommandAllocator >> Command Allocator Creation");

        return hr;
    }

    HRESULT CreateCommandList(ComPtr<ID3D12GraphicsCommandList>& pOutCommandList, ID3D12Device2* pDevice, ID3D12CommandAllocator* pCommandAllocator, D3D12_COMMAND_LIST_TYPE type) noexcept
    {
        HRESULT hr = S_OK;

        hr = pDevice->CreateCommandList(0u, type, pCommandAllocator, nullptr, IID_PPV_ARGS(&pOutCommandList));
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateCommandList >> Command List Creation");

        hr = pOutCommandList->Close();
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateCommandList >> Command List Closing");

        return hr;
    }

    HRESULT CreateCommandQueue(ComPtr<ID3D12CommandQueue>& pOutCommandQueue, ID3D12Device2* pDevice, D3D12_COMMAND_LIST_TYPE type) noexcept
    {
        HRESULT hr = S_OK;

        // Describe and create the command queue
        D3D12_COMMAND_QUEUE_DESC queueDesc =
        {
            .Type = type,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0,
        };
        hr = pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pOutCommandQueue));
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateCommandQueue >> Command Queue Creation");

        return hr;
    }

    HRESULT CreateDescriptorHeap(ComPtr<ID3D12DescriptorHeap>& pOutDescriptorHeap, ID3D12Device2* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT uNumDescriptors) noexcept
    {
        HRESULT hr = S_OK;

        // Create a render target view
        D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc =
        {
            .Type = type,
            .NumDescriptors = uNumDescriptors,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        };
        hr = pDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&pOutDescriptorHeap));
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateDescriptorHeap >> Descriptor Heap Creation");

        return hr;
    }

    HRESULT CreateDevice(ComPtr<ID3D12Device2>& pOutDevice, IDXGIAdapter4* pDxgiAdapter) noexcept
    {
        HRESULT hr = S_OK;

        hr = D3D12CreateDevice(pDxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pOutDevice));
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateDevice >> Hardware Device Creation");

        hr = D3D12CreateDevice(pDxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pOutDevice));
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateDevice >> Warp Device Creation");

        return hr;
    }

    HRESULT CreateEventHandle(HANDLE& outFenceEvent)
    {
        HRESULT hr = S_OK;

        outFenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!outFenceEvent)
        {
            hr = E_FAIL;
        }

        return hr;
    }

    HRESULT CreateFence(ComPtr<ID3D12Fence>& pOutFence, ID3D12Device2* pDevice) noexcept
    {
        HRESULT hr = S_OK;

        hr = pDevice->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pOutFence));
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateFence >> Fence Creation");

        return hr;
    }

    HRESULT CreateSwapChain(ComPtr<IDXGISwapChain4>& pOutSwapChain, HWND hWnd, IDXGIFactory4* pDxgiFactory, ID3D12CommandQueue* pCommandQueue, UINT uWidth, UINT uHeight, UINT uNumBuffers, _In_ BOOL bIsTearingSupported) noexcept
    {
        HRESULT hr = S_OK;

        DXGI_SWAP_CHAIN_DESC1 sd =
        {
            .Width = uWidth,
            .Height = uHeight,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .Stereo = FALSE,
            .SampleDesc = { 1u, 0u },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = uNumBuffers,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
            // It is recommended to always allow tearing if tearing support is available
            .Flags = bIsTearingSupported ? static_cast<UINT>(DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) : 0u
        };

        ComPtr<IDXGISwapChain1> pSwapChain1;
        hr = pDxgiFactory->CreateSwapChainForHwnd(
            pCommandQueue,
            hWnd,
            &sd,
            nullptr,
            nullptr,
            &pSwapChain1
        );
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateSwapChain >> Swap Chain Creation");
        hr = pSwapChain1.As(&pOutSwapChain);
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::CreateSwapChain >> Swap Chain 1 to 4");

        return hr;
    }

    HRESULT Flush(UINT64& uFenceValue, ID3D12CommandQueue* pCommandQueue, ID3D12Fence* pFence, HANDLE fenceEvent) noexcept
    {
        HRESULT hr = S_OK;

        hr = Signal(uFenceValue, pCommandQueue, pFence);
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::Flush >> Signal");

        hr = WaitForFenceValue(pFence, uFenceValue, fenceEvent);
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::Flush >> Wait for Fence Value");

        return hr;
    }

    HRESULT Signal(UINT64& uOutFenceValue, ID3D12CommandQueue* pCommandQueue, ID3D12Fence* pFence) noexcept
    {
        HRESULT hr = S_OK;

        ++uOutFenceValue;
        hr = pCommandQueue->Signal(pFence, uOutFenceValue);
        CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::Signal >> Command Queue Signal");

        return hr;
    }

    HRESULT UpdateRenderTargetViews(ComPtr<ID3D12Resource>* ppOutBackBuffers, _In_ UINT uNumBackBuffers, ID3D12Device2* pDevice, IDXGISwapChain4* pSwapChain, ID3D12DescriptorHeap* pDescriptorHeap) noexcept
    {
        HRESULT hr = S_OK;

        UINT uRtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(pDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        for (UINT i = 0u; i < uNumBackBuffers; ++i)
        {
            hr = pSwapChain->GetBuffer(i, IID_PPV_ARGS(&ppOutBackBuffers[i]));
            CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::Initialize >> Swap Chain get buffer");

            pDevice->CreateRenderTargetView(ppOutBackBuffers[i].Get(), nullptr, rtvHandle);

            rtvHandle.Offset(uRtvDescriptorSize);
        }

        return hr;
    }

    HRESULT WaitForFenceValue(ID3D12Fence* pFence, UINT64 uFenceValue, HANDLE fenceEvent, DWORD dwMilliseconds) noexcept
    {
        HRESULT hr = S_OK;

        if (pFence->GetCompletedValue() < uFenceValue)
        {
            hr = pFence->SetEventOnCompletion(uFenceValue, fenceEvent);
            CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::WaitForFenceValue >> Set Event on Completion");

            ::WaitForSingleObject(fenceEvent, dwMilliseconds);
        }

        return hr;
    }

    HRESULT WaitForFenceValue(ID3D12Fence* pFence, UINT64 uFenceValue, HANDLE fenceEvent) noexcept
    {
        return WaitForFenceValue(pFence, uFenceValue, fenceEvent, DWORD_MAX);
    }

	HRESULT UpdateBufferResource(
		ID3D12Resource** ppOutDestinationResource,
		ID3D12Resource** ppOutIntermediateResource,
		ID3D12Device* pDevice, 
		ID3D12GraphicsCommandList2* pCommandList, 
		size_t numElements, 
		size_t elementSize, 
		const void* pBufferData, 
		D3D12_RESOURCE_FLAGS flags
	) noexcept
	{
		HRESULT hr = S_OK;

		size_t bufferSize = numElements * elementSize;

		// Create a committed resource for the GPU resource in a default heap
		hr = pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(ppOutDestinationResource)
		);
		CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::UpdateBufferResource >> Creating committed resource to destination resource");

		// Create an committed resource for the upload
		if (pBufferData)
		{
			hr = pDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(ppOutIntermediateResource)
			);
			CHECK_AND_RETURN_HRESULT(hr, L"GraphicsCommon::UpdateBufferResource >> Creating committed resource to intermediate resource");

			D3D12_SUBRESOURCE_DATA subresourceData =
			{
				.pData = pBufferData,
				.RowPitch = bufferSize,
				.SlicePitch = subresourceData.RowPitch
			};

			UpdateSubresources(pCommandList, *ppOutDestinationResource, *ppOutIntermediateResource, 0u, 0u, 1u, &subresourceData);
		}

		return hr;
	}

	HRESULT UpdateBufferResource(
		ID3D12Resource** ppOutDestinationResource,
		ID3D12Resource** ppOutIntermediateResource,
		ID3D12Device* pDevice,
		ID3D12GraphicsCommandList2* pCommandList,
		size_t numElements,
		size_t elementSize,
		const void* pBufferData
	) noexcept
	{
		return UpdateBufferResource(ppOutDestinationResource, ppOutIntermediateResource, pDevice, pCommandList, numElements, elementSize, pBufferData, D3D12_RESOURCE_FLAG_NONE);
	}
}