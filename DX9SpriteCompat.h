/**
 * @file DX9SpriteCompat.h
 * @brief Wonderking 프로젝트 호환 레이어 - 기존 D3D_DrawTexture 시그니처를 새 배칭 렌더러로 라우팅
 * @details 기존 DrawPrimitiveUP 기반 코드를 인덱스 버퍼 배칭 시스템으로 전환하는 호환 인터페이스
 * 
 * 사용 방법:
 * 1. 기존 D3D_DrawTexture 호출 전 WK_BeginSpriteBatch() 호출
 * 2. 기존 D3D_DrawTexture 호출을 WK_DrawSprite()로 교체 (시그니처 동일)
 * 3. 렌더링 완료 후 WK_EndSpriteBatch() 호출
 * 
 * @author DX9Wrapp
 * @date 2026-01-20
 */
#pragma once

#include "DX9Sprite.h"

namespace dx9
{

//==============================================================================
// 전역 스프라이트 렌더러 인스턴스 접근
//==============================================================================

/**
 * @brief 전역 스프라이트 렌더러 인스턴스 획득
 * @return 스프라이트 렌더러 포인터 (싱글톤)
 */
C_DX9_SPRITE_RENDERER* WK_GetSpriteRenderer();

/**
 * @brief 전역 스프라이트 렌더러 초기화
 * @param _pDevice Direct3D 디바이스 포인터
 * @param _nScreenWidth 화면 너비
 * @param _nScreenHeight 화면 높이
 * @return 성공 시 true
 */
bool WK_InitializeSpriteRenderer(LPDIRECT3DDEVICE9 _pDevice, UINT _nScreenWidth, UINT _nScreenHeight);

/**
 * @brief 전역 스프라이트 렌더러 해제
 */
void WK_ShutdownSpriteRenderer();

//==============================================================================
// Wonderking 호환 API - 기존 D3D_DrawTexture와 동일한 시그니처
//==============================================================================

/**
 * @brief 스프라이트 배치 시작
 * @details 프레임 또는 씬 단위로 호출. 내부적으로 배칭 모드 시작
 */
void WK_BeginSpriteBatch();

/**
 * @brief 스프라이트 배치 종료 및 플러시
 * @details 누적된 모든 스프라이트를 GPU로 전송
 */
void WK_EndSpriteBatch();

/**
 * @brief Wonderking 호환 스프라이트 그리기 함수 (텍스처 인덱스 버전)
 * @details 기존 D3D_DrawTexture와 동일한 시그니처. WK_SetTextureArray로 등록된 배열 사용.
 * 
 * @param _pDevice Direct3D 디바이스 (호환성용, 내부적으로 사용하지 않음)
 * @param _rectDest 화면 출력 영역 (픽셀 좌표)
 * @param _nTextureIndex 텍스처 배열 인덱스 (Surface[num])
 * @param _rectSrc 소스 텍스처 영역 (텍셀 좌표)
 * @param _nSrcFullWidth 소스 텍스처 전체 너비
 * @param _nSrcFullHeight 소스 텍스처 전체 높이
 * @param _nBlendingType 블렌딩 타입 (0=반투명, 1=광원, 2=일반, 3=구름, 4=그림자, 5=오퍼서티, 6=백서페이스, 7=화면출력)
 * @param _nLighting 조명 값 (0-255)
 * @param _fAlpha 알파 값 (0.0-1.0)
 * @param _fScale 스케일 값 (1.0 = 원본 크기)
 * @param _nAngle 회전 각도 (도 단위)
 * @param _colorVertex 정점 색상 (COLORREF)
 * @param _nInvert 좌우 반전 플래그 (0=정상, 1=좌우반전)
 * 
 * @note 텍스처 포인터는 외부에서 WK_SetTextureArray()로 등록해야 함
 */
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
);

/**
 * @brief Wonderking 호환 스프라이트 그리기 함수 (직접 텍스처 포인터 버전)
 * @details 텍스처 배열 등록 없이 직접 텍스처 포인터를 전달. DDraw.cpp에서 사용.
 * 
 * @param _pTexture 텍스처 포인터 (nullptr이면 무시)
 * @param _rectDest 화면 출력 영역 (픽셀 좌표)
 * @param _rectSrc 소스 텍스처 영역 (텍셀 좌표)
 * @param _nSrcFullWidth 소스 텍스처 전체 너비
 * @param _nSrcFullHeight 소스 텍스처 전체 높이
 * @param _nBlendingType 블렌딩 타입 (0-7)
 * @param _nLighting 조명 값 (0-255)
 * @param _fAlpha 알파 값 (0.0-1.0)
 * @param _fScale 스케일 값 (1.0 = 원본 크기)
 * @param _nAngle 회전 각도 (도 단위)
 * @param _colorVertex 정점 색상 (COLORREF)
 * @param _nInvert 좌우 반전 플래그 (0=정상, 1=좌우반전)
 */
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
);

/**
 * @brief 텍스처 배열 등록
 * @details Wonderking의 Surface[] 배열을 스프라이트 렌더러에 연결
 * @param _ppTextures 텍스처 포인터 배열
 * @param _nCount 텍스처 개수
 */
void WK_SetTextureArray(LPDIRECT3DTEXTURE9* _ppTextures, int _nCount);

/**
 * @brief 개별 텍스처 등록
 * @param _nIndex 텍스처 인덱스
 * @param _pTexture 텍스처 포인터
 */
void WK_SetTexture(int _nIndex, LPDIRECT3DTEXTURE9 _pTexture);

/**
 * @brief 화면 크기 설정
 * @param _nWidth 화면 너비
 * @param _nHeight 화면 높이
 */
void WK_SetScreenSize(UINT _nWidth, UINT _nHeight);

//==============================================================================
// 블렌딩 모드 변환 (Wonderking → DX9Sprite)
//==============================================================================

/**
 * @brief Wonderking 블렌딩 타입을 DX9Sprite 블렌딩 모드로 변환
 * @param _nWKBlendType Wonderking 블렌딩 타입 (0-7)
 * @return DX9Sprite 블렌딩 모드
 * 
 * 매핑:
 *   0 (반투명)     → BLEND_ALPHA
 *   1 (광원)       → BLEND_LIGHT
 *   2 (일반)       → BLEND_NORMAL
 *   3 (구름)       → BLEND_CLOUD
 *   4 (그림자)     → BLEND_SHADOW
 *   5 (오퍼서티)   → BLEND_OPACITY
 *   6 (백서페이스) → BLEND_BACKSURFACE
 *   7 (화면출력)   → BLEND_NORMAL
 */
E_BLEND_MODE WK_ConvertBlendMode(int _nWKBlendType);

} // namespace dx9