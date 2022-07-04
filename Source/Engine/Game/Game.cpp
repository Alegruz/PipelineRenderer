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
        //, m_scenes()
        , m_pszMainSceneName(nullptr)
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


        HRESULT hr = m_pMainWindow->Initialize(hInstance, nCmdShow, m_pszGameName);
        CHECK_AND_RETURN_HRESULT(hr, L"Game::Initialize >> MainWindow::Initialize");

        hr = m_pRenderer->Initialize(m_pMainWindow->GetWindow());
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

                m_pRenderer->HandleInput(m_pMainWindow->GetKeyboardInput(), m_pMainWindow->GetMouseRelativeMovement(), deltaTime);
                m_pMainWindow->ResetMouseMovement();
                m_pRenderer->Update(deltaTime);
                m_pRenderer->Render();
            }
        }

        return static_cast<INT>(msg.wParam);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Game::GetGameName

      Summary:  Returns the name of the game

      Returns:  PCWSTR
                  Name of the game
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    PCWSTR Game::GetGameName() const
    {
        return m_pszGameName;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Game::GetWindow

      Summary:  Returns the main window

      Returns:  std::unique_ptr<MainWindow>&
                  The main window
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    constexpr std::unique_ptr<MainWindow>& Game::GetWindow() noexcept
    {
        return m_pMainWindow;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Game::GetRenderer

      Summary:  Returns the renderer

      Returns:  std::unique_ptr<Renderer>&
                  The renderer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    std::unique_ptr<Renderer>& Game::GetRenderer()
    {
        return m_pRenderer;
    }
}