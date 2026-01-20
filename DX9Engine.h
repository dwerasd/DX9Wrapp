/**
 * @file DX9Engine.h
 * @brief DirectX 9 3D 그래픽 엔진 - RealSpace3 대체용
 * @details RaiderZ 3D 게임 엔진의 그래픽 렌더링 코어
 *          RealSpace3의 RDevice/RDeviceD3D를 대체하기 위한 엔진
 *
 * @author DX9Wrapp
 * @date 2026-01-20
 *
 * 사용 예시:
 * @code
 * // 초기화
 * dx9::C_DX9_ENGINE* pEngine = dx9::C_DX9_ENGINE::GetInstance();
 * pEngine->Initialize(hWnd, 1920, 1080, true);
 *
 * // 렌더링 루프
 * pEngine->BeginScene();
 * pEngine->Clear(true, true, false, 0x00000000);
 *
 * // 변환 설정
 * pEngine->SetTransform(dx9::E_TRANSFORM_TYPE::WORLD, matWorld);
 * pEngine->SetTransform(dx9::E_TRANSFORM_TYPE::VIEW, matView);
 * pEngine->SetTransform(dx9::E_TRANSFORM_TYPE::PROJECTION, matProj);
 *
 * // 렌더링
 * pEngine->SetVertexBuffer(pVB, 0, 0);
 * pEngine->SetIndexBuffer(pIB);
 * pEngine->SetTexture(0, pTexture);
 * pEngine->DrawIndexedPrimitive(dx9::E_PRIMITIVE_TYPE::TRIANGLELIST, 0, 0, nVertices, 0, nPrimitives);
 *
 * pEngine->EndScene();
 * pEngine->Present();
 *
 * // 해제
 * pEngine->Shutdown();
 * @endcode
 */
#pragma once

#include "DX9Def.h"
#include "DX9Types.h"
#include "DX9DeviceState.h"
#include "DX9Buffer.h"
#include "DX9Texture.h"
#include "DX9Matrix.h"
#include "DX9Viewport.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <list>

namespace dx9
{

	//============================================================================
	// 전방 선언
	//============================================================================
	class C_DX9_ENGINE;
	class C_DX9_SPRITE_RENDERER;

	//============================================================================
	// 열거형 정의 (RealSpace3 호환)
	//============================================================================

	/**
	 * @brief 필모드 (와이어프레임, 솔리드)
	 */
	enum class E_FILL_MODE
	{
		POINT = 0,      ///< 점 렌더링
		WIREFRAME,      ///< 와이어프레임
		SOLID           ///< 솔리드
	};

	/**
	 * @brief 블렌딩 상수
	 */
	enum class E_BLEND
	{
		ZERO = 0,
		ONE,
		SRCCOLOR,
		INVSRCCOLOR,
		DESTCOLOR,
		INVDESTCOLOR,
		SRCALPHA,
		INVSRCALPHA,
		DESTALPHA,
		INVDESTALPHA,
		SRCALPHASAT,
		BLENDFACTOR,
		INVBLENDFACTOR,
		NONE            ///< 블렌딩 비활성화
	};

	/**
	 * @brief 블렌딩 연산
	 */
	enum class E_BLEND_OP
	{
		ADD = 0,
		SUBTRACT,
		REVSUBTRACT,
		MIN,
		MAX
	};

	/**
	 * @brief 비교 함수
	 */
	enum class E_CMP_FUNC
	{
		NEVER = 0,
		LESS,
		EQUAL,
		LESSEQUAL,
		GREATER,
		NOTEQUAL,
		GREATEREQUAL,
		ALWAYS,
		END
	};

	/**
	 * @brief 프리미티브 타입
	 */
	enum class E_PRIMITIVE_TYPE
	{
		POINTLIST = 0,
		LINELIST,
		LINESTRIP,
		TRIANGLELIST,
		TRIANGLESTRIP,
		TRIANGLEFAN
	};

	/**
	 * @brief 컬링 모드
	 */
	enum class E_CULL_MODE
	{
		NONE = 0,       ///< 컬링 없음
		CCW,            ///< 반시계방향 컬링
		CW              ///< 시계방향 컬링
	};

	/**
	 * @brief 스텐실 연산
	 */
	enum class E_STENCIL_OP
	{
		KEEP = 0,
		ZERO,
		REPLACE,
		INCRSAT,
		DECRSAT,
		INVERT,
		INCR,
		DECR
	};

	/**
	 * @brief 텍스처 주소 모드
	 */
	enum class E_TEXTURE_ADDRESS
	{
		WRAP = 0,
		MIRROR,
		CLAMP,
		BORDER,
		MIRRORONCE
	};

	/**
	 * @brief 텍스처 필터 타입
	 */
	enum class E_TEXTURE_FILTER
	{
		POINT = 0,              ///< 포인트 필터링
		LINEAR,                 ///< 리니어 필터링
		BILINEAR,               ///< 바이리니어 (밉맵 포인트)
		TRILINEAR,              ///< 트라이리니어 (밉맵 리니어)
		BILINEAR_ANISO,         ///< 이방성 바이리니어
		TRILINEAR_ANISO         ///< 이방성 트라이리니어
	};

