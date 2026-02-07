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
	/// @brief DX9 디바이스 모드 열거형
	/// @details 2D 전용 또는 3D(2D GUI 포함) 모드 선택
	enum _E_DX9_DEVICE_MODE_
	{
		DX9_DEVICE_MODE_2D = 0,		///< 순수 2D 게임용 (Z-Buffer 비활성화, 단순 블렌딩)
		DX9_DEVICE_MODE_3D = 1		///< 3D 게임용 (Z-Buffer 활성화, 3D + 2D GUI)
	};
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
		_E_DX9_DEVICE_MODE_ eDeviceMode;				///< 디바이스 모드 (2D/3D)
		bool bWindowMode, bVerticalSync, bCursor;
#if defined(_USE_FAKE_VERTEX_)
		bool bUseFakeVertex;
#endif
		long nLastDeviceStatus;							// 마지막 디바이스 상태.

		HWND hWnd;
		_DVECTOR2 v2DisplayPos;
		_DVECTOR2 v2DisplaySize;

		dk::DRECT rectRender;

		LPDIRECT3D9					pDirect3D9;			// 다이렉트 3D의 객체
		LPDIRECT3DDEVICE9			pDevice;			// 3D의 디바이스 장치
		//LPD3DXSPRITE				pSprite;

		D3DCAPS9					d3dCaps;			// for Init
		_D3DFORMAT					d3dFormat;			// for FullScreen
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

		std::list<_DX9_VERTEX_BUFFER*>	listDX9VertexBuffers;
		std::list<_DX9_INDEX_BUFFER*>	listDX9IndexBuffers;

		std::list<_DX9_TEXTURE*>		listDX9Textures;

		std::list<_DX9_FONT*>			listDX9Fonts;

		//std::list<C_DX9_FONT*>	m_fonts;			// for LostDevice

		// DX9 상태 백업용(for 2D)
		LPDIRECT3DSTATEBLOCK9	pStateBlock;
		_DMATRIX9				matWorld, matView, matProjection;

		unsigned char bytAlphaBlend;

		long CheckResourceFormat(_D3DFORMAT _fmt, D3DRESOURCETYPE _resType, DWORD _dwUsage);
		void ClearDX9States();

		void InitDeviceDefault();

		/// @brief 2D 전용 모드 초기화 (Z-Buffer 비활성화)
		void Init2DMode();

		/// @brief 3D 모드 초기화 (Z-Buffer 활성화, 3D 렌더 스테이트)
		void Init3DMode();

		UINT GetSamplerNumberToSaveIndex(UINT _nStage);	// sampler 번호 -> 내부 저장용 index 로 변환
		UINT GetSaveIndexToSamplerNumber(UINT _nIndex);	//  내부 저장용 index -> sampler 번호 로 변환

	protected:
		virtual void OnShowCursor(bool b);

	public:
		C_DX9_DEVICE(bool _bWindowMode = true, bool _bVerticalSync = false);
		~C_DX9_DEVICE();

		/// @brief 디바이스 초기화
		/// @param _hWnd 윈도우 핸들
		/// @param _sizeScreen 화면 크기
		/// @param _eMode 디바이스 모드 (기본값: DX9_DEVICE_MODE_2D - 하위 호환성 유지)
		/// @return 생성된 Direct3D 디바이스 포인터
		LPDIRECT3DDEVICE9 Init(HWND _hWnd, dk::DSIZE _sizeScreen, _E_DX9_DEVICE_MODE_ _eMode = DX9_DEVICE_MODE_2D);

		/// @brief 현재 디바이스 모드 반환
		_E_DX9_DEVICE_MODE_ GetDeviceMode() const { return eDeviceMode; }

		/// @brief 3D 모드인지 확인
		bool Is3DMode() const { return (DX9_DEVICE_MODE_3D == eDeviceMode); }
#if defined(_USE_FAKE_VERTEX_)
		void InitFakeVertex();
		void SetScreenSize();
		void DrawTexture2D(
			_DX9_TEXTURE* _pTexture
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

		void InitPresentParameters(_DX9_PRESENT_PARAMETERS* _d3dpp);

		_DEVICE_STATUS_ GetDeviceStatus();

		bool IsCursorVisible();
		bool ShowCursor(bool b);

		void OnLostDevice();
		void RestoreDevice();

		bool QueryFeature(_DX9_QUERY_FEATURE_TYPE_ _feature);

#if defined(LAYERED_WINDOW)
		_IMAGE imgBack;

		bool bInitLayeredWindow;
		dk::DSIZE sizeLayeredScreen;
		LPDIRECT3DTEXTURE9 pLayeredTexture;
		LPDIRECT3DSURFACE9 pSurface, pRenderTargetSurface, pDepthStencilSurface;
		bool ImageCreate(_IMAGE* pImage);
		void ImageDestroy(_IMAGE* pImage);
		bool InitLayeredTexture(_D3DFORMAT format, _D3DFORMAT depthStencil);
		void CopyLayeredTextureImage();
		void RedrawLayeredWindow16();
#endif
		// 버텍스버퍼에서 구조체 사이즈는 LostDevice 때문에 복구용으로 저장한다
		_DX9_VERTEX_BUFFER* wrappCreateVertexBuffer(UINT _nStructSize, DWORD _nCreateCount, DWORD _dwFVF = 0, DWORD _dwFlags = 0, LPVOID _pData = nullptr);
		void DeleteVertexBuffer(_DX9_VERTEX_BUFFER* _pDX9VertexBuffer);

		_DX9_INDEX_BUFFER* wrappCreateIndexBuffer(UINT _nCreateCount, DWORD _dwFlags = 0, LPVOID _pData = nullptr);
		void DeleteIndexBuffer(_DX9_INDEX_BUFFER* _pDX9IndexBuffer);

		_DX9_TEXTURE* wrappCreateTexture(DWORD _nWidth = 0, DWORD _nHeight = 0, _D3DFORMAT _d3dFormat = D3DFMT_A8R8G8B8, DWORD _dwFlags = 0);
		_DX9_TEXTURE* wrappCreateTexture(LPCWSTR pFile);
		void DeleteTexture(_DX9_TEXTURE* _pDX9Texture);

		_DX9_FONT* wrappCreateFont(LPCWSTR _wszName, int _nSize, UINT _nWeight = FW_NORMAL, UINT _nCharset = DEFAULT_CHARSET, bool _bItalic = false, bool _bAntiAliased = false);
		void wrappDeleteFont(_DX9_FONT* _pDX9Font);

		HRESULT wrappBeginScene();
		void wrappEndScene();
		void wrappClear(DWORD _dwColor = DARK_COL32_BLACK, DWORD _dwFlags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, float _fZ = 1.0f, DWORD _dwStencil = 0, DWORD _dwIndex = 0);
		HRESULT wrappPresent(HWND hDestWindowOverride = 0, LPRECT pDst = nullptr, LPRECT pSrc = nullptr, RGNDATA* pDirtyRegion = nullptr);

		void wrappSetTexture(int nStage, _DX9_TEXTURE* _pDX9Texture);

		HRESULT wrappDrawIndexedPrimitive(D3DPRIMITIVETYPE _PrimitiveType, INT _nBaseVertexIndex, UINT _nMinVertexIndex, UINT _nNumVertices, UINT _nStartIndex, UINT _nPrimCount);

		void wrappSetRenderState(D3DRENDERSTATETYPE _State, DWORD _Value);
		void wrappSetSamplerState(DWORD _Sampler, D3DSAMPLERSTATETYPE _Type, DWORD _Value);

		HRESULT wrappTestCooperativeLevel() { return(pDevice->TestCooperativeLevel()); }

		void wrappSetAlphaRef(DWORD dwRef);
		void wrappSetAlphaFunc(_D3D_CMP_FUNC Func);

		void wrappSetFVF(DWORD _fvf);
		void wrappSetTextureStageState(int _nStage, _TEXTURE_STAGE_STATE_TYPE_ _nStageStateType, unsigned int _value);

		void ShaderOff();

		void wrappSetVertexBuffer(_DX9_VERTEX_BUFFER* _pDX9VertexBuffer, int _nStream = 0, UINT _nOffset = 0);
		void wrappSetIndexBuffer(_DX9_INDEX_BUFFER* _pDX9IndexBuffer);

		void wrappSetVertexShaderConstantF(UINT _StartRegister, const float* _pConstantData, UINT _Vector4fCount);

		void wrappSetViewport(dx9::LPDVIEWPORT9 _pViewport);
		void wrappSetViewport(DWORD _x, DWORD _y, DWORD _nWidth, DWORD _nHeight, float _fMinZ = 0.0f, float _fMaxZ = 1.0f);
		dx9::LPDVIEWPORT9 GetViewport();

		void wrappSetTextureFilter(int _nSampler, _TEXTURE_FILTER_TYPE _type);

		void wrappSetTransform(_SETTREANSFORM_TYPE _type, const _DMATRIX9* _matrix);
		_DMATRIX9 wrappGetTransform(_SETTREANSFORM_TYPE type) const;

		LPDIRECT3DDEVICE9 GetDevice() { return(pDevice); }

		void SetDisplaySize(WORD _nWidth, WORD _nHeight) { v2DisplaySize.Set(_nWidth, _nHeight); }
		_DVECTOR2* GetDisplayPos() { return(&v2DisplayPos); }
		_DVECTOR2* GetDisplaySize() { return(&v2DisplaySize); }

		bool ResetDevice(dk::LPDSIZE _pSize = nullptr);

		//LPD3DXSPRITE GetSprite() { return(pSprite); }

		long GetStatus();
		dk::DSIZE GetMaxTextureSize() { return((d3dCaps.MaxTextureWidth, d3dCaps.MaxTextureHeight)); }

		void Begin2D();
		void End2D();
	};
}