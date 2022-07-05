#include "pch.h"

#include "Window/MainWindow.h"

namespace pr
{
    MainWindow::MainWindow(const std::shared_ptr<EventManager>& pEventManager) noexcept
        : m_KeyboardInput()
        , m_pEventManager(pEventManager)
        , m_MouseInput()
        , m_bIsFullscreen(FALSE)
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::Initialize

      Summary:  Initializes main window

      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                  Is a flag that says whether the main application window
                  will be minimized, maximized, or shown normally
                PCWSTR pszWindowName
                  The window name

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT MainWindow::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName)
    {
        RAWINPUTDEVICE rid =
        {
            .usUsagePage = 0x01,    // MOUSE
            .usUsage = 0x02,
            .dwFlags = 0u,          // Default Flags
            .hwndTarget = nullptr
        };
        if (!RegisterRawInputDevices(&rid, 1u, sizeof(rid)))
        {
            return E_FAIL;
        }

        return initialize(
            hInstance,
            nCmdShow,
            pszWindowName,
            //WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            DEFAULT_WIDTH,
            DEFAULT_HEIGHT
        );
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetWindowClassName

      Summary:  Returns the name of the window class

      Returns:  PCWSTR
                  Name of the window class
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    PCWSTR MainWindow::GetWindowClassName() const
    {
        return L"LibraryWindowClass";
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::HandleMessage

      Summary:  Handles the messages

      Args:     UINT uMessage
                  Message code
                WPARAM wParam
                  Additional data the pertains to the message
                LPARAM lParam
                  Additional data the pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    LRESULT MainWindow::HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_CLOSE:
            if (MessageBox(
                m_hWnd,
                L"Really quit?",
                LPSZ_ENGINE_TITLE,
                MB_OKCANCEL) == IDOK
                )
            {
                HMENU hMenu = GetMenu(m_hWnd);
                if (hMenu)
                {
                    DestroyMenu(hMenu);
                }
                DestroyWindow(m_hWnd);
                UnregisterClass(GetWindowClassName(), m_hInstance);
            }
            // Else: user canceled. Do nothing.
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_INPUT:
        {
            UINT uDataSize = 0u;

            GetRawInputData(
                reinterpret_cast<HRAWINPUT>(lParam),
                RID_INPUT,
                nullptr,
                &uDataSize,
                sizeof(RAWINPUTHEADER)
            );
            if (uDataSize > 0)
            {
                std::unique_ptr<BYTE[]> rawData = std::make_unique<BYTE[]>(uDataSize);
                if (GetRawInputData(
                    reinterpret_cast<HRAWINPUT>(lParam),
                    RID_INPUT,
                    rawData.get(),
                    &uDataSize,
                    sizeof(RAWINPUTHEADER)
                ))
                {
                    RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawData.get());
                    if (raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        m_MouseInput.lRelativeX = raw->data.mouse.lLastX;
                        m_MouseInput.lRelativeY = raw->data.mouse.lLastY;

                        RECT rc;
                        RECT rc2;
                        POINT p1;
                        POINT p2;

                        GetWindowRect(m_hWnd, &rc2);
                        GetClientRect(m_hWnd, &rc);
                        p1.x = rc.left;
                        p1.y = rc.top;
                        p2.x = rc.right;
                        p2.y = rc.bottom;

                        ClientToScreen(m_hWnd, &p1);
                        ClientToScreen(m_hWnd, &p2);

                        rc.left = p1.x;
                        rc.top = rc2.top;
                        rc.right = p2.x;
                        rc.bottom = p2.y;

                        ClipCursor(&rc);
                    }
                }
            }

            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }
        case WM_KEYDOWN:
        {
            if (!m_KeyboardInput.IsButtonPressing(static_cast<BYTE>(wParam)))
            {
                constexpr const size_t KEY_NAME_SIZE = 32;
                WCHAR szKeyName[KEY_NAME_SIZE] = { L'\0', };
                GetKeyNameText(static_cast<LONG>(lParam), szKeyName, KEY_NAME_SIZE);
                OutputDebugString(L"Button ");
                OutputDebugString(szKeyName);
                OutputDebugString(L" Pressed!\n");
            }

            m_KeyboardInput.SetButton(static_cast<BYTE>(wParam));

            if (m_KeyboardInput.IsButtonPressed(VK_F11))
            {
                SetFullscreen(!m_bIsFullscreen);
                m_KeyboardInput.ProcessedButton(VK_F11);
                break;
            }
        }
            break;
        case WM_KEYUP:
            m_KeyboardInput.ClearButton(static_cast<BYTE>(wParam));
            {
                constexpr const size_t KEY_NAME_SIZE = 32;
                WCHAR szKeyName[KEY_NAME_SIZE] = { L'\0', };
                GetKeyNameText(static_cast<LONG>(lParam), szKeyName, KEY_NAME_SIZE);
                OutputDebugString(L"Button ");
                OutputDebugString(szKeyName);
                OutputDebugString(L" Released!\n");
            }
            break;
        case WM_MOUSEWHEEL:
        {
            m_MouseInput.hZDelta = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
        }
        break;
        case WM_SIZE:
        {
            RECT rc = {};
            ::GetClientRect(m_hWnd, &rc);
            INT nWidth = rc.right - rc.left;
            INT nHeight = rc.bottom - rc.top;

            EventMessage msg =
            {
                .Type = eEventType::RESIZE,
                .uParam0 = static_cast<UINT64>(nWidth),
                .uParam1 = static_cast<UINT64>(nHeight)
            };
            //m_pEventManager->AddEvent(msg);
            WCHAR szDebugMsg[64] = { L'\0', };
            swprintf_s(szDebugMsg, L"%d, %d\n", nWidth, nHeight);
            OutputDebugString(szDebugMsg);
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(m_hWnd, &ps);
            EndPaint(m_hWnd, &ps);
            break;
        }
        default:
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }

    void MainWindow::SetFullscreen(BOOL bSetFullscreen) noexcept
    {
        if (m_bIsFullscreen != bSetFullscreen)
        {
            m_bIsFullscreen = bSetFullscreen;

            if (m_bIsFullscreen)
            {
                // Store the current window dimensions so they can be restored 
                // when switching out of fullscreen state.
                ::GetWindowRect(m_hWnd, &m_WindowRect);

                // Set the window style to a borderless window so the client area fills
                // the entire screen.
                UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

                ::SetWindowLongW(m_hWnd, GWL_STYLE, windowStyle);

                // Query the name of the nearest display device for the window.
               // This is required to set the fullscreen dimensions of the window
               // when using a multi-monitor setup.
                HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
                MONITORINFOEX monitorInfo = {};
                monitorInfo.cbSize = sizeof(MONITORINFOEX);
                ::GetMonitorInfo(hMonitor, &monitorInfo);

                ::SetWindowPos(m_hWnd, HWND_TOP,
                    monitorInfo.rcMonitor.left,
                    monitorInfo.rcMonitor.top,
                    monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                    monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                    SWP_FRAMECHANGED | SWP_NOACTIVATE);

                ::ShowWindow(m_hWnd, SW_MAXIMIZE);
            }
            else
            {
                // Restore all the window decorators.
                ::SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

                ::SetWindowPos(m_hWnd, HWND_NOTOPMOST,
                    m_WindowRect.left,
                    m_WindowRect.top,
                    m_WindowRect.right - m_WindowRect.left,
                    m_WindowRect.bottom - m_WindowRect.top,
                    SWP_FRAMECHANGED | SWP_NOACTIVATE);

                ::ShowWindow(m_hWnd, SW_NORMAL);
            }
        }
    }

    KeyboardInput& MainWindow::GetKeyboardInput() noexcept
    {
        return m_KeyboardInput;
    }

    const KeyboardInput& MainWindow::GetKeyboardInput() const noexcept
    {
        return m_KeyboardInput;
    }

    const MouseInput& MainWindow::GetMouseInput() const
    {
        return m_MouseInput;
    }

    void MainWindow::ResetMouseMovement()
    {
        memset(&m_MouseInput, 0, sizeof(MouseInput));
    }
}