	/**
	 * @brief 변환 타입
	 */
	enum class E_TRANSFORM_TYPE
	{
		WORLD = 0,
		VIEW,
		PROJECTION,
		TEXTURE0,
		TEXTURE1,
		TEXTURE2,
		TEXTURE3,
		TEXTURE4,
		TEXTURE5,
		TEXTURE6,
		TEXTURE7,
		MAX
	};

	/**
	 * @brief 큐브맵 면
	 */
	enum class E_CUBEMAP_FACE
	{
		POSITIVE_X = 0,
		NEGATIVE_X,
		POSITIVE_Y,
		NEGATIVE_Y,
		POSITIVE_Z,
		NEGATIVE_Z,
		MAX
	};

	/**
	 * @brief 쿼리 기능 타입
	 */
	enum class E_QUERY_FEATURE
	{
		HWSKINNING = 0,         ///< 하드웨어 스키닝 지원
		VERTEXTEXTURE,          ///< 버텍스 텍스처 지원
		SHADER_MODEL_3,         ///< 셰이더 모델 3.0
		SHADER_MODEL_4,         ///< 셰이더 모델 4.0
		INSTANCING,             ///< 인스턴싱 지원
		RENDER_TO_TEXTURE       ///< 렌더 투 텍스처
	};

	/**
	 * @brief 텍스처 생성 플래그
	 */
	enum E_TEXTURE_CREATE_FLAGS
	{
		TCF_CLAMP = 0x001,    ///< 클램프 주소 모드
		TCF_CUBEMAP = 0x008,    ///< 큐브맵
		TCF_NORMALMAP = 0x020,    ///< 노멀맵
		TCF_AUTOGENMIPMAP = 0x040,    ///< 밉맵 자동 생성
		TCF_RENDERTARGET = 0x080,    ///< 렌더 타겟
		TCF_DYNAMIC = 0x100,    ///< 동적 텍스처
		TCF_UNREDUCIBLE = 0x200,    ///< 해상도 축소 불가
		TCF_SYSTEMMEM = 0x400,    ///< 시스템 메모리
		TCF_DEPTHSTENCIL = 0x800     ///< 깊이 스텐실
	};

	//============================================================================
	// 렌더 프로파일 정보
	//============================================================================
	struct _ENGINE_PROFILE_INFO
	{
		int nDrawPrimitive;              ///< DrawPrimitive 폴리곤 수
		int nDrawIndexedPrimitive;       ///< DrawIndexedPrimitive 폴리곤 수
		int nDrawPrimitiveUP;            ///< DrawPrimitiveUP 폴리곤 수
		int nDrawIndexedPrimitiveUP;     ///< DrawIndexedPrimitiveUP 폴리곤 수

		int nDrawPrimitiveCalls;         ///< DrawPrimitive 호출 수
		int nDrawIndexedPrimitiveCalls;  ///< DrawIndexedPrimitive 호출 수
		int nDrawPrimitiveUPCalls;       ///< DrawPrimitiveUP 호출 수
		int nDrawIndexedPrimitiveUPCalls;///< DrawIndexedPrimitiveUP 호출 수

		UINT nReducedDPCalls;            ///< 인스턴싱으로 줄어든 DP 호출
		UINT nReducedDIPCalls;           ///< 인스턴싱으로 줄어든 DIP 호출

		_ENGINE_PROFILE_INFO()
		{
			Reset();
		}

		void Reset()
		{
			nDrawPrimitive = nDrawIndexedPrimitive = 0;
			nDrawPrimitiveUP = nDrawIndexedPrimitiveUP = 0;
			nDrawPrimitiveCalls = nDrawIndexedPrimitiveCalls = 0;
			nDrawPrimitiveUPCalls = nDrawIndexedPrimitiveUPCalls = 0;
			nReducedDPCalls = nReducedDIPCalls = 0;
		}

		void AddDP(int _nPolys, UINT _nInstanceCount)
		{
			nDrawPrimitiveCalls++;
			nDrawPrimitive += _nPolys * _nInstanceCount;
			nReducedDPCalls += _nInstanceCount - 1;
		}

		void AddDIP(int _nPolys, UINT _nInstanceCount)
		{
			nDrawIndexedPrimitiveCalls++;
			nDrawIndexedPrimitive += _nPolys * _nInstanceCount;
			nReducedDIPCalls += _nInstanceCount - 1;
		}

		void AddDPUP(int _nPolys)
		{
			nDrawPrimitiveUPCalls++;
			nDrawPrimitiveUP += _nPolys;
		}

		void AddDIPUP(int _nPolys)
		{
			nDrawIndexedPrimitiveUPCalls++;
			nDrawIndexedPrimitiveUP += _nPolys;
		}

		int GetTotalPolygons() const
		{
			return nDrawPrimitive + nDrawIndexedPrimitive + nDrawPrimitiveUP + nDrawIndexedPrimitiveUP;
		}

		int GetTotalDrawCalls() const
		{
			return nDrawPrimitiveCalls + nDrawIndexedPrimitiveCalls + nDrawPrimitiveUPCalls + nDrawIndexedPrimitiveUPCalls;
		}
	};

	//============================================================================
	// 엔진 초기화 설정 구조체
	//============================================================================
	struct _ENGINE_3D_CONFIG
	{
		HWND hWnd;                       ///< 윈도우 핸들
		UINT nScreenWidth;               ///< 화면 너비
		UINT nScreenHeight;              ///< 화면 높이
		bool bWindowed;                  ///< 창 모드 여부
		bool bVSync;                     ///< 수직동기화
		D3DFORMAT eBackBufferFormat;     ///< 백버퍼 포맷
		D3DFORMAT eDepthStencilFormat;   ///< 깊이 스텐실 포맷
		UINT nMaxTextures;               ///< 최대 텍스처 수
		bool bMultiThread;               ///< 멀티스레드 지원

		_ENGINE_3D_CONFIG()
			: hWnd(nullptr)
			, nScreenWidth(1920)
			, nScreenHeight(1080)
			, bWindowed(true)
			, bVSync(false)
			, eBackBufferFormat(D3DFMT_A8R8G8B8)
			, eDepthStencilFormat(D3DFMT_D24S8)
			, nMaxTextures(4096)
			, bMultiThread(false)
		{
		}
	};

	//============================================================================
	// 잠긴 사각형 구조체 (텍스처 락용)
	//============================================================================
	struct _LOCKED_RECT
	{
		INT nPitch;                      ///< 피치 (바이트 단위)
		void* pBits;                     ///< 픽셀 데이터 포인터

		_LOCKED_RECT()
			: nPitch(0)
			, pBits(nullptr)
		{
		}
	};

	//============================================================================
	// C_DX9_ENGINE 클래스
	// 3D 렌더링 통합 엔진 (싱글톤)
	//============================================================================
	class C_DX9_ENGINE
	{
	private:
		//------------------------------------------------------------------------
		// 싱글톤 인스턴스
		//------------------------------------------------------------------------
		static C_DX9_ENGINE* s_pInstance;

		//------------------------------------------------------------------------
		// DirectX 오브젝트
		//------------------------------------------------------------------------
		LPDIRECT3D9 m_pD3D;                          ///< Direct3D 인터페이스
		LPDIRECT3DDEVICE9 m_pDevice;                 ///< 디바이스
		D3DPRESENT_PARAMETERS m_d3dpp;               ///< 프레젠트 파라미터
		D3DCAPS9 m_d3dCaps;                          ///< 디바이스 능력

		//------------------------------------------------------------------------
		// 설정 및 상태
		//------------------------------------------------------------------------
		_ENGINE_3D_CONFIG m_config;                  ///< 엔진 설정
		_ENGINE_PROFILE_INFO m_profileCurrent;       ///< 현재 프레임 프로파일
		_ENGINE_PROFILE_INFO m_profileLast;          ///< 이전 프레임 프로파일
		bool m_bInitialized;                         ///< 초기화 완료 여부
		bool m_bDeviceLost;                          ///< 디바이스 손실 상태
		bool m_bInScene;                             ///< BeginScene ~ EndScene 사이인지
		bool m_bOwnDevice;                           ///< 디바이스 소유 여부
		bool m_bCursorVisible;                       ///< 커서 표시 여부

		//------------------------------------------------------------------------
		// 렌더 타겟 관리
		//------------------------------------------------------------------------
		static constexpr UINT MAX_RENDER_TARGETS = 4;
		_DX9_TEXTURE* m_pCurrentRenderTargets[MAX_RENDER_TARGETS];
		int m_nCurrentRenderTargetSurface[MAX_RENDER_TARGETS];
		_DX9_TEXTURE* m_pCurrentDepthStencil;

		//------------------------------------------------------------------------
		// 텍스처 관리
		//------------------------------------------------------------------------
		static constexpr UINT MAX_TEXTURE_STAGES = 16;
		static constexpr UINT MAX_VERTEX_SAMPLERS = 4;
		static constexpr UINT MAX_IMAGE_UNITS = MAX_TEXTURE_STAGES + MAX_VERTEX_SAMPLERS;

		_DX9_TEXTURE* m_pCurrentTextures[MAX_IMAGE_UNITS];
		E_TEXTURE_FILTER m_eCurrentTextureFilter[MAX_IMAGE_UNITS];
		E_TEXTURE_ADDRESS m_eCurrentTextureAddress[MAX_IMAGE_UNITS][3];  // U, V, W
		UINT m_nCurrentTextureStageState[MAX_IMAGE_UNITS][_D3DTSS_MAX];
		DWORD m_dwCurrentMaxAnisotropy[MAX_IMAGE_UNITS];
		float m_fCurrentMipmapLodBias[MAX_IMAGE_UNITS];
		DWORD m_dwCurrentTextureBorderColor[MAX_IMAGE_UNITS];

		//------------------------------------------------------------------------
		// 버퍼 관리
		//------------------------------------------------------------------------
		static constexpr UINT MAX_VERTEX_STREAMS = 4;

		DWORD m_dwCurrentFVF;
		LPDIRECT3DVERTEXDECLARATION9 m_pCurrentVertexDecl;
		_DX9_VERTEX_BUFFER* m_pCurrentVertexBuffers[MAX_VERTEX_STREAMS];
		UINT m_nCurrentVertexOffsets[MAX_VERTEX_STREAMS];
		UINT m_nCurrentVertexFrequency[MAX_VERTEX_STREAMS];
		_DX9_INDEX_BUFFER* m_pCurrentIndexBuffer;
		UINT m_nCurrentInstanceCount;

		std::list<_DX9_VERTEX_BUFFER*> m_listVertexBuffers;
		std::list<_DX9_INDEX_BUFFER*> m_listIndexBuffers;
		std::list<_DX9_TEXTURE*> m_listTextures;

		//------------------------------------------------------------------------
		// 블렌딩 상태
		//------------------------------------------------------------------------
		bool m_bCurrentBlendEnable;
		E_BLEND m_eCurrentSrcBlend;
		E_BLEND m_eCurrentDstBlend;
		E_BLEND_OP m_eCurrentBlendOp;

		bool m_bCurrentSeparateBlendEnable;
		E_BLEND m_eCurrentSeparateSrcBlend;
		E_BLEND m_eCurrentSeparateDstBlend;
		E_BLEND_OP m_eCurrentSeparateBlendOp;

		DWORD m_dwCurrentBlendFactor;
		DWORD m_dwCurrentTextureFactor;

		//------------------------------------------------------------------------
		// 깊이/스텐실 상태
		//------------------------------------------------------------------------
		E_CMP_FUNC m_eCurrentDepthFunc;
		bool m_bCurrentDepthTestEnable;
		bool m_bCurrentDepthWriteEnable;

		E_CMP_FUNC m_eCurrentStencilFunc;
		E_CMP_FUNC m_eCurrentStencilBackFunc;
		DWORD m_dwCurrentStencilRef;
		DWORD m_dwCurrentStencilMask;
		bool m_bCurrentStencilEnable;
		E_STENCIL_OP m_eCurrentStencilPassOp;
		E_STENCIL_OP m_eCurrentStencilFailOp;
		E_STENCIL_OP m_eCurrentStencilZFailOp;

		//------------------------------------------------------------------------
		// 기타 렌더 상태
		//------------------------------------------------------------------------
		bool m_bCurrentColorWriteEnable;
		E_CULL_MODE m_eCurrentCullMode;
		E_FILL_MODE m_eCurrentFillMode;
		_DMATRIX9 m_matCurrentTransform[static_cast<int>(E_TRANSFORM_TYPE::MAX)];

		bool m_bCurrentLightingEnable;
		bool m_bCurrentNormalizeNormals;
		DWORD m_dwCurrentAmbientColor;

		bool m_bCurrentAlphaTestEnable;
		DWORD m_dwCurrentAlphaRef;
		E_CMP_FUNC m_eCurrentAlphaFunc;

		bool m_bCurrentClippingEnable;
		bool m_bCurrentScissorTestEnable;

		bool m_bCurrentFogEnable;
		DWORD m_dwCurrentFogColor;
		float m_fCurrentFogNear;
		float m_fCurrentFogFar;
		float m_fCurrentFogDensity;
		bool m_bCurrentFogLinear;
		bool m_bCurrentFogPixel;
		bool m_bCurrentFogRange;

		bool m_bCurrentVertexBlendEnable;
		bool m_bCurrentIndexedVertexBlendEnable;
		bool m_bCurrentSpecularEnable;
		bool m_bCurrentColorVertexEnable;

		DWORD m_dwCurrentClipPlaneEnable;
		float m_fCurrentDepthBias;
		float m_fCurrentSlopeScaleDepthBias;

		//------------------------------------------------------------------------
		// 추가 상태 캐싱 (텍스처 스테이지, 스텐실, 포인트)
		//------------------------------------------------------------------------
		DWORD m_dwCurrentStencilWriteMask;
		E_STENCIL_OP m_eCurrentStencilCCWPassOp;
		E_STENCIL_OP m_eCurrentStencilCCWFailOp;
		E_STENCIL_OP m_eCurrentStencilCCWZFailOp;
		bool m_bCurrentTwoSidedStencil;

		float m_fCurrentPointSize;
		bool m_bCurrentPointSpriteEnable;

		//------------------------------------------------------------------------
		// 뷰포트 및 시저 렉트
		//------------------------------------------------------------------------
		_DVIEWPORT9 m_currentViewport;
		RECT m_rectCurrentScissor;

		//------------------------------------------------------------------------
		// 타이밍
		//------------------------------------------------------------------------
		UINT m_nFrameCount;
		UINT m_nLastFPSFrameCount;
		float m_fFPS;
		DWORD m_dwLastFPSTime;
		DWORD m_dwLastFlipTime;
		DWORD m_dwLastElapsedTime;

		//------------------------------------------------------------------------
		// 기본 텍스처
		//------------------------------------------------------------------------
		_DX9_TEXTURE* m_pDefaultTexture;
		_DX9_TEXTURE* m_pDefaultNoiseTexture;

		//------------------------------------------------------------------------
		// 2D 렌더링 모드 상태
		//------------------------------------------------------------------------
		bool m_bIn2DMode;                            ///< 2D 모드 여부
		_DMATRIX9 m_matSaved2DView;                  ///< 2D 진입 전 View 행렬
		_DMATRIX9 m_matSaved2DProj;                  ///< 2D 진입 전 Projection 행렬
		_DMATRIX9 m_matSaved2DWorld;                 ///< 2D 진입 전 World 행렬

		//------------------------------------------------------------------------
		// 생성자/소멸자 (private - 싱글톤)
		//------------------------------------------------------------------------
		C_DX9_ENGINE();
		~C_DX9_ENGINE();
		C_DX9_ENGINE(const C_DX9_ENGINE&) = delete;
		C_DX9_ENGINE& operator=(const C_DX9_ENGINE&) = delete;

		//------------------------------------------------------------------------
		// 내부 함수
		//------------------------------------------------------------------------
		bool CreateDevice();
		void InitDeviceDefault();
		void ClearStates();

		UINT GetSamplerToIndex(UINT _nSampler);
		UINT GetIndexToSampler(UINT _nIndex);

	public:
		//========================================================================
		// 싱글톤 접근
		//========================================================================

		/**
		 * @brief 싱글톤 인스턴스 획득
		 * @return 엔진 인스턴스 포인터
		 */
		static C_DX9_ENGINE* GetInstance();

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
		bool Initialize(const _ENGINE_3D_CONFIG& _config);

		/**
		 * @brief 외부 디바이스로 엔진 초기화
		 * @param _pExternalDevice 외부 D3D9 디바이스
		 * @param _nScreenWidth 화면 너비
		 * @param _nScreenHeight 화면 높이
		 * @return 성공 시 true
		 */
		bool InitializeWithExternalDevice(LPDIRECT3DDEVICE9 _pExternalDevice, UINT _nScreenWidth, UINT _nScreenHeight);

		/**
		 * @brief 엔진 종료 및 리소스 해제
		 */
		void Shutdown();

		/**
		 * @brief 디바이스 리셋 (설정 변경 후)
		 * @return 성공 시 true
		 */
		bool ResetDevice();

		//========================================================================
		// 디바이스 상태/정보
		//========================================================================

		LPDIRECT3DDEVICE9 GetDevice() const { return m_pDevice; }
		LPDIRECT3D9 GetD3D() const { return m_pD3D; }
		const D3DCAPS9& GetCaps() const { return m_d3dCaps; }

		int GetScreenWidth() const { return static_cast<int>(m_config.nScreenWidth); }
		int GetScreenHeight() const { return static_cast<int>(m_config.nScreenHeight); }
		D3DFORMAT GetDepthFormat() const { return m_config.eDepthStencilFormat; }

		DWORD GetLastElapsedTime() const { return m_dwLastElapsedTime; }
		UINT GetFrameCount() const { return m_nFrameCount; }
		float GetFrameRate() const { return m_fFPS; }

		bool IsInitialized() const { return m_bInitialized; }
		bool IsDeviceLost() const { return m_bDeviceLost; }

		/**
		 * @brief 디바이스 기능 쿼리
		 * @param _eFeature 쿼리할 기능
		 * @return 지원 시 true
		 */
		bool QueryFeature(E_QUERY_FEATURE _eFeature);

		/**
		 * @brief 디바이스 상태 쿼리
		 * @return 디바이스 상태
		 */
		_DEVICE_STATUS_ QueryStatus();

		/**
		 * @brief 커서 표시 여부
		 */
		bool IsCursorVisible() const { return m_bCursorVisible; }
		bool ShowCursor(bool _bShow);

		const _ENGINE_PROFILE_INFO& GetProfileInfo() const { return m_profileLast; }
		const _ENGINE_3D_CONFIG& GetConfig() const { return m_config; }

		//========================================================================
		// 씬 렌더링
		//========================================================================

		/**
		 * @brief 씬 렌더링 시작
		 * @return 성공 시 true
		 */
		bool BeginScene();

		/**
		 * @brief 씬 렌더링 종료
		 */
		void EndScene();

		/**
		 * @brief 화면 클리어
		 * @param _bTarget 타겟 클리어 여부
		 * @param _bDepth 깊이 버퍼 클리어 여부
		 * @param _bStencil 스텐실 버퍼 클리어 여부
		 * @param _dwColor 클리어 색상
		 * @param _fDepth 클리어 깊이값
		 * @param _dwStencil 클리어 스텐실값
		 * @param _nTargetIdx 렌더 타겟 인덱스
		 */
		void Clear(bool _bTarget = true, bool _bDepth = true, bool _bStencil = false,
			DWORD _dwColor = 0, float _fDepth = 1.0f, DWORD _dwStencil = 0, DWORD _nTargetIdx = 0);

		/**
		 * @brief 화면 출력 (Flip)
		 */
		void Flip();

		/**
		 * @brief 화면 출력 (Present)
		 * @return 성공 시 true
		 */
		bool Present(HWND _hDestWindow = nullptr, LPRECT _pDst = nullptr, LPRECT _pSrc = nullptr);

		//========================================================================
		// 렌더 타겟/깊이 버퍼 관리
		//========================================================================

		/**
		 * @brief 렌더 타겟 설정
		 * @param _nIndex 렌더 타겟 인덱스 (0~3)
		 * @param _pTexture 텍스처
		 * @param _nSurface 서페이스 인덱스 (큐브맵용)
		 * @return 이전 렌더 타겟
		 */
		_DX9_TEXTURE* SetRenderTarget(UINT _nIndex, _DX9_TEXTURE* _pTexture, int _nSurface = 0);
		_DX9_TEXTURE* GetRenderTarget(UINT _nIndex) const;

		/**
		 * @brief 깊이 스텐실 버퍼 설정
		 * @param _pTexture 텍스처
		 * @return 이전 깊이 스텐실 버퍼
		 */
		_DX9_TEXTURE* SetDepthStencilBuffer(_DX9_TEXTURE* _pTexture);
		_DX9_TEXTURE* GetDepthStencilBuffer() const;

		//========================================================================
		// 버텍스 버퍼 관리
		//========================================================================

		/**
		 * @brief 버텍스 버퍼 생성
		 * @param _nSize 버퍼 크기 (바이트)
		 * @param _nVertexSize 버텍스 크기
		 * @param _pData 초기 데이터 (nullptr 가능)
		 * @param _dwFlags 생성 플래그
		 * @return 버텍스 버퍼 포인터
		 */
		_DX9_VERTEX_BUFFER* CreateVertexBuffer(UINT _nSize, UINT _nVertexSize, const void* _pData = nullptr, DWORD _dwFlags = 0);
		void DeleteVertexBuffer(_DX9_VERTEX_BUFFER* _pBuffer);

		void* LockVertexBuffer(_DX9_VERTEX_BUFFER* _pBuffer, DWORD _dwFlags = 0, UINT _nOffset = 0, UINT _nSize = 0);
		bool UnlockVertexBuffer(_DX9_VERTEX_BUFFER* _pBuffer);

		/**
		 * @brief 버텍스 버퍼 바인딩
		 * @param _pBuffer 버텍스 버퍼
		 * @param _nStream 스트림 인덱스
		 * @param _nOffset 오프셋
		 */
		void SetVertexBuffer(_DX9_VERTEX_BUFFER* _pBuffer, UINT _nStream = 0, UINT _nOffset = 0);

		//========================================================================
		// 인덱스 버퍼 관리
		//========================================================================

		/**
		 * @brief 인덱스 버퍼 생성
		 * @param _nIndices 인덱스 수
		 * @param _b32Bit 32비트 인덱스 여부
		 * @param _pData 초기 데이터
		 * @param _dwFlags 생성 플래그
		 * @return 인덱스 버퍼 포인터
		 */
		_DX9_INDEX_BUFFER* CreateIndexBuffer(UINT _nIndices, bool _b32Bit = false, const void* _pData = nullptr, DWORD _dwFlags = 0);
		void DeleteIndexBuffer(_DX9_INDEX_BUFFER* _pBuffer);

		void* LockIndexBuffer(_DX9_INDEX_BUFFER* _pBuffer, DWORD _dwFlags = 0, UINT _nOffset = 0, UINT _nSize = 0);
		bool UnlockIndexBuffer(_DX9_INDEX_BUFFER* _pBuffer);

		void SetIndexBuffer(_DX9_INDEX_BUFFER* _pBuffer);

		//========================================================================
		// 버텍스 포맷
		//========================================================================

		/**
		 * @brief FVF 설정
		 * @param _dwFVF FVF 플래그
		 */
		void SetFVF(DWORD _dwFVF);
		DWORD GetFVF() const { return m_dwCurrentFVF; }

		/**
		 * @brief 버텍스 선언 설정
		 * @param _pDecl 버텍스 선언
		 */
		void SetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 _pDecl);

		//========================================================================
		// 텍스처 관리
		//========================================================================

		/**
		 * @brief 텍스처 생성 (빈 텍스처)
		 * @param _nWidth 너비
		 * @param _nHeight 높이
		 * @param _eFormat 포맷
		 * @param _dwFlags 생성 플래그
		 * @return 텍스처 포인터
		 */
		_DX9_TEXTURE* CreateTexture(UINT _nWidth, UINT _nHeight, D3DFORMAT _eFormat = D3DFMT_A8R8G8B8, DWORD _dwFlags = 0);

		/**
		 * @brief 텍스처 로드 (파일에서)
		 * @param _strFilePath 파일 경로
		 * @param _eFilter 필터 타입
		 * @param _dwFlags 생성 플래그
		 * @return 텍스처 포인터
		 */
		_DX9_TEXTURE* CreateTextureFromFile(const std::wstring& _strFilePath, E_TEXTURE_FILTER _eFilter = E_TEXTURE_FILTER::LINEAR, DWORD _dwFlags = TCF_AUTOGENMIPMAP);

		/**
		 * @brief 렌더 타겟 텍스처 생성
		 */
		_DX9_TEXTURE* CreateRenderTargetTexture(UINT _nWidth, UINT _nHeight, D3DFORMAT _eFormat, E_TEXTURE_FILTER _eFilter = E_TEXTURE_FILTER::LINEAR, DWORD _dwFlags = 0);

		/**
		 * @brief 깊이 스텐실 텍스처 생성
		 */
		_DX9_TEXTURE* CreateDepthStencilTexture(UINT _nWidth, UINT _nHeight, D3DFORMAT _eFormat);

		void DeleteTexture(_DX9_TEXTURE* _pTexture);

		/**
		 * @brief 텍스처 스테이지에 바인딩
		 * @param _nStage 스테이지 번호
		 * @param _pTexture 텍스처
		 */
		void SetTexture(UINT _nStage, _DX9_TEXTURE* _pTexture);
		_DX9_TEXTURE* GetTexture(UINT _nStage) const;

		/**
		 * @brief 텍스처 락
		 */
		bool LockTexture(_DX9_TEXTURE* _pTexture, int _nLevel, _LOCKED_RECT* _pLockedRect, const RECT* _pRect = nullptr, DWORD _dwFlags = 0);
		bool UnlockTexture(_DX9_TEXTURE* _pTexture, int _nLevel);

		/**
		 * @brief 기본 텍스처 획득
		 */
		_DX9_TEXTURE* GetDefaultTexture() const { return m_pDefaultTexture; }
		_DX9_TEXTURE* GetDefaultNoiseTexture() const { return m_pDefaultNoiseTexture; }

		//========================================================================
		// 텍스처 상태
		//========================================================================

		void SetTextureStageState(UINT _nStage, _TEXTURE_STAGE_STATE_TYPE_ _eType, UINT _nValue);
		void SetTextureFilter(UINT _nSampler, E_TEXTURE_FILTER _eFilter);
		void SetTextureAddress(UINT _nSampler, E_TEXTURE_ADDRESS _eU, E_TEXTURE_ADDRESS _eV, E_TEXTURE_ADDRESS _eW);
		void SetTextureAddressClamp(UINT _nSampler);
		void SetTextureAddressWrap(UINT _nSampler);
		void SetTextureMipmapLodBias(UINT _nStage, float _fBias);
		void SetTextureMaxAnisotropy(UINT _nStage, DWORD _dwValue);
		void SetTextureBorderColor(UINT _nSampler, DWORD _dwColor);

		//========================================================================
		// 변환 행렬
		//========================================================================

		void SetTransform(E_TRANSFORM_TYPE _eType, const _DMATRIX9& _matrix);
		_DMATRIX9 GetTransform(E_TRANSFORM_TYPE _eType) const;

		//========================================================================
		// 뷰포트
		//========================================================================

		void SetViewport(const _DVIEWPORT9& _viewport);
		void SetViewport(DWORD _x, DWORD _y, DWORD _nWidth, DWORD _nHeight, float _fMinZ = 0.0f, float _fMaxZ = 1.0f);
		_DVIEWPORT9 GetViewport() const { return m_currentViewport; }

		//========================================================================
		// 블렌딩 상태
		//========================================================================

		/**
		 * @brief 블렌딩 설정
		 * @param _eSrc 소스 블렌드
		 * @param _eDst 대상 블렌드
		 * @param _eOp 블렌드 연산
		 */
		void SetBlending(E_BLEND _eSrc, E_BLEND _eDst = E_BLEND::NONE, E_BLEND_OP _eOp = E_BLEND_OP::ADD);
		void SetSeparateBlending(E_BLEND _eSrc, E_BLEND _eDst = E_BLEND::NONE, E_BLEND_OP _eOp = E_BLEND_OP::ADD);
		void SetBlendFactor(DWORD _dwColor);
		void SetTextureFactor(DWORD _dwColor);

		//========================================================================
		// 알파 테스트
		//========================================================================

		void SetAlphaTestEnable(bool _bEnable);
		void SetAlphaRef(DWORD _dwRef);
		void SetAlphaFunc(E_CMP_FUNC _eFunc);
		bool GetAlphaTestEnable() const { return m_bCurrentAlphaTestEnable; }
		DWORD GetAlphaRef() const { return m_dwCurrentAlphaRef; }
		E_CMP_FUNC GetAlphaFunc() const { return m_eCurrentAlphaFunc; }

		//========================================================================
		// 깊이/스텐실 상태
		//========================================================================

		void SetDepthFunc(E_CMP_FUNC _eFunc);
		void SetDepthEnable(bool _bEnable, bool _bWriteEnable = true);
		void SetColorWriteEnable(bool _bEnable);

		void SetStencilEnable(bool _bEnable);
		void SetStencilTwoSide(bool _bEnable);
		void SetStencilRef(DWORD _dwValue);
		void SetStencilMask(DWORD _dwValue);
		void SetStencilWriteMask(DWORD _dwValue);
		void SetStencilFunc(E_CMP_FUNC _eFunc);
		void SetStencilOp(E_STENCIL_OP _ePass, E_STENCIL_OP _eFail, E_STENCIL_OP _eZFail);
		void SetStencilCCWFunc(E_CMP_FUNC _eFunc);
		void SetStencilCCWOp(E_STENCIL_OP _ePass, E_STENCIL_OP _eFail, E_STENCIL_OP _eZFail);

		//========================================================================
		// 기타 렌더 상태
		//========================================================================

		void SetCullMode(E_CULL_MODE _eMode);
		void SetFillMode(E_FILL_MODE _eMode);
		E_FILL_MODE GetFillMode() const { return m_eCurrentFillMode; }

		void SetClipPlanes(const D3DXPLANE* _pPlanes, int _nCount);
		void SetDepthBias(float _fDepthBias = 0.0f, float _fSlopeScale = 0.0f);

		void SetLighting(bool _bEnable);
		void SetAmbient(DWORD _dwColor);
		void SetNormalizeNormals(bool _bEnable);

		void SetFogEnable(bool _bEnable);
		void SetFog(bool _bLinear, DWORD _dwColor, float _fNear, float _fFar, float _fDensity = 1.0f, bool _bPixel = true, bool _bRange = false);
		void SetFogColor(DWORD _dwColor);
		bool GetFogEnable() const { return m_bCurrentFogEnable; }
		DWORD GetFogColor() const { return m_dwCurrentFogColor; }
		float GetFogNear() const { return m_fCurrentFogNear; }
		float GetFogFar() const { return m_fCurrentFogFar; }

		void SetClipping(bool _bEnable);
		void SetScissorTestEnable(bool _bEnable);
		void SetScissorRect(const RECT* _pRect);

		void SetPointSize(DWORD _nSize);
		void SetPointSpriteEnable(bool _bEnable);

		void SetVertexBlendEnable(bool _bEnable);
		void SetIndexedVertexBlendEnable(bool _bEnable);
		void SetSpecularEnable(bool _bEnable);
		void SetColorVertex(bool _bEnable);

		//========================================================================
		// 드로우 프리미티브
		//========================================================================

		/**
		 * @brief DrawPrimitive
		 */
		bool DrawPrimitive(E_PRIMITIVE_TYPE _eType, UINT _nStart, UINT _nPrimitiveCount);

		/**
		 * @brief DrawIndexedPrimitive
		 */
		bool DrawIndexedPrimitive(E_PRIMITIVE_TYPE _eType, INT _nBaseVertexIndex, UINT _nMinIndex, UINT _nNumVertices,
			UINT _nStartIndex, UINT _nPrimitiveCount);

		/**
		 * @brief DrawPrimitiveUP (버텍스 버퍼 없이)
		 */
		bool DrawPrimitiveUP(E_PRIMITIVE_TYPE _eType, UINT _nPrimitiveCount, const void* _pVertexData, UINT _nVertexStride);

		/**
		 * @brief DrawIndexedPrimitiveUP (버퍼 없이)
		 */
		bool DrawIndexedPrimitiveUP(E_PRIMITIVE_TYPE _eType, UINT _nMinVertexIndex, UINT _nNumVertices, UINT _nPrimitiveCount,
			const void* _pIndexData, const void* _pVertexData, UINT _nVertexStride, bool _b32BitIndex = false);

		//========================================================================
		// 인스턴싱
		//========================================================================

		/**
		 * @brief 인스턴싱 설정
		 */
		void SetupForRenderInstancing(_DX9_VERTEX_BUFFER* _pInstanceBuffer, UINT _nInstanceCount, UINT _nOffset = 0);
		void RestoreForRenderInstancing();

		//========================================================================
		// 셰이더 관련
		//========================================================================

		/**
		 * @brief 셰이더 비활성화
		 */
		void ShaderOff();

		//========================================================================
		// 2D 렌더링 지원 (UI, 폰트 등)
		//========================================================================

		/**
		 * @brief 2D 렌더링 모드 시작
		 * @details 직교 투영 행렬 설정, World/View를 단위행렬로 설정
		 *          뷰포트 픽셀과 1:1 매핑되도록 설정
		 * @return 성공 시 true
		 *
		 * 사용 예시:
		 * @code
		 * pEngine->Begin2D();
		 * // 2D 렌더링 (UI, 폰트, 스프라이트 등)
		 * pEngine->DrawPrimitiveUP(E_PRIMITIVE_TYPE::TRIANGLELIST, 2, vertices, sizeof(VERTEX_2D));
		 * pEngine->End2D();
		 * @endcode
		 */
		bool Begin2D();

		/**
		 * @brief 2D 렌더링 모드 종료
		 * @details 이전 변환 행렬 복원
		 */
		void End2D();

		/**
		 * @brief 2D 모드 여부 확인
		 */
		bool IsIn2DMode() const { return m_bIn2DMode; }

		//========================================================================
		// 라이트/머티리얼 (3D 렌더링용)
		//========================================================================

		/**
		 * @brief 라이트 설정
		 * @param _nIndex 라이트 인덱스 (0~7)
		 * @param _pLight D3DLIGHT9 구조체 포인터
		 */
		void SetLight(int _nIndex, const D3DLIGHT9* _pLight);

		/**
		 * @brief 라이트 활성화/비활성화
		 * @param _nIndex 라이트 인덱스
		 * @param _bEnable 활성화 여부
		 */
		void SetLightEnable(int _nIndex, bool _bEnable);

		/**
		 * @brief 머티리얼 설정
		 * @param _pMaterial D3DMATERIAL9 구조체 포인터
		 */
		void SetMaterial(const D3DMATERIAL9* _pMaterial);

		//========================================================================
		// 외부 엔진 연동 (RealSpace3 등)
		//========================================================================

		/**
		 * @brief Direct3D Device 포인터 획득
		 * @details 외부 라이브러리/미들웨어 연동용
		 * @return LPDIRECT3DDEVICE9 (void*로 캐스팅)
		 */
		void* GetLPDIRECT3DDEVICE9() const { return m_pDevice; }

		/**
		 * @brief D3DPRESENT_PARAMETERS 포인터 획득
		 * @return D3DPRESENT_PARAMETERS 포인터
		 */
		void* GetD3DPRESENT_PARAMETERS() { return &m_d3dpp; }

		//========================================================================
		// 유틸리티
		//========================================================================

		/**
		 * @brief 스크린샷 저장
		 */
		bool SaveScreenShot(const std::wstring& _strFilePath);

		/**
		 * @brief 디바이스 손실 처리
		 */
		void OnLostDevice();
		void OnResetDevice();
	};

	//============================================================================
	// 전역 헬퍼 함수
	//============================================================================

	/**
	 * @brief 전역 3D 엔진 인스턴스 획득
	 */
	inline C_DX9_ENGINE* Get3DEngine()
	{
		return C_DX9_ENGINE::GetInstance();
	}

} // namespace dx9