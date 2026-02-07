/**
 * @file DX9SpriteCompat.cpp
 * @brief Wonderking 프로젝트 호환 레이어 구현
 * @details 기존 D3D_DrawTexture 시그니처를 새 배칭 렌더러로 라우팅
 * @author DX9Wrapp
 * @date 2026-01-20
 */

#include "framework.h"
#include "DX9SpriteCompat.h"
#include "DX9SpriteEngine.h"  // C_DX9_SPRITE_ENGINE 사용을 위해 추가
#include "DX9Math.h"
#include <DarkCore/DPrint.h>  // DBGPRINT 매크로 사용

namespace dx9
{

	//==============================================================================
	// 전역 상태
	//==============================================================================

	/// @brief 전역 스프라이트 렌더러 인스턴스
	static C_DX9_SPRITE_RENDERER* g_pSpriteRenderer = nullptr;

	/// @brief 텍스처 배열 포인터 (Wonderking Surface[] 연결용)
	static LPDIRECT3DTEXTURE9* g_ppTextureArray = nullptr;

	/// @brief 텍스처 배열 크기
	static int g_nTextureCount = 0;

	//==============================================================================
	// 전역 스프라이트 렌더러 인스턴스 접근
	//==============================================================================

	C_DX9_SPRITE_RENDERER* WK_GetSpriteRenderer()
	{
		return g_pSpriteRenderer;
	}

	bool WK_InitializeSpriteRenderer(LPDIRECT3DDEVICE9 _pDevice, UINT _nScreenWidth, UINT _nScreenHeight)
	{
		// 이미 초기화되어 있으면 해제 후 재생성
		if (g_pSpriteRenderer != nullptr)
		{
			g_pSpriteRenderer->Release();
			delete g_pSpriteRenderer;
			g_pSpriteRenderer = nullptr;
		}

		g_pSpriteRenderer = new C_DX9_SPRITE_RENDERER();
		if (g_pSpriteRenderer == nullptr)
		{
			return false;
		}

		if (!g_pSpriteRenderer->Initialize(_pDevice, _nScreenWidth, _nScreenHeight))
		{
			delete g_pSpriteRenderer;
			g_pSpriteRenderer = nullptr;
			return false;
		}

		return true;
	}

	void WK_ShutdownSpriteRenderer()
	{
		if (g_pSpriteRenderer != nullptr)
		{
			g_pSpriteRenderer->Release();
			delete g_pSpriteRenderer;
			g_pSpriteRenderer = nullptr;
		}

		g_ppTextureArray = nullptr;
		g_nTextureCount = 0;
	}

	//==============================================================================
	// Wonderking 호환 API
	//==============================================================================

	void WK_BeginSpriteBatch()
	{
		if (g_pSpriteRenderer != nullptr)
		{
			g_pSpriteRenderer->Begin();
		}
	}

	void WK_EndSpriteBatch()
	{
		if (g_pSpriteRenderer != nullptr)
		{
			g_pSpriteRenderer->End();
		}
	}

	void WK_DrawSprite(
		LPDIRECT3DDEVICE9 _pDevice,
		RECT _rectDest,
		int _nTextureIndex,
		RECT _rectSrc,
		int _nSrcFullWidth,
		int _nSrcFullHeight,
		int _nBlendingType,
		int _nLighting,
		float _fAlpha,
		float _fScale,
		int _nAngle,
		COLORREF _colorVertex,
		int _nInvert
	)
	{
		// 미사용 파라미터 억제
		(void)_pDevice;

		// 렌더러 유효성 검사
		if (g_pSpriteRenderer == nullptr)
		{
			return;
		}

		// 텍스처 인덱스 유효성 검사
		if (g_ppTextureArray == nullptr || _nTextureIndex < 0 || _nTextureIndex >= g_nTextureCount)
		{
			return;
		}

		// 텍스처 포인터 획득
		const LPDIRECT3DTEXTURE9 pTexture_ = g_ppTextureArray[_nTextureIndex];
		if (pTexture_ == nullptr)
		{
			return;
		}

		// 정점 색상 계산 (COLORREF → D3DCOLOR)
		// COLORREF: 0x00BBGGRR
		// D3DCOLOR: 0xAARRGGBB
		BYTE byR_ = GetRValue(_colorVertex);
		BYTE byG_ = GetGValue(_colorVertex);
		BYTE byB_ = GetBValue(_colorVertex);

		// 조명 값 적용 (0-255 → 0.0-1.0 스케일로 정점 색상에 곱함)
		const float fLightScale_ = static_cast<float>(_nLighting) / 255.0f;
		byR_ = static_cast<BYTE>(dx9::Clamp(static_cast<float>(byR_) * fLightScale_, 0.0f, 255.0f));
		byG_ = static_cast<BYTE>(dx9::Clamp(static_cast<float>(byG_) * fLightScale_, 0.0f, 255.0f));
		byB_ = static_cast<BYTE>(dx9::Clamp(static_cast<float>(byB_) * fLightScale_, 0.0f, 255.0f));

		// D3DCOLOR 조합 (ARGB) - 알파는 DrawCompat 내부에서 적용
		DWORD dwColor_ = D3DCOLOR_XRGB(byR_, byG_, byB_);

		// DrawCompat 호출 - Wonderking D3D_DrawTexture와 동일한 파라미터
		g_pSpriteRenderer->DrawCompat(
			_rectDest,
			pTexture_,
			_rectSrc,
			_nSrcFullWidth,
			_nSrcFullHeight,
			_nBlendingType,
			_nLighting,
			_fAlpha,
			_fScale,
			_nAngle,
			dwColor_,
			_nInvert
		);
	}

	//==============================================================================
	// WK_DrawSpriteWithTexture - 텍스처 포인터 직접 전달 버전
	//==============================================================================
	static int s_nDrawCount = 0;  // 디버그용 호출 카운터

	void WK_DrawSpriteWithTexture(
		LPDIRECT3DTEXTURE9 _pTexture,
		RECT _rectDest,
		RECT _rectSrc,
		int _nSrcFullWidth,
		int _nSrcFullHeight,
		int _nBlendingType,
		int _nLighting,
		float _fAlpha,
		float _fScale,
		int _nAngle,
		COLORREF _colorVertex,
		int _nInvert
	)
	{
		++s_nDrawCount;

		// C_DX9_SPRITE_ENGINE의 스프라이트 렌더러 사용 (이미 초기화됨)
		const C_DX9_SPRITE_ENGINE* pEngine_ = C_DX9_SPRITE_ENGINE::GetInstance();
		if (pEngine_ == nullptr)
		{
			// if (s_nDrawCount <= 3) DBGPRINT("[WK_DrawSprite] ERROR: pEngine_ == nullptr");
			return;
		}
		if (!pEngine_->IsInitialized())
		{
			// if (s_nDrawCount <= 3) DBGPRINT("[WK_DrawSprite] ERROR: Engine not initialized");
			return;
		}

		C_DX9_SPRITE_RENDERER* pRenderer_ = pEngine_->GetSpriteRenderer();
		if (pRenderer_ == nullptr)
		{
			// if (s_nDrawCount <= 3) DBGPRINT("[WK_DrawSprite] ERROR: pRenderer_ == nullptr");
			return;
		}

		// 텍스처 유효성 검사
		if (_pTexture == nullptr)
		{
			// if (s_nDrawCount <= 3) DBGPRINT("[WK_DrawSprite] ERROR: _pTexture == nullptr");
			return;
		}

		// if (s_nDrawCount <= 5)
		// {
		//     DBGPRINT("[WK_DrawSprite] #%d: Dest(%d,%d,%d,%d) Src(%d,%d,%d,%d) TexSize=%dx%d Alpha=%.2f Blend=%d",
		//         s_nDrawCount,
		//         _rectDest.left, _rectDest.top, _rectDest.right, _rectDest.bottom,
		//         _rectSrc.left, _rectSrc.top, _rectSrc.right, _rectSrc.bottom,
		//         _nSrcFullWidth, _nSrcFullHeight, _fAlpha, _nBlendingType);
		// }

		// 정점 색상 계산 (COLORREF → D3DCOLOR)
		// COLORREF: 0x00BBGGRR
		// D3DCOLOR: 0xAARRGGBB
		// 
		// 주의: _nLighting은 조명 강도가 아니라 "가산 블렌딩 활성화" 플래그 (0 또는 1)
		// RGB는 _colorVertex에서 직접 가져옴
		const BYTE byR_ = GetRValue(_colorVertex);
		const BYTE byG_ = GetGValue(_colorVertex);
		const BYTE byB_ = GetBValue(_colorVertex);

		// D3DCOLOR 조합 (ARGB) - 알파는 DrawCompat 내부에서 적용
		const DWORD dwColor_ = D3DCOLOR_XRGB(byR_, byG_, byB_);

		// DrawCompat 호출 - Wonderking D3D_DrawTexture와 동일한 파라미터
		pRenderer_->DrawCompat(
			_rectDest,
			_pTexture,
			_rectSrc,
			_nSrcFullWidth,
			_nSrcFullHeight,
			_nBlendingType,
			_nLighting,
			_fAlpha,
			_fScale,
			_nAngle,
			dwColor_,
			_nInvert
		);
	}

	void WK_SetTextureArray(LPDIRECT3DTEXTURE9* _ppTextures, int _nCount)
	{
		g_ppTextureArray = _ppTextures;
		g_nTextureCount = _nCount;
	}

	void WK_SetTexture(int _nIndex, LPDIRECT3DTEXTURE9 _pTexture)
	{
		if (g_ppTextureArray != nullptr && _nIndex >= 0 && _nIndex < g_nTextureCount)
		{
			g_ppTextureArray[_nIndex] = _pTexture;
		}
	}

	void WK_SetScreenSize(UINT _nWidth, UINT _nHeight)
	{
		if (g_pSpriteRenderer != nullptr)
		{
			g_pSpriteRenderer->SetScreenSize(_nWidth, _nHeight);
		}
	}

	//==============================================================================
	// 블렌딩 모드 변환
	//==============================================================================

	E_BLEND_MODE WK_ConvertBlendMode(int _nWKBlendType)
	{
		switch (_nWKBlendType)
		{
		case 0:  // 반투명
			return E_BLEND_MODE::BLEND_ALPHA;
		case 1:  // 광원 (가산 블렌딩)
			return E_BLEND_MODE::BLEND_ADDITIVE;
		case 2:  // 일반
			return E_BLEND_MODE::BLEND_NORMAL;
		case 3:  // 구름
			return E_BLEND_MODE::BLEND_CLOUD;
		case 4:  // 그림자
			return E_BLEND_MODE::BLEND_SHADOW;
		case 5:  // 오퍼서티
			return E_BLEND_MODE::BLEND_OPACITY;
		case 6:  // 백서페이스 (클리어 또는 직접출력)
			return E_BLEND_MODE::BLEND_CLEAR;
		case 7:  // 화면출력
		default:
			return E_BLEND_MODE::BLEND_NORMAL;
		}
	}

} // namespace dx9