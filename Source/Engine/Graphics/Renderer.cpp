#include "pch.h"

#include "Graphics/Renderer.h"
#include "Utility/Utility.h"

namespace pr
{
    void GetHardwareAdapter(_Out_ IDXGIAdapter1** ppAdapter, _In_ IDXGIFactory1* pFactory, _In_ BOOL bRequestHighPerformanceAdapter);
    void GetHardwareAdapter(_Out_ IDXGIAdapter1** ppAdapter, _In_ IDXGIFactory1* pFactory);

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,
                  m_swapChain1, m_renderTargetView, m_vertexShader,
                  m_pixelShader, m_vertexLayout, m_vertexBuffer].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderer::Renderer() noexcept
        : m_Camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
        , m_Projection()
        , m_pDevice()
        , m_pCommandQueue()
        , m_pSwapChain()
        , m_pBackBuffers{}
        , m_pCommandList()
        , m_pCommandAllocators{}
        , m_pRTVDescriptorHeap()
        , m_uRTVDescriptorSize(0u)
        , m_uCurrentBackBufferIndex(0u)
        , m_DriverType(D3D_DRIVER_TYPE_UNKNOWN)
        , m_FeatureLevel(D3D_FEATURE_LEVEL_12_1)
        , m_FenceEvent()
        , m_pFence()
        , m_uFenceValue(0)
        , m_aFrameFenceValues{}
        , m_bIsVSyncEnabled(TRUE)
        , m_bIsTearingSupported(FALSE)
        , m_bIsFullScreen(FALSE)
    {
    }

    //std::shared_ptr<Scene> Renderer::sMainScene = nullptr;

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                  m_d3dDevice1, m_immediateContext1, m_swapChain1,
                  m_swapChain, m_renderTargetView, m_vertexShader,
                  m_vertexLayout, m_pixelShader, m_vertexBuffer].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT uWidth = static_cast<UINT>(rc.right - rc.left);
        UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

#if _DEBUG
        // Enable the D3D12 debug layer
        {
            ComPtr<ID3D12Debug> pDebugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
            {
                pDebugController->EnableDebugLayer();
            }
        }
#endif

        ComPtr<IDXGIFactory4> pFactory;
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> DXGI Factory creation");

        D3D_DRIVER_TYPE aDriverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT uNumDriverTypes = ARRAYSIZE(aDriverTypes);

        D3D_FEATURE_LEVEL aFeatureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT uNumFeatureLevels = ARRAYSIZE(aFeatureLevels);

        {
            m_DriverType = aDriverTypes[0];
            ComPtr<IDXGIAdapter1> pHardwareAdapter;
            GetHardwareAdapter(&pHardwareAdapter, pFactory.Get());

            hr = D3D12CreateDevice(pHardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));

            CheckHresult(hr, L"Renderer::Initialize >> Hardware Device Creation");
        }

        if (FAILED(hr))
        {
            m_DriverType = aDriverTypes[1];
            ComPtr<IDXGIAdapter> pWarpAdapter;
            hr = pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));
            CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Warp Adapter Creation");

            hr = D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));

            CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Warp Device Creation");
        }

        // Describe and create the command queue
        D3D12_COMMAND_QUEUE_DESC queueDesc =
        {
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE
        };
        hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Command Queue Creation");

        // Describe and create the swap chain
        DXGI_SWAP_CHAIN_DESC sd =
        {
            .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = NUM_FRAMEBUFFERS,
            .OutputWindow = hWnd,
            .Windowed = TRUE,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD
        };
        
        ComPtr<IDXGISwapChain> pSwapChain;

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        //dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Swap Chain Creation");

        // Create a render target view
        //ComPtr<ID3D11Texture2D> pBackBuffer;
        //hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        //hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        // Create depth stencil texture
        //D3D11_TEXTURE2D_DESC descDepth =
        //{
        //    .Width = uWidth,
        //    .Height = uHeight,
        //    .MipLevels = 1u,
        //    .ArraySize = 1u,
        //    .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        //    .SampleDesc = {.Count = 1u, .Quality = 0u },
        //    .Usage = D3D11_USAGE_DEFAULT,
        //    .BindFlags = D3D11_BIND_DEPTH_STENCIL,
        //    .CPUAccessFlags = 0u,
        //    .MiscFlags = 0u
        //};
        //hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        // Create the depth stencil view
        //D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        //{
        //    .Format = descDepth.Format,
        //    .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
        //    .Texture2D = {.MipSlice = 0 }
        //};
        //hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        //m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        //D3D11_VIEWPORT vp =
        //{
        //    .TopLeftX = 0.0f,
        //    .TopLeftY = 0.0f,
        //    .Width = static_cast<FLOAT>(uWidth),
        //    .Height = static_cast<FLOAT>(uHeight),
        //    .MinDepth = 0.0f,
        //    .MaxDepth = 1.0f,
        //};
        //m_immediateContext->RSSetViewports(1, &vp);

        // Set primitive topology
        //m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Create the constant buffers
        //D3D11_BUFFER_DESC bd =
        //{
        //    .ByteWidth = sizeof(CBChangeOnResize),
        //    .Usage = D3D11_USAGE_DEFAULT,
        //    .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
        //    .CPUAccessFlags = 0
        //};
        //hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        // Initialize the projection matrix
        m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

        //CBChangeOnResize cbChangesOnResize =
        //{
        //    .Projection = XMMatrixTranspose(m_projection)
        //};
        //m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

        //bd.ByteWidth = sizeof(CBLights);
        //bd.Usage = D3D11_USAGE_DEFAULT;
        //bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        //bd.CPUAccessFlags = 0u;

        //hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        //bd.ByteWidth = sizeof(CBShadowMatrix);
        //bd.Usage = D3D11_USAGE_DEFAULT;
        //bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        //bd.CPUAccessFlags = 0u;

        //hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbShadowMatrix.GetAddressOf());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        //m_camera.Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

        //if (!m_scenes.contains(m_pszMainSceneName))
        //{
        //    return E_FAIL;
        //}

        //hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        //hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        //m_shadowMapTexture = std::make_shared<RenderTexture>(uWidth, uHeight);
        //hr = m_shadowMapTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        //for (size_t i = 0; i < NUM_LIGHTS; ++i)
        //{
        //    m_scenes[m_pszMainSceneName]->GetPointLight(i)->Initialize(uWidth, uHeight);
        //}

        return S_OK;
    }

    //HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
    //{
    //    if (m_scenes.contains(pszSceneName))
    //    {
    //        return E_FAIL;
    //    }

    //    m_scenes[pszSceneName] = scene;

    //    return S_OK;
    //}

    //std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
    //{
    //    if (m_scenes.contains(pszSceneName))
    //    {
    //        return m_scenes[pszSceneName];
    //    }

    //    return nullptr;
    //}

    //HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    //{
    //    if (!m_scenes.contains(pszSceneName))
    //    {
    //        return E_FAIL;
    //    }

    //    m_pszMainSceneName = pszSceneName;

    //    return S_OK;
    //}

    //void Renderer::SetShadowMapShaders(_In_ std::shared_ptr<ShadowVertexShader> vertexShader, _In_ std::shared_ptr<PixelShader> pixelShader)
    //{
    //    m_shadowVertexShader = move(vertexShader);
    //    m_shadowPixelShader = move(pixelShader);
    //}

    void Renderer::HandleInput(_In_ const KeyboardInput& input, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_Camera.HandleInput(input, mouseRelativeMovement, deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        //m_scenes[m_pszMainSceneName]->Update(deltaTime);

        m_Camera.Update(deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_DriverType;
    }

    void GetHardwareAdapter(_Out_ IDXGIAdapter1** ppAdapter, _In_ IDXGIFactory1* pFactory, _In_ BOOL bRequestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;

        ComPtr<IDXGIFactory6> factory6;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    bRequestHighPerformanceAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        if (adapter.Get() == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        *ppAdapter = adapter.Detach();
    }

    void GetHardwareAdapter(_Out_ IDXGIAdapter1** ppAdapter, _In_ IDXGIFactory1* pFactory)
    {
        GetHardwareAdapter(ppAdapter, pFactory, FALSE);
    }
}