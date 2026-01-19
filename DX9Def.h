#pragma once


#include <directxsdk/d3d9.h>
#pragma comment(lib, "d3d9")

#include <directxsdk/d3dx9.h>
#pragma comment(lib, "d3dx9")

#include <directxsdk/dxerr.h>
#pragma comment(lib, "dxerr")
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#pragma comment(lib, "legacy_stdio_definitions")	// dxerr 에서 _vsntprintf 링크에러
#endif

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "uuid")

#include <mmsystem.h>
#pragma comment(lib, "winmm")

#define DIRECTSOUND_VERSION 0x800
#include <dsound.h>
#pragma comment(lib, "dsound")

#include <directxsdk/d3d9types.h>
#include <directxsdk/d3dx9math.h>
#include <directxsdk/d3dx9shader.h>

enum _DEVICE_STATUS_
{
	_DX9_DEVICE_LOST = 0
	, _DX9_DEVICE_OK
	, _DX9_DEVICE_RESTORED
	, _DX9_DEVICE_DESTROY
};

#if (_MSC_VER > 1600) && (__cplusplus >= 201103L)
constexpr size_t _MAX_UNIT_COUNT_ = (1 << 7);
#else
#define _MAX_UNIT_COUNT_	(1 << 7)
#endif

//#pragma warning(disable : 4201)
