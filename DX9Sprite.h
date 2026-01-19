#pragma once

//============================================================================
// DX9Sprite.h
// 고성능 2D 스프라이트 렌더러
// - 인덱스 버퍼 기반 배칭 시스템으로 드로우콜 최소화
// - 7가지 블렌딩 모드 지원 (Wonderking 호환)
// - 회전, 스케일, 반전, 색상 변조 지원
//============================================================================

#include "DX9Def.h"
#include "DX9Types.h"
#include "DX9Vector2.h"
#include "DX9Vector3.h"
#include "DX9Matrix.h"

#include <vector>

namespace dx9
{
	//------------------------------------------------------------------------
	// 블렌딩 모드 열거형 (Wonderking DDraw.cpp 호환)
	//------------------------------------------------------------------------
	enum class E_BLEND_MODE : unsigned char
	{
		BLEND_ALPHA = 0,        // 반투명 (SrcAlpha, InvSrcAlpha) - lighting=0
		BLEND_ADDITIVE = 1,     // 광원/가산 (SrcAlpha, One)
		BLEND_NORMAL = 2,       // 기본 (SrcAlpha, InvSrcAlpha)
		BLEND_CLOUD = 3,        // 구름 (SrcColor, DestAlpha)
		BLEND_SHADOW = 4,       // 그림자 (Zero, SrcColor)
		BLEND_OPACITY = 5,      // 오퍼서티 (SrcAlpha, InvSrcAlpha)
		BLEND_CLEAR = 6,        // 클리어 (Zero, Zero)
		BLEND_COPY = 7,         // 직접출력 (One, Zero)
		BLEND_ALPHA_ADDITIVE,   // 반투명+가산 (One, One) - lighting=1
		BLEND_COUNT
	};

	//------------------------------------------------------------------------
	// 스프라이트 버텍스 구조체
	// FVF: D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1
	//------------------------------------------------------------------------
	struct _SPRITE_VERTEX
	{
		float x, y, z;          // 위치 (z는 항상 0)
		DWORD dwColor;          // 색상 (D3DCOLOR)
		float u, v;             // 텍스처 좌표

		static constexpr DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	};

	//------------------------------------------------------------------------
	// 스프라이트 인스턴스 데이터 (배칭용)
	//------------------------------------------------------------------------
	struct _SPRITE_INSTANCE
	{
		LPDIRECT3DTEXTURE9 pTexture;    // 텍스처 포인터
		RECT rcDest;                     // 화면 출력 영역
		RECT rcSrc;                      // 소스 텍스처 영역
		int nTexWidth;                   // 텍스처 전체 너비
		int nTexHeight;                  // 텍스처 전체 높이
		DWORD dwColor;                   // 색상 (D3DCOLOR_ARGB)
		float fScale;                    // 스케일
		float fAngle;                    // 회전 각도 (라디안)
		E_BLEND_MODE eBlendMode;         // 블렌딩 모드
		bool bInvert;                    // 좌우 반전
		bool bLighting;                  // 라이팅 플래그
	};

	//------------------------------------------------------------------------
	// 배치 정보 구조체
	//------------------------------------------------------------------------
	struct _SPRITE_BATCH
	{
		LPDIRECT3DTEXTURE9 pTexture;    // 배치 텍스처
		E_BLEND_MODE eBlendMode;         // 블렌딩 모드
		bool bLighting;                  // 라이팅 플래그
		UINT nStartVertex;               // 시작 버텍스 인덱스
		UINT nVertexCount;               // 버텍스 수
		UINT nStartIndex;                // 시작 인덱스
		UINT nPrimitiveCount;            // 프리미티브(삼각형) 수
	};

	//------------------------------------------------------------------------
	// C_DX9_SPRITE_RENDERER - 고성능 2D 스프라이트 렌더러
	//------------------------------------------------------------------------
	class C_DX9_SPRITE_RENDERER
	{
	public:
		// 최대 배치 크기 (한 번에 렌더링할 수 있는 스프라이트 수)
		static constexpr UINT MAX_BATCH_SIZE = 4096;
		static constexpr UINT VERTICES_PER_SPRITE = 4;
		static constexpr UINT INDICES_PER_SPRITE = 6;

	private:
		LPDIRECT3DDEVICE9 m_pDevice;                    // D3D9 디바이스
		LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;        // 동적 버텍스 버퍼
		LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;          // 정적 인덱스 버퍼
		
		// 투영 행렬 (2D 직교 투영)
		_DMATRIX9 m_matProjection;
		UINT m_nScreenWidth;
		UINT m_nScreenHeight;

		// 배칭 시스템
		std::vector<_SPRITE_INSTANCE> m_vInstances;     // 대기 중인 스프라이트들
		std::vector<_SPRITE_BATCH> m_vBatches;          // 배치 정보
		
		// 렌더링 상태
		bool m_bBegun;                                   // Begin() 호출 여부
		UINT m_nSpriteCount;                            // 현재 프레임 스프라이트 수
		UINT m_nDrawCallCount;                          // 현재 프레임 드로우콜 수

		// 버텍스 데이터 (CPU 측 버퍼)
		std::vector<_SPRITE_VERTEX> m_vVertices;

		// 내부 함수들
		void SetupBlendMode(E_BLEND_MODE _eMode, bool _bLighting);
		void BuildBatches();
		void RenderBatches();
		void GenerateSpriteVertices(const _SPRITE_INSTANCE& _instance, _SPRITE_VERTEX* _pOutVertices);

	public:
		C_DX9_SPRITE_RENDERER();
		~C_DX9_SPRITE_RENDERER();

		//--------------------------------------------------------------------
		// 초기화/해제
		//--------------------------------------------------------------------
		bool Initialize(LPDIRECT3DDEVICE9 _pDevice, UINT _nScreenWidth, UINT _nScreenHeight);
		void Release();
		void OnLostDevice();
		void OnResetDevice();

		//--------------------------------------------------------------------
		// 화면 크기 변경
		//--------------------------------------------------------------------
		void SetScreenSize(UINT _nScreenWidth, UINT _nScreenHeight);

		//--------------------------------------------------------------------
		// 렌더링 시작/종료
		//--------------------------------------------------------------------
		void Begin();
		void End();

		//--------------------------------------------------------------------
		// 스프라이트 그리기 (배칭 큐에 추가)
		// _rcDest: 화면 출력 영역
		// _pTexture: 텍스처 포인터
		// _rcSrc: 소스 텍스처 영역
		// _nTexWidth, _nTexHeight: 텍스처 전체 크기
		// _eBlendMode: 블렌딩 모드
		// _bLighting: 라이팅 활성화
		// _fAlpha: 알파값 (0.0~1.0)
		// _fScale: 스케일
		// _fAngle: 회전 각도 (도 단위)
		// _dwColor: 색상 (D3DCOLOR_ARGB)
		// _bInvert: 좌우 반전
		//--------------------------------------------------------------------
		void Draw(
			const RECT& _rcDest,
			LPDIRECT3DTEXTURE9 _pTexture,
			const RECT& _rcSrc,
			int _nTexWidth,
			int _nTexHeight,
			E_BLEND_MODE _eBlendMode = E_BLEND_MODE::BLEND_NORMAL,
			bool _bLighting = false,
			float _fAlpha = 1.0f,
			float _fScale = 1.0f,
			float _fAngle = 0.0f,
			DWORD _dwColor = 0xFFFFFFFF,
			bool _bInvert = false
		);

		//--------------------------------------------------------------------
		// Wonderking 호환 인터페이스
		// D3D_DrawTexture()와 동일한 파라미터
		//--------------------------------------------------------------------
		void DrawCompat(
			const RECT& _rcDest,
			LPDIRECT3DTEXTURE9 _pTexture,
			const RECT& _rcSrc,
			int _nTexWidth,
			int _nTexHeight,
			int _nBlendType,       // 0~7
			int _nLighting,        // 0 또는 1
			float _fAlpha,
			float _fScale,
			int _nAngle,           // 도 단위 정수
			DWORD _dwColor,
			int _nInvert
		);

		//--------------------------------------------------------------------
		// 즉시 렌더링 (배칭 없이 즉시 그리기)
		// 특수한 경우에만 사용 (렌더 타겟 변경 등)
		//--------------------------------------------------------------------
		void DrawImmediate(
			const RECT& _rcDest,
			LPDIRECT3DTEXTURE9 _pTexture,
			const RECT& _rcSrc,
			int _nTexWidth,
			int _nTexHeight,
			E_BLEND_MODE _eBlendMode = E_BLEND_MODE::BLEND_NORMAL,
			bool _bLighting = false,
			float _fAlpha = 1.0f,
			float _fScale = 1.0f,
			float _fAngle = 0.0f,
			DWORD _dwColor = 0xFFFFFFFF,
			bool _bInvert = false
		);

		//--------------------------------------------------------------------
		// 배칭 강제 플러시 (현재 큐 즉시 렌더링)
		//--------------------------------------------------------------------
		void Flush();

		//--------------------------------------------------------------------
		// 통계 정보
		//--------------------------------------------------------------------
		UINT GetSpriteCount() const { return m_nSpriteCount; }
		UINT GetDrawCallCount() const { return m_nDrawCallCount; }
		UINT GetBatchCount() const { return static_cast<UINT>(m_vBatches.size()); }
	};

} // namespace dx9

