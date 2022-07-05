#include "pch.h"

#include "Game/Game.h"
#include "Graphics/Renderer.h"
#include "Utility/Utility.h"
#include "Window/MainWindow.h"

namespace pr
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Game::Game

      Summary:  Constructor

      Args:     PCWSTR pszGameName
                  Name of the game

      Modifies: [m_pszGameName, m_mainWindow, m_renderer].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Game::Game(_In_ PCWSTR pszGameName) noexcept
        : m_pEventManager(std::make_unique<EventManager>())
        , m_pszGameName(pszGameName)
        , m_pMainWindow(std::make_unique<MainWindow>(m_pEventManager))
        , m_pRenderer(std::make_unique<Renderer>())
        , m_pScene(std::make_unique<Scene>())
        , m_uNumFrames(0u)
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Game::Initialize

      Summary:  Initializes the components of the game

      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                  Is a flag that says whether the main application window
                  will be minimized, maximized, or shown normally

      Modifies: [m_mainWindow, m_renderer].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Game::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
    {
        // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
        // Using this awareness context allows the client area of the window 
        // to achieve 100% scaling while still allowing non-client window content to 
        // be rendered in a DPI sensitive fashion.
        SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        HRESULT hr = S_OK;
        
        if (!m_pScene)
        {
            hr = E_FAIL;
            CHECK_AND_RETURN_HRESULT(hr, L"Game::Initialize >> Scene is nullptr");
        }

        hr = m_pMainWindow->Initialize(hInstance, nCmdShow, m_pszGameName);
        CHECK_AND_RETURN_HRESULT(hr, L"Game::Initialize >> MainWindow::Initialize");

        hr = m_pRenderer->Initialize(m_pMainWindow->GetWindow(), m_pScene);
        CHECK_AND_RETURN_HRESULT(hr, L"Game::Initialize >> Renderer::Initialize");

        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Game::Run

      Summary:  Runs the game loop

      Returns:  INT
                  Status code to return to the operating system
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    INT Game::Run()
    {
        MSG msg = { 0 };
        LARGE_INTEGER startingTime;
        LARGE_INTEGER endingTime;
        LARGE_INTEGER elapsedMicroseconds;
        LARGE_INTEGER frequency;

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&startingTime);

        WCHAR szDebugMsg[64] = { L'\0', };
        DOUBLE averageDeltaTime = 0.0;
        while (WM_QUIT != msg.message)
        {
            if (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                // Update our time
                QueryPerformanceCounter(&endingTime);
                elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
                elapsedMicroseconds.QuadPart *= 1000000;
                elapsedMicroseconds.QuadPart /= frequency.QuadPart;
                QueryPerformanceFrequency(&frequency);
                QueryPerformanceCounter(&startingTime);

                FLOAT deltaTime = static_cast<FLOAT>(elapsedMicroseconds.QuadPart) / 1000000.0f;
                //FLOAT fps = 1.0f / deltaTime;
                //swprintf_s(szDebugMsg, L"FPS: %f\n", fps);
                //OutputDebugString(szDebugMsg);

                size_t uNumEvents = m_pEventManager->GetSize();
                EventMessage* pEventMessages = m_pEventManager->GetEvents();
                for (size_t i = 0; i < uNumEvents; ++i)
                {
                    switch (pEventMessages[i].Type)
                    {
                    case eEventType::RESIZE:
                    {
                        m_pRenderer->Resize(static_cast<UINT>(pEventMessages[i].uParam0), static_cast<UINT>(pEventMessages[i].uParam1));
                    }
                    break;
                    default:
                        assert(false);
                        break;
                    }
                }

                m_pRenderer->HandleInput(m_pMainWindow->GetKeyboardInput(), m_pMainWindow->GetMouseInput(), deltaTime);
                m_pMainWindow->ResetMouseMovement();
                m_pRenderer->Update(deltaTime);
                if (FAILED(m_pRenderer->Render(m_pScene)))
                {
                    PostQuitMessage(0);
                }

                ++m_uNumFrames;
                averageDeltaTime += deltaTime;
            }
        }
        averageDeltaTime /= m_uNumFrames;
        swprintf_s(szDebugMsg, L"Average Delta Time: %lf, Average FPS: %lf\n", averageDeltaTime, 1.0 / averageDeltaTime);
        OutputDebugString(szDebugMsg);

        return static_cast<INT>(msg.wParam);
    }

    HRESULT Game::AddScene(std::unique_ptr<Scene>&& pScene) noexcept
    {
        if (!pScene)
        {
            return E_INVALIDARG;
        }

        m_pScene = std::move(pScene);

        return S_OK;
    }
}