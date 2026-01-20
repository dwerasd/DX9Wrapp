#pragma once

/**
 * @file DX9Font.h
 * @brief DirectX 9 고성능 폰트 시스템
 * @details RealSpace3의 RFont를 대체하는 최적화된 폰트 렌더링 시스템
 *          GDI 기반 글리프 래스터라이징 + 텍스처 아틀라스 캐싱
 *
 * @author DX9Wrapp
 * @date 2026-01-20
 *
 * 사용 예시:
 * @code
 * // 폰트 시스템 초기화
 * dx9::C_DX9_FONT_SYSTEM::GetInstance()->Initialize(pDevice);
 *
 * // 폰트 생성
 * dx9::C_DX9_FONT* pFont = new dx9::C_DX9_FONT();
 * pFont->Create(L"맑은 고딕", 16, false, false, 1, 1.0f, true);
 *
 * // 2D 텍스트 출력
 * dx9::C_DX9_FONT_SYSTEM::GetInstance()->BeginFont();
 * pFont->DrawText2D(100.0f, 100.0f, L"안녕하세요!", -1, 0xFFFFFFFF, 0xFF000000);
 * dx9::C_DX9_FONT_SYSTEM::GetInstance()->EndFont();
 *
 * // 3D 텍스트 출력
 * dx9::C_DX9_FONT_SYSTEM::GetInstance()->BeginFont();
 * pFont->DrawText3D(vWorldPos, L"3D 텍스트", -1, 0xFFFFFFFF);
 * dx9::C_DX9_FONT_SYSTEM::GetInstance()->EndFont();
 *
 * // 해제
 * delete pFont;
 * dx9::C_DX9_FONT_SYSTEM::GetInstance()->Finalize();
 * @endcode
 */

#include <DarkCore/DDef.h>
#include <DarkCore/DString.h>

#include "DX9Def.h"

#include <map>
#include <list>
#include <vector>
#include <string>

namespace dx9
{
	//============================================================================
	// 전방 선언
	//============================================================================
	class C_DX9_ENGINE;
	class C_DX9_FONT;
	class C_DX9_FONT_SYSTEM;
	struct _FONT_GLYPH_INFO;
	struct _FONT_TEXTURE_CELL;

	//============================================================================
	// 상수 정의
	//============================================================================
	constexpr int FONT_TEXTURE_SIZE = 1024;         ///< 폰트 텍스처 크기 (1024x1024)
	constexpr int FONT_CELL_SIZE = 32;              ///< 셀 크기 (32x32)
	constexpr int FONT_MAX_CELLS = (FONT_TEXTURE_SIZE * FONT_TEXTURE_SIZE) / (FONT_CELL_SIZE * FONT_CELL_SIZE);  ///< 최대 셀 수 (1024)
	constexpr int FONT_MAX_BATCH = 4000;            ///< 최대 배치 문자 수

	//============================================================================
	// 폰트 버텍스 구조체
	//============================================================================
	/**
	 * @brief 폰트 렌더링용 버텍스
	 */
	struct _FONT_VERTEX
	{
		float x, y, z;      ///< 위치
		DWORD dwColor;      ///< 색상 (ARGB)
		float u, v;         ///< 텍스처 좌표
	};
	constexpr DWORD FONT_VERTEX_FVF = (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	//============================================================================
	// 글리프 정보 구조체
	//============================================================================
	/**
	 * @brief 단일 글자의 캐시 정보
	 */
	struct _FONT_GLYPH_INFO
	{
		int nWidth;                 ///< 글리프 너비
		int nHeight;                ///< 글리프 높이
		int nTextureID;             ///< 텍스처 아틀라스 ID
		int nCellIndex;             ///< 셀 인덱스
	};

	//============================================================================
	// 텍스처 셀 정보 구조체
	//============================================================================
	/**
	 * @brief 폰트 텍스처 아틀라스 내 셀 정보
	 */
	struct _FONT_TEXTURE_CELL
	{
		int nID;                    ///< 현재 할당된 글리프 ID
		int nIndex;                 ///< 셀 인덱스
		std::list<_FONT_TEXTURE_CELL*>::iterator iter;  ///< LRU 리스트 반복자
	};

	//============================================================================
	// 레거시 D3DXFont 래퍼 (기존 호환)
	//============================================================================
	/**
	 * @brief 기존 D3DXFont 기반 폰트 (레거시 호환용)
	 */
	struct _DX9_FONT
	{
	private:
		LPD3DXFONT pFont{ nullptr };
		LPDIRECT3DDEVICE9 pDevice{ nullptr };

		std::wstring wstrName;
		UINT nSize;
		UINT nWeight;
		UINT nCharset;
		bool bItalic;
		bool bAntiAliased;

	public:
		_DX9_FONT(
			LPDIRECT3DDEVICE9 pDevice
			, LPCWSTR _wszName
			, int _nSize
			, UINT _nWeight = FW_NORMAL
			, UINT _nCharset = HANGUL_CHARSET
			, bool _bItalic = false
			, bool _bAntiAliased = false
		);
		virtual ~_DX9_FONT();

		void Create();
		void Release();

		LPD3DXFONT GetFont();

		void OnLostDevice();
		void OnResetDevice();
	};

	//============================================================================
	// C_DX9_FONT_TEXTURE - 폰트 텍스처 아틀라스
	//============================================================================
	/**
	 * @brief 여러 글자를 저장하는 텍스처 아틀라스
	 * @details LRU 방식으로 셀을 관리하여 가장 오래된 글리프부터 교체
	 */
	class C_DX9_FONT_TEXTURE
	{
		friend class C_DX9_FONT;
		friend class C_DX9_FONT_SYSTEM;

	private:
		LPDIRECT3DDEVICE9 m_pDevice;                ///< D3D 디바이스

		//------------------------------------------------------------------------
		// GDI 리소스
		//------------------------------------------------------------------------
		HDC m_hDC;                                  ///< 메모리 DC
		HBITMAP m_hBitmapFont;                      ///< 폰트 비트맵
		HBITMAP m_hBitmapOutline;                   ///< 외곽선 비트맵
		DWORD* m_pBitsFont;                         ///< 폰트 비트맵 픽셀
		DWORD* m_pBitsOutline;                      ///< 외곽선 비트맵 픽셀

		//------------------------------------------------------------------------
		// 텍스처
		//------------------------------------------------------------------------
		LPDIRECT3DTEXTURE9 m_pTextureFont;          ///< 폰트 텍스처
		LPDIRECT3DTEXTURE9 m_pTextureOutline;       ///< 외곽선 텍스처

		//------------------------------------------------------------------------
		// 셀 관리
		//------------------------------------------------------------------------
		int m_nCellCountX;                          ///< 가로 셀 수
		int m_nCellCountY;                          ///< 세로 셀 수
		int m_nTotalCells;                          ///< 총 셀 수
		int m_nLastUsedID;                          ///< 마지막 할당 ID

		_FONT_TEXTURE_CELL* m_pCells;               ///< 셀 배열
		std::list<_FONT_TEXTURE_CELL*> m_listLRU;   ///< LRU 우선순위 리스트

	public:
		C_DX9_FONT_TEXTURE();
		~C_DX9_FONT_TEXTURE();

		/**
		 * @brief 텍스처 아틀라스 생성
		 * @param _pDevice D3D 디바이스
		 * @return 성공 여부
		 */
		bool Create(LPDIRECT3DDEVICE9 _pDevice);

		/**
		 * @brief 리소스 해제
		 */
		void Destroy();

		/**
		 * @brief 글리프 갱신 필요 여부 확인
		 * @param _nCellIndex 셀 인덱스
		 * @param _nID 글리프 ID
		 * @return 갱신 필요 시 true
		 */
		bool IsNeedUpdate(int _nCellIndex, int _nID);

		/**
		 * @brief 글리프 비트맵 생성 및 텍스처 업로드
		 * @param _hFont 폰트 핸들
		 * @param _pInfo 글리프 정보
		 * @param _wszChar 문자 (1-2자)
		 * @param _nCharLen 문자 길이
		 * @param _nOutlineWidth 외곽선 두께
		 * @param _fOutlineOpacity 외곽선 불투명도
		 * @return 성공 여부
		 */
		bool MakeGlyphBitmap(HFONT _hFont, _FONT_GLYPH_INFO* _pInfo, const wchar_t* _wszChar, int _nCharLen, int _nOutlineWidth, float _fOutlineOpacity);

		/**
		 * @brief 문자 너비 측정
		 * @param _hFont 폰트 핸들
		 * @param _wszChar 문자
		 * @param _nCharLen 문자 길이
		 * @return 픽셀 너비
		 */
		int GetCharWidth(HFONT _hFont, const wchar_t* _wszChar, int _nCharLen);

		// Getter
		HDC GetDC() const { return m_hDC; }
		int GetWidth() const { return FONT_TEXTURE_SIZE; }
		int GetHeight() const { return FONT_TEXTURE_SIZE; }
		int GetCellCountX() const { return m_nCellCountX; }
		int GetCellCountY() const { return m_nCellCountY; }
		int GetLastUsedID() const { return m_nLastUsedID; }
		LPDIRECT3DTEXTURE9 GetTextureFont() const { return m_pTextureFont; }
		LPDIRECT3DTEXTURE9 GetTextureOutline() const { return m_pTextureOutline; }

	private:
		/**
		 * @brief 텍스처에 글리프 업로드
		 */
		bool UploadToTexture(LPDIRECT3DTEXTURE9 _pTexture, int _nCellIndex, int _nWidth, int _nHeight, DWORD* _pBits);

		/**
		 * @brief 8방향 외곽선 그리기 (최적화됨)
		 */
		void DrawOutline8Dir(const wchar_t* _wszChar, int _nCharLen, int _nOutlineWidth);
	};

	//============================================================================
	// C_DX9_FONT_SYSTEM - 폰트 시스템 (싱글톤)
	//============================================================================
	/**
	 * @brief 폰트 시스템 관리자 (싱글톤)
	 * @details 폰트 텍스처, 버텍스 버퍼, 렌더 스테이트 관리
	 */
	class C_DX9_FONT_SYSTEM
	{
	private:
		static C_DX9_FONT_SYSTEM* s_pInstance;

		LPDIRECT3DDEVICE9 m_pDevice;                ///< D3D 디바이스
		C_DX9_FONT_TEXTURE m_fontTexture;           ///< 폰트 텍스처 아틀라스

		//------------------------------------------------------------------------
		// 배치 렌더링 버퍼
		//------------------------------------------------------------------------
		_FONT_VERTEX m_vFontBuffer[4 * FONT_MAX_BATCH];      ///< 폰트 버텍스 버퍼
		_FONT_VERTEX m_vOutlineBuffer[4 * FONT_MAX_BATCH];   ///< 외곽선 버텍스 버퍼
		WORD m_vIndexBuffer[6 * FONT_MAX_BATCH];             ///< 인덱스 버퍼

		int m_nBatchCount;                          ///< 현재 배치 문자 수
		int m_nCurrentOutlineWidth;                 ///< 현재 외곽선 두께

		//------------------------------------------------------------------------
		// 상태 관리
		//------------------------------------------------------------------------
	bool m_bInFontMode;                         ///< BeginFont/EndFont 사이인지
	D3DXMATRIX m_matSavedView;                  ///< 저장된 View 행렬
	D3DXMATRIX m_matSavedProj;                  ///< 저장된 Projection 행렬		//------------------------------------------------------------------------
		// 외부 폰트 리소스
		//------------------------------------------------------------------------
		std::vector<std::wstring> m_vFontFiles;             ///< 로드된 폰트 파일 목록
		std::vector<HANDLE> m_vFontMemoryHandles;           ///< 메모리 폰트 핸들

	private:
		C_DX9_FONT_SYSTEM();
		~C_DX9_FONT_SYSTEM();

	public:
		/**
		 * @brief 싱글톤 인스턴스 획득
		 */
		static C_DX9_FONT_SYSTEM* GetInstance();

		/**
		 * @brief 싱글톤 인스턴스 해제
		 */
		static void DestroyInstance();

		/**
		 * @brief 폰트 시스템 초기화
		 * @param _pDevice D3D 디바이스
		 * @return 성공 여부
		 */
		bool Initialize(LPDIRECT3DDEVICE9 _pDevice);

		/**
		 * @brief 폰트 시스템 종료
		 */
		void Finalize();

		/**
		 * @brief 폰트 렌더링 시작
		 * @return 성공 여부
		 */
		bool BeginFont();

		/**
		 * @brief 폰트 렌더링 종료
		 * @return 성공 여부
		 */
		bool EndFont();

		/**
		 * @brief 버퍼 플러시 (즉시 그리기)
		 */
		void FlushFont();

		/**
		 * @brief 외부 폰트 파일 추가
		 * @param _wszFontPath 폰트 파일 경로
		 * @return 성공 여부
		 */
		bool AddFontFromFile(const wchar_t* _wszFontPath);

		/**
		 * @brief 메모리에서 폰트 추가
		 * @param _pData 폰트 데이터
		 * @param _nSize 데이터 크기
		 * @return 성공 여부
		 */
		bool AddFontFromMemory(const void* _pData, DWORD _nSize);

		// Getter
		C_DX9_FONT_TEXTURE* GetFontTexture() { return &m_fontTexture; }
		LPDIRECT3DDEVICE9 GetDevice() const { return m_pDevice; }
		bool IsInFontMode() const { return m_bInFontMode; }
	const D3DXMATRIX& GetSavedView() const { return m_matSavedView; }
	const D3DXMATRIX& GetSavedProj() const { return m_matSavedProj; }		/**
		 * @brief 배치 버퍼에 글리프 추가
		 */
		void AddGlyphToBatch(
			float _fX, float _fY, float _fWidth, float _fHeight,
			float _fU0, float _fV0, float _fU1, float _fV1,
			DWORD _dwColorFont, DWORD _dwColorOutline,
			int _nOutlineWidth);

		/**
		 * @brief 현재 배치 수
		 */
		int GetBatchCount() const { return m_nBatchCount; }
	};

	//============================================================================
	// C_DX9_FONT - 개별 폰트 인스턴스
	//============================================================================
	/**
	 * @brief 개별 폰트 인스턴스
	 * @details 특정 폰트명/크기/스타일에 대한 글리프 캐시 관리
	 */
	class C_DX9_FONT
	{
	private:
		HFONT m_hFont;                              ///< GDI 폰트 핸들
		std::map<WORD, _FONT_GLYPH_INFO*> m_mapGlyphs;   ///< 글리프 캐시 (키: wchar_t 값)

		int m_nFontHeight;                          ///< 폰트 높이
		int m_nOutlineWidth;                        ///< 외곽선 두께
		float m_fOutlineOpacity;                    ///< 외곽선 불투명도
		bool m_bAntiAlias;                          ///< 안티앨리어싱 여부

	public:
		C_DX9_FONT();
		~C_DX9_FONT();

		/**
		 * @brief 폰트 생성
		 * @param _wszFontName 폰트 이름 (예: L"맑은 고딕")
		 * @param _nHeight 폰트 크기 (픽셀)
		 * @param _bBold 굵게
		 * @param _bItalic 기울임
		 * @param _nOutlineWidth 외곽선 두께 (0이면 없음)
		 * @param _fOutlineOpacity 외곽선 불투명도 (0.0 ~ 1.0)
		 * @param _bAntiAlias 안티앨리어싱
		 * @return 성공 여부
		 */
		bool Create(
			const wchar_t* _wszFontName,
			int _nHeight,
			bool _bBold = false,
			bool _bItalic = false,
			int _nOutlineWidth = 0,
			float _fOutlineOpacity = 1.0f,
			bool _bAntiAlias = false);

		/**
		 * @brief 폰트 해제
		 */
		void Destroy();

		/**
		 * @brief 2D 텍스트 출력
		 * @param _fX 화면 X 좌표
		 * @param _fY 화면 Y 좌표
		 * @param _wszText 출력할 텍스트
		 * @param _nLength 텍스트 길이 (-1이면 자동)
		 * @param _dwColorFont 폰트 색상 (ARGB)
		 * @param _dwColorOutline 외곽선 색상 (ARGB)
		 * @param _fScale 스케일
		 */
		void DrawText2D(
			float _fX, float _fY,
			const wchar_t* _wszText,
			int _nLength = -1,
			DWORD _dwColorFont = 0xFFFFFFFF,
			DWORD _dwColorOutline = 0xFF000000,
			float _fScale = 1.0f);

		/**
		 * @brief 3D 공간에 텍스트 출력 (빌보드 방식)
		 * @param _matWorld 월드 변환 행렬
		 * @param _wszText 출력할 텍스트
		 * @param _nLength 텍스트 길이 (-1이면 자동)
		 * @param _dwColor 폰트 색상 (ARGB)
		 * @return 성공 여부
		 */
		bool DrawText3D(
			const D3DXMATRIX& _matWorld,
			const wchar_t* _wszText,
			int _nLength = -1,
			DWORD _dwColor = 0xFFFFFFFF);

		/**
		 * @brief 3D 위치에 텍스트 출력 (2D 투영 방식)
		 * @param _vWorldPos 월드 좌표
		 * @param _wszText 출력할 텍스트
		 * @param _nLength 텍스트 길이 (-1이면 자동)
		 * @param _dwColor 폰트 색상 (ARGB)
		 * @return 성공 여부
		 */
		bool DrawText3D(
			const D3DXVECTOR3& _vWorldPos,
			const wchar_t* _wszText,
			int _nLength = -1,
			DWORD _dwColor = 0xFFFFFFFF);

		/**
		 * @brief 텍스트 너비 측정
		 * @param _wszText 측정할 텍스트
		 * @param _nLength 텍스트 길이 (-1이면 자동)
		 * @return 픽셀 너비
		 */
		int GetTextWidth(const wchar_t* _wszText, int _nLength = -1);

		/**
		 * @brief 텍스트 크기 측정
		 * @param _wszText 측정할 텍스트
		 * @param _nLength 텍스트 길이 (-1이면 자동)
		 * @return SIZE 구조체 (cx=너비, cy=높이)
		 */
		SIZE GetTextSize(const wchar_t* _wszText, int _nLength = -1);

		// Getter
		int GetHeight() const { return m_nFontHeight; }
		int GetOutlineWidth() const { return m_nOutlineWidth; }
		HFONT GetFontHandle() const { return m_hFont; }

	private:
		/**
		 * @brief 글리프 정보 획득 (캐시 미스 시 생성)
		 */
		_FONT_GLYPH_INFO* GetGlyphInfo(WORD _wKey, const wchar_t* _wszChar, int _nCharLen);
	};

}  // namespace dx9