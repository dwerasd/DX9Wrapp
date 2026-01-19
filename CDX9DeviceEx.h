#pragma once


#include <vector>

#include <DarkCore/DDef.h>
#include <DarkCore/DTypes.h>
#include <DarkCore/DColor.h>
#include <DarkCore/DMemory.h>
#include <DarkCore/DSingleton.h>

#include "DX9Def.h"
#include "DX9Types.h"
#include "DX9Vector2.h"
#include "DX9Vector3.h"
#include "DX9Vector4.h"
#include "DX9Matrix.h"
#include "DX9Viewport.h"



//#define LAYERED_WINDOW
namespace dx9
{
#if defined(LAYERED_WINDOW)
	struct _IMAGE
	{
		long nWidth;
		long nHeight;
		int nPitch;
		HDC hdc;
		HBITMAP hBitmap;
		BITMAPINFO info;
		LPBYTE pPixels;
	};
#endif
	class C_DX9_DEVICEEX
		: public dk::C_ALIGNED_ALLOCATION_POLICY
		//, public _DX9_RENDER_STATES
	{
	private:
		bool bWindowMode, bCursor, bVerticalSync;
		long nLastDeviceStatus;							// 마지막 디바이스 상태.

		HWND hWnd;
		dx9::DVECTOR2 v2DisplayPos;
		dx9::DVECTOR2 v2DisplaySize;

		dk::DRECT rectRender;
		
		LPDIRECT3D9EX				pDirect3D9Ex;		// 다이렉트 3D의 객체
		LPDIRECT3DDEVICE9EX			pDevice;		// 3D의 디바이스 장치
		LPD3DXSPRITE				pSprite;

		D3DCAPS9					d3dcaps;

		// DX9 상태 백업용(for 2D)
		LPDIRECT3DSTATEBLOCK9		pDX9StateBlock{ nullptr };
		dx9::DMATRIX9				matWorld, matView, matProjection;

		// 디렉 함수 호출 횟수를 줄이기 위해 상태를 저장해놓는다.
		DWORD dwRenderState[256];
		DWORD dwSamplerState[16];

		unsigned char bytAlphaBlend;

		void InitPresentParameters(D3DPRESENT_PARAMETERS* d3dpp);
		long CheckResourceFormat(D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage);

		void ClearDX9States();


	protected:
		virtual void OnShowCursor(bool b);

	public:
		C_DX9_DEVICEEX();
		~C_DX9_DEVICEEX();

		LPDIRECT3DDEVICE9EX Init(HWND _hWnd, dk::DSIZE _sizeScreen = { 3840, 2160 });
		void Destroy();

		_DEVICE_STATUS_ GetDeviceStatus();

		bool IsCursorVisible();
		bool ShowCursor(bool b);

		void OnLostDevice();
		void OnResetDevice();

		bool QueryFeature(_DX9_QUERY_FEATURE_TYPE_ feature);

#if defined(LAYERED_WINDOW)
		dk::DSIZE sizeLayeredScreen;
		_IMAGE imgBack;

		BOOL bInitLayeredWindow;
		LPDIRECT3DTEXTURE9 pLayeredTexture;
		LPDIRECT3DSURFACE9 pSurface, pRenderTargetSurface, pDepthStencilSurface;
		BOOL ImageCreate(_IMAGE* pImage);
		void ImageDestroy(_IMAGE* pImage);
		BOOL InitLayeredTexture(D3DFORMAT format, D3DFORMAT depthStencil);
		void CopyLayeredTextureImage();
		void RedrawLayeredWindow16();
#endif

		HRESULT BeginScene();
		void EndScene();
		void Clear(DWORD _dwFlags = D3DCLEAR_TARGET, DWORD _dwColor = DARK_COLOR_GRAY, float _fZ = 1.0f, DWORD _dwStencil = 0, DWORD _dwIndex = 0);
		void Present(HWND hDestWindowOverride = 0, LPRECT pDst = nullptr, LPRECT pSrc = nullptr, RGNDATA* pDirtyRegion = nullptr);

		HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);

		void SetRenderState(D3DRENDERSTATETYPE _State, DWORD _Value);
		void SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);

		LPDIRECT3DDEVICE9EX GetDevice() { return pDevice; }
		bool ResetDevice(dk::LPDSIZE _pSize = nullptr);

		LPD3DXSPRITE GetSprite() { return(pSprite); }

		long GetStatus();

		void Begin2D();
		void End2D();
	};
}