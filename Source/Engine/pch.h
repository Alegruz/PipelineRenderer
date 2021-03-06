/*+===================================================================
  File:      COMMON.H

  Summary:   Common header file that contains common header files and
			 macros used for the Library project of Game Graphics
			 Programming course.

  ?? 2022    Minha Ju
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

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

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
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ppltasks.h>
#include <functional>

#include "Resource.h"

using namespace Microsoft::WRL;
using namespace DirectX;

namespace pr
{
	const std::filesystem::path CONTENTS_PATH(L"Contents");
	const std::filesystem::path SHADERS_PATH(CONTENTS_PATH / L"Shaders");

	constexpr LPCWSTR LPSZ_ENGINE_TITLE = L"Pipeline Renderer";
	constexpr const size_t DEFAULT_WIDTH = 1280;
	constexpr const size_t DEFAULT_HEIGHT = 720;
	
	inline constexpr size_t ConvertKbToBytes(size_t kilobytes)
	{
		return kilobytes * 1024;
	}

	inline constexpr size_t ConvertMbToBytes(size_t megabytes)
	{
		return megabytes * 1024 * 1024;
	}

	constexpr const size_t _64KB = ConvertKbToBytes(64);
	constexpr const size_t _1MB = ConvertMbToBytes(1);
	constexpr const size_t _2MB = ConvertMbToBytes(2);
	constexpr const size_t _4MB = ConvertMbToBytes(4);
	constexpr const size_t _8MB = ConvertMbToBytes(8);
	constexpr const size_t _16MB = ConvertMbToBytes(16);
	constexpr const size_t _32MB = ConvertMbToBytes(32);
	constexpr const size_t _64MB = ConvertMbToBytes(64);
	constexpr const size_t _128MB = ConvertMbToBytes(128);
	constexpr const size_t _256MB = ConvertMbToBytes(256);
}
