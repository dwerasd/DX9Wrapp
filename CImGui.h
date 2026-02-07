#pragma once


#include <unordered_map>

#include <DarkCore/DDef.h>
#include <DarkCore/DTypes.h>
#include <DarkCore/DColor.h>
#include <DarkCore/DString.h>
#include <DarkCore/DUtil.h>
#include <DarkCore/DLocale.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <ImGui/imgui_impl_dx9.h>
#include <ImGui/imgui_impl_win32.h>

#include <ImGui/implot.h>
#include <ImGui/implot_internal.h>
#pragma comment(lib, "ImGui")

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include "DX9Def.h"

#define _USE_LIB_IMGUI_

class C_IMGUI
{
protected:
	bool bVerticalSync{ false }, bResetDevice{ false };	// 수직동기화

	HWND hWnd{ nullptr };
	LPDIRECT3DDEVICE9 pDevice{ nullptr };
	D3DPRESENT_PARAMETERS g_d3dpp{};
	dk::_DCOLOR dkClearColor{};

	ImVec2 v2MainWindowSize{};	// 이건 ImGui 용


public:
	explicit C_IMGUI(bool _bVerticalSync = true);
	virtual ~C_IMGUI();

	void Init_ImGui(HWND _hWnd, LPDIRECT3DDEVICE9 _pDevice = nullptr, bool _bVerticalSync = true);
	virtual long Update_ImGui();
	void Draw_ImGui();
	void Destroy_ImGui();

	void Clear(DWORD _dwColor = 0, DWORD _dwFlags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, float _fZ = 1.0f, DWORD _dwStencil = 0, DWORD _dwIndex = 0);
	HRESULT BeginScene();
	void EndScene();
	void Present(HWND hDestWindowOverride = 0, LPRECT pDst = nullptr, LPRECT pSrc = nullptr, RGNDATA* pDirtyRegion = nullptr);
	void ResetDevice();
	void SetWindowSize(UINT _x, UINT _y);

	bool WndProc_ImGui(HWND _hWnd, UINT _nMessage, WPARAM _wParam, LPARAM _lParam);
};


//extern UMAP_UTF8_STRINGS umapUtf8Strings;


//extern UMAP_IMGUI_COLORS umapImGuiColors;

LPCSTR toUtf8(LPCSTR _pszData);
LPCSTR toUtf8(LPCWSTR _pwszData);

LPCSTR GetColorName(size_t _nIndex);
ImU32 GetColorOfText(LPCSTR _pColorName);
ImVec4 GetColorVec4OfText(LPCSTR _pColorName);