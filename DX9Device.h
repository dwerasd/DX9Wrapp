#pragma once


#include <array>
#include <vector>

#include <DarkCore/DDef.h>
#include <DarkCore/DTypes.h>
#include <DarkCore/DColor.h>
#include <DarkCore/DMemory.h>
#include <DarkCore/DSingleton.h>

#include "DX9Def.h"
#include "DX9Types.h"
#include "DX9DeviceState.h"
#include "DX9Types.h"
#include "DX9Vector2.h"
#include "DX9Vector3.h"
#include "DX9Vector4.h"
/*
#include "DX9Matrix.h"
#include "DX9Viewport.h"
#include "DX9Buffer.h"
#include "DX9Texture.h"
*/
#include "DX9Font.h"



//#define LAYERED_WINDOW
//#define _USE_FAKE_VERTEX_
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
	class C_DX9_DEVICE
		: public C_DX9_DEVICE_STATE
		//, public _DX9_RENDER_STATES
	{
	private:
		bool bWindowMode, bVerticalSync, bCursor;
#if defined(_USE_FAKE_VERTEX_)
		bool bUseFakeVertex;
#endif
		long nLastDeviceStatus;							// 마지막 디바이스 상태.

		HWND hWnd;
		DVECTOR2 v2DisplayPos;
		DVECTOR2 v2DisplaySize;

		dk::DRECT rectRender;

		LPDIRECT3D9					pDirect3D9;			// 다이렉트 3D의 객체
		LPDIRECT3DDEVICE9			pDevice;			// 3D의 디바이스 장치
		//LPD3DXSPRITE				pSprite;

		D3DCAPS9					d3dCaps;			// for Init
		D3DFORMAT					d3dFormat;			// for FullScreen
#if defined(_USE_FAKE_VERTEX_)
		LPDIRECT3DTEXTURE9			pTextureCombination;
		LPDIRECT3DVERTEXBUFFER9		pFakeVertexBuffer;
#endif
		//LPDIRECT3DSURFACE9		pFrameBuffer;			// 사용해봐야함
		//LPDIRECT3DSURFACE9		pDepthStencilBuffer;	// 사용해봐야함

		// 모든 버텍스 인덱스 버퍼는 여기에서 할당받고 관리해야한다 LostDevice 때문에
		//typedef std::unordered_map<UINT, _DX9_VERTEX_BUFFER*> UMAP_VERTEX_BUFFERS;
		//UMAP_VERTEX_BUFFERS umapVertexBuffers;
		//typedef std::unordered_map<UINT, _DX9_INDEX_BUFFER*> UMAP_INDEX_BUFFERS;
		//UMAP_INDEX_BUFFERS umapIndexBuffers;

		std::list<LPDX9_VERTEX_BUFFER>	listDX9VertexBuffers;
		std::list<LPDX9_INDEX_BUFFER>	listDX9IndexBuffers;

		std::list<LPDX9_TEXTURE>		listDX9Textures;

		std::list<LPDX9_FONT>			listDX9Fonts;

		//std::list<C_DX9_FONT*>	m_fonts;			// for LostDevice

		// DX9 상태 백업용(for 2D)
		LPDIRECT3DSTATEBLOCK9	pStateBlock;
		DMATRIX9				matWorld, matView, matProjection;

		unsigned char bytAlphaBlend;

		long CheckResourceFormat(D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage);
		void ClearDX9States();

		void InitDeviceDefault();

		UINT GetSamplerNumberToSaveIndex(UINT _nStage);	// sampler 번호 -> 내부 저장용 index 로 변환
		UINT GetSaveIndexToSamplerNumber(UINT _nIndex);	//  내부 저장용 index -> sampler 번호 로 변환

	protected:
		virtual void OnShowCursor(bool b);

	public:
		C_DX9_DEVICE(bool _bWindowMode = true, bool _bVerticalSync = false);
		~C_DX9_DEVICE();

		LPDIRECT3DDEVICE9 Init(HWND _hWnd, dk::DSIZE _sizeScreen);
#if defined(_USE_FAKE_VERTEX_)
		void InitFakeVertex();
		void SetScreenSize();
		void DrawTexture2D(
			LPDX9_TEXTURE _pTexture
			, dk::DRECT _rcDisplay
			, dk::DRECT _rcSource
			, DWORD _dwColor = D3DCOLOR_ARGB(255, 255, 255, 255)
			, BYTE _nBlendingType = 0
			, bool _bLighting = false
			, float _fAlpha = 1.0f
			, float _fScale = 1.0f
			, int _nAngle = 0
			, bool _bInvert = false
		);
#endif
		void Destroy();

		void InitPresentParameters(LPDX9_PRESENT_PARAMETERS d3dpp);

		_DEVICE_STATUS_ GetDeviceStatus();

		bool IsCursorVisible();
		bool ShowCursor(bool b);

		void OnLostDevice();
		void RestoreDevice();

		bool QueryFeature(_DX9_QUERY_FEATURE_TYPE_ feature);

#if defined(LAYERED_WINDOW)
		_IMAGE imgBack;

		bool bInitLayeredWindow;
		dk::DSIZE sizeLayeredScreen;
		LPDIRECT3DTEXTURE9 pLayeredTexture;
		LPDIRECT3DSURFACE9 pSurface, pRenderTargetSurface, pDepthStencilSurface;
		bool ImageCreate(_IMAGE* pImage);
		void ImageDestroy(_IMAGE* pImage);
		bool InitLayeredTexture(D3DFORMAT format, D3DFORMAT depthStencil);
		void CopyLayeredTextureImage();
		void RedrawLayeredWindow16();
#endif
		// 버텍스버퍼에서 구조체 사이즈는 LostDevice 때문에 복구용으로 저장한다
		LPDX9_VERTEX_BUFFER wrappCreateVertexBuffer(UINT _nStructSize, DWORD _nCreateSize, DWORD _dwFVF = 0, DWORD _dwFlags = 0, LPVOID _pData = nullptr);
		void DeleteVertexBuffer(LPDX9_VERTEX_BUFFER _pDX9VertexBuffer);

		LPDX9_INDEX_BUFFER wrappCreateIndexBuffer(UINT _nIndices, DWORD _dwFlags = 0, LPVOID _data = nullptr);
		void DeleteIndexBuffer(LPDX9_INDEX_BUFFER _pDX9IndexBuffer);

		LPDX9_TEXTURE wrappCreateTexture(DWORD _nWidth = 0, DWORD _nHeight = 0, D3DFORMAT _d3dFormat = D3DFMT_A8R8G8B8, DWORD _dwFlags = 0);
		LPDX9_TEXTURE wrappCreateTexture(LPCWSTR pFile);
		void DeleteTexture(LPDX9_TEXTURE _pDX9Texture);

		LPDX9_FONT wrappCreateFont(LPCWSTR _wszName, int _nSize, UINT _nWeight = FW_NORMAL, UINT _nCharset = DEFAULT_CHARSET, bool _bItalic = false, bool _bAntiAliased = false);
		void wrappDeleteFont(LPDX9_FONT _pDX9Font);

		HRESULT wrappBeginScene();
		void wrappEndScene();
		void wrappClear(DWORD _dwColor = DARK_COL32_BLACK, DWORD _dwFlags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, float _fZ = 1.0f, DWORD _dwStencil = 0, DWORD _dwIndex = 0);
		HRESULT wrappPresent(HWND hDestWindowOverride = 0, LPRECT pDst = nullptr, LPRECT pSrc = nullptr, RGNDATA* pDirtyRegion = nullptr);

		void wrappSetTexture(int nStage, LPDX9_TEXTURE pTexture);

		HRESULT wrappDrawIndexedPrimitive(D3DPRIMITIVETYPE _PrimitiveType, INT _nBaseVertexIndex, UINT _nMinVertexIndex, UINT _nNumVertices, UINT _nStartIndex, UINT _nPrimCount);

		void wrappSetRenderState(D3DRENDERSTATETYPE _State, DWORD _Value);
		void wrappSetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);

		HRESULT wrappTestCooperativeLevel() { return(pDevice->TestCooperativeLevel()); }

		void wrappSetAlphaRef(DWORD dwRef);
		void wrappSetAlphaFunc(_D3D_CMP_FUNC Func);

		void wrappSetFVF(DWORD _fvf);
		void wrappSetTextureStageState(int nStage, _TEXTURE_STAGE_STATE_TYPE_ nStageStateType, unsigned int value);

		void ShaderOff();

		void wrappSetVertexBuffer(LPDX9_VERTEX_BUFFER _pDX9VertexBuffer, int _nStream = 0, UINT _nOffset = 0);
		void wrappSetIndexBuffer(LPDX9_INDEX_BUFFER pDX9IndexBuffer);

		void wrappSetVertexShaderConstantF(UINT _StartRegister, const float* _pConstantData, UINT _Vector4fCount);

		void wrappSetViewport(dx9::LPDVIEWPORT9 _pViewport);
		void wrappSetViewport(DWORD _x, DWORD _y, DWORD _nWidth, DWORD _nHeight, float _fMinZ = 0.0f, float _fMaxZ = 1.0f);
		dx9::LPDVIEWPORT9 GetViewport();

		void wrappSetTextureFilter(int nSampler, _TEXTURE_FILTER_TYPE type);

		void wrappSetTransform(_SETTREANSFORM_TYPE type, const DMATRIX9* matrix);
		DMATRIX9 wrappGetTransform(_SETTREANSFORM_TYPE type) const;

		LPDIRECT3DDEVICE9 GetDevice() { return(pDevice); }

		void SetDisplaySize(WORD _nWidth, WORD _nHeight) { v2DisplaySize.Set(_nWidth, _nHeight); }
		LPDVECTOR2 GetDisplayPos() { return(&v2DisplayPos); }
		LPDVECTOR2 GetDisplaySize() { return(&v2DisplaySize); }

		bool ResetDevice(dk::LPDSIZE _pSize = nullptr);

		//LPD3DXSPRITE GetSprite() { return(pSprite); }

		long GetStatus();
		dk::DSIZE GetMaxTextureSize() { return((d3dCaps.MaxTextureWidth, d3dCaps.MaxTextureHeight)); }

		void Begin2D();
		void End2D();
	};
}