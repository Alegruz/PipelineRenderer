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

#include "Event/EventManager.h"
#include "Graphics/Renderer.h"
#include "Scene/Scene.h"
#include "Window/MainWindow.h"

namespace pr
{
    // Forward Declarations

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
        static UINT64 GetNumFrames() noexcept;

    public:
        explicit Game(_In_ PCWSTR pszGameName) noexcept;
        Game(const Game& other) = delete;
        Game(Game&& other) = delete;
        Game& operator=(const Game& other) = delete;
        Game& operator=(Game&& other) = delete;
        ~Game() = default;

        HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow);
        INT Run();

        HRESULT AddScene(std::unique_ptr<Scene>&& pScene) noexcept;

    private:
        std::shared_ptr<EventManager> m_pEventManager;      // 16   >>  16

        PCWSTR m_pszGameName;                               // 8    >>  24
        std::unique_ptr<MainWindow> m_pMainWindow;          // 8    >>  32
        std::unique_ptr<Renderer> m_pRenderer;              // 8    >>  40
        std::unique_ptr<Scene> m_pScene;                    // 8    >>  48
        static UINT64 ms_uNumFrames;                                // 8    >>  56
    };
    static_assert(sizeof(Game) == 48);
}