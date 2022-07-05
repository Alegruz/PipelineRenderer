#include "pch.h"

#include "Graphics/Renderer.h"

#include "Graphics/CommandQueue.h"
#include "Graphics/GraphicsCommon.h"
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
        , m_pSwapChain()
        , m_apBackBuffers{}
        , m_pRtvDescriptorHeap()
        , m_pDepthBuffer()
        , m_pDsvDescriptorHeap()
        , m_pRootSignature()
        , m_pPipelineState()
        , m_pDirectCommandQueue()
        , m_pComputeCommandQueue()
        , m_pCopyCommandQueue()
        , m_Viewport(CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<FLOAT>(DEFAULT_WIDTH), static_cast<FLOAT>(DEFAULT_HEIGHT) })
        , m_ScissorsRect(CD3DX12_RECT{ 0, 0, LONG_MAX, LONG_MAX })
        , m_uRtvDescriptorSize(0u)
        , m_uCurrentBackBufferIndex(0u)
        , m_DriverType(D3D_DRIVER_TYPE_UNKNOWN)
        , m_FeatureLevel(D3D_FEATURE_LEVEL_12_1)
        , m_auFrameFenceValues{}
        //, m_pBaseCube(std::make_shared<BaseCube>())
        , m_uWidth(DEFAULT_WIDTH)
        , m_uHeight(DEFAULT_HEIGHT)
        , m_FoV(45.0f)
        , m_bIsVSyncEnabled(TRUE)
        , m_bIsTearingSupported(FALSE)
        , m_bIsFullScreen(FALSE)
    {
    }

    //std::shared_ptr<Scene> Renderer::sMainScene = nullptr;

    Renderer::~Renderer() noexcept
    {
        flush();
    }

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
    HRESULT Renderer::Initialize(_In_ HWND hWnd, _In_ std::unique_ptr<Scene>& pScene) noexcept
    {
        HRESULT hr = S_OK;

        // Check for DirectX Math library support
        if (!XMVerifyCPUSupport())
        {
            hr = E_FAIL;
            CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Failed to verify DirectX Math library support");
        }

        RECT rc;
        GetClientRect(hWnd, &rc);
        m_uWidth = static_cast<UINT>(rc.right - rc.left);
        m_uHeight = static_cast<UINT>(rc.bottom - rc.top);

#if _DEBUG
        // Enable the D3D12 debug layer
        {
            ComPtr<ID3D12Debug> pDebugController;
            hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController));
            CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Enabling debug layer failed");
            pDebugController->EnableDebugLayer();
        }
#endif

        UINT uCreateFactoryFlags = 0u;
#if _DEBUG
        uCreateFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
        ComPtr<IDXGIFactory4> pDxgiFactory;
        hr = CreateDXGIFactory2(uCreateFactoryFlags, IID_PPV_ARGS(&pDxgiFactory));
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> DXGI Factory creation");

        D3D_DRIVER_TYPE aDriverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        //UINT uNumDriverTypes = ARRAYSIZE(aDriverTypes);

        D3D_FEATURE_LEVEL aFeatureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        //UINT uNumFeatureLevels = ARRAYSIZE(aFeatureLevels);

        ComPtr<IDXGIAdapter4> pDxgiAdapter;
        hr = CreateAdapter(pDxgiAdapter, pDxgiFactory.Get(), FALSE);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Create Adapter");

        hr = CreateDevice(m_pDevice, pDxgiAdapter.Get());
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Create Device");

#if _DEBUG
        ComPtr<ID3D12InfoQueue> pInfoQueue;
        hr = m_pDevice.As(&pInfoQueue);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Info Queue Creation");
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

        // Suppress whole categories of messages
        // D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY aSeverities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO,
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID aDenyIds[] =
        {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
        };

        D3D12_INFO_QUEUE_FILTER newFilter =
        {
            .DenyList = {.NumSeverities = ARRAYSIZE(aSeverities), .pSeverityList = aSeverities, .NumIDs = ARRAYSIZE(aDenyIds), .pIDList = aDenyIds},
        };

        hr = pInfoQueue->PushStorageFilter(&newFilter);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> ID3D12InfoQueue::PushStorageFilter");
#endif

        m_pDirectCommandQueue = std::make_shared<CommandQueue>(m_pDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);
        m_pComputeCommandQueue = std::make_shared<CommandQueue>(m_pDevice, D3D12_COMMAND_LIST_TYPE_COMPUTE);
        m_pCopyCommandQueue = std::make_shared<CommandQueue>(m_pDevice, D3D12_COMMAND_LIST_TYPE_COPY);

        hr = m_pDirectCommandQueue->Initialize();
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Initializing direct command queue");

        hr = m_pComputeCommandQueue->Initialize();
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Initializing direct command queue");

        hr = m_pCopyCommandQueue->Initialize();
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Initializing direct command queue");

        // Describe and create the swap chain
        m_bIsTearingSupported = checkTearingSupport();
        hr = CreateSwapChain(m_pSwapChain, hWnd, pDxgiFactory.Get(), m_pDirectCommandQueue->GetD3D12CommandQueue().Get(), m_uWidth, m_uHeight, NUM_FRAMEBUFFERS, m_bIsTearingSupported);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Swap Chain Creation");

        hr = CreateDescriptorHeap(m_pRtvDescriptorHeap, m_pDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NUM_FRAMEBUFFERS);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Descriptor Heap Creation");
        m_uRtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        hr = UpdateRenderTargetViews(m_apBackBuffers, NUM_FRAMEBUFFERS, m_pDevice.Get(), m_pSwapChain.Get(), m_pRtvDescriptorHeap.Get());
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Update RTV");

        

        //m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        //D3D11_VIEWPORT vp =
        //{
        //    .TopLeftX = 0.0f,
        //    .TopLeftY = 0.0f,
        //    .Width = static_cast<FLOAT>(m_uWidth),
        //    .Height = static_cast<FLOAT>(m_uHeight),
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
        m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(m_uWidth) / static_cast<FLOAT>(m_uHeight), 0.01f, 1000.0f);

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

        // Load the vertex shader
        ComPtr<ID3DBlob> pVertexShaderBlob;
        hr = D3DReadFileToBlob(L"Contents/Shaders/VertexShader.cso", &pVertexShaderBlob);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Reading vertex shader");

        // Load the pixel shader
        ComPtr<ID3DBlob> pPixelShaderBlob;
        hr = D3DReadFileToBlob(L"Contents/Shaders/PixelShader.cso", &pPixelShaderBlob);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Reading pixel shader");

        // Create the vertex input layout
        D3D12_INPUT_ELEMENT_DESC aInputLayout[] =
        {
            {
                .SemanticName = "POSITION",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0,
            },
            {
                .SemanticName = "NORMAL",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0,
            },
            {
                .SemanticName = "TEXCOORD",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0,
            },
        };

        // Create a root signature
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData =
        {
            .HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1,
        };
        hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData));
        if (FAILED(hr))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        // Allow input layout and deny unnecessary access to certain pipeline stages
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        // A single 32-bit constant root parameter that is used by the vertex shader
        CD3DX12_ROOT_PARAMETER1 aRootParameters[3] = {};
        aRootParameters[0].InitAsConstants((sizeof(XMMATRIX) + sizeof(XMFLOAT4)) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
        aRootParameters[1].InitAsConstants(sizeof(XMMATRIX) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
        aRootParameters[2].InitAsConstants(sizeof(XMMATRIX) / 4, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(ARRAYSIZE(aRootParameters), aRootParameters, 0, nullptr, rootSignatureFlags);

        // Serialize the root signature
        ComPtr<ID3DBlob> pRootSignatureBlob;
        ComPtr<ID3DBlob> pErrorBlob;
        hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &pRootSignatureBlob, &pErrorBlob);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Serializing versioned root signature");

        // Create the root signature
        hr = m_pDevice->CreateRootSignature(0, pRootSignatureBlob->GetBufferPointer(), pRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Creating root signature");

        struct PipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS VertexShader;
            CD3DX12_PIPELINE_STATE_STREAM_PS PixelShader;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DsvFormat;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RtvFormats;
        };

        D3D12_RT_FORMAT_ARRAY rtvFormats =
        {
            .RTFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
            .NumRenderTargets = 1,
        };

        PipelineStateStream pipelineStateStream =
        {
            .pRootSignature = m_pRootSignature.Get(),
            .InputLayout = D3D12_INPUT_LAYOUT_DESC{ .pInputElementDescs = aInputLayout, .NumElements = ARRAYSIZE(aInputLayout) },
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .VertexShader = CD3DX12_SHADER_BYTECODE(pVertexShaderBlob.Get()),
            .PixelShader = CD3DX12_SHADER_BYTECODE(pPixelShaderBlob.Get()),
            .DsvFormat = DXGI_FORMAT_D32_FLOAT,
            .RtvFormats = rtvFormats
        };

        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc =
        {
            .SizeInBytes = sizeof(PipelineStateStream),
            .pPipelineStateSubobjectStream = &pipelineStateStream
        };
        hr = m_pDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_pPipelineState));
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Creating pipeline state");

        ComPtr<ID3D12GraphicsCommandList2> pCommandList;
        hr = m_pCopyCommandQueue->GetCommandList(pCommandList);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Getting command list from direct command queue");

        hr = pScene->Initialize(m_pDevice.Get(), pCommandList.Get());
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Initializing scene");

        UINT64 uFenceValue = 0u;
        hr = m_pCopyCommandQueue->ExecuteCommandList(uFenceValue, pCommandList.Get());
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Executing direct command");

        m_pCopyCommandQueue->WaitForFenceValue(uFenceValue);

        // Resize / create the depth buffer
        hr = CreateDescriptorHeap(m_pDsvDescriptorHeap, m_pDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1u);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Initialize >> Creating descriptor heap for DSV");

        resizeDepthBuffer(m_uWidth, m_uHeight);

        //hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        //m_shadowMapTexture = std::make_shared<RenderTexture>(m_uWidth, m_uHeight);
        //hr = m_shadowMapTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        //for (size_t i = 0; i < NUM_LIGHTS; ++i)
        //{
        //    m_scenes[m_pszMainSceneName]->GetPointLight(i)->Initialize(m_uWidth, m_uHeight);
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

    void Renderer::HandleInput(_In_ KeyboardInput& input, _In_ const MouseInput& mouseInput, _In_ FLOAT deltaTime)
    {
        if (input.IsButtonPressed('V'))
        {
            m_bIsVSyncEnabled = !m_bIsVSyncEnabled;
            input.ProcessedButton('V');

            OutputDebugString(L"V Sync ");
            if (m_bIsVSyncEnabled)
            {
                OutputDebugString(L"Enabled\n");
            }
            else
            {
                OutputDebugString(L"Disabled\n");
            }
        }

        m_Camera.HandleInput(input, mouseInput, deltaTime);
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
    HRESULT Renderer::Render(const std::unique_ptr<Scene>& pScene)
    {
        HRESULT hr = S_OK;
        ComPtr<ID3D12GraphicsCommandList2> pCommandList;
        hr = m_pDirectCommandQueue->GetCommandList(pCommandList);
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Render >> Getting command list");

        UINT uCurrentBackBufferIndex = m_uCurrentBackBufferIndex;
        ID3D12Resource* pBackBuffer = m_apBackBuffers[m_uCurrentBackBufferIndex].Get();
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = getCurrentRtv();
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

        // Clear
        {
            // Transit to RENDER_TARGET state
            TransitResource(pCommandList.Get(), pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

            static constexpr const FLOAT CLEAR_COLOR[] = { 0.4f, 0.6f, 0.9f, 1.0f };
            
            ClearRtv(pCommandList.Get(), rtv, CLEAR_COLOR);
            ClearDepth(pCommandList.Get(), dsv);
        }

        pCommandList->SetPipelineState(m_pPipelineState.Get());
        pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());
        pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        pCommandList->RSSetViewports(1, &m_Viewport);
        pCommandList->RSSetScissorRects(1, &m_ScissorsRect);

        pCommandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

        Camera::ConstantBuffer cbCamera =
        {
            .View = m_Camera.GetView(),
        };
        XMStoreFloat4(&cbCamera.Position, m_Camera.GetAt());

        pCommandList->SetGraphicsRoot32BitConstants(0, (sizeof(XMMATRIX) + sizeof(XMFLOAT4)) / 4, &cbCamera, 0);
        pCommandList->SetGraphicsRoot32BitConstants(1, (sizeof(XMMATRIX)) / 4, &m_Projection, 0);

        for (auto& iter : pScene->GetRenderables())
        {
            pCommandList->IASetVertexBuffers(0, 1, &iter.second->GetVertexBufferView());
            pCommandList->IASetIndexBuffer(&iter.second->GetIndexBufferView());

            pCommandList->SetGraphicsRoot32BitConstants(2, (sizeof(XMMATRIX)) / 4, &iter.second->GetWorldMatrix(), 0);

            for (UINT i = 0u; i < iter.second->GetNumMeshes(); ++i)
            {
                pCommandList->DrawIndexedInstanced(
                    iter.second->GetMesh(i).uNumIndices,
                    1, 
                    iter.second->GetMesh(i).uBaseIndex,
                    iter.second->GetMesh(i).uBaseVertex,
                    0
                );
            }
        }

        // Present
        {
            TransitResource(pCommandList.Get(), pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

            hr = m_pDirectCommandQueue->ExecuteCommandList(m_auFrameFenceValues[uCurrentBackBufferIndex], pCommandList.Get());
            CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Render >> Executing direct command queue");

            hr = present(uCurrentBackBufferIndex);
            CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Render >> Presenting");

            m_pDirectCommandQueue->WaitForFenceValue(m_auFrameFenceValues[uCurrentBackBufferIndex]);
        }

        return hr;
    }

    HRESULT Renderer::Resize(UINT uWidth, UINT uHeight) noexcept
    {
        HRESULT hr = S_OK;
        if (m_uWidth != uWidth || m_uHeight != uHeight)
        {
            if (uWidth == 0 && uHeight == 0)
            {
                hr = E_INVALIDARG;
                CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Resize >> Resizing to 0 x 0");
            }

            // Don't allow 0 size swap chain back buffers
            m_uWidth = uWidth;
            m_uHeight = uHeight;

            // Flush the GPU queue to make sure the swap chain's back buffers
            // are not being referenced by an in-flight command list.
            flush();

            for (size_t i = 0; i < NUM_FRAMEBUFFERS; ++i)
            {
                // Any references to the back buffers must be released
                // before the swap chain can be resized.
                m_apBackBuffers[i].Reset();
                //m_auFrameFenceValues[i] = m_auFrameFenceValues[m_uCurrentBackBufferIndex];
            }

            DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
            hr = m_pSwapChain->GetDesc(&swapChainDesc);
            CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Resize >> Get Swap Chain Description");
            hr = m_pSwapChain->ResizeBuffers(NUM_FRAMEBUFFERS, m_uWidth, m_uHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
            CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Resize >> Resize Buffers");

            m_uCurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();

            hr = UpdateRenderTargetViews(m_apBackBuffers, NUM_FRAMEBUFFERS, m_pDevice.Get(), m_pSwapChain.Get(), m_pRtvDescriptorHeap.Get());
            CHECK_AND_RETURN_HRESULT(hr, L"Renderer::Resize >> Updating RTVs");
        }

        return hr;
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

    BOOL Renderer::checkTearingSupport() const noexcept
    {
        BOOL bAllowsTearing = FALSE;

        // Rather than create the DXGI 1.5 factory interface directly, we create the
        // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
        // graphics debugging tools which will not support the 1.5 factory interface 
        // until a future update.

        ComPtr<IDXGIFactory4> pDxgiFactory4;
        if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory4))))
        {
            ComPtr<IDXGIFactory5> pDxgiFactory5;
            if (SUCCEEDED(pDxgiFactory4.As(&pDxgiFactory5)))
            {
                if (FAILED(pDxgiFactory5->CheckFeatureSupport(
                        DXGI_FEATURE_PRESENT_ALLOW_TEARING, 
                        &bAllowsTearing, 
                        sizeof(bAllowsTearing)
                    )))
                {
                    bAllowsTearing = FALSE;
                }
            }
        }

        return bAllowsTearing;
    }

    HRESULT Renderer::flush() noexcept
    {
        HRESULT hr = S_OK;
        hr = m_pDirectCommandQueue->Flush();
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::flush >> Direct command queue flush");
        hr = m_pComputeCommandQueue->Flush();
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::flush >> Compute command queue flush");
        hr = m_pCopyCommandQueue->Flush();
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::flush >> Copy command queue flush");
        return hr;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Renderer::getCurrentRtv() const noexcept
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_uCurrentBackBufferIndex, m_uRtvDescriptorSize);
    }

    HRESULT Renderer::present(_Out_ UINT& uOutCurrentBackBufferIndex) noexcept
    {
        HRESULT hr = S_OK;

        UINT uSyncInterval = m_bIsVSyncEnabled ? 1 : 0;
        UINT uPresentFlags = m_bIsTearingSupported && !m_bIsVSyncEnabled ? DXGI_PRESENT_ALLOW_TEARING : 0;
        hr = m_pSwapChain->Present(uSyncInterval, uPresentFlags);
        m_uCurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
        uOutCurrentBackBufferIndex = m_uCurrentBackBufferIndex;

        return hr;
    }

    HRESULT Renderer::resizeDepthBuffer(UINT uWidth, UINT uHeight) noexcept
    {
        HRESULT hr = S_OK;

        hr = flush();
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::resizeDepthBuffer >> flush");

        if (uWidth == 0 && uHeight == 0)
        {
            hr = E_INVALIDARG;
            CHECK_AND_RETURN_HRESULT(hr, L"Renderer::resizeDepthBuffer >> New size is 0 x 0");
        }

        // Resize screen dependent resources
        // Create a depth buffer
        D3D12_CLEAR_VALUE optimizedClearValue =
        {
            .Format = DXGI_FORMAT_D32_FLOAT,
            .DepthStencil = { .Depth = 1.0f, .Stencil = 0 }
        };
            
        CD3DX12_HEAP_PROPERTIES heapPropertiesDefault(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC dsvResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_D32_FLOAT,
            uWidth,
            uHeight,
            1,
            0,
            1,
            0,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        );

        hr = m_pDevice->CreateCommittedResource(
            &heapPropertiesDefault,
            D3D12_HEAP_FLAG_NONE,
            &dsvResourceDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&m_pDepthBuffer)
        );
        CHECK_AND_RETURN_HRESULT(hr, L"Renderer::resizeDepthBuffer >> Creating committed resource");

        // Update the depth-stencil view
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc =
        {
            .Format = DXGI_FORMAT_D32_FLOAT,
            .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
            .Flags = D3D12_DSV_FLAG_NONE,
            .Texture2D = {.MipSlice = 0 },
        };
        m_pDevice->CreateDepthStencilView(m_pDepthBuffer.Get(), &dsvDesc, m_pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        return hr;
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