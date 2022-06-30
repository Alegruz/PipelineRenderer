/*+===================================================================
  File:      GAME.H

  Summary:   Game header file contains declarations of functions
			 used for the lab samples of Game Graphics Programming 
             course.

  Classes:  Game

  ?2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "pch.h"

//#include "Renderer/Renderer.h"
//#include "Scene/Scene.h"
#include "Window/MainWindow.h"

namespace pr
{
    /*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
      Class:    Game

      Summary:  Main game engine class

      Methods:  Initialize
                  Initializes the components of the game
                Run
                  Runs the game loop
                GetGameName
                  Returns the name of the game
                GetWindow
                  Returns the reference to the unique pointer to the
                  main window
                GetRenderer
                  Returns the reference to the unique pointer to the
                  renderer
                Game
                  Constructor.
                ~Game
                  Destructor.
    C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
    class Game final
    {
    public:
        explicit Game(_In_ PCWSTR pszGameName) noexcept;
        Game(const Game& Other) = delete;
        Game(Game&& Other) = delete;
        Game& operator=(const Game& Other) = delete;
        Game& operator=(Game&& Other) = delete;
        ~Game() = default;

        HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow);
        INT Run();

        PCWSTR GetGameName() const;
        constexpr std::unique_ptr<MainWindow>& GetWindow() noexcept;
        //std::unique_ptr<Renderer>& GetRenderer();

    private:
        PCWSTR m_pszGameName;
        std::unique_ptr<MainWindow> m_pMainWindow;
        //std::unique_ptr<Renderer> m_renderer;
        //std::unordered_map<std::wstring, std::unique_ptr<Scene>> m_scenes;
        PCWSTR m_pszMainSceneName;
    };
}