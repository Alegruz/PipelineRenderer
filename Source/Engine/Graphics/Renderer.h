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
//#include "Light/PointLight.h"
//#include "Model/Model.h"
//#include "Renderer/DataTypes.h"
//#include "Renderer/Renderable.h"
//#include "Scene/Scene.h"
//#include "Shader/PixelShader.h"
//#include "Shader/VertexShader.h"
#include "Window/MainWindow.h"
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
        ~Renderer() = default;

        HRESULT Initialize(_In_ HWND hWnd);

        void HandleInput(_In_ const KeyboardInput& input, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime);
        void Update(_In_ FLOAT deltaTime);
        void Render();

        D3D_DRIVER_TYPE GetDriverType() const;

        static constexpr const size_t NUM_FRAMEBUFFERS = 3;

    private:
        Camera m_Camera;                                // 272

        XMMATRIX m_Projection;                          // 336

        ComPtr<ID3D12Device2> m_pDevice;                // 352
        ComPtr<ID3D12CommandQueue> m_pCommandQueue;     // 352
        ComPtr<IDXGISwapChain4> m_pSwapChain;           // 368
        ComPtr<ID3D12Resource> m_pBackBuffers[NUM_FRAMEBUFFERS];    // 384
        ComPtr<ID3D12GraphicsCommandList> m_pCommandList;           // 400
        ComPtr<ID3D12CommandAllocator> m_pCommandAllocators[NUM_FRAMEBUFFERS];  // 416
        ComPtr<ID3D12DescriptorHeap> m_pRTVDescriptorHeap;                      // 432

        UINT m_uRTVDescriptorSize;                      // 432
        UINT m_uCurrentBackBufferIndex;                 // 432

        D3D_DRIVER_TYPE m_DriverType;                   // 448
        D3D_FEATURE_LEVEL m_FeatureLevel;               // 448

        HANDLE m_FenceEvent;                            // 448
        ComPtr<ID3D12Fence> m_pFence;                   // 464
        UINT64 m_uFenceValue;                           // 464
        UINT64 m_aFrameFenceValues[NUM_FRAMEBUFFERS];   // 496

        BOOL m_bIsVSyncEnabled;                         // 496
        BOOL m_bIsTearingSupported;                     // 496
        BOOL m_bIsFullScreen;                           // 512
    };
    static_assert(sizeof(Renderer) % 16 == 0);
    static_assert(sizeof(Renderer) == 512);
}