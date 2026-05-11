// DX9Font_DW.h: DirectWrite 기반 고품질 DX9 폰트 시스템.
// 기존 C_DX9_FONT (GDI + 외곽선/3D/LRU) 와 별개 — 라이브러리 회귀 방지 위해 신규 클래스.
//
// 글리프 추출: IDWriteFontFace + CreateGlyphRunAnalysis (sub-pixel positioning, OpenType,
//              Variable Font, 한글 hinting). 알파 비트맵 → A8R8G8B8 D3D9 텍스처 atlas.
// 패킹: shelf packing (글리프별 가변 크기, atlas 효율 ~80%).
// 렌더링: 자체 vertex buffer batch (D3DFVF_XYZRHW, sub-pixel 위치 보존).
//         BeginRender → RenderText × N → EndRender 패턴 (배치 1회 flush).
//
// 의존성: dwrite.lib (Windows Vista SP2+, 7/8/10/11). XP 미지원.
// D3D9 와 DirectWrite 는 서로 독립 — 동일 디바이스 위에서 함께 사용 가능.
#pragma once

#include "DX9Def.h"

#include <dwrite_3.h>
#include <wrl/client.h>

#include <unordered_map>

#pragma comment(lib, "dwrite.lib")


namespace dx9
{
	//============================================================================
	// 글리프 메타 (DirectWrite 정밀 메트릭)
	//============================================================================
	struct _DX9_FONT_DW_GLYPH
	{
		int16_t m_sOffsetX;		// pen X 기준 비트맵 left (음수 가능, DirectWrite bounds.left)
		int16_t m_sOffsetY;		// baseline 기준 비트맵 top (음수 = baseline 위)
		uint16_t m_uBmpWidth;	// 비트맵 픽셀 너비 (실제 글리프 크기)
		uint16_t m_uBmpHeight;
		uint16_t m_uAtlasX;		// atlas 내 좌상단
		uint16_t m_uAtlasY;
		float m_fAdvance;		// sub-pixel 정밀 advance
	};


	//============================================================================
	// 자체 vertex (XYZRHW: transformed, 2D, 픽셀 좌표 — w=1, -0.5 픽셀 보정 적용)
	//============================================================================
	struct _DX9_FONT_DW_VERTEX
	{
		float x, y, z, rhw;	// transformed 좌표 (rhw=1.0)
		D3DCOLOR dwColor;
		float u, v;
	};
	inline constexpr DWORD DX9_FONT_DW_FVF = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);


	//============================================================================
	// C_DX9_FONT_DW
	//============================================================================
	class C_DX9_FONT_DW
	{
	private:
		static constexpr uint32_t DEFAULT_ATLAS_SIZE = 1024;
		static constexpr uint32_t DEFAULT_MAX_QUADS = 4096;	// 한 BeginRender 안에서 최대 글자 수

		// DirectWrite 공유 자원 (라이브러리 단위 — DX12/DX11 의 factory 와 별도 인스턴스).
		static Microsoft::WRL::ComPtr<IDWriteFactory3> s_pFactory;
		static Microsoft::WRL::ComPtr<IDWriteFontSetBuilder1> s_pSetBuilder;
		static Microsoft::WRL::ComPtr<IDWriteFontCollection1> s_pCustomCollection;
		static bool s_bCustomDirty;

		static bool ensureFactory_();
		static bool ensureCustomCollection_();

		// 폰트 페이스 + 메트릭.
		Microsoft::WRL::ComPtr<IDWriteFontFace> m_pFace;
		float m_fEmSize;
		float m_fAscent;
		float m_fDescent;
		float m_fLineGap;
		uint32_t m_uDesignUnitsPerEm;

		// D3D9 디바이스 (소유 X — 호출자 디바이스).
		LPDIRECT3DDEVICE9 m_pDevice;

		// Atlas 텍스처 (MANAGED 풀 — 디바이스 lost/reset 자동 복원).
		LPDIRECT3DTEXTURE9 m_pTexture;
		uint32_t m_uAtlasSize;
		uint32_t m_uShelfX, m_uShelfY, m_uShelfH;
		bool m_bAtlasFull;

		// 글리프 캐시.
		std::unordered_map<wchar_t, _DX9_FONT_DW_GLYPH> m_mapGlyphs;

		// 자체 quad batch.
		LPDIRECT3DVERTEXBUFFER9 m_pVB;	// DYNAMIC | WRITEONLY, D3DPOOL_DEFAULT
		LPDIRECT3DINDEXBUFFER9 m_pIB;	// 정적, D3DPOOL_MANAGED
		uint32_t m_uMaxQuads;
		uint32_t m_uQuadCount;			// 현재 BeginRender~EndRender 사이 누적
		uint32_t m_uVBOffset;			// 다음 lock 시 NOOVERWRITE offset
		bool m_bRendering;				// BeginRender 활성

		// 렌더 상태 백업 (BeginRender 시 저장 → EndRender 시 복원).
		// 호출자의 다른 렌더링과 격리.
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

		// TTF process-private 등록.
		static bool RegisterFontFile(const wchar_t* _pPath);
		static void CleanupStatic();

		// 폰트 초기화.
		// _pDevice    : 호출자의 D3D9 디바이스 (Reset 책임 호출자).
		// _pFaceName  : 폰트 family (등록 TTF → 시스템 폰트 순 검색).
		// _uFontHeight: em 크기 픽셀 단위.
		// _bBold/_bItalic.
		// _uAtlasSize : 텍스처 한 변 (default 1024).
		// _uMaxQuads  : BeginRender 한 번에 그릴 수 있는 최대 글자 수 (default 4096).
		bool Initialize(LPDIRECT3DDEVICE9 _pDevice,
			const wchar_t* _pFaceName,
			uint32_t _uFontHeight,
			bool _bBold,
			bool _bItalic = false,
			uint32_t _uAtlasSize = DEFAULT_ATLAS_SIZE,
			uint32_t _uMaxQuads = DEFAULT_MAX_QUADS);

		void Shutdown();

		// 디바이스 lost/reset 처리 — vertex buffer (DEFAULT 풀) 재생성.
		// atlas (MANAGED) 와 글리프 캐시는 자동 보존.
		void OnLostDevice();
		bool OnResetDevice();

		// BeginRender ~ RenderText (가변 횟수) ~ EndRender 사이클.
		// EndRender 가 누적 quad batch 를 한 번에 DrawIndexedPrimitive.
		// 외부 디바이스 상태는 저장/복원되어 다른 렌더링과 격리.
		bool BeginRender();
		void EndRender();

		// 텍스트 출력 (BeginRender 이후).
		// _fX, _fY = 텍스트 좌상단 (ascent 자동 적용).
		// _dwColor = D3DCOLOR (ARGB).
		void RenderText(float _fX, float _fY, const wchar_t* _pText,
			D3DCOLOR _dwColor, float _fScale = 1.0f);

		void RenderTextFmt(float _fX, float _fY, D3DCOLOR _dwColor, float _fScale,
			const wchar_t* _pFmt, ...);

		// 너비 측정 (sub-pixel 정밀, 캐시 미스 글리프도 등록).
		float MeasureText(const wchar_t* _pText, float _fScale = 1.0f);

		// 폰트 메트릭 (스케일 적용).
		float GetAscent(float _fScale = 1.0f) const { return m_fAscent * _fScale; }
		float GetDescent(float _fScale = 1.0f) const { return m_fDescent * _fScale; }
		float GetLineHeight(float _fScale = 1.0f) const
		{
			return (m_fAscent + m_fDescent + m_fLineGap) * _fScale;
		}

		// 접근자.
		LPDIRECT3DTEXTURE9 GetAtlasTexture() const { return m_pTexture; }
		uint32_t GetFontHeightPixels() const { return static_cast<uint32_t>(m_fEmSize); }
		uint32_t GetCachedGlyphCount() const { return static_cast<uint32_t>(m_mapGlyphs.size()); }
		bool IsInitialized() const { return m_pTexture != nullptr && m_pFace != nullptr; }
	};

} // namespace dx9
