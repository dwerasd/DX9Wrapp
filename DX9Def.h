#pragma once

// GUID 심볼 정의를 위해 DirectX 헤더 전에 포함 필수
#include <initguid.h>
#pragma comment(lib, "dxguid")
#pragma comment(lib, "uuid")

#include <directxsdk/d3d9.h>
#pragma comment(lib, "d3d9")

#include <directxsdk/d3dx9.h>
#pragma comment(lib, "d3dx9")

#include <directxsdk/dxerr.h>
#pragma comment(lib, "dxerr")

#if (_MSC_VER > 1900)
#pragma comment(lib, "legacy_stdio_definitions.lib")	// dxerr 에서 _vsntprintf 링크에러
#endif

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8")


#include <mmsystem.h>
#pragma comment(lib, "winmm")

#define DIRECTSOUND_VERSION 0x800
#include <dsound.h>
#pragma comment(lib, "dsound")

#pragma comment(lib, "imm32")

#include <directxsdk/d3d9types.h>
#if (_MSC_VER > 1900)
#include <directxsdk/d3dx9math.h>
#include <directxsdk/d3dx9shader.h>
#endif

enum _DEVICE_STATUS_
{
	_DX9_DEVICE_LOST = 0
	, _DX9_DEVICE_OK
	, _DX9_DEVICE_RESTORED
	, _DX9_DEVICE_DESTROY
};

//#define USE_D3D
#define MAX_UNIT_COUNT	(1 << 7)

#pragma warning(disable : 4201)
