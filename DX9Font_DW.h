// DX9Font_DW.h: 동적 글리프 아틀라스 폰트 시스템 (GDI 기반, JotaX RFont 패턴).
// 기존 C_DX9_FONT (D3DXFont 래퍼) 와 별개 — 라이브러리 회귀 방지 위해 신규 클래스.
//
// 패턴 — JotaX C_GAME_UI 의 RFont (on-demand GDI 글리프 → D3D9 텍스처 셀 업로드).
// 자체 D3D9 동적 텍스처 (D3DPOOL_MANAGED, A8R8G8B8) + 자체 vertex buffer batch.
// 클래스명 _DW 는 "고품질 폰트" 의미로 유지 (DirectWrite 의미 X — GDI 사용).
// 의존성 0 (DirectWrite/FreeType 사용 안 함, OS GDI 만 사용).
#pragma once

#include "DX9Def.h"

#include <unordered_map>


namespace dx9
{
	//============================================================================
	// 글리프 메타 (셀 인덱스 + advance/높이)
	//============================================================================
	struct _DX9_FONT_DW_GLYPH
	{
		uint16_t m_uCellIndex;	// 아틀라스 셀 인덱스 (0 ~ cellCount-1)
		uint8_t m_uWidth;		// 글리프 advance 너비 (픽셀)
		uint8_t m_uHeight;
	};


	//============================================================================
	// 자체 vertex (XYZRHW: transformed 2D, w=1, -0.5 픽셀 보정)
	//============================================================================
	struct _DX9_FONT_DW_VERTEX
	{
		float x, y, z, rhw;
		D3DCOLOR dwColor;
		float u, v;
	};
	inline constexpr DWORD DX9_FONT_DW_FVF = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);


	//============================================================================
	// C_DX9_FONT_DW — GDI 글리프 아틀라스 + D3D9 자체 batch
	//============================================================================
	class C_DX9_FONT_DW
	{
	private:
		static constexpr uint32_t DEFAULT_ATLAS_SIZE = 1024;
		static constexpr uint32_t DEFAULT_CELL_SIZE = 32;
		static constexpr uint32_t DEFAULT_MAX_QUADS = 4096;

		// D3D9 디바이스 (소유 X).
		LPDIRECT3DDEVICE9 m_pDevice;

		// 아틀라스 텍스처 (MANAGED — 디바이스 lost/reset 자동 보존).
		LPDIRECT3DTEXTURE9 m_pTexture;
		uint32_t m_uAtlasSize;		// 텍스처 한 변
		uint32_t m_uCellSize;		// 셀 한 변
		uint32_t m_uCellsX;
		uint32_t m_uCellsY;
		uint32_t m_uCellCount;

		// 폰트 + GDI 글리프 렌더링.
		uint32_t m_uFontHeight;
		HDC m_hDC;
		HFONT m_hFont;
		HBITMAP m_hBitmap;
		uint32_t* m_pBits;			// DIBSection 픽셀 (cellSize × cellSize)

		// 글리프 캐시.
		std::unordered_map<wchar_t, _DX9_FONT_DW_GLYPH> m_mapGlyphs;
		uint32_t m_uNextCell;

		// 자체 quad batch.
		LPDIRECT3DVERTEXBUFFER9 m_pVB;	// DYNAMIC | WRITEONLY, D3DPOOL_DEFAULT
		LPDIRECT3DINDEXBUFFER9 m_pIB;	// 정적, D3DPOOL_MANAGED
		uint32_t m_uMaxQuads;
		uint32_t m_uQuadCount;
		uint32_t m_uVBOffset;
		bool m_bRendering;

		// 렌더 상태 백업 (BeginRender ~ EndRender 격리).
		struct _STATE_BACKUP
		{
			DWORD dwFVF;
			IDirect3DVertexShader9* pVS;
			IDirect3DPixelShader9* pPS;
			IDirect3DBaseTexture9* pTex0;
			IDirect3DIndexBuffer9* pIB;
			IDirect3DVertexBuffer9* pVB;
			UINT uVBOffset, uVBStride;
			DWORD aRS[12];
			DWORD aTSS[6];
			DWORD aSamp[5];
		};
		_STATE_BACKUP m_Backup;

		bool createBuffers_();
		void releaseBuffers_();
		const _DX9_FONT_DW_GLYPH* getGlyph_(wchar_t _wChar);
		void flushBatch_();
		void saveState_();
		void restoreState_();
		void setupState_();
		void appendQuad_(float _fX, float _fY, float _fW, float _fH,
			float _fU0, float _fV0, float _fU1, float _fV1, D3DCOLOR _dwColor);

	public:
		C_DX9_FONT_DW();
		~C_DX9_FONT_DW();
		C_DX9_FONT_DW(const C_DX9_FONT_DW&) = delete;
		C_DX9_FONT_DW& operator=(const C_DX9_FONT_DW&) = delete;

		// 폰트 초기화.
		// _pDevice    : 호출자의 D3D9 디바이스 (Reset 책임 호출자).
		// _pFaceName  : 폰트 family (시스템 설치 폰트, 예 L"맑은 고딕").
		// _uFontHeight: em 크기 픽셀 단위.
		// _bBold      : 굵게.
		// _uCellSize  : 셀 한 변 (default 32). _uFontHeight 보다 충분히 커야 함.
		// _uAtlasSize : 텍스처 한 변 (default 1024). _uCellSize 의 배수.
		// _uMaxQuads  : BeginRender 한 번에 그릴 수 있는 최대 글자 수.
		bool Initialize(LPDIRECT3DDEVICE9 _pDevice,
			const wchar_t* _pFaceName,
			uint32_t _uFontHeight,
			bool _bBold,
			uint32_t _uCellSize = DEFAULT_CELL_SIZE,
			uint32_t _uAtlasSize = DEFAULT_ATLAS_SIZE,
			uint32_t _uMaxQuads = DEFAULT_MAX_QUADS);

		void Shutdown();

		// 디바이스 lost/reset — DEFAULT 풀 VB 재생성.
		void OnLostDevice();
		bool OnResetDevice();

		// BeginRender ~ RenderText × N ~ EndRender 사이클.
		bool BeginRender();
		void EndRender();

		// 텍스트 출력 (BeginRender 이후).
		void RenderText(float _fX, float _fY, const wchar_t* _pText,
			D3DCOLOR _dwColor, float _fScale = 1.0f);

		void RenderTextFmt(float _fX, float _fY, D3DCOLOR _dwColor, float _fScale,
			const wchar_t* _pFmt, ...);

		// 너비 측정.
		float MeasureText(const wchar_t* _pText, float _fScale = 1.0f);

		// 접근자.
		LPDIRECT3DTEXTURE9 GetAtlasTexture() const { return m_pTexture; }
		uint32_t GetFontHeight() const { return m_uFontHeight; }
		uint32_t GetCellSize() const { return m_uCellSize; }
		uint32_t GetCachedGlyphCount() const { return m_uNextCell; }
		bool IsInitialized() const { return m_pTexture != nullptr && m_hFont != nullptr; }
	};

} // namespace dx9
