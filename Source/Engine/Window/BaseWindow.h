/*+===================================================================
  File:      BASEWINDOW.H

  Summary:   BaseWindow header file contains declarations of the 
             base class of all windows used in the library.

  Classes: BaseWindow<DerivedType>

  ?2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "pch.h"

#include "Utility/Utility.h"

namespace pr
{
    /*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
      Class:    BaseWindow

      Summary:  An abstract base class from specific window

      Methods:  WindowProc
                  The window procedure of the window
                Initialize
                    Purely virtual function that initializes window
                GetWindowClassName
                    Purely virtual function that returns the name of
                    the window class
                HandleMessage
                    Purely virtual function that that handles the
                    messages
                GetWindow
                  Getter for the handle to the window
                BaseWindow
                  Constructor.
                ~BaseWindow
                  Destructor.
    C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
    template <class DerivedType>
    class BaseWindow
    {
    public:
        static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

        explicit constexpr BaseWindow() noexcept;
        BaseWindow(const BaseWindow& other) = delete;
        BaseWindow(BaseWindow&& other) = delete;
        BaseWindow& operator=(const BaseWindow& other) = delete;
        BaseWindow& operator=(BaseWindow&& other) = delete;
        virtual ~BaseWindow() = default;

        virtual HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) = 0;
        virtual PCWSTR GetWindowClassName() const = 0;
        virtual LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) = 0;

        HWND GetWindow() const;

    protected:
        HRESULT initialize(
            _In_ HINSTANCE hInstance,
            _In_ INT nCmdShow,
            _In_ PCWSTR pszWindowName,
            _In_ DWORD dwStyle,
            _In_opt_ INT x = CW_USEDEFAULT,
            _In_opt_ INT y = CW_USEDEFAULT,
            _In_opt_ INT nWidth = CW_USEDEFAULT,
            _In_opt_ INT nHeight = CW_USEDEFAULT,
            _In_opt_ HWND hWndParent = nullptr,
            _In_opt_ HMENU hMenu = nullptr
        );

        RECT m_WindowRect;
        HINSTANCE m_hInstance;
        HWND m_hWnd;
        LPCWSTR m_pszWindowName;
    };

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   BaseWindow<DerivedType>::WindowProc

      Summary:  Defines the behavior of the window?its appearance, how
                it interacts with the user, and so forth

      Args:     HWND hWnd
                  Handle to the window
                UINT uMessage
                  Message code
                WPARAM wParam
                  Additional data the pertains to the message
                LPARAM lParam
                  Additional data the pertains to the message

      Modifies: [m_hWnd].

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    template<class DerivedType>
    inline LRESULT BaseWindow<DerivedType>::WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        DerivedType* pThis = nullptr;

        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<DerivedType*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

            pThis->m_hWnd = hWnd;
        }
        else
        {
            pThis = reinterpret_cast<DerivedType*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   BaseWindow<DerivedType>::BaseWindow

      Summary:  Constructor

      Modifies: [m_hInstance, m_hWnd, m_pszWindowName].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    template<class DerivedType>
    inline constexpr BaseWindow<DerivedType>::BaseWindow() noexcept
        : m_WindowRect()
        , m_hInstance(nullptr)
        , m_hWnd(nullptr)
        , m_pszWindowName(nullptr)
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   BaseWindow<DerivedType>::GetWindow()

      Summary:  Returns the handle to the window

      Returns:  HWND
                  The handle to the window
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    template<class DerivedType>
    inline HWND BaseWindow<DerivedType>::GetWindow() const
    {
        return m_hWnd;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   BaseWindow<DerivedType>::initialize

      Summary:  Registers the window class and creates a window

      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                  Is a flag that says whether the main application window
                  will be minimized, maximized, or shown normally
                PCWSTR pszWindowName
                  The window name
                DWORD dwStyle
                  The style of the window being created
                INT x
                  The initial horizontal position of the window
                INT y
                  The initial vertical position of the window
                INT nWidth
                  The width, in device units, of the window
                INT nHeight
                  The height, in device units, of the window
                HWND hWndParent
                  A handle to the parent or owner window of the window
                  being created
                HMENU hMenu
                  A handle to a menu, or specifies a child-window
                  identifier depending on the window style

      Modifies: [m_hInstance, m_pszWindowName, m_hWnd].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    template<class DerivedType>
    inline HRESULT BaseWindow<DerivedType>::initialize(
        _In_ HINSTANCE hInstance,
        _In_ INT nCmdShow,
        _In_ PCWSTR pszWindowName,
        _In_ DWORD dwStyle,
        _In_opt_ INT x,
        _In_opt_ INT y,
        _In_opt_ INT nWidth,
        _In_opt_ INT nHeight,
        _In_opt_ HWND hWndParent,
        _In_opt_ HMENU hMenu)
    {
        // Register the window class
        WNDCLASSEX wcex =
        {
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = WindowProc,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = hInstance,
            .hIcon = LoadIcon(hInstance, reinterpret_cast<LPCWSTR>(IDI_TUTORIAL)),
            .hCursor = LoadCursor(nullptr, IDC_ARROW),
            .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
            .lpszMenuName = nullptr,
            .lpszClassName = GetWindowClassName(),
            .hIconSm = LoadIcon(wcex.hInstance, reinterpret_cast<LPCWSTR>(IDI_TUTORIAL))
        };

        if (!RegisterClassEx(&wcex))
        {
            DWORD dwError = GetLastError();

            MessageBox(
                nullptr,
                L"Call to RegisterEx failed!",
                LPSZ_ENGINE_TITLE,
                NULL
            );

            if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            {
                HRESULT hr = HRESULT_FROM_WIN32(dwError);
                CHECK_AND_RETURN_HRESULT(hr, L"BaseWindow<DerivedType>::initialize >> Call to RegisterEx failed!");
            }

            return E_FAIL;
        }

        // Create window
        m_hInstance = hInstance;
        m_pszWindowName = pszWindowName;
        m_WindowRect =
        { 
            .left = 0, 
            .top = 0, 
            .right = static_cast<LONG>(nWidth), 
            .bottom = static_cast<LONG>(nHeight) 
        };
        AdjustWindowRect(&m_WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

        m_hWnd = CreateWindow(
            GetWindowClassName(),                                       // Window class
            m_pszWindowName,                                            // Window text
            dwStyle,   // Window style

            // Size and Position
            x, y, m_WindowRect.right - m_WindowRect.left, m_WindowRect.bottom - m_WindowRect.top,
            hWndParent,    // Parent window
            hMenu,    // Menu
            m_hInstance,  // Instance handle
            this     // Additional application data
        );

        if (!m_hWnd)
        {
            DWORD dwError = GetLastError();

            MessageBox(
                nullptr,
                L"Call to CreateWindowEx failed!",
                LPSZ_ENGINE_TITLE,
                NULL
            );

            if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            {
                return HRESULT_FROM_WIN32(dwError);
            }

            return E_FAIL;
        }

        ShowWindow(m_hWnd, nCmdShow);

        return S_OK;
    }
}