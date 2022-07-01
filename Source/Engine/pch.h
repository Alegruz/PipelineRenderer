/*+===================================================================
  File:      COMMON.H

  Summary:   Common header file that contains common header files and
			 macros used for the Library project of Game Graphics
			 Programming course.

  ¨Ï 2022    Minha Ju
===================================================================+*/

#pragma once

#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#ifndef  UNICODE
#define UNICODE
#endif // ! UNICODE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // ! WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <shellapi.h>

#include <wrl.h>
#include <wrl/client.h>
#include <wrl/event.h>

// DirectX 12 specific headers
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <directxcolors.h>

// D3D12 extension library
#include <d3dx12.h>

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// STL Headers
#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cwctype>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ppltasks.h>
#include <functional>

#include "Resource.h"

constexpr LPCWSTR LPSZ_ENGINE_TITLE = L"Pipeline Renderer";

using namespace Microsoft::WRL;
using namespace DirectX;
