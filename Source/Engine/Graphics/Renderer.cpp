#include "Renderer/Renderer.h"

namespace library
{
#ifdef LAB10
    namespace lab10
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView, m_vertexShader,
                      m_pixelShader, m_vertexLayout, m_vertexBuffer].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
            , m_depthStencil()
            , m_depthStencilView()
            , m_cbChangeOnResize()
            , m_cbShadowMatrix()
            , m_pszMainSceneName(nullptr)
            , m_padding{ '\0' }
            , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
            , m_projection()
            , m_scenes()
            , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
            , m_shadowMapTexture()
            , m_shadowVertexShader()
            , m_shadowPixelShader()
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

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd =
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = {.Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd =
                {
                    .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = {.Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .OutputWindow = hWnd,
                    .Windowed = TRUE
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create depth stencil texture
            D3D11_TEXTURE2D_DESC descDepth =
            {
                .Width = uWidth,
                .Height = uHeight,
                .MipLevels = 1u,
                .ArraySize = 1u,
                .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };
            hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
            {
                .Format = descDepth.Format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D = {.MipSlice = 0 }
            };
            hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

            // Setup the viewport
            D3D11_VIEWPORT vp =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Create the constant buffers
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(CBChangeOnResize),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                .CPUAccessFlags = 0
            };
            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Initialize the projection matrix
            m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

            CBChangeOnResize cbChangesOnResize =
            {
                .Projection = XMMatrixTranspose(m_projection)
            };
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

            bd.ByteWidth = sizeof(CBLights);
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0u;

            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            bd.ByteWidth = sizeof(CBShadowMatrix);
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0u;

            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbShadowMatrix.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_camera.Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

            if (!m_scenes.contains(m_pszMainSceneName))
            {
                return E_FAIL;
            }

            hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr))
            {
                return hr;
            }

            m_shadowMapTexture = std::make_shared<RenderTexture>(uWidth, uHeight);
            hr = m_shadowMapTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr))
            {
                return hr;
            }

            for (size_t i = 0; i < NUM_LIGHTS; ++i)
            {
                m_scenes[m_pszMainSceneName]->GetPointLight(i)->Initialize(uWidth, uHeight);
            }

            return S_OK;
        }

        HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
        {
            if (m_scenes.contains(pszSceneName))
            {
                return E_FAIL;
            }

            m_scenes[pszSceneName] = scene;

            return S_OK;
        }

        std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
        {
            if (m_scenes.contains(pszSceneName))
            {
                return m_scenes[pszSceneName];
            }

            return nullptr;
        }

        HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
        {
            if (!m_scenes.contains(pszSceneName))
            {
                return E_FAIL;
            }

            m_pszMainSceneName = pszSceneName;

            return S_OK;
        }

        void Renderer::SetShadowMapShaders(_In_ std::shared_ptr<ShadowVertexShader> vertexShader, _In_ std::shared_ptr<PixelShader> pixelShader)
        {
            m_shadowVertexShader = move(vertexShader);
            m_shadowPixelShader = move(pixelShader);
        }

        void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
        {
            m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Update

          Summary:  Update the renderables each frame

          Args:     FLOAT deltaTime
                      Time difference of a frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Update(_In_ FLOAT deltaTime)
        {
            m_scenes[m_pszMainSceneName]->Update(deltaTime);

            m_camera.Update(deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            RenderSceneToTexture();
            
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Clear the depth buffer to 1.0 (max depth)
            m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            CBChangeOnCameraMovement cbChangeOnCameraMovement =
            {
                .View = XMMatrixTranspose(m_camera.GetView())
            };
            XMStoreFloat4(&(cbChangeOnCameraMovement.CameraPosition), m_camera.GetEye());
            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cbChangeOnCameraMovement, 0, 0);

            CBLights cbLights = {};
            for (size_t i = 0; i < NUM_LIGHTS; ++i)
            {
                cbLights.LightPositions[i] = m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetPosition();
                cbLights.LightColors[i] = m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetColor();
                cbLights.LightViews[i] = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetViewMatrix());
                cbLights.LightProjections[i] = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetProjectionMatrix());
            }
            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLights, 0, 0);

            //static CHAR szMessage[256];
            //sprintf_s(
            //    szMessage,
            //    "light0: %f, %f, %f\n"
            //    "\tlight1: %f, %f, %f\n",
            //    cbLights.LightPositions[0].x, cbLights.LightPositions[0].y, cbLights.LightPositions[0].z,
            //    cbLights.LightPositions[1].x, cbLights.LightPositions[1].y, cbLights.LightPositions[1].z
            //);
            //OutputDebugStringA(szMessage);

            for (auto it = m_scenes[m_pszMainSceneName]->GetRenderables().begin(); it != m_scenes[m_pszMainSceneName]->GetRenderables().end(); ++it)
            {
                // Set Vertex buffer
                UINT aStrides[2] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(NormalData)) };
                UINT aOffsets[2] = { 0u, 0u };
                ID3D11Buffer* aBuffers[2]
                {
                    it->second->GetVertexBuffer().Get(),
                    it->second->GetNormalBuffer().Get()
                };

                m_immediateContext->IASetVertexBuffers(0u, 2u, aBuffers, aStrides, aOffsets);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .OutputColor = it->second->GetOutputColor(),
                    .HasNormalMap = it->second->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                m_immediateContext->PSSetShaderResources(2u, 1u, m_shadowMapTexture->GetShaderResourceView().GetAddressOf());
                m_immediateContext->PSSetSamplers(2u, 1u, m_shadowMapTexture->GetSamplerState().GetAddressOf());

                for (UINT i = 0u; i < it->second->GetNumMeshes(); ++i)
                {
                    UINT uMaterialIndex = it->second->GetMesh(i).uMaterialIndex;

                    assert(uMaterialIndex < it->second->GetNumMaterials() || uMaterialIndex == Renderable::INVALID_MATERIAL);

                    // Render a triangle
                    ID3D11ShaderResourceView* aViews[2]
                    {
                        m_invalidTexture->GetTextureResourceView().Get(),
                        m_invalidTexture->GetTextureResourceView().Get()
                    };

                    ID3D11SamplerState* aSamplers[2]
                    {
                        m_invalidTexture->GetSamplerState().Get(),
                        m_invalidTexture->GetSamplerState().Get()
                    };

                    if (uMaterialIndex != Renderable::INVALID_MATERIAL)
                    {
                        if (it->second->GetMaterial(uMaterialIndex)->pDiffuse)
                        {
                            aViews[0] = it->second->GetMaterial(uMaterialIndex)->pDiffuse->GetTextureResourceView().Get();
                            aSamplers[0] = it->second->GetMaterial(uMaterialIndex)->pDiffuse->GetSamplerState().Get();
                        }

                        if (it->second->GetMaterial(uMaterialIndex)->pNormal)
                        {
                            aViews[1] = it->second->GetMaterial(uMaterialIndex)->pNormal->GetTextureResourceView().Get();
                            aSamplers[1] = it->second->GetMaterial(uMaterialIndex)->pNormal->GetSamplerState().Get();
                        }
                    }

                    m_immediateContext->PSSetShaderResources(
                        0u,
                        2u,
                        aViews
                    );
                    m_immediateContext->PSSetSamplers(
                        0u,
                        2u,
                        aSamplers
                    );

                    m_immediateContext->DrawIndexed(
                        it->second->GetMesh(i).uNumIndices,
                        it->second->GetMesh(i).uBaseIndex,
                        static_cast<INT>(it->second->GetMesh(i).uBaseVertex)
                    );
                }
            }

            //for (std::unique_ptr<Renderable>& renderable : scene.GetRenderables())
            for (UINT i = 0u; i < m_scenes[m_pszMainSceneName]->GetVoxels().size(); ++i)
            {
                std::shared_ptr<Voxel> voxel = m_scenes[m_pszMainSceneName]->GetVoxels()[i];
                // Set vertex buffer
                UINT aStrides[3] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(NormalData)),
                                     static_cast<UINT>(sizeof(InstanceData)) };
                UINT aOffsets[3] = { 0u, 0u, 0u };
                ID3D11Buffer* aBuffers[3]
                {
                    voxel->GetVertexBuffer().Get(),
                    voxel->GetNormalBuffer().Get(),
                    voxel->GetInstanceBuffer().Get()
                };

                m_immediateContext->IASetVertexBuffers(0u, 3u, aBuffers, aStrides, aOffsets);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(voxel->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(voxel->GetWorldMatrix()),
                    .OutputColor = voxel->GetOutputColor(),
                    .HasNormalMap = voxel->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(voxel->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(voxel->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
                m_immediateContext->PSSetShader(voxel->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                m_immediateContext->PSSetShaderResources(2u, 1u, m_shadowMapTexture->GetShaderResourceView().GetAddressOf());
                m_immediateContext->PSSetSamplers(2u, 1u, m_shadowMapTexture->GetSamplerState().GetAddressOf());

                for (UINT uMeshIdx = 0u; uMeshIdx < voxel->GetNumMeshes(); ++uMeshIdx)
                {
                    UINT uMaterialIndex = voxel->GetMesh(uMeshIdx).uMaterialIndex;

                    assert(uMaterialIndex < voxel->GetNumMaterials() || uMaterialIndex == Renderable::INVALID_MATERIAL);

                    // Render a triangle
                    ID3D11ShaderResourceView* aViews[2]
                    {
                        m_invalidTexture->GetTextureResourceView().Get(),
                        m_invalidTexture->GetTextureResourceView().Get()
                    };

                    ID3D11SamplerState* aSamplers[2]
                    {
                        m_invalidTexture->GetSamplerState().Get(),
                        m_invalidTexture->GetSamplerState().Get()
                    };

                    if (uMaterialIndex != Renderable::INVALID_MATERIAL)
                    {
                        if (voxel->GetMaterial(uMaterialIndex)->pDiffuse)
                        {
                            aViews[0] = voxel->GetMaterial(uMaterialIndex)->pDiffuse->GetTextureResourceView().Get();
                            aSamplers[0] = voxel->GetMaterial(uMaterialIndex)->pDiffuse->GetSamplerState().Get();
                        }

                        if (voxel->GetMaterial(uMaterialIndex)->pNormal)
                        {
                            aViews[1] = voxel->GetMaterial(uMaterialIndex)->pNormal->GetTextureResourceView().Get();
                            aSamplers[1] = voxel->GetMaterial(uMaterialIndex)->pNormal->GetSamplerState().Get();
                        }
                    }

                    m_immediateContext->PSSetShaderResources(
                        0u,
                        2u,
                        aViews
                    );
                    m_immediateContext->PSSetSamplers(
                        0u,
                        2u,
                        aSamplers
                    );

                    m_immediateContext->DrawIndexedInstanced(
                        voxel->GetMesh(uMeshIdx).uNumIndices,
                        voxel->GetNumInstances(),
                        voxel->GetMesh(uMeshIdx).uBaseIndex,
                        static_cast<INT>(voxel->GetMesh(uMeshIdx).uBaseVertex),
                        0u
                    );
                }
            }

            for (auto it = m_scenes[m_pszMainSceneName]->GetModels().begin(); it != m_scenes[m_pszMainSceneName]->GetModels().end(); ++it)
            {
                // Set vertex buffer
                UINT aStrides[3] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(NormalData)),
                                     static_cast<UINT>(sizeof(AnimationData)) };
                UINT aOffsets[3] = { 0u, 0u, 0u };
                ID3D11Buffer* aBuffers[3]
                {
                    it->second->GetVertexBuffer().Get(),
                    it->second->GetNormalBuffer().Get(),
                    it->second->GetAnimationBuffer().Get()
                };

                m_immediateContext->IASetVertexBuffers(0u, 3u, aBuffers, aStrides, aOffsets);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .OutputColor = it->second->GetOutputColor(),
                    .HasNormalMap = it->second->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                CBSkinning* cbSkinning = reinterpret_cast<CBSkinning*>(malloc(sizeof(CBSkinning)));
                for (UINT i = 0u; i < it->second->GetBoneTransforms().size(); ++i)
                {
                    cbSkinning->BoneTransforms[i] = XMMatrixTranspose(it->second->GetBoneTransforms().at(i));
                }
                m_immediateContext->UpdateSubresource(it->second->GetSkinningConstantBuffer().Get(), 0u, nullptr, cbSkinning, 0u, 0u);

                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(4, 1, it->second->GetSkinningConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                m_immediateContext->PSSetShaderResources(2u, 1u, m_shadowMapTexture->GetShaderResourceView().GetAddressOf());
                m_immediateContext->PSSetSamplers(2u, 1u, m_shadowMapTexture->GetSamplerState().GetAddressOf());

                for (UINT i = 0u; i < it->second->GetNumMeshes(); ++i)
                {
                    UINT uMaterialIndex = it->second->GetMesh(i).uMaterialIndex;

                    assert(uMaterialIndex < it->second->GetNumMaterials() || uMaterialIndex == Renderable::INVALID_MATERIAL);

                    // Render a triangle
                    ID3D11ShaderResourceView* aViews[2]
                    {
                        m_invalidTexture->GetTextureResourceView().Get(),
                        m_invalidTexture->GetTextureResourceView().Get()
                    };

                    ID3D11SamplerState* aSamplers[2]
                    {
                        m_invalidTexture->GetSamplerState().Get(),
                        m_invalidTexture->GetSamplerState().Get()
                    };

                    if (uMaterialIndex != Renderable::INVALID_MATERIAL)
                    {
                        if (it->second->GetMaterial(uMaterialIndex)->pDiffuse)
                        {
                            aViews[0] = it->second->GetMaterial(uMaterialIndex)->pDiffuse->GetTextureResourceView().Get();
                            aSamplers[0] = it->second->GetMaterial(uMaterialIndex)->pDiffuse->GetSamplerState().Get();
                        }

                        if (it->second->GetMaterial(uMaterialIndex)->pNormal)
                        {
                            aViews[1] = it->second->GetMaterial(uMaterialIndex)->pNormal->GetTextureResourceView().Get();
                            aSamplers[1] = it->second->GetMaterial(uMaterialIndex)->pNormal->GetSamplerState().Get();
                        }
                    }

                    m_immediateContext->PSSetShaderResources(
                        0u,
                        2u,
                        aViews
                    );
                    m_immediateContext->PSSetSamplers(
                        0u,
                        2u,
                        aSamplers
                    );

                    m_immediateContext->DrawIndexed(
                        it->second->GetMesh(i).uNumIndices,
                        it->second->GetMesh(i).uBaseIndex,
                        static_cast<INT>(it->second->GetMesh(i).uBaseVertex)
                    );
                }
            }

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }

        void Renderer::RenderSceneToTexture()
        {	
            //Unbind current pixel shader resources
            ID3D11ShaderResourceView* const pSRV[2] = { NULL, NULL };
            m_immediateContext->PSSetShaderResources(0, 2, pSRV);
            m_immediateContext->PSSetShaderResources(2, 1, pSRV);

            // Set the render target to be the render to texture.
            m_immediateContext->OMSetRenderTargets(1, m_shadowMapTexture->GetRenderTargetView().GetAddressOf(), m_depthStencilView.Get());

            // Clear the back buffer.
            m_immediateContext->ClearRenderTargetView(m_shadowMapTexture->GetRenderTargetView().Get(), Colors::White);

            // Clear the depth buffer.
            m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            // Rneder renderables
            for (auto it = m_scenes[m_pszMainSceneName]->GetRenderables().begin(); it != m_scenes[m_pszMainSceneName]->GetRenderables().end(); ++it)
            {
                // Set Vertex buffer
                UINT uStride = sizeof(SimpleVertex);
                UINT uOffset = 0;
                m_immediateContext->IASetVertexBuffers(0u, 1u, it->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());

                // Update variables
                CBShadowMatrix cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .View = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0)->GetViewMatrix()),
                    .Projection = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0)->GetProjectionMatrix()),
                    .IsVoxel = false
                };

                m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_cbShadowMatrix.GetAddressOf());
                m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0u);

                for (UINT i = 0u; i < it->second->GetNumMeshes(); ++i)
                {
                    m_immediateContext->DrawIndexed(
                        it->second->GetMesh(i).uNumIndices,
                        it->second->GetMesh(i).uBaseIndex,
                        static_cast<INT>(it->second->GetMesh(i).uBaseVertex)
                    );
                }
            }

            // Render voxels
            for (UINT i = 0u; i < m_scenes[m_pszMainSceneName]->GetVoxels().size(); ++i)
            {
                std::shared_ptr<Voxel> voxel = m_scenes[m_pszMainSceneName]->GetVoxels()[i];

                // Set vertex buffer
                UINT aStrides[2] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(InstanceData)) };
                UINT aOffsets[2] = { 0u, 0u };
                ID3D11Buffer* aBuffers[2]
                {
                    voxel->GetVertexBuffer().Get(), voxel->GetInstanceBuffer().Get()
                };

                m_immediateContext->IASetVertexBuffers(0u, 2u, aBuffers, aStrides, aOffsets);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());

                // Update variables
                CBShadowMatrix cb =
                {
                    .World = XMMatrixTranspose(voxel->GetWorldMatrix()),
                    .View = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0)->GetViewMatrix()),
                    .Projection = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0)->GetProjectionMatrix()),
                    .IsVoxel = true
                };

                m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_cbShadowMatrix.GetAddressOf());
                m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0u);

                for (UINT uMeshIdx = 0u; uMeshIdx < voxel->GetNumMeshes(); ++uMeshIdx)
                {
                    m_immediateContext->DrawIndexedInstanced(
                        voxel->GetMesh(uMeshIdx).uNumIndices,
                        voxel->GetNumInstances(),
                        voxel->GetMesh(uMeshIdx).uBaseIndex,
                        static_cast<INT>(voxel->GetMesh(uMeshIdx).uBaseVertex),
                        0u
                    );
                }
            }

            // Render models
            for (auto it = m_scenes[m_pszMainSceneName]->GetModels().begin(); it != m_scenes[m_pszMainSceneName]->GetModels().end(); ++it)
            {
                // Set vertex buffer
                UINT uStride = sizeof(SimpleVertex);
                UINT uOffset = 0;
                m_immediateContext->IASetVertexBuffers(0u, 1u, it->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());

                // Update variables
                CBShadowMatrix cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .View = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0)->GetViewMatrix()),
                    .Projection = XMMatrixTranspose(m_scenes[m_pszMainSceneName]->GetPointLight(0)->GetProjectionMatrix()),
                    .IsVoxel = false
                };

                m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_cbShadowMatrix.GetAddressOf());
                m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0u);

                for (UINT i = 0u; i < it->second->GetNumMeshes(); ++i)
                {
                    m_immediateContext->DrawIndexed(
                        it->second->GetMesh(i).uNumIndices,
                        it->second->GetMesh(i).uBaseIndex,
                        static_cast<INT>(it->second->GetMesh(i).uBaseVertex)
                    );
                }
            }

            // Reset the render target back to the original back buffer and not the render to texture anymore.
            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::GetDriverType

          Summary:  Returns the Direct3D driver type

          Returns:  D3D_DRIVER_TYPE
                      The Direct3D driver type used
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        D3D_DRIVER_TYPE Renderer::GetDriverType() const
        {
            return m_driverType;
        }
    }
#endif

#ifdef LAB09
    namespace lab09
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView, m_vertexShader,
                      m_pixelShader, m_vertexLayout, m_vertexBuffer].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
            , m_depthStencil()
            , m_depthStencilView()
            , m_cbChangeOnResize()
            , m_pszMainSceneName(nullptr)
            , m_padding{ '\0' }
            , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
            , m_projection()
            , m_scenes()
            , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
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

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd =
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = {.Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd =
                {
                    .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = {.Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .OutputWindow = hWnd,
                    .Windowed = TRUE
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create depth stencil texture
            D3D11_TEXTURE2D_DESC descDepth =
            {
                .Width = uWidth,
                .Height = uHeight,
                .MipLevels = 1u,
                .ArraySize = 1u,
                .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };
            hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
            {
                .Format = descDepth.Format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D = {.MipSlice = 0 }
            };
            hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

            // Setup the viewport
            D3D11_VIEWPORT vp =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Create the constant buffers
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(CBChangeOnResize),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                .CPUAccessFlags = 0
            };
            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Initialize the projection matrix
            m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

            CBChangeOnResize cbChangesOnResize =
            {
                .Projection = XMMatrixTranspose(m_projection)
            };
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

            bd.ByteWidth = sizeof(CBLights);
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0u;

            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_camera.Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

            if (!m_scenes.contains(m_pszMainSceneName))
            {
                return E_FAIL;
            }

            hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }

        HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
        {
            if (m_scenes.contains(pszSceneName))
            {
                return E_FAIL;
            }

            m_scenes[pszSceneName] = scene;

            return S_OK;
        }

        std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
        {
            if (m_scenes.contains(pszSceneName))
            {
                return m_scenes[pszSceneName];
            }
            
            return nullptr;
        }

        HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
        {
            if (!m_scenes.contains(pszSceneName))
            {
                return E_FAIL;
            }

            m_pszMainSceneName = pszSceneName;

            return S_OK;
        }

        void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
        {
            m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Update

          Summary:  Update the renderables each frame

          Args:     FLOAT deltaTime
                      Time difference of a frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Update(_In_ FLOAT deltaTime)
        {
            m_scenes[m_pszMainSceneName]->Update(deltaTime);

            m_camera.Update(deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Clear the depth buffer to 1.0 (max depth)
            m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            CBChangeOnCameraMovement cbChangeOnCameraMovement =
            {
                .View = XMMatrixTranspose(m_camera.GetView())
            };
            XMStoreFloat4(&(cbChangeOnCameraMovement.CameraPosition), m_camera.GetEye());
            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cbChangeOnCameraMovement, 0, 0);

            CBLights cbLights = {};
            for (size_t i = 0; i < NUM_LIGHTS; ++i)
            {
                cbLights.LightPositions[i] = m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetPosition();
                cbLights.LightColors[i] = m_scenes[m_pszMainSceneName]->GetPointLight(i)->GetColor();
            }
            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLights, 0, 0);

            static CHAR szMessage[256];
            sprintf_s(
                szMessage,
                "light0: %f, %f, %f\n"
                "\tlight1: %f, %f, %f\n",
                cbLights.LightPositions[0].x, cbLights.LightPositions[0].y, cbLights.LightPositions[0].z,
                cbLights.LightPositions[1].x, cbLights.LightPositions[1].y, cbLights.LightPositions[1].z
            );
            OutputDebugStringA(szMessage);

            for (auto it = m_scenes[m_pszMainSceneName]->GetRenderables().begin(); it != m_scenes[m_pszMainSceneName]->GetRenderables().end(); ++it)
            {
                // Set Vertex buffer
                UINT aStrides[2] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(NormalData)) };
                UINT aOffsets[2] = { 0u, 0u };
                ID3D11Buffer* aBuffers[2]
                {
                    it->second->GetVertexBuffer().Get(),
                    it->second->GetNormalBuffer().Get()
                };

                m_immediateContext->IASetVertexBuffers(0u, 2u, aBuffers, aStrides, aOffsets);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .OutputColor = it->second->GetOutputColor(),
                    .HasNormalMap = it->second->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                for (UINT i = 0u; i < it->second->GetNumMeshes(); ++i)
                {
                    UINT uMaterialIndex = it->second->GetMesh(i).uMaterialIndex;

                    assert(uMaterialIndex < it->second->GetNumMaterials() || uMaterialIndex == Renderable::INVALID_MATERIAL);

                    // Render a triangle
                    ID3D11ShaderResourceView* aViews[2]
                    {
                        m_invalidTexture->GetTextureResourceView().Get(),
                        m_invalidTexture->GetTextureResourceView().Get()
                    };

                    ID3D11SamplerState* aSamplers[2]
                    {
                        m_invalidTexture->GetSamplerState().Get(),
                        m_invalidTexture->GetSamplerState().Get()
                    };

                    if (uMaterialIndex != Renderable::INVALID_MATERIAL)
                    {
                        if (it->second->GetMaterial(uMaterialIndex)->pDiffuse)
                        {
                            aViews[0] = it->second->GetMaterial(uMaterialIndex)->pDiffuse->GetTextureResourceView().Get();
                            aSamplers[0] = it->second->GetMaterial(uMaterialIndex)->pDiffuse->GetSamplerState().Get();
                        }

                        if (it->second->GetMaterial(uMaterialIndex)->pNormal)
                        {
                            aViews[1] = it->second->GetMaterial(uMaterialIndex)->pNormal->GetTextureResourceView().Get();
                            aSamplers[1] = it->second->GetMaterial(uMaterialIndex)->pNormal->GetSamplerState().Get();
                        }
                    }

                    m_immediateContext->PSSetShaderResources(
                        0u,
                        2u,
                        aViews
                    );
                    m_immediateContext->PSSetSamplers(
                        0u,
                        2u,
                        aSamplers
                    );

                    m_immediateContext->DrawIndexed(
                        it->second->GetMesh(i).uNumIndices,
                        it->second->GetMesh(i).uBaseIndex,
                        static_cast<INT>(it->second->GetMesh(i).uBaseVertex)
                    );
                }
            }

            //for (std::unique_ptr<Renderable>& renderable : scene.GetRenderables())
            for (UINT i = 0u; i < m_scenes[m_pszMainSceneName]->GetVoxels().size(); ++i)
            {
                std::shared_ptr<Voxel> voxel = m_scenes[m_pszMainSceneName]->GetVoxels()[i];
                // Set vertex buffer
                UINT aStrides[3] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(NormalData)),
                                     static_cast<UINT>(sizeof(InstanceData)) };
                UINT aOffsets[3] = { 0u, 0u, 0u };
                ID3D11Buffer* aBuffers[3]
                {
                    voxel->GetVertexBuffer().Get(),
                    voxel->GetNormalBuffer().Get(),
                    voxel->GetInstanceBuffer().Get()
                };

                m_immediateContext->IASetVertexBuffers(0u, 3u, aBuffers, aStrides, aOffsets);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(voxel->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(voxel->GetWorldMatrix()),
                    .OutputColor = voxel->GetOutputColor(),
                    .HasNormalMap = voxel->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(voxel->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(voxel->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(voxel->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                for (UINT uMeshIdx = 0u; uMeshIdx < voxel->GetNumMeshes(); ++uMeshIdx)
                {
                    UINT uMaterialIndex = voxel->GetMesh(uMeshIdx).uMaterialIndex;

                    assert(uMaterialIndex < voxel->GetNumMaterials() || uMaterialIndex == Renderable::INVALID_MATERIAL);

                    // Render a triangle
                    ID3D11ShaderResourceView* aViews[2]
                    {
                        m_invalidTexture->GetTextureResourceView().Get(),
                        m_invalidTexture->GetTextureResourceView().Get()
                    };

                    ID3D11SamplerState* aSamplers[2]
                    {
                        m_invalidTexture->GetSamplerState().Get(),
                        m_invalidTexture->GetSamplerState().Get()
                    };

                    if (uMaterialIndex != Renderable::INVALID_MATERIAL)
                    {
                        if (voxel->GetMaterial(uMaterialIndex)->pDiffuse)
                        {
                            aViews[0] = voxel->GetMaterial(uMaterialIndex)->pDiffuse->GetTextureResourceView().Get();
                            aSamplers[0] = voxel->GetMaterial(uMaterialIndex)->pDiffuse->GetSamplerState().Get();
                        }

                        if (voxel->GetMaterial(uMaterialIndex)->pNormal)
                        {
                            aViews[1] = voxel->GetMaterial(uMaterialIndex)->pNormal->GetTextureResourceView().Get();
                            aSamplers[1] = voxel->GetMaterial(uMaterialIndex)->pNormal->GetSamplerState().Get();
                        }
                    }

                    m_immediateContext->PSSetShaderResources(
                        0u,
                        2u,
                        aViews
                    );
                    m_immediateContext->PSSetSamplers(
                        0u,
                        2u,
                        aSamplers
                    );

                    m_immediateContext->DrawIndexedInstanced(
                        voxel->GetMesh(uMeshIdx).uNumIndices,
                        voxel->GetNumInstances(),
                        voxel->GetMesh(uMeshIdx).uBaseIndex,
                        static_cast<INT>(voxel->GetMesh(uMeshIdx).uBaseVertex),
                        0u
                    );
                }
            }

            for (auto it = m_scenes[m_pszMainSceneName]->GetModels().begin(); it != m_scenes[m_pszMainSceneName]->GetModels().end(); ++it)
            {
                // Set vertex buffer
                UINT aStrides[3] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(NormalData)),
                                     static_cast<UINT>(sizeof(AnimationData)) };
                UINT aOffsets[3] = { 0u, 0u, 0u };
                ID3D11Buffer* aBuffers[3]
                {
                    it->second->GetVertexBuffer().Get(),
                    it->second->GetNormalBuffer().Get(),
                    it->second->GetAnimationBuffer().Get()
                };

                m_immediateContext->IASetVertexBuffers(0u, 3u, aBuffers, aStrides, aOffsets);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .OutputColor = it->second->GetOutputColor(),
                    .HasNormalMap = it->second->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                CBSkinning* cbSkinning = reinterpret_cast<CBSkinning*>(malloc(sizeof(CBSkinning)));
                for (UINT i = 0u; i < it->second->GetBoneTransforms().size(); ++i)
                {
                    cbSkinning->BoneTransforms[i] = XMMatrixTranspose(it->second->GetBoneTransforms().at(i));
                }
                m_immediateContext->UpdateSubresource(it->second->GetSkinningConstantBuffer().Get(), 0u, nullptr, cbSkinning, 0u, 0u);

                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(4, 1, it->second->GetSkinningConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                for (UINT i = 0u; i < it->second->GetNumMeshes(); ++i)
                {
                    UINT uMaterialIndex = it->second->GetMesh(i).uMaterialIndex;

                    assert(uMaterialIndex < it->second->GetNumMaterials() || uMaterialIndex == Renderable::INVALID_MATERIAL);

                    // Render a triangle
                    ID3D11ShaderResourceView* aViews[2]
                    {
                        m_invalidTexture->GetTextureResourceView().Get(),
                        m_invalidTexture->GetTextureResourceView().Get()
                    };

                    ID3D11SamplerState* aSamplers[2]
                    {
                        m_invalidTexture->GetSamplerState().Get(),
                        m_invalidTexture->GetSamplerState().Get()
                    };

                    if (uMaterialIndex != Renderable::INVALID_MATERIAL)
                    {
                        if (it->second->GetMaterial(uMaterialIndex)->pDiffuse)
                        {
                            aViews[0] = it->second->GetMaterial(uMaterialIndex)->pDiffuse->GetTextureResourceView().Get();
                            aSamplers[0] = it->second->GetMaterial(uMaterialIndex)->pDiffuse->GetSamplerState().Get();
                        }

                        if (it->second->GetMaterial(uMaterialIndex)->pNormal)
                        {
                            aViews[1] = it->second->GetMaterial(uMaterialIndex)->pNormal->GetTextureResourceView().Get();
                            aSamplers[1] = it->second->GetMaterial(uMaterialIndex)->pNormal->GetSamplerState().Get();
                        }
                    }

                    m_immediateContext->PSSetShaderResources(
                        0u,
                        2u,
                        aViews
                    );
                    m_immediateContext->PSSetSamplers(
                        0u,
                        2u,
                        aSamplers
                    );

                    m_immediateContext->DrawIndexed(
                        it->second->GetMesh(i).uNumIndices,
                        it->second->GetMesh(i).uBaseIndex,
                        static_cast<INT>(it->second->GetMesh(i).uBaseVertex)
                    );
                }
            }

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::GetDriverType

          Summary:  Returns the Direct3D driver type

          Returns:  D3D_DRIVER_TYPE
                      The Direct3D driver type used
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        D3D_DRIVER_TYPE Renderer::GetDriverType() const
        {
            return m_driverType;
        }
    }
#endif

#ifdef LAB08
    namespace lab08
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView, m_vertexShader,
                      m_pixelShader, m_vertexLayout, m_vertexBuffer].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
            , m_depthStencil()
            , m_depthStencilView()
            , m_cbChangeOnResize()
            , m_pszMainSceneName(nullptr)
            , m_padding{ '\0' }
            , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
            , m_projection()
            , m_renderables()
            , m_aPointLights()
            , m_vertexShaders()
            , m_pixelShaders()
            , m_scenes()
        {
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
        HRESULT Renderer::Initialize(_In_ HWND hWnd)
        {
            HRESULT hr = S_OK;

            RECT rc;
            GetClientRect(hWnd, &rc);
            UINT uWidth = static_cast<UINT>(rc.right - rc.left);
            UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd =
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = {.Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd =
                {
                    .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = {.Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .OutputWindow = hWnd,
                    .Windowed = TRUE
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create depth stencil texture
            D3D11_TEXTURE2D_DESC descDepth =
            {
                .Width = uWidth,
                .Height = uHeight,
                .MipLevels = 1u,
                .ArraySize = 1u,
                .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };
            hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
            {
                .Format = descDepth.Format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D = {.MipSlice = 0 }
            };
            hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

            // Setup the viewport
            D3D11_VIEWPORT vp =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Create the constant buffers
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(CBChangeOnResize),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                .CPUAccessFlags = 0
            };
            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Initialize the projection matrix
            m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

            CBChangeOnResize cbChangesOnResize =
            {
                .Projection = XMMatrixTranspose(m_projection)
            };
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

            bd.ByteWidth = sizeof(CBLights);
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0u;

            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_camera.Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

            for (auto it = m_vertexShaders.begin(); it != m_vertexShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_pixelShaders.begin(); it != m_pixelShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }

            for (auto it = m_models.begin(); it != m_models.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }

            for (auto it = m_scenes.begin(); it != m_scenes.end(); ++it)
            {
                hr = it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
                if (FAILED(hr))
                {
                    return hr;
                }
            }

            if (!m_scenes.contains(m_pszMainSceneName))
            {
                return E_FAIL;
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddRenderable

          Summary:  Add a renderable object and initialize the object

          Args:     PCWSTR pszRenderableName
                      Key of the renderable object
                    const std::shared_ptr<Renderable>& renderable
                      Unique pointer to the renderable object

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code.
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
        {
            if (m_renderables.contains(pszRenderableName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName] = renderable;

            return S_OK;
        }

        HRESULT Renderer::AddModel(_In_ PCWSTR pszModelName, _In_ const std::shared_ptr<Model>& pModel)
        {
            if (m_models.contains(pszModelName))
            {
                return E_FAIL;
            }

            m_models[pszModelName] = pModel;

            return S_OK;
        }

        HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
        {
            HRESULT hr = S_OK;

            if (index >= NUM_LIGHTS)
            {
                return E_FAIL;
            }

            m_aPointLights[index] = pPointLight;

            return hr;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddVertexShader

          Summary:  Add the vertex shader into the renderer and initialize it

          Args:     PCWSTR pszVertexShaderName
                      Key of the vertex shader
                    const std::shared_ptr<VertexShader>&
                      Vertex shader to add

          Modifies: [m_vertexShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            if (m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_vertexShaders[pszVertexShaderName] = vertexShader;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddPixelShader

          Summary:  Add the pixel shader into the renderer and initialize it

          Args:     PCWSTR pszPixelShaderName
                      Key of the pixel shader
                    const std::shared_ptr<PixelShader>&
                      Pixel shader to add

          Modifies: [m_pixelShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            if (m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_pixelShaders[pszPixelShaderName] = pixelShader;

            return S_OK;
        }

        HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, const std::filesystem::path& sceneFileDirectory)
        {
            if (m_scenes.contains(pszSceneName))
            {
                return E_FAIL;
            }

            m_scenes[pszSceneName] = std::make_shared<Scene>(sceneFileDirectory);

            return S_OK;
        }

        HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
        {
            if (!m_scenes.contains(pszSceneName))
            {
                return E_FAIL;
            }

            m_pszMainSceneName = pszSceneName;

            return S_OK;
        }

        void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
        {
            m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Update

          Summary:  Update the renderables each frame

          Args:     FLOAT deltaTime
                      Time difference of a frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Update(_In_ FLOAT deltaTime)
        {
            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Update(deltaTime);
            }

            for (auto it = m_models.begin(); it != m_models.end(); ++it)
            {
                it->second->Update(deltaTime);
            }

            m_aPointLights[0]->Update(deltaTime);
            m_aPointLights[1]->Update(deltaTime);

            m_camera.Update(deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Clear the depth buffer to 1.0 (max depth)
            m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            CBChangeOnCameraMovement cbChangeOnCameraMovement =
            {
                .View = XMMatrixTranspose(m_camera.GetView())
            };
            XMStoreFloat4(&(cbChangeOnCameraMovement.CameraPosition), m_camera.GetEye());
            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cbChangeOnCameraMovement, 0, 0);

            CBLights cbLights = {};
            for (size_t i = 0; i < NUM_LIGHTS; ++i)
            {
                cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
                cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
            }

            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLights, 0, 0);

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                // Set vertex buffer
                UINT uStride = static_cast<UINT>(sizeof(SimpleVertex));
                UINT uOffset = 0u;

                m_immediateContext->IASetVertexBuffers(0u, 1u, it->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .OutputColor = it->second->GetOutputColor()
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                for (UINT i = 0u; i < it->second->GetNumMeshes(); ++i)
                {
                    UINT uMaterialIndex = it->second->GetMesh(i).uMaterialIndex;

                    assert(uMaterialIndex < it->second->GetNumMaterials());

                    // Render a triangle
                    if (it->second->GetMaterial(uMaterialIndex).pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(
                            0u,
                            1u,
                            it->second->GetMaterial(uMaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf()
                        );
                        m_immediateContext->PSSetSamplers(
                            0u,
                            1u,
                            it->second->GetMaterial(uMaterialIndex).pDiffuse->GetSamplerState().GetAddressOf()
                        );
                    }

                    m_immediateContext->DrawIndexed(
                        it->second->GetMesh(i).uNumIndices,
                        it->second->GetMesh(i).uBaseIndex,
                        static_cast<INT>(it->second->GetMesh(i).uBaseVertex)
                    );
                }
            }

            //for (std::unique_ptr<Renderable>& renderable : scene.GetRenderables())
            for (UINT i = 0u; i < m_scenes[m_pszMainSceneName]->GetVoxels().size(); ++i)
            {
                std::shared_ptr<Voxel> voxel = m_scenes[m_pszMainSceneName]->GetVoxels()[i];
                // Set vertex buffer
                UINT aStrides[2] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(InstanceData)) };
                UINT aOffsets[2] = { 0u, 0u };
                ID3D11Buffer* aBuffers[2]
                {
                    voxel->GetVertexBuffer().Get(),
                    voxel->GetInstanceBuffer().Get()
                };

                m_immediateContext->IASetVertexBuffers(0u, 2u, aBuffers, aStrides, aOffsets);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(voxel->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(voxel->GetWorldMatrix()),
                    .OutputColor = voxel->GetOutputColor(),
                    .DebugIndex = XMUINT2(0, 0)
                };
                m_immediateContext->UpdateSubresource(voxel->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(voxel->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(voxel->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                for (UINT uMeshIdx = 0u; uMeshIdx < voxel->GetNumMeshes(); ++uMeshIdx)
                {
                    UINT uMaterialIndex = voxel->GetMesh(uMeshIdx).uMaterialIndex;

                    assert(uMaterialIndex < voxel->GetNumMaterials());

                    // Render a triangle
                    if (voxel->GetMaterial(uMaterialIndex).pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(
                            0u,
                            1u,
                            voxel->GetMaterial(uMaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf()
                        );
                        m_immediateContext->PSSetSamplers(
                            0u,
                            1u,
                            voxel->GetMaterial(uMaterialIndex).pDiffuse->GetSamplerState().GetAddressOf()
                        );
                    }

                    m_immediateContext->DrawIndexedInstanced(
                        voxel->GetMesh(uMeshIdx).uNumIndices,
                        voxel->GetNumInstances(),
                        voxel->GetMesh(uMeshIdx).uBaseIndex,
                        static_cast<INT>(voxel->GetMesh(uMeshIdx).uBaseVertex),
                        0u
                    );
                }
            }

            for (auto it = m_models.begin(); it != m_models.end(); ++it)
            {
                // Set vertex buffer
                UINT aStrides[2] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(AnimationData)) };
                UINT aOffsets[2] = { 0u, 0u };
                ID3D11Buffer* aBuffers[2]
                {
                    it->second->GetVertexBuffer().Get(),
                    it->second->GetAnimationBuffer().Get()
                };

                m_immediateContext->IASetVertexBuffers(0u, 2u, aBuffers, aStrides, aOffsets);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .OutputColor = it->second->GetOutputColor(),
                    .DebugIndex = XMUINT2(0, 0)
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                CBSkinning* cbSkinning = reinterpret_cast<CBSkinning*>(malloc(sizeof(CBSkinning)));
                for (UINT i = 0u; i < it->second->GetBoneTransforms().size(); ++i)
                {
                    cbSkinning->BoneTransforms[i] = XMMatrixTranspose(it->second->GetBoneTransforms().at(i));
                }
                m_immediateContext->UpdateSubresource(it->second->GetSkinningConstantBuffer().Get(), 0u, nullptr, cbSkinning, 0u, 0u);

                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(4, 1, it->second->GetSkinningConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                for (UINT i = 0u; i < it->second->GetNumMeshes(); ++i)
                {
                    UINT uMaterialIndex = it->second->GetMesh(i).uMaterialIndex;

                    assert(uMaterialIndex < it->second->GetNumMaterials());

                    // Render a triangle
                    if (it->second->GetMaterial(uMaterialIndex).pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(
                            0u,
                            1u,
                            it->second->GetMaterial(uMaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf()
                            );
                        m_immediateContext->PSSetSamplers(
                            0u,
                            1u,
                            it->second->GetMaterial(uMaterialIndex).pDiffuse->GetSamplerState().GetAddressOf()
                            );
                    }

                    m_immediateContext->DrawIndexed(
                        it->second->GetMesh(i).uNumIndices,
                        it->second->GetMesh(i).uBaseIndex,
                        static_cast<INT>(it->second->GetMesh(i).uBaseVertex)
                        );
                }
            }

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetVertexShaderOfRenderable

          Summary:  Sets the vertex shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszVertexShaderName
                      Key of the vertex shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetPixelShaderOfRenderable

          Summary:  Sets the pixel shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszPixelShaderName
                      Key of the pixel shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

            return S_OK;
        }

        HRESULT Renderer::SetVertexShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszVertexShaderName)
        {
            if (!m_models.contains(pszModelName) || !m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_models[pszModelName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

            return S_OK;
        }

        HRESULT Renderer::SetPixelShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszPixelShaderName)
        {
            if (!m_models.contains(pszModelName) || !m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_models[pszModelName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetVertexShaderOfScene

          Summary:  Sets the vertex shader for the voxels in a scene

          Args:     PCWSTR pszSceneName
                      Key of the scene
                    PCWSTR pszVertexShaderName
                      Key of the vertex shader

          Modifies: [m_scenes].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
        {
            if (!m_scenes.contains(pszSceneName) || !m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            for (std::shared_ptr<Voxel>& voxel : m_scenes[pszSceneName]->GetVoxels())
            {
                voxel->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetPixelShaderOfScene

          Summary:  Sets the pixel shader for the voxels in a scene

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszPixelShaderName
                      Key of the pixel shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
        {
            if (!m_scenes.contains(pszSceneName) || !m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            for (std::shared_ptr<Voxel>& voxel : m_scenes[pszSceneName]->GetVoxels())
            {
                voxel->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
            }

            return S_OK;
        }


        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::GetDriverType

          Summary:  Returns the Direct3D driver type

          Returns:  D3D_DRIVER_TYPE
                      The Direct3D driver type used
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        D3D_DRIVER_TYPE Renderer::GetDriverType() const
        {
            return m_driverType;
        }
    }
#endif

#ifdef ASS02
    namespace ass02
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView, m_vertexShader,
                      m_pixelShader, m_vertexLayout, m_vertexBuffer].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
            , m_depthStencil()
            , m_depthStencilView()
            , m_cbChangeOnResize()
            , m_pszMainSceneName(nullptr)
            , m_padding{ '\0' }
            , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
            , m_projection()
            , m_renderables()
            , m_aPointLights()
            , m_vertexShaders()
            , m_pixelShaders()
            , m_scenes()
        {
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
        HRESULT Renderer::Initialize(_In_ HWND hWnd)
        {
            HRESULT hr = S_OK;

            RECT rc;
            GetClientRect(hWnd, &rc);
            UINT uWidth = static_cast<UINT>(rc.right - rc.left);
            UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd =
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = {.Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd =
                {
                    .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = {.Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .OutputWindow = hWnd,
                    .Windowed = TRUE
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create depth stencil texture
            D3D11_TEXTURE2D_DESC descDepth =
            {
                .Width = uWidth,
                .Height = uHeight,
                .MipLevels = 1u,
                .ArraySize = 1u,
                .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };
            hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
            {
                .Format = descDepth.Format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D = {.MipSlice = 0 }
            };
            hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

            // Setup the viewport
            D3D11_VIEWPORT vp =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Create the constant buffers
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(CBChangeOnResize),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                .CPUAccessFlags = 0
            };
            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Initialize the projection matrix
            m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 200.0f);

            CBChangeOnResize cbChangesOnResize =
            {
                .Projection = XMMatrixTranspose(m_projection)
            };
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

            bd.ByteWidth = sizeof(CBLights);
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0u;

            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_camera.Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

            for (auto it = m_vertexShaders.begin(); it != m_vertexShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_pixelShaders.begin(); it != m_pixelShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }

            for (auto it = m_scenes.begin(); it != m_scenes.end(); ++it)
            {
                hr = it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
                if (FAILED(hr))
                {
                    return hr;
                }
            }

            if (!m_scenes.contains(m_pszMainSceneName))
            {
                return E_FAIL;
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddRenderable

          Summary:  Add a renderable object and initialize the object

          Args:     PCWSTR pszRenderableName
                      Key of the renderable object
                    const std::shared_ptr<Renderable>& renderable
                      Unique pointer to the renderable object

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code.
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
        {
            if (m_renderables.contains(pszRenderableName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName] = renderable;

            return S_OK;
        }

        HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
        {
            HRESULT hr = S_OK;

            if (index >= NUM_LIGHTS)
            {
                return E_FAIL;
            }

            m_aPointLights[index] = pPointLight;

            return hr;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddVertexShader

          Summary:  Add the vertex shader into the renderer and initialize it

          Args:     PCWSTR pszVertexShaderName
                      Key of the vertex shader
                    const std::shared_ptr<VertexShader>&
                      Vertex shader to add

          Modifies: [m_vertexShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            if (m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_vertexShaders[pszVertexShaderName] = vertexShader;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddPixelShader

          Summary:  Add the pixel shader into the renderer and initialize it

          Args:     PCWSTR pszPixelShaderName
                      Key of the pixel shader
                    const std::shared_ptr<PixelShader>&
                      Pixel shader to add

          Modifies: [m_pixelShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            if (m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_pixelShaders[pszPixelShaderName] = pixelShader;

            return S_OK;
        }

        HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, const std::filesystem::path& sceneFilePath)
        {
            if (m_scenes.contains(pszSceneName))
            {
                return E_FAIL;
            }

            m_scenes[pszSceneName] = std::make_shared<Scene>(sceneFilePath);

            return S_OK;
        }

        HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
        {
            if (!m_scenes.contains(pszSceneName))
            {
                return E_FAIL;
            }

            m_pszMainSceneName = pszSceneName;

            return S_OK;
        }

        void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
        {
            m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Update

          Summary:  Update the renderables each frame

          Args:     FLOAT deltaTime
                      Time difference of a frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Update(_In_ FLOAT deltaTime)
        {
            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Update(deltaTime);
            }

            m_aPointLights[0]->Update(deltaTime);
            m_aPointLights[1]->Update(deltaTime);

            m_camera.Update(deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Clear the depth buffer to 1.0 (max depth)
            m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            CBChangeOnCameraMovement cbChangeOnCameraMovement =
            {
                .View = XMMatrixTranspose(m_camera.GetView())
            };
            XMStoreFloat4(&(cbChangeOnCameraMovement.CameraPosition), m_camera.GetEye());
            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cbChangeOnCameraMovement, 0, 0);

            CBLights cbLights = {};
            for (size_t i = 0; i < NUM_LIGHTS; ++i)
            {
                cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
                cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
            }
            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLights, 0, 0);

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                // Set vertex buffer
                UINT uStride = static_cast<UINT>(sizeof(SimpleVertex));
                UINT uOffset = 0u;

                m_immediateContext->IASetVertexBuffers(0u, 1u, it->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .OutputColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                for (UINT i = 0u; i < it->second->GetNumMeshes(); ++i)
                {
                    UINT uMaterialIndex = it->second->GetMesh(i).uMaterialIndex;

                    assert(uMaterialIndex < it->second->GetNumMaterials());

                    // Render a triangle
                    if (it->second->GetMaterial(uMaterialIndex).pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(
                            0u,
                            1u,
                            it->second->GetMaterial(uMaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf()
                        );
                        m_immediateContext->PSSetSamplers(
                            0u,
                            1u,
                            it->second->GetMaterial(uMaterialIndex).pDiffuse->GetSamplerState().GetAddressOf()
                        );
                    }

                    m_immediateContext->DrawIndexed(
                        it->second->GetMesh(i).uNumIndices,
                        it->second->GetMesh(i).uBaseIndex,
                        static_cast<INT>(it->second->GetMesh(i).uBaseVertex)
                        );
                }
            }

            //for (std::unique_ptr<Renderable>& renderable : scene.GetRenderables())
            for (UINT i = 0u; i < m_scenes[m_pszMainSceneName]->GetVoxels().size(); ++i)
            {
                std::shared_ptr<Voxel> voxel = m_scenes[m_pszMainSceneName]->GetVoxels()[i];
                // Set vertex buffer
                UINT aStrides[2] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(InstanceData)) };
                UINT aOffsets[2] = { 0u, 0u };
                ID3D11Buffer* aBuffers[2]
                {
                    voxel->GetVertexBuffer().Get(),
                    voxel->GetInstanceBuffer().Get()
                };

                m_immediateContext->IASetVertexBuffers(0u, 2u, aBuffers, aStrides, aOffsets);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(voxel->GetWorldMatrix()),
                    .OutputColor = voxel->GetOutputColor()
                };
                m_immediateContext->UpdateSubresource(voxel->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(voxel->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(voxel->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                for (UINT uMeshIdx = 0u; uMeshIdx < voxel->GetNumMeshes(); ++uMeshIdx)
                {
                    UINT uMaterialIndex = voxel->GetMesh(uMeshIdx).uMaterialIndex;

                    assert(uMaterialIndex < voxel->GetNumMaterials());

                    // Render a triangle
                    if (voxel->GetMaterial(uMaterialIndex).pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(
                            0u,
                            1u,
                            voxel->GetMaterial(uMaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf()
                        );
                        m_immediateContext->PSSetSamplers(
                            0u,
                            1u,
                            voxel->GetMaterial(uMaterialIndex).pDiffuse->GetSamplerState().GetAddressOf()
                        );
                    }

                    m_immediateContext->DrawIndexedInstanced(
                        voxel->GetMesh(uMeshIdx).uNumIndices,
                        voxel->GetNumInstances(),
                        voxel->GetMesh(uMeshIdx).uBaseIndex,
                        static_cast<INT>(voxel->GetMesh(uMeshIdx).uBaseVertex),
                        0u
                    );
                }
            }

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetVertexShaderOfRenderable

          Summary:  Sets the vertex shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszVertexShaderName
                      Key of the vertex shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetPixelShaderOfRenderable

          Summary:  Sets the pixel shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszPixelShaderName
                      Key of the pixel shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetVertexShaderOfScene

          Summary:  Sets the vertex shader for the voxels in a scene

          Args:     PCWSTR pszSceneName
                      Key of the scene
                    PCWSTR pszVertexShaderName
                      Key of the vertex shader

          Modifies: [m_scenes].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
        {
            if (!m_scenes.contains(pszSceneName) || !m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            for (std::shared_ptr<Voxel>& voxel : m_scenes[pszSceneName]->GetVoxels())
            {
                voxel->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetPixelShaderOfScene

          Summary:  Sets the pixel shader for the voxels in a scene

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszPixelShaderName
                      Key of the pixel shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
        {
            if (!m_scenes.contains(pszSceneName) || !m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            for (std::shared_ptr<Voxel>& voxel : m_scenes[pszSceneName]->GetVoxels())
            {
                voxel->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::GetDriverType

          Summary:  Returns the Direct3D driver type

          Returns:  D3D_DRIVER_TYPE
                      The Direct3D driver type used
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        D3D_DRIVER_TYPE Renderer::GetDriverType() const
        {
            return m_driverType;
        }
    }
#endif

#ifdef LAB07
    namespace lab07
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView, m_vertexShader,
                      m_pixelShader, m_vertexLayout, m_vertexBuffer].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
            , m_depthStencil()
            , m_depthStencilView()
            , m_cbChangeOnResize()
            //, m_padding{ '\0', }
            , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
            , m_projection()
            , m_renderables()
            , m_aPointLights()
            , m_vertexShaders()
            , m_pixelShaders()
        {
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
        HRESULT Renderer::Initialize(_In_ HWND hWnd)
        {
            HRESULT hr = S_OK;

            RECT rc;
            GetClientRect(hWnd, &rc);
            UINT uWidth = static_cast<UINT>(rc.right - rc.left);
            UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd =
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = {.Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd =
                {
                    .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = {.Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .OutputWindow = hWnd,
                    .Windowed = TRUE
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create depth stencil texture
            D3D11_TEXTURE2D_DESC descDepth =
            {
                .Width = uWidth,
                .Height = uHeight,
                .MipLevels = 1u,
                .ArraySize = 1u,
                .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };
            hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
            {
                .Format = descDepth.Format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D = {.MipSlice = 0 }
            };
            hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

            // Setup the viewport
            D3D11_VIEWPORT vp =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Create the constant buffers
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(CBChangeOnResize),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                .CPUAccessFlags = 0
            };
            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Initialize the projection matrix
            m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 100.0f);

            CBChangeOnResize cbChangesOnResize =
            {
                .Projection = XMMatrixTranspose(m_projection)
            };
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

            bd.ByteWidth = sizeof(CBLights);
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0u;

            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_camera.Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

            for (auto it = m_vertexShaders.begin(); it != m_vertexShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_pixelShaders.begin(); it != m_pixelShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddRenderable

          Summary:  Add a renderable object and initialize the object

          Args:     PCWSTR pszRenderableName
                      Key of the renderable object
                    const std::shared_ptr<Renderable>& renderable
                      Unique pointer to the renderable object

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code.
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
        {
            if (m_renderables.contains(pszRenderableName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName] = renderable;

            return S_OK;
        }

        HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
        {
            HRESULT hr = S_OK;

            if (index >= NUM_LIGHTS)
            {
                return E_FAIL;
            }

            m_aPointLights[index] = pPointLight;

            return hr;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddVertexShader

          Summary:  Add the vertex shader into the renderer and initialize it

          Args:     PCWSTR pszVertexShaderName
                      Key of the vertex shader
                    const std::shared_ptr<VertexShader>&
                      Vertex shader to add

          Modifies: [m_vertexShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            if (m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_vertexShaders[pszVertexShaderName] = vertexShader;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddPixelShader

          Summary:  Add the pixel shader into the renderer and initialize it

          Args:     PCWSTR pszPixelShaderName
                      Key of the pixel shader
                    const std::shared_ptr<PixelShader>&
                      Pixel shader to add

          Modifies: [m_pixelShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            if (m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_pixelShaders[pszPixelShaderName] = pixelShader;

            return S_OK;
        }

        void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
        {
            m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Update

          Summary:  Update the renderables each frame

          Args:     FLOAT deltaTime
                      Time difference of a frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Update(_In_ FLOAT deltaTime)
        {
            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Update(deltaTime);
            }

            m_aPointLights[0]->Update(deltaTime);
            m_aPointLights[1]->Update(deltaTime);

            m_camera.Update(deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Clear the depth buffer to 1.0 (max depth)
            m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            CBChangeOnCameraMovement cbChangeOnCameraMovement =
            {
                .View = XMMatrixTranspose(m_camera.GetView())
            };
            XMStoreFloat4(&(cbChangeOnCameraMovement.CameraPosition), m_camera.GetEye());
            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cbChangeOnCameraMovement, 0, 0);

            CBLights cbLights = {};
            for (size_t i = 0; i < NUM_LIGHTS; ++i)
            {
                cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
                cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
            }
            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLights, 0, 0);

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                // Set vertex buffer
                UINT uStride = sizeof(SimpleVertex);
                UINT uOffset = 0;
                m_immediateContext->IASetVertexBuffers(0u, 1u, it->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .OutputColor = it->second->GetOutputColor()
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                for (UINT i = 0u; i < it->second->GetNumMeshes(); ++i)
                {
                    UINT uMaterialIndex = it->second->GetMesh(i).uMaterialIndex;

                    assert(uMaterialIndex < it->second->GetNumMaterials());

                    // Render a triangle
                    if (it->second->GetMaterial(uMaterialIndex).pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(
                            0u, 
                            1u, 
                            it->second->GetMaterial(uMaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf()
                            );
                        m_immediateContext->PSSetSamplers(
                            0u, 
                            1u, 
                            it->second->GetMaterial(uMaterialIndex).pDiffuse->GetSamplerState().GetAddressOf()
                            );
                    }

                    if (it->second->GetMaterial(uMaterialIndex).pSpecularExponent)
                    {
                        m_immediateContext->PSSetShaderResources(
                            1u,
                            1u,
                            it->second->GetMaterial(uMaterialIndex).pSpecularExponent->GetTextureResourceView().GetAddressOf()
                        );
                        m_immediateContext->PSSetSamplers(
                            1u,
                            1u,
                            it->second->GetMaterial(uMaterialIndex).pSpecularExponent->GetSamplerState().GetAddressOf()
                        );
                    }

                    m_immediateContext->DrawIndexed(
                        it->second->GetMesh(i).uNumIndices,
                        it->second->GetMesh(i).uBaseIndex,
                        static_cast<INT>(it->second->GetMesh(i).uBaseVertex)
                        );
                }
            }

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetVertexShaderOfRenderable

          Summary:  Sets the vertex shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszVertexShaderName
                      Key of the vertex shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetPixelShaderOfRenderable

          Summary:  Sets the pixel shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszPixelShaderName
                      Key of the pixel shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::GetDriverType

          Summary:  Returns the Direct3D driver type

          Returns:  D3D_DRIVER_TYPE
                      The Direct3D driver type used
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        D3D_DRIVER_TYPE Renderer::GetDriverType() const
        {
            return m_driverType;
        }
    }
#endif

#ifdef LAB06
    namespace lab06
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView, m_vertexShader,
                      m_pixelShader, m_vertexLayout, m_vertexBuffer].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
            , m_depthStencil()
            , m_depthStencilView()
            , m_cbChangeOnResize()
            , m_cbLights()
            //, m_padding{ '\0', }
            , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
            , m_projection()
            , m_renderables()
            , m_aPointLights()
            , m_vertexShaders()
            , m_pixelShaders()
        {
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
        HRESULT Renderer::Initialize(_In_ HWND hWnd)
        {
            HRESULT hr = S_OK;

            RECT rc;
            GetClientRect(hWnd, &rc);
            UINT uWidth = static_cast<UINT>(rc.right - rc.left);
            UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd = 
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = { .Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd = 
                {
                    .BufferDesc = { .Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = { .Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .OutputWindow = hWnd,
                    .Windowed = TRUE
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create depth stencil texture
            D3D11_TEXTURE2D_DESC descDepth =
            {
                .Width = uWidth,
                .Height = uHeight,
                .MipLevels = 1u,
                .ArraySize = 1u,
                .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };
            hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = 
            {
                .Format = descDepth.Format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D = { .MipSlice = 0 }
            };
            hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

            // Setup the viewport
            D3D11_VIEWPORT vp =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Create the constant buffers
            D3D11_BUFFER_DESC bd = 
            {
                .ByteWidth = sizeof(CBChangeOnResize),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                .CPUAccessFlags = 0
            };
            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Initialize the projection matrix
            m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 100.0f);

            CBChangeOnResize cbChangesOnResize =
            {
                .Projection = XMMatrixTranspose(m_projection)
            };
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

            bd.ByteWidth = sizeof(CBLights);
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0u;

            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            CBLights cbLights = {};
            for (size_t i = 0; i < NUM_LIGHTS; ++i)
            {
                cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
                cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
            }
            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLights, 0, 0);

            m_camera.Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

            for (auto it = m_vertexShaders.begin(); it != m_vertexShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_pixelShaders.begin(); it != m_pixelShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddRenderable

          Summary:  Add a renderable object and initialize the object

          Args:     PCWSTR pszRenderableName
                      Key of the renderable object
                    const std::shared_ptr<Renderable>& renderable
                      Unique pointer to the renderable object

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code.
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
        {
            if (m_renderables.contains(pszRenderableName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName] = renderable;

            return S_OK;
        }

        HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
        {
            HRESULT hr = S_OK;

            if (index >= NUM_LIGHTS)
            {
                return E_FAIL;
            }

            m_aPointLights[index] = pPointLight;

            return hr;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddVertexShader

          Summary:  Add the vertex shader into the renderer and initialize it

          Args:     PCWSTR pszVertexShaderName
                      Key of the vertex shader
                    const std::shared_ptr<VertexShader>&
                      Vertex shader to add

          Modifies: [m_vertexShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            if (m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_vertexShaders[pszVertexShaderName] = vertexShader;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddPixelShader

          Summary:  Add the pixel shader into the renderer and initialize it

          Args:     PCWSTR pszPixelShaderName
                      Key of the pixel shader
                    const std::shared_ptr<PixelShader>&
                      Pixel shader to add

          Modifies: [m_pixelShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            if (m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_pixelShaders[pszPixelShaderName] = pixelShader;

            return S_OK;
        }

        void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
        {
            m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Update

          Summary:  Update the renderables each frame

          Args:     FLOAT deltaTime
                      Time difference of a frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Update(_In_ FLOAT deltaTime)
        {
            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Update(deltaTime);
            }

            for (UINT i = 0; i < NUM_LIGHTS; ++i)
            {
                m_aPointLights[i]->Update(deltaTime);
            }

            m_camera.Update(deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Clear the depth buffer to 1.0 (max depth)
            m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            CBChangeOnCameraMovement cbChangeOnCameraMovement =
            {
                .View = XMMatrixTranspose(m_camera.GetView())
            };
            XMStoreFloat4(&cbChangeOnCameraMovement.CameraPosition, m_camera.GetEye());
            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cbChangeOnCameraMovement, 0, 0);

            CBLights cbLights = {};
            for (size_t i = 0; i < NUM_LIGHTS; ++i)
            {
                cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
                cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
            }
            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLights, 0, 0);

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                // Set vertex buffer
                UINT uStride = sizeof(SimpleVertex);
                UINT uOffset = 0;
                m_immediateContext->IASetVertexBuffers(0u, 1u, it->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .OutputColor = it->second->GetOutputColor()
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                // Render a triangle
                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
                if (it->second->HasTexture())
                {
                    m_immediateContext->PSSetShaderResources(0, 1, it->second->GetTextureResourceView().GetAddressOf());
                    m_immediateContext->PSSetSamplers(0, 1, it->second->GetSamplerState().GetAddressOf());
                }
                m_immediateContext->DrawIndexed(it->second->GetNumIndices(), 0u, 0);   // 36 vertices needed for 12 triangles in a triangle list
            }

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetVertexShaderOfRenderable

          Summary:  Sets the vertex shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszVertexShaderName
                      Key of the vertex shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetPixelShaderOfRenderable

          Summary:  Sets the pixel shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszPixelShaderName
                      Key of the pixel shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::GetDriverType

          Summary:  Returns the Direct3D driver type

          Returns:  D3D_DRIVER_TYPE
                      The Direct3D driver type used
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        D3D_DRIVER_TYPE Renderer::GetDriverType() const
        {
            return m_driverType;
        }
    }
#endif

#ifdef LAB05
    namespace lab05
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView, m_vertexShader,
                      m_pixelShader, m_vertexLayout, m_vertexBuffer].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
            , m_depthStencil()
            , m_depthStencilView()
            , m_cbChangeOnResize()
            , m_padding{ '\0', }
            , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
            , m_projection()
            , m_renderables()
            , m_vertexShaders()
            , m_pixelShaders()
        {
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
        HRESULT Renderer::Initialize(_In_ HWND hWnd)
        {
            HRESULT hr = S_OK;

            RECT rc;
            GetClientRect(hWnd, &rc);
            UINT uWidth = static_cast<UINT>(rc.right - rc.left);
            UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd =
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = {.Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 2u,
                    .SwapEffect = DXGI_SWAP_EFFECT_DISCARD
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd =
                {
                    .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = {.Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 2u,
                    .OutputWindow = hWnd,
                    .Windowed = TRUE,
                    .SwapEffect = DXGI_SWAP_EFFECT_DISCARD
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create depth stencil texture
            D3D11_TEXTURE2D_DESC descDepth =
            {
                .Width = uWidth,
                .Height = uHeight,
                .MipLevels = 1u,
                .ArraySize = 1u,
                .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };
            hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
            {
                .Format = descDepth.Format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D = {.MipSlice = 0 }
            };
            hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

            // Setup the viewport
            D3D11_VIEWPORT vp =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Create the constant buffers
            D3D11_BUFFER_DESC bd = 
            {
                .ByteWidth = sizeof(CBChangeOnResize),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                .CPUAccessFlags = 0
            };
            hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Initialize the projection matrix
            m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 100.0f);

            CBChangeOnResize cbChangesOnResize;
            cbChangesOnResize.Projection = XMMatrixTranspose(m_projection);
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

            m_camera.Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

            for (auto it = m_vertexShaders.begin(); it != m_vertexShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_pixelShaders.begin(); it != m_pixelShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddRenderable

          Summary:  Add a renderable object and initialize the object

          Args:     PCWSTR pszRenderableName
                      Key of the renderable object
                    const std::shared_ptr<Renderable>& renderable
                      Unique pointer to the renderable object

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code.
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
        {
            if (m_renderables.contains(pszRenderableName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName] = renderable;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddVertexShader

          Summary:  Add the vertex shader into the renderer and initialize it

          Args:     PCWSTR pszVertexShaderName
                      Key of the vertex shader
                    const std::shared_ptr<VertexShader>&
                      Vertex shader to add

          Modifies: [m_vertexShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            if (m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_vertexShaders[pszVertexShaderName] = vertexShader;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddPixelShader

          Summary:  Add the pixel shader into the renderer and initialize it

          Args:     PCWSTR pszPixelShaderName
                      Key of the pixel shader
                    const std::shared_ptr<PixelShader>&
                      Pixel shader to add

          Modifies: [m_pixelShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            if (m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_pixelShaders[pszPixelShaderName] = pixelShader;

            return S_OK;
        }

        void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
        {
            m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Update

          Summary:  Update the renderables each frame

          Args:     FLOAT deltaTime
                      Time difference of a frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Update(_In_ FLOAT deltaTime)
        {
            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Update(deltaTime);
            }
            m_camera.Update(deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Clear the depth buffer to 1.0 (max depth)
            m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            CBChangeOnCameraMovement cbChangeOnCameraMovement =
            {
                .View = XMMatrixTranspose(m_camera.GetView())
            };
            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cbChangeOnCameraMovement, 0, 0);

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                // Set vertex buffer
                UINT uStride = sizeof(SimpleVertex);
                UINT uOffset = 0;
                m_immediateContext->IASetVertexBuffers(0u, 1u, it->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                CBChangesEveryFrame cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix())
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                // Render a triangle
                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShaderResources(0, 1, it->second->GetTextureResourceView().GetAddressOf());
                m_immediateContext->PSSetSamplers(0, 1, it->second->GetSamplerState().GetAddressOf());
                m_immediateContext->DrawIndexed(it->second->GetNumIndices(), 0u, 0u);   // 36 vertices needed for 12 triangles in a triangle list
            }

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetVertexShaderOfRenderable

          Summary:  Sets the vertex shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszVertexShaderName
                      Key of the vertex shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetPixelShaderOfRenderable

          Summary:  Sets the pixel shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszPixelShaderName
                      Key of the pixel shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::GetDriverType

          Summary:  Returns the Direct3D driver type

          Returns:  D3D_DRIVER_TYPE
                      The Direct3D driver type used
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        D3D_DRIVER_TYPE Renderer::GetDriverType() const
        {
            return m_driverType;
        }
    }
#endif

#ifdef ASS01
    namespace ass01
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView, m_vertexShader,
                      m_pixelShader, m_vertexLayout, m_vertexBuffer].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
            , m_depthStencil()
            , m_depthStencilView()
            //, m_padding{ '\0' }
            , m_camera(XMVectorSet(0.0f, 3.0f, -4.0f, 0.0f))
            , m_projection()
            , m_renderables()
            , m_vertexShaders()
            , m_pixelShaders()
        {
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
        HRESULT Renderer::Initialize(_In_ HWND hWnd)
        {
            HRESULT hr = S_OK;

            RECT rc;
            GetClientRect(hWnd, &rc);
            UINT uWidth = static_cast<UINT>(rc.right - rc.left);
            UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd = 
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = { .Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .SwapEffect = DXGI_SWAP_EFFECT_DISCARD
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd = 
                {
                    .BufferDesc = { .Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = { .Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .OutputWindow = hWnd,
                    .Windowed = FALSE,
                    .SwapEffect = DXGI_SWAP_EFFECT_DISCARD
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create depth stencil texture
            D3D11_TEXTURE2D_DESC descDepth =
            {
                .Width = uWidth,
                .Height = uHeight,
                .MipLevels = 1u,
                .ArraySize = 1u,
                .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .SampleDesc = { .Count = 1u, .Quality = 0u },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };
            hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = 
            {
                .Format = descDepth.Format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D = { .MipSlice = 0 }
            };
            hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

            // Setup the viewport
            D3D11_VIEWPORT vp = 
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Initialize the projection matrix
            m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 100.0f);

            for (auto it = m_vertexShaders.begin(); it != m_vertexShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_pixelShaders.begin(); it != m_pixelShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddRenderable

          Summary:  Add a renderable object and initialize the object

          Args:     PCWSTR pszRenderableName
                      Key of the renderable object
                    const std::shared_ptr<Renderable>& renderable
                      Unique pointer to the renderable object

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code.
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
        {
            if (m_renderables.contains(pszRenderableName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName] = renderable;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddVertexShader

          Summary:  Add the vertex shader into the renderer and initialize it

          Args:     PCWSTR pszVertexShaderName
                      Key of the vertex shader
                    const std::shared_ptr<VertexShader>&
                      Vertex shader to add

          Modifies: [m_vertexShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            if (m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_vertexShaders[pszVertexShaderName] = vertexShader;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddPixelShader

          Summary:  Add the pixel shader into the renderer and initialize it

          Args:     PCWSTR pszPixelShaderName
                      Key of the pixel shader
                    const std::shared_ptr<PixelShader>&
                      Pixel shader to add

          Modifies: [m_pixelShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            if (m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_pixelShaders[pszPixelShaderName] = pixelShader;

            return S_OK;
        }

        void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
        {
            m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Update

          Summary:  Update the renderables each frame

          Args:     FLOAT deltaTime
                      Time difference of a frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Update(_In_ FLOAT deltaTime)
        {
            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Update(deltaTime);
            }

            m_camera.Update(deltaTime);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Clear the depth buffer to 1.0 (max depth)
            m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                // Set vertex buffer
                UINT uStride = sizeof(SimpleVertex);
                UINT uOffset = 0;
                m_immediateContext->IASetVertexBuffers(0u, 1u, it->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                ConstantBuffer cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .View = XMMatrixTranspose(m_camera.GetView()),
                    .Projection = XMMatrixTranspose(m_projection)
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                // Render a triangle
                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->DrawIndexed(it->second->GetNumIndices(), 0u, 0u);   // 36 vertices needed for 12 triangles in a triangle list
            }

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetVertexShaderOfRenderable

          Summary:  Sets the vertex shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszVertexShaderName
                      Key of the vertex shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetPixelShaderOfRenderable

          Summary:  Sets the pixel shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszPixelShaderName
                      Key of the pixel shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::GetDriverType

          Summary:  Returns the Direct3D driver type

          Returns:  D3D_DRIVER_TYPE
                      The Direct3D driver type used
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        D3D_DRIVER_TYPE Renderer::GetDriverType() const
        {
            return m_driverType;
        }
    }
#endif

#ifdef LAB04
    namespace lab04
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView, m_vertexShader,
                      m_pixelShader, m_vertexLayout, m_vertexBuffer].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
            , m_depthStencil()
            , m_depthStencilView()
            //, m_padding{ '\0' }
            , m_view()
            , m_projection()
            , m_renderables()
            , m_vertexShaders()
            , m_pixelShaders()
        {
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
        HRESULT Renderer::Initialize(_In_ HWND hWnd)
        {
            HRESULT hr = S_OK;

            RECT rc;
            GetClientRect(hWnd, &rc);
            UINT uWidth = static_cast<UINT>(rc.right - rc.left);
            UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd = 
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = { .Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd = 
                {
                    .BufferDesc = { .Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = { .Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .OutputWindow = hWnd,
                    .Windowed = TRUE
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create depth stencil texture
            D3D11_TEXTURE2D_DESC descDepth =
            {
                .Width = uWidth,
                .Height = uHeight,
                .MipLevels = 1u,
                .ArraySize = 1u,
                .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .SampleDesc = { .Count = 1u, .Quality = 0u },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };
            hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = 
            {
                .Format = descDepth.Format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D = { .MipSlice = 0 }
            };
            hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

            // Setup the viewport
            D3D11_VIEWPORT vp = 
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Initialize the view matrix
            XMVECTOR eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
            XMVECTOR at = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            m_view = XMMatrixLookAtLH(eye, at, up);

            // Initialize the projection matrix
            m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 100.0f);

            for (auto it = m_vertexShaders.begin(); it != m_vertexShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_pixelShaders.begin(); it != m_pixelShaders.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get());
            }

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddRenderable

          Summary:  Add a renderable object and initialize the object

          Args:     PCWSTR pszRenderableName
                      Key of the renderable object
                    const std::shared_ptr<Renderable>& renderable
                      Unique pointer to the renderable object

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code.
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
        {
            if (m_renderables.contains(pszRenderableName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName] = renderable;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddVertexShader

          Summary:  Add the vertex shader into the renderer and initialize it

          Args:     PCWSTR pszVertexShaderName
                      Key of the vertex shader
                    const std::shared_ptr<VertexShader>&
                      Vertex shader to add

          Modifies: [m_vertexShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
        {
            if (m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_vertexShaders[pszVertexShaderName] = vertexShader;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::AddPixelShader

          Summary:  Add the pixel shader into the renderer and initialize it

          Args:     PCWSTR pszPixelShaderName
                      Key of the pixel shader
                    const std::shared_ptr<PixelShader>&
                      Pixel shader to add

          Modifies: [m_pixelShaders].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
        {
            if (m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_pixelShaders[pszPixelShaderName] = pixelShader;

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Update

          Summary:  Update the renderables each frame

          Args:     FLOAT deltaTime
                      Time difference of a frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Update(_In_ FLOAT deltaTime)
        {
            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                it->second->Update(deltaTime);
            }
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Clear the depth buffer to 1.0 (max depth)
            m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            for (auto it = m_renderables.begin(); it != m_renderables.end(); ++it)
            {
                // Set vertex buffer
                UINT uStride = sizeof(SimpleVertex);
                UINT uOffset = 0;
                m_immediateContext->IASetVertexBuffers(0u, 1u, it->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

                // Set index buffer
                m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                // Set the input layout
                m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

                // Update variables
                ConstantBuffer cb =
                {
                    .World = XMMatrixTranspose(it->second->GetWorldMatrix()),
                    .View = XMMatrixTranspose(m_view),
                    .Projection = XMMatrixTranspose(m_projection)
                };
                m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0u, nullptr, &cb, 0u, 0u);

                // Render a triangle
                m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0, 1, it->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0u);
                m_immediateContext->DrawIndexed(it->second->GetNumIndices(), 0u, 0u);   // 36 vertices needed for 12 triangles in a triangle list
            }

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetVertexShaderOfRenderable

          Summary:  Sets the vertex shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszVertexShaderName
                      Key of the vertex shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_vertexShaders.contains(pszVertexShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::SetPixelShaderOfRenderable

          Summary:  Sets the pixel shader for a renderable

          Args:     PCWSTR pszRenderableName
                      Key of the renderable
                    PCWSTR pszPixelShaderName
                      Key of the pixel shader

          Modifies: [m_renderables].

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
        {
            if (!m_renderables.contains(pszRenderableName) || !m_pixelShaders.contains(pszPixelShaderName))
            {
                return E_FAIL;
            }

            m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::GetDriverType

          Summary:  Returns the Direct3D driver type

          Returns:  D3D_DRIVER_TYPE
                      The Direct3D driver type used
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        D3D_DRIVER_TYPE Renderer::GetDriverType() const
        {
            return m_driverType;
        }
    }
#endif

#ifdef LAB03
    namespace lab03
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView, m_vertexShader,
                      m_pixelShader, m_vertexLayout, m_vertexBuffer].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
            , m_vertexShader()
            , m_pixelShader()
            , m_vertexLayout()
            , m_vertexBuffer()
        {
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
        HRESULT Renderer::Initialize(_In_ HWND hWnd)
        {
            HRESULT hr = S_OK;

            RECT rc;
            GetClientRect(hWnd, &rc);
            UINT uWidth = static_cast<UINT>(rc.right - rc.left);
            UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd = 
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = { .Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd = 
                {
                    .BufferDesc = { .Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = { .Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .OutputWindow = hWnd,
                    .Windowed = TRUE
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

            // Setup the viewport
            D3D11_VIEWPORT vp =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            // Compile the vertex shader
            ComPtr<ID3DBlob> vertexShaderBlob;
            hr = compileShaderFromFile(L"../Library/Shaders/Lab03.fxh", "VS", "vs_5_0", vertexShaderBlob.GetAddressOf());
            if (FAILED(hr))
            {
                MessageBox(
                    nullptr,
                    L"The FX file cannot be compiled. Please run this executable from the directory that contains the FX file.",
                    L"Error",
                    MB_OK
                );
                return hr;
            }

            // Create the vertex shader
            hr = m_d3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Define the input layout
            D3D11_INPUT_ELEMENT_DESC aLayouts[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            UINT uNumElements = ARRAYSIZE(aLayouts);

            // Create the input layout
            hr = m_d3dDevice->CreateInputLayout(aLayouts, uNumElements, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), m_vertexLayout.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Set the input layout
            m_immediateContext->IASetInputLayout(m_vertexLayout.Get());

            // Compile the pixel shader
            ComPtr<ID3DBlob> pixelShaderBlob;
            hr = compileShaderFromFile(L"../Library/Shaders/Lab03.fxh", "PS", "ps_5_0", pixelShaderBlob.GetAddressOf());
            if (FAILED(hr))
            {
                MessageBox(
                    nullptr,
                    L"The FX file cannot be compiled. Please run this executable from the directory that contains the FX file.",
                    L"Error",
                    MB_OK
                );
                return hr;
            }

            // Create the pixel shader
            hr = m_d3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Create vertex buffer
            SimpleVertex aVertices[] =
            {
                { XMFLOAT3(0.0f, 0.5f, 0.5f) },
                { XMFLOAT3(0.5f, -0.5f, 0.5f) },
                { XMFLOAT3(-0.5f, -0.5f, 0.5f) },
            };
            D3D11_BUFFER_DESC bd = {};
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(SimpleVertex) * 3;
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.CPUAccessFlags = 0;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = aVertices;
            hr = m_d3dDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            // Set vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0;
            m_immediateContext->IASetVertexBuffers(0u, 1u, m_vertexBuffer.GetAddressOf(), &uStride, &uOffset);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Render a triangle
            m_immediateContext->VSSetShader(m_vertexShader.Get(), nullptr, 0u);
            m_immediateContext->PSSetShader(m_pixelShader.Get(), nullptr, 0u);
            m_immediateContext->Draw(3u, 0u);

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::compileShaderFromFile

          Summary:  Helper for compiling shaders with D3DCompile

          Args:     PCWSTR pszFileName
                      A pointer to a constant null-terminated string that
                      contains the name of the file that contains the
                      shader code
                    PCSTR pszEntryPoint
                      A pointer to a constant null-terminated string that
                      contains the name of the shader entry point function
                      where shader execution begins
                    PCSTR pszShaderModel
                      A pointer to a constant null-terminated string that
                      specifies the shader target or set of shader
                      features to compile against
                    ID3DBlob** ppBlobOut
                      A pointer to a variable that receives a pointer to
                      the ID3DBlob interface that you can use to access
                      the compiled code

          Returns:  HRESULT
                      Status code
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        HRESULT Renderer::compileShaderFromFile(_In_ PCWSTR pszFileName, _In_ PCSTR pszEntryPoint, _In_ PCSTR pszShaderModel, _Outptr_ ID3DBlob** ppBlobOut)
        {
            HRESULT hr = S_OK;

            DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
            // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
            // Setting this flag improves the shader debugging experience, but still allows 
            // the shaders to be optimized and to run exactly the way they will run in 
            // the release configuration of this program.
            dwShaderFlags |= D3DCOMPILE_DEBUG;

            // Disable optimizations to further improve shader debugging
            dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            ComPtr<ID3DBlob> errorBlob;
            hr = D3DCompileFromFile(pszFileName, nullptr, nullptr, pszEntryPoint, pszShaderModel, dwShaderFlags, 0u, ppBlobOut, errorBlob.GetAddressOf());
            if (FAILED(hr))
            {
                if (errorBlob.Get())
                {
                    OutputDebugStringA(reinterpret_cast<PCSTR>(errorBlob->GetBufferPointer()));
                }
                return hr;
            }

            return S_OK;
        }
    }
#endif

#ifdef LAB02
    namespace lab02
    {
        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Renderer

          Summary:  Constructor

          Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                      m_immediateContext, m_immediateContext1, m_swapChain,
                      m_swapChain1, m_renderTargetView].
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        Renderer::Renderer()
            : m_driverType(D3D_DRIVER_TYPE_NULL)
            , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
            , m_d3dDevice()
            , m_d3dDevice1()
            , m_immediateContext()
            , m_immediateContext1()
            , m_swapChain()
            , m_swapChain1()
            , m_renderTargetView()
        {
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Initialize

          Summary:  Creates Direct3D device and swap chain

          Args:     HWND hWnd
                      Handle to the window

          Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                      m_d3dDevice1, m_immediateContext1, m_swapChain1,
                      m_swapChain, m_renderTargetView].

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

            UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
            uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };
            UINT numDriverTypes = ARRAYSIZE(driverTypes);

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };
            UINT numFeatureLevels = ARRAYSIZE(featureLevels);

            for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
            {
                m_driverType = driverTypes[driverTypeIndex];
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                    hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                        D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
                }

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
            ComPtr<IDXGIFactory1> dxgiFactory;
            {
                ComPtr<IDXGIDevice> dxgiDevice;
                hr = m_d3dDevice.As(&dxgiDevice);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IDXGIAdapter> adapter;
                    hr = dxgiDevice->GetAdapter(&adapter);
                    if (SUCCEEDED(hr))
                    {
                        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    }
                }
            }
            if (FAILED(hr))
            {
                return hr;
            }

            // Create swap chain
            ComPtr<IDXGIFactory2> dxgiFactory2;
            hr = dxgiFactory.As(&dxgiFactory2);
            if (SUCCEEDED(hr))
            {
                // DirectX 11.1 or later
                hr = m_d3dDevice.As(&m_d3dDevice1);
                if (SUCCEEDED(hr))
                {
                    m_immediateContext.As(&m_immediateContext1);
                }

                DXGI_SWAP_CHAIN_DESC1 sd = 
                {
                    .Width = uWidth,
                    .Height = uHeight,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .SampleDesc = { .Count = 1u, .Quality = 0u },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u
                };

                hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = m_swapChain1.As(&m_swapChain);
                }
            }
            else
            {
                // DirectX 11.0 systems
                DXGI_SWAP_CHAIN_DESC sd = 
                {
                    .BufferDesc = { .Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                    .SampleDesc = { .Count = 1, .Quality = 0 },
                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                    .BufferCount = 1u,
                    .OutputWindow = hWnd,
                    .Windowed = TRUE
                };

                hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
            }

            // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
            dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

            if (FAILED(hr))
            {
                return hr;
            }

            // Create a render target view
            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

            // Setup the viewport
            D3D11_VIEWPORT vp =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<FLOAT>(uWidth),
                .Height = static_cast<FLOAT>(uHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            m_immediateContext->RSSetViewports(1, &vp);

            return S_OK;
        }

        /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
          Method:   Renderer::Render

          Summary:  Render the frame
        M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
        void Renderer::Render()
        {
            // Clear the back buffer
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

            // Present the information rendered to the back buffer to the front buffer (the screen)
            m_swapChain->Present(0, 0);
        }
    }
#endif
}