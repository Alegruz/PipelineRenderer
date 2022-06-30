/*+===================================================================
  File:      BASEWINDOW.H

  Summary:   BaseWindow header file contains declarations of the
             base class of all windows used in the library.

  Classes: BaseWindow<DerivedType>

  Functions: InitWindow, InitDevice

  ?2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "pch.h"

#include "Input/Input.h"
#include "Window/BaseWindow.h"

namespace pr
{
    /*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
      Class:    MainWindow

      Summary:  Main window to be used in game engine

      Methods:  Initialize
                  Initializes window
                GetWindowClassName
                  Returns the name of the window class
                HandleMessage
                  Handles the messages
                MainWindow
                  Constructor.
                ~MainWindow
                  Destructor.
    C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
    class MainWindow : public BaseWindow<MainWindow>
    {
    public:
        MainWindow() = default;
        MainWindow(const MainWindow& other) = delete;
        MainWindow(MainWindow&& other) = delete;
        MainWindow& operator=(const MainWindow& other) = delete;
        MainWindow& operator=(MainWindow&& other) = delete;
        virtual ~MainWindow() = default;

        HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) override;
        PCWSTR GetWindowClassName() const override;
        LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) override;

        constexpr const KeyboardInput& GetKeyboardInput() const noexcept;
        const MouseRelativeMovement& GetMouseRelativeMovement() const;
        void ResetMouseMovement();

    private:
        KeyboardInput m_KeyboardInput;
        MouseRelativeMovement m_mouseRelativeMovement;
    };
}
