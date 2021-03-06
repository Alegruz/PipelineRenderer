/*+===================================================================
  File:      MAIN.CPP

  Summary:   This application demonstrates creating different pipelines

  Origin:    http://msdn.microsoft.com/en-us/library/windows/apps/ff729718.aspx

  Originally created by Microsoft Corporation under MIT License
  ?? 2022    Minha Ju
===================================================================+*/

#include "Game/Game.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/Model.h"
#include "Utility/Utility.h"

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: wWinMain

  Summary:  Entry point to the program. Initializes everything and
            goes into a message processing loop. Idle time is used to
            render the scene.

  Args:     HINSTANCE hInstance
              Handle to an instance.
            HINSTANCE hPrevInstance
              Has no meaning.
            LPWSTR lpCmdLine
              Contains the command-line arguments as a Unicode
              string
            INT nCmdShow
              Flag that says whether the main application window
              will be minimized, maximized, or shown normally

  Returns:  INT
              Status code.
-----------------------------------------------------------------F-F*/
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hr = S_OK;

    std::unique_ptr<pr::Game> pGame = std::make_unique<pr::Game>(L"Pipeline Renderer");

    std::unique_ptr<pr::Scene> pScene = std::make_unique<pr::Scene>();
    std::shared_ptr<pr::BaseCube> pCube = std::make_unique<pr::BaseCube>();
    
    hr = pScene->AddRenderable(L"Cube", pCube);
    CHECK_AND_RETURN_HRESULT(hr, L"Adding cube to scene");

    std::shared_ptr<pr::Model> pModel = std::make_shared<pr::Model>(L"Contents/Sponza/sponza.obj");
    //std::shared_ptr<pr::Model> pModel = std::make_shared<pr::Model>(L"Contents/Main/NewSponza_Main_FBX_YUp.fbx");
    pModel->Scale(0.1f, 0.1f, 0.1f);

    hr = pScene->AddRenderable(L"Model", pModel);
    CHECK_AND_RETURN_HRESULT(hr, L"Adding sponza model to scene");

    hr = pGame->AddScene(std::move(pScene));
    CHECK_AND_RETURN_HRESULT(hr, L"Adding scene");

    hr = pGame->Initialize(hInstance, nCmdShow);
    CHECK_AND_RETURN_HRESULT(hr, L"Game Initialization");

    return pGame->Run();
}