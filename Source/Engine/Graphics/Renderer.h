/*+===================================================================
  File:      RENDERER.H

  Summary:   Renderer header file contains declarations of Renderer 
             class used for the lab samples of Game Graphics 
             Programming course.

  Classes: Renderer

  ?2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "pch.h"

#include "Camera/Camera.h"
#include "Graphics/BaseCube.h"
#include "Graphics/CommandQueue.h"
#include "Input/Input.h"
//#include "Light/PointLight.h"
//#include "Model/Model.h"
//#include "Renderer/DataTypes.h"
//#include "Renderer/Renderable.h"
//#include "Scene/Scene.h"
//#include "Shader/PixelShader.h"
//#include "Shader/VertexShader.h"
//#include "Texture/RenderTexture.h"
//#include "Shader/ShadowVertexShader.h"

namespace pr
{
    /*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
      Class:    Renderer

      Summary:  Renderer initializes Direct3D, and renders renderable
                data onto the screen

      Methods:  Initialize
                  Creates Direct3D device and swap chain
                AddRenderable
                  Add a renderable object and initialize the object
                Update
                  Update the renderables each frame
                Render
                  Renders the frame
                GetDriverType
                  Returns the Direct3D driver type
                Renderer
                  Constructor.
                ~Renderer
                  Destructor.
    C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
    class Renderer final
    {
    public:
        explicit Renderer() noexcept;
        Renderer(const Renderer& other) = delete;
        Renderer(Renderer&& other) = delete;
        Renderer& operator=(const Renderer& other) = delete;
        Renderer& operator=(Renderer&& other) = delete;
        ~Renderer() noexcept;

        HRESULT Initialize(_In_ HWND hWnd);

        void HandleInput(_In_ KeyboardInput& input, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime);
        void Update(_In_ FLOAT deltaTime);
        void Render();

        HRESULT Resize(UINT uWidth, UINT uHeight) noexcept;

        D3D_DRIVER_TYPE GetDriverType() const;

        static constexpr const size_t NUM_FRAMEBUFFERS = 3;

    private:
        BOOL checkTearingSupport() const noexcept;

    private:
        Camera m_Camera;                                                        // 272      >>  272

        XMMATRIX m_Projection;                                                  // 64       >>  336

        ComPtr<ID3D12Device2> m_pDevice;                                        // 8 + 0    >>  352
        ComPtr<IDXGISwapChain4> m_pSwapChain;                                   // 8 + 8    >>  352
        ComPtr<ID3D12Resource> m_apBackBuffers[NUM_FRAMEBUFFERS];               // 16 + 0    >>  368 >>  8 + 0  >>  384
        ComPtr<ID3D12DescriptorHeap> m_pRtvDescriptorHeap;                      // 8 + 8    >>  384
        ComPtr<ID3D12Resource> m_pDepthBuffer;                                  // 8 + 0    >>  400
        ComPtr<ID3D12DescriptorHeap> m_pDsvDescriptorHeap;                      // 8 + 8    >>  400
        ComPtr<ID3D12RootSignature> m_RootSignature;                            // 8 + 0    >>  416
        ComPtr<ID3D12PipelineState> m_PipelineState;                            // 8 + 8    >>  416

        std::shared_ptr<CommandQueue> m_pDirectCommandQueue;                    // 16 + 0   >>  432
        std::shared_ptr<CommandQueue> m_pComputeCommandQueue;                   // 16 + 0   >>  448
        std::shared_ptr<CommandQueue> m_pCopyCommandQueue;                      // 16 + 0   >>  464

        D3D12_VIEWPORT m_Viewport;                                              // 16 + 0   >>  480 >>  8 + 0   >>  496
        D3D12_RECT m_ScissorsRect;                                              // 8 + 8    >>  496 >>  8 + 0   >>  512

        UINT m_uRtvDescriptorSize;                                              // 4 + 8    >>  512
        UINT m_uCurrentBackBufferIndex;                                         // 4 + 12   >>  512

        D3D_DRIVER_TYPE m_DriverType;                                           // 4 + 0    >>  528
        D3D_FEATURE_LEVEL m_FeatureLevel;                                       // 4 + 4    >>  528

        UINT64 m_auFrameFenceValues[NUM_FRAMEBUFFERS];                          // 8 + 8    >>  528 >>  16 + 0   >> 544

        std::shared_ptr<BaseCube> m_pBaseCube;                                  // 16 + 0   >>  560

        UINT m_uWidth;                                                          // 4 + 0    >>  576
        UINT m_uHeight;                                                         // 4 + 4    >>  576
        FLOAT m_FoV;                                                            // 4 + 8    >>  576

        BOOL m_bIsVSyncEnabled;                                                 // 4 + 12   >>  576
        BOOL m_bIsTearingSupported;                                             // 4 + 0    >>  592
        BOOL m_bIsFullScreen;                                                   // 4 + 4    >>  592
    };
    static_assert(sizeof(Renderer) % 16 == 0);
    static_assert(sizeof(Renderer) == 592);
}