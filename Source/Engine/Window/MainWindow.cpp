#include "pch.h"

#include "Window/MainWindow.h"

namespace pr
{
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
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            800,
            600
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
                        m_MouseRelativeMovement.X = raw->data.mouse.lLastX;
                        m_MouseRelativeMovement.Y = raw->data.mouse.lLastY;

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
            if (!m_KeyboardInput.IsButtonPressed(static_cast<BYTE>(wParam)))
            {
                constexpr const size_t KEY_NAME_SIZE = 32;
                WCHAR szKeyName[KEY_NAME_SIZE] = { L'\0', };
                GetKeyNameText(static_cast<LONG>(lParam), szKeyName, KEY_NAME_SIZE);
                OutputDebugString(L"Button ");
                OutputDebugString(szKeyName);
                OutputDebugString(L" Pressed!\n");
            }
        }
            m_KeyboardInput.SetButton(static_cast<BYTE>(wParam));
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
            // Note that this tutorial does not handle resizing (WM_SIZE) requests,
            // so we created the window without the resize border.
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

    const KeyboardInput& MainWindow::GetKeyboardInput() const noexcept
    {
        return m_KeyboardInput;
    }

    const MouseRelativeMovement& MainWindow::GetMouseRelativeMovement() const
    {
        return m_MouseRelativeMovement;
    }

    void MainWindow::ResetMouseMovement()
    {
        memset(&m_MouseRelativeMovement, 0, sizeof(MouseRelativeMovement));
    }
}
