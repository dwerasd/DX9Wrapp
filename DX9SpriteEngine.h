/**
 * @file DX9SpriteEngine.h
 * @brief DirectX 9 2D 스프라이트 엔진 - 체계적인 렌더링 시스템
 * @details Wonderking 등 2D 게임 프로젝트를 위한 통합 렌더링 엔진
 *          D3D11로의 확장을 고려한 인터페이스 설계
 * 
 * @author DX9Wrapp
 * @date 2026-01-20
 * 
 * 사용 예시:
 * @code
 * // 초기화
 * dx9::C_DX9_SPRITE_ENGINE* pEngine = dx9::C_DX9_SPRITE_ENGINE::GetInstance();
 * pEngine->Initialize(hWnd, 1024, 768, false);
 * 
 * // 텍스처 로드
 * int nTexId = pEngine->LoadTexture(L"sprite.png");
 * 
 * // 렌더링 루프
 * pEngine->BeginFrame();
 * pEngine->DrawSprite(nTexId, 100, 100, 64, 64);
 * pEngine->EndFrame();
 * pEngine->Present();
 * 
 * // 해제
 * pEngine->Shutdown();
 * @endcode
 */
#pragma once

#include "DX9Def.h"
#include "DX9Types.h"
#include "DX9Sprite.h"
#include "DX9Texture.h"

#include <vector>
#include <unordered_map>
#include <string>

namespace dx9
{

//============================================================================
// 전방 선언
//============================================================================
class C_DX9_SPRITE_ENGINE;

//============================================================================
// 텍스처 슬롯 정보 구조체
//============================================================================
struct _TEXTURE_SLOT
{
	LPDIRECT3DTEXTURE9 pTexture;    ///< 텍스처 포인터
	int nWidth;                      ///< 텍스처 너비
	int nHeight;                     ///< 텍스처 높이
	std::wstring strFilePath;        ///< 파일 경로 (재로드용)
	bool bManaged;                   ///< 엔진이 관리하는 텍스처인지 (true면 해제 시 Release)
	
	_TEXTURE_SLOT()
		: pTexture(nullptr)
		, nWidth(0)
		, nHeight(0)
		, bManaged(false)
	{}
};

//============================================================================
// 엔진 초기화 설정 구조체
//============================================================================
struct _ENGINE_CONFIG
{
	HWND hWnd;                       ///< 윈도우 핸들
	UINT nScreenWidth;               ///< 화면 너비
	UINT nScreenHeight;              ///< 화면 높이
	bool bWindowed;                  ///< 창 모드 여부
	bool bVSync;                     ///< 수직동기화
	D3DFORMAT eBackBufferFormat;     ///< 백버퍼 포맷
	UINT nMaxTextures;               ///< 최대 텍스처 슬롯 수
	
	_ENGINE_CONFIG()
		: hWnd(nullptr)
		, nScreenWidth(1024)
		, nScreenHeight(768)
		, bWindowed(true)
		, bVSync(false)
		, eBackBufferFormat(D3DFMT_A8R8G8B8)
		, nMaxTextures(4096)
	{}
};

//============================================================================
// 엔진 통계 정보
//============================================================================
struct _ENGINE_STATS
{
	UINT nFrameCount;                ///< 총 프레임 수
	UINT nDrawCallsPerFrame;         ///< 프레임당 드로우콜
	UINT nSpritesPerFrame;           ///< 프레임당 스프라이트 수
	UINT nTextureChanges;            ///< 텍스처 전환 횟수
	float fFrameTime;                ///< 프레임 시간 (ms)
	float fFPS;                      ///< 초당 프레임
	
	_ENGINE_STATS()
		: nFrameCount(0)
		, nDrawCallsPerFrame(0)
		, nSpritesPerFrame(0)
		, nTextureChanges(0)
		, fFrameTime(0.0f)
		, fFPS(0.0f)
	{}
};

//============================================================================
// C_DX9_SPRITE_ENGINE 클래스
// 2D 스프라이트 렌더링 통합 엔진 (싱글톤)
//============================================================================
class C_DX9_SPRITE_ENGINE
{
private:
	//------------------------------------------------------------------------
	// 싱글톤 인스턴스
	//------------------------------------------------------------------------
	static C_DX9_SPRITE_ENGINE* s_pInstance;

	//------------------------------------------------------------------------
	// DirectX 오브젝트
	//------------------------------------------------------------------------
	LPDIRECT3D9 m_pD3D;                          ///< Direct3D 인터페이스
	LPDIRECT3DDEVICE9 m_pDevice;                 ///< 디바이스
	D3DPRESENT_PARAMETERS m_d3dpp;               ///< 프레젠트 파라미터
	
	//------------------------------------------------------------------------
	// 스프라이트 렌더러
	//------------------------------------------------------------------------
	C_DX9_SPRITE_RENDERER* m_pSpriteRenderer;    ///< 배칭 스프라이트 렌더러
	
	//------------------------------------------------------------------------
	// 텍스처 관리
	//------------------------------------------------------------------------
	std::vector<_TEXTURE_SLOT> m_vTextures;      ///< 텍스처 슬롯 배열
	std::unordered_map<std::wstring, int> m_mapTextureIndex; ///< 경로→인덱스 맵
	
	//------------------------------------------------------------------------
	// 설정 및 상태
	//------------------------------------------------------------------------
	_ENGINE_CONFIG m_config;                     ///< 엔진 설정
	_ENGINE_STATS m_stats;                       ///< 통계 정보
	bool m_bInitialized;                         ///< 초기화 완료 여부
	bool m_bDeviceLost;                          ///< 디바이스 손실 상태
	bool m_bInFrame;                             ///< BeginFrame ~ EndFrame 사이인지
	
	//------------------------------------------------------------------------
	// 타이밍
	//------------------------------------------------------------------------
	LARGE_INTEGER m_liFrequency;                 ///< 타이머 주파수
	LARGE_INTEGER m_liLastFrameTime;             ///< 마지막 프레임 시간
	LARGE_INTEGER m_liLastFPSUpdate;             ///< 마지막 FPS 갱신 시간
	UINT m_nFrameCountForFPS;                    ///< FPS 계산용 프레임 카운트

	//------------------------------------------------------------------------
	// 생성자/소멸자 (private - 싱글톤)
	//------------------------------------------------------------------------
	C_DX9_SPRITE_ENGINE();
	~C_DX9_SPRITE_ENGINE();
	C_DX9_SPRITE_ENGINE(const C_DX9_SPRITE_ENGINE&) = delete;
	C_DX9_SPRITE_ENGINE& operator=(const C_DX9_SPRITE_ENGINE&) = delete;

	//------------------------------------------------------------------------
	// 내부 함수
	//------------------------------------------------------------------------
	bool CreateDevice();
	void ResetDevice();
	bool HandleDeviceLost();

public:
	//========================================================================
	// 싱글톤 접근
	//========================================================================
	
	/**
	 * @brief 싱글톤 인스턴스 획득
	 * @return 엔진 인스턴스 포인터
	 */
	static C_DX9_SPRITE_ENGINE* GetInstance();
	
	/**
	 * @brief 싱글톤 인스턴스 해제
	 */
	static void DestroyInstance();

	//========================================================================
	// 초기화/해제
	//========================================================================
	
	/**
	 * @brief 엔진 초기화 (기본 설정)
	 * @param _hWnd 윈도우 핸들
	 * @param _nWidth 화면 너비
	 * @param _nHeight 화면 높이
	 * @param _bWindowed 창 모드 여부
	 * @return 성공 시 true
	 */
	bool Initialize(HWND _hWnd, UINT _nWidth, UINT _nHeight, bool _bWindowed = true);
	
	/**
	 * @brief 엔진 초기화 (상세 설정)
	 * @param _config 엔진 설정 구조체
	 * @return 성공 시 true
	 */
	bool Initialize(const _ENGINE_CONFIG& _config);
	
	/**
	 * @brief 엔진 종료 및 리소스 해제
	 */
	void Shutdown();
	
	/**
	 * @brief 화면 크기 변경
	 * @param _nWidth 새 너비
	 * @param _nHeight 새 높이
	 * @return 성공 시 true
	 */
	bool Resize(UINT _nWidth, UINT _nHeight);

	//========================================================================
	// 텍스처 관리
	//========================================================================
	
	/**
	 * @brief 파일에서 텍스처 로드
	 * @param _strFilePath 파일 경로
	 * @param _nColorKey 컬러키 (0이면 사용 안 함)
	 * @return 텍스처 슬롯 인덱스 (-1이면 실패)
	 */
	int LoadTexture(const std::wstring& _strFilePath, D3DCOLOR _nColorKey = 0);
	
	/**
	 * @brief 특정 슬롯에 텍스처 로드
	 * @param _nSlot 슬롯 인덱스
	 * @param _strFilePath 파일 경로
	 * @param _nColorKey 컬러키
	 * @return 성공 시 true
	 */
	bool LoadTextureToSlot(int _nSlot, const std::wstring& _strFilePath, D3DCOLOR _nColorKey = 0);
	
	/**
	 * @brief 외부 텍스처 등록 (엔진이 Release하지 않음)
	 * @param _pTexture 텍스처 포인터
	 * @param _nWidth 텍스처 너비
	 * @param _nHeight 텍스처 높이
	 * @return 텍스처 슬롯 인덱스
	 */
	int RegisterTexture(LPDIRECT3DTEXTURE9 _pTexture, int _nWidth, int _nHeight);
	
	/**
	 * @brief 특정 슬롯에 외부 텍스처 등록
	 * @param _nSlot 슬롯 인덱스
	 * @param _pTexture 텍스처 포인터
	 * @param _nWidth 텍스처 너비
	 * @param _nHeight 텍스처 높이
	 */
	void SetTextureSlot(int _nSlot, LPDIRECT3DTEXTURE9 _pTexture, int _nWidth, int _nHeight);
	
	/**
	 * @brief 텍스처 해제
	 * @param _nSlot 슬롯 인덱스
	 */
	void UnloadTexture(int _nSlot);
	
	/**
	 * @brief 모든 텍스처 해제
	 */
	void UnloadAllTextures();
	
	/**
	 * @brief 텍스처 정보 획득
	 * @param _nSlot 슬롯 인덱스
	 * @return 텍스처 슬롯 정보 (없으면 nullptr)
	 */
	const _TEXTURE_SLOT* GetTextureInfo(int _nSlot) const;

	//========================================================================
	// 프레임 관리
	//========================================================================
	
	/**
	 * @brief 프레임 시작 (씬 클리어 포함)
	 * @param _dwClearColor 클리어 색상 (기본: 검정)
	 * @return 성공 시 true (디바이스 손실 시 false)
	 */
	bool BeginFrame(D3DCOLOR _dwClearColor = D3DCOLOR_XRGB(0, 0, 0));
	
	/**
	 * @brief 프레임 종료 (배칭 플러시)
	 */
	void EndFrame();
	
	/**
	 * @brief 화면 출력
	 * @return 성공 시 true
	 */
	bool Present();

	//========================================================================
	// 스프라이트 그리기 - 간단한 인터페이스
	//========================================================================
	
	/**
	 * @brief 스프라이트 그리기 (전체 텍스처)
	 * @param _nTexSlot 텍스처 슬롯 인덱스
	 * @param _fX 화면 X 좌표
	 * @param _fY 화면 Y 좌표
	 * @param _fAlpha 알파값 (0.0~1.0)
	 */
	void DrawSprite(int _nTexSlot, float _fX, float _fY, float _fAlpha = 1.0f);
	
	/**
	 * @brief 스프라이트 그리기 (크기 지정)
	 * @param _nTexSlot 텍스처 슬롯 인덱스
	 * @param _fX 화면 X 좌표
	 * @param _fY 화면 Y 좌표
	 * @param _fWidth 출력 너비
	 * @param _fHeight 출력 높이
	 * @param _fAlpha 알파값
	 */
	void DrawSprite(int _nTexSlot, float _fX, float _fY, float _fWidth, float _fHeight, float _fAlpha = 1.0f);
	
	/**
	 * @brief 스프라이트 그리기 (소스 영역 지정)
	 * @param _nTexSlot 텍스처 슬롯
	 * @param _rcDest 화면 출력 영역
	 * @param _rcSrc 소스 텍스처 영역
	 * @param _fAlpha 알파값
	 */
	void DrawSprite(int _nTexSlot, const RECT& _rcDest, const RECT& _rcSrc, float _fAlpha = 1.0f);

	//========================================================================
	// 스프라이트 그리기 - 상세 인터페이스
	//========================================================================
	
	/**
	 * @brief 스프라이트 그리기 (전체 옵션)
	 * @param _nTexSlot 텍스처 슬롯
	 * @param _rcDest 화면 출력 영역
	 * @param _rcSrc 소스 텍스처 영역
	 * @param _eBlendMode 블렌딩 모드
	 * @param _fAlpha 알파값
	 * @param _fScale 스케일
	 * @param _fAngle 회전 각도 (도)
	 * @param _dwColor 색상 변조
	 * @param _bInvert 좌우 반전
	 */
	void DrawSpriteEx(
		int _nTexSlot,
		const RECT& _rcDest,
		const RECT& _rcSrc,
		E_BLEND_MODE _eBlendMode = E_BLEND_MODE::BLEND_ALPHA,
		float _fAlpha = 1.0f,
		float _fScale = 1.0f,
		float _fAngle = 0.0f,
		D3DCOLOR _dwColor = 0xFFFFFFFF,
		bool _bInvert = false
	);
	
	/**
	 * @brief Wonderking 호환 스프라이트 그리기
	 * @details 기존 D3D_DrawTexture와 동일한 시그니처
	 */
	void DrawSpriteCompat(
		int _nTexSlot,
		const RECT& _rcDest,
		const RECT& _rcSrc,
		int _nBlendType,
		int _nLighting,
		float _fAlpha,
		float _fScale,
		int _nAngle,
		D3DCOLOR _dwColor,
		int _nInvert
	);

	//========================================================================
	// 유틸리티
	//========================================================================
	
	/**
	 * @brief 디바이스 포인터 획득
	 * @return D3D9 디바이스 포인터
	 */
	LPDIRECT3DDEVICE9 GetDevice() const { return m_pDevice; }
	
	/**
	 * @brief 스프라이트 렌더러 획득
	 * @return 스프라이트 렌더러 포인터
	 */
	C_DX9_SPRITE_RENDERER* GetSpriteRenderer() const { return m_pSpriteRenderer; }
	
	/**
	 * @brief 엔진 설정 획득
	 * @return 엔진 설정 구조체 참조
	 */
	const _ENGINE_CONFIG& GetConfig() const { return m_config; }
	
	/**
	 * @brief 엔진 통계 획득
	 * @return 통계 정보 구조체 참조
	 */
	const _ENGINE_STATS& GetStats() const { return m_stats; }
	
	/**
	 * @brief 화면 너비 획득
	 */
	UINT GetScreenWidth() const { return m_config.nScreenWidth; }
	
	/**
	 * @brief 화면 높이 획득
	 */
	UINT GetScreenHeight() const { return m_config.nScreenHeight; }
	
	/**
	 * @brief 초기화 여부 확인
	 */
	bool IsInitialized() const { return m_bInitialized; }
	
	/**
	 * @brief 디바이스 손실 여부 확인
	 */
	bool IsDeviceLost() const { return m_bDeviceLost; }

	//========================================================================
	// 배칭 제어
	//========================================================================
	
	/**
	 * @brief 현재 배치 강제 플러시
	 * @details 렌더 타겟 변경 등 특수 상황에서 사용
	 */
	void Flush();
};

//============================================================================
// 전역 헬퍼 함수 (Wonderking 호환용)
//============================================================================

/**
 * @brief 전역 엔진 인스턴스 획득 (편의 함수)
 */
inline C_DX9_SPRITE_ENGINE* GetSpriteEngine()
{
	return C_DX9_SPRITE_ENGINE::GetInstance();
}

} // namespace dx9