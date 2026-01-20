/**
 * @file DX9Engine.cpp
 * @brief DirectX 9 3D 그래픽 엔진 구현
 * @details C_DX9_ENGINE 클래스의 메서드 구현
 *
 * @author DX9Wrapp
 * @date 2026-01-20
 */

#include "framework.h"
#include "DX9Engine.h"

namespace dx9
{

	//============================================================================
	// 정적 멤버 초기화
	//============================================================================
	C_DX9_ENGINE* C_DX9_ENGINE::s_pInstance = nullptr;

	//============================================================================
	// D3D 변환 테이블
	//============================================================================
	static constexpr D3DPRIMITIVETYPE s_d3dPrimitiveTypes[] =
	{
		D3DPT_POINTLIST,      // E_PRIMITIVE_TYPE::POINTLIST
		D3DPT_LINELIST,       // E_PRIMITIVE_TYPE::LINELIST
		D3DPT_LINESTRIP,      // E_PRIMITIVE_TYPE::LINESTRIP
		D3DPT_TRIANGLELIST,   // E_PRIMITIVE_TYPE::TRIANGLELIST
		D3DPT_TRIANGLESTRIP,  // E_PRIMITIVE_TYPE::TRIANGLESTRIP
		D3DPT_TRIANGLEFAN     // E_PRIMITIVE_TYPE::TRIANGLEFAN
	};

	static constexpr D3DBLEND s_d3dBlends[] =
	{
		D3DBLEND_ZERO,            // E_BLEND::ZERO
		D3DBLEND_ONE,             // E_BLEND::ONE
		D3DBLEND_SRCCOLOR,        // E_BLEND::SRCCOLOR
		D3DBLEND_INVSRCCOLOR,     // E_BLEND::INVSRCCOLOR
		D3DBLEND_DESTCOLOR,       // E_BLEND::DESTCOLOR
		D3DBLEND_INVDESTCOLOR,    // E_BLEND::INVDESTCOLOR
		D3DBLEND_SRCALPHA,        // E_BLEND::SRCALPHA
		D3DBLEND_INVSRCALPHA,     // E_BLEND::INVSRCALPHA
		D3DBLEND_DESTALPHA,       // E_BLEND::DESTALPHA
		D3DBLEND_INVDESTALPHA,    // E_BLEND::INVDESTALPHA
		D3DBLEND_SRCALPHASAT,     // E_BLEND::SRCALPHASAT
		D3DBLEND_BLENDFACTOR,     // E_BLEND::BLENDFACTOR
		D3DBLEND_INVBLENDFACTOR,  // E_BLEND::INVBLENDFACTOR
		D3DBLEND_ZERO             // E_BLEND::NONE (사용 안 함)
	};

	static constexpr D3DBLENDOP s_d3dBlendOps[] =
	{
		D3DBLENDOP_ADD,           // E_BLEND_OP::ADD
		D3DBLENDOP_SUBTRACT,      // E_BLEND_OP::SUBTRACT
		D3DBLENDOP_REVSUBTRACT,   // E_BLEND_OP::REVSUBTRACT
		D3DBLENDOP_MIN,           // E_BLEND_OP::MIN
		D3DBLENDOP_MAX            // E_BLEND_OP::MAX
	};

	static constexpr D3DCMPFUNC s_d3dCmpFuncs[] =
	{
		D3DCMP_NEVER,             // E_CMP_FUNC::NEVER
		D3DCMP_LESS,              // E_CMP_FUNC::LESS
		D3DCMP_EQUAL,             // E_CMP_FUNC::EQUAL
		D3DCMP_LESSEQUAL,         // E_CMP_FUNC::LESSEQUAL
		D3DCMP_GREATER,           // E_CMP_FUNC::GREATER
		D3DCMP_NOTEQUAL,          // E_CMP_FUNC::NOTEQUAL
		D3DCMP_GREATEREQUAL,      // E_CMP_FUNC::GREATEREQUAL
		D3DCMP_ALWAYS             // E_CMP_FUNC::ALWAYS
	};

	static constexpr D3DCULL s_d3dCullModes[] =
	{
		D3DCULL_NONE,             // E_CULL_MODE::NONE
		D3DCULL_CCW,              // E_CULL_MODE::CCW
		D3DCULL_CW                // E_CULL_MODE::CW
	};

	static constexpr D3DFILLMODE s_d3dFillModes[] =
	{
		D3DFILL_POINT,            // E_FILL_MODE::POINT
		D3DFILL_WIREFRAME,        // E_FILL_MODE::WIREFRAME
		D3DFILL_SOLID             // E_FILL_MODE::SOLID
	};

	static constexpr D3DSTENCILOP s_d3dStencilOps[] =
	{
		D3DSTENCILOP_KEEP,        // E_STENCIL_OP::KEEP
		D3DSTENCILOP_ZERO,        // E_STENCIL_OP::ZERO
		D3DSTENCILOP_REPLACE,     // E_STENCIL_OP::REPLACE
		D3DSTENCILOP_INCRSAT,     // E_STENCIL_OP::INCRSAT
		D3DSTENCILOP_DECRSAT,     // E_STENCIL_OP::DECRSAT
		D3DSTENCILOP_INVERT,      // E_STENCIL_OP::INVERT
		D3DSTENCILOP_INCR,        // E_STENCIL_OP::INCR
		D3DSTENCILOP_DECR         // E_STENCIL_OP::DECR
	};

	static constexpr D3DTEXTUREADDRESS s_d3dTextureAddressModes[] =
	{
		D3DTADDRESS_WRAP,         // E_TEXTURE_ADDRESS::WRAP
		D3DTADDRESS_MIRROR,       // E_TEXTURE_ADDRESS::MIRROR
		D3DTADDRESS_CLAMP,        // E_TEXTURE_ADDRESS::CLAMP
		D3DTADDRESS_BORDER,       // E_TEXTURE_ADDRESS::BORDER
		D3DTADDRESS_MIRRORONCE    // E_TEXTURE_ADDRESS::MIRRORONCE
	};

	static constexpr D3DTRANSFORMSTATETYPE s_d3dTransformTypes[] =
	{
		D3DTS_WORLD,              // E_TRANSFORM_TYPE::WORLD
		D3DTS_VIEW,               // E_TRANSFORM_TYPE::VIEW
		D3DTS_PROJECTION,         // E_TRANSFORM_TYPE::PROJECTION
		D3DTS_TEXTURE0,           // E_TRANSFORM_TYPE::TEXTURE0
		D3DTS_TEXTURE1,           // E_TRANSFORM_TYPE::TEXTURE1
		D3DTS_TEXTURE2,           // E_TRANSFORM_TYPE::TEXTURE2
		D3DTS_TEXTURE3,           // E_TRANSFORM_TYPE::TEXTURE3
		D3DTS_TEXTURE4,           // E_TRANSFORM_TYPE::TEXTURE4
		D3DTS_TEXTURE5,           // E_TRANSFORM_TYPE::TEXTURE5
		D3DTS_TEXTURE6,           // E_TRANSFORM_TYPE::TEXTURE6
		D3DTS_TEXTURE7            // E_TRANSFORM_TYPE::TEXTURE7
	};

	//============================================================================
	// 텍스처 필터 설정 테이블
	//============================================================================
	struct _TEXTURE_FILTER_SET
	{
		D3DTEXTUREFILTERTYPE minFilter;
		D3DTEXTUREFILTERTYPE magFilter;
		D3DTEXTUREFILTERTYPE mipFilter;
	};

	static constexpr _TEXTURE_FILTER_SET s_textureFilterSets[] =
	{
		{ D3DTEXF_POINT,       D3DTEXF_POINT,       D3DTEXF_NONE   },  // POINT
		{ D3DTEXF_LINEAR,      D3DTEXF_LINEAR,      D3DTEXF_NONE   },  // LINEAR
		{ D3DTEXF_LINEAR,      D3DTEXF_LINEAR,      D3DTEXF_POINT  },  // BILINEAR
		{ D3DTEXF_LINEAR,      D3DTEXF_LINEAR,      D3DTEXF_LINEAR },  // TRILINEAR
		{ D3DTEXF_ANISOTROPIC, D3DTEXF_ANISOTROPIC, D3DTEXF_POINT  },  // BILINEAR_ANISO
		{ D3DTEXF_ANISOTROPIC, D3DTEXF_ANISOTROPIC, D3DTEXF_LINEAR }   // TRILINEAR_ANISO
	};

	//============================================================================
	// 생성자/소멸자
	//============================================================================
	C_DX9_ENGINE::C_DX9_ENGINE()
		: m_pD3D(nullptr)
		, m_pDevice(nullptr)
		, m_bInitialized(false)
		, m_bDeviceLost(false)
		, m_bInScene(false)
		, m_bOwnDevice(true)
		, m_bCursorVisible(true)
		, m_pCurrentDepthStencil(nullptr)
		, m_dwCurrentFVF(0)
		, m_pCurrentVertexDecl(nullptr)
		, m_pCurrentIndexBuffer(nullptr)
		, m_nCurrentInstanceCount(0)
		, m_bCurrentBlendEnable(false)
		, m_eCurrentSrcBlend(E_BLEND::ONE)
		, m_eCurrentDstBlend(E_BLEND::ZERO)
		, m_eCurrentBlendOp(E_BLEND_OP::ADD)
		, m_bCurrentSeparateBlendEnable(false)
		, m_eCurrentSeparateSrcBlend(E_BLEND::ONE)
		, m_eCurrentSeparateDstBlend(E_BLEND::ZERO)
		, m_eCurrentSeparateBlendOp(E_BLEND_OP::ADD)
		, m_dwCurrentBlendFactor(0xFFFFFFFF)
		, m_dwCurrentTextureFactor(0xFFFFFFFF)
		, m_eCurrentDepthFunc(E_CMP_FUNC::LESSEQUAL)
		, m_bCurrentDepthTestEnable(true)
		, m_bCurrentDepthWriteEnable(true)
		, m_eCurrentStencilFunc(E_CMP_FUNC::ALWAYS)
		, m_eCurrentStencilBackFunc(E_CMP_FUNC::ALWAYS)
		, m_dwCurrentStencilRef(0)
		, m_dwCurrentStencilMask(0xFFFFFFFF)
		, m_bCurrentStencilEnable(false)
		, m_eCurrentStencilPassOp(E_STENCIL_OP::KEEP)
		, m_eCurrentStencilFailOp(E_STENCIL_OP::KEEP)
		, m_eCurrentStencilZFailOp(E_STENCIL_OP::KEEP)
		, m_bCurrentColorWriteEnable(true)
		, m_eCurrentCullMode(E_CULL_MODE::CCW)
		, m_eCurrentFillMode(E_FILL_MODE::SOLID)
		, m_bCurrentLightingEnable(false)
		, m_bCurrentNormalizeNormals(false)
		, m_dwCurrentAmbientColor(0)
		, m_bCurrentAlphaTestEnable(false)
		, m_dwCurrentAlphaRef(0)
		, m_eCurrentAlphaFunc(E_CMP_FUNC::ALWAYS)
		, m_bCurrentClippingEnable(true)
		, m_bCurrentScissorTestEnable(false)
		, m_bCurrentFogEnable(false)
		, m_dwCurrentFogColor(0)
		, m_fCurrentFogNear(0.0f)
		, m_fCurrentFogFar(1.0f)
		, m_fCurrentFogDensity(1.0f)
		, m_bCurrentFogLinear(true)
		, m_bCurrentFogPixel(true)
		, m_bCurrentFogRange(false)
		, m_bCurrentVertexBlendEnable(false)
		, m_bCurrentIndexedVertexBlendEnable(false)
		, m_bCurrentSpecularEnable(false)
		, m_bCurrentColorVertexEnable(true)
		, m_dwCurrentClipPlaneEnable(0)
		, m_fCurrentDepthBias(0.0f)
		, m_fCurrentSlopeScaleDepthBias(0.0f)
		, m_dwCurrentStencilWriteMask(0xFFFFFFFF)
		, m_eCurrentStencilCCWPassOp(E_STENCIL_OP::KEEP)
		, m_eCurrentStencilCCWFailOp(E_STENCIL_OP::KEEP)
		, m_eCurrentStencilCCWZFailOp(E_STENCIL_OP::KEEP)
		, m_bCurrentTwoSidedStencil(false)
		, m_fCurrentPointSize(1.0f)
		, m_bCurrentPointSpriteEnable(false)
		, m_nFrameCount(0)
		, m_nLastFPSFrameCount(0)
		, m_fFPS(0.0f)
		, m_dwLastFPSTime(0)
		, m_dwLastFlipTime(0)
		, m_dwLastElapsedTime(0)
		, m_pDefaultTexture(nullptr)
		, m_pDefaultNoiseTexture(nullptr)
		, m_bIn2DMode(false)
	{
		ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
		ZeroMemory(&m_d3dCaps, sizeof(m_d3dCaps));
		ZeroMemory(&m_rectCurrentScissor, sizeof(m_rectCurrentScissor));

		// 렌더 타겟 초기화
		for (UINT i = 0; i < MAX_RENDER_TARGETS; ++i)
		{
			m_pCurrentRenderTargets[i] = nullptr;
			m_nCurrentRenderTargetSurface[i] = 0;
		}

		// 텍스처 상태 초기화
		for (UINT i = 0; i < MAX_IMAGE_UNITS; ++i)
		{
			m_pCurrentTextures[i] = nullptr;
			m_eCurrentTextureFilter[i] = E_TEXTURE_FILTER::LINEAR;
			m_eCurrentTextureAddress[i][0] = E_TEXTURE_ADDRESS::WRAP;
			m_eCurrentTextureAddress[i][1] = E_TEXTURE_ADDRESS::WRAP;
			m_eCurrentTextureAddress[i][2] = E_TEXTURE_ADDRESS::WRAP;
			m_dwCurrentMaxAnisotropy[i] = 1;
			m_fCurrentMipmapLodBias[i] = 0.0f;
			m_dwCurrentTextureBorderColor[i] = 0;

			for (UINT j = 0; j < _D3DTSS_MAX; ++j)
			{
				m_nCurrentTextureStageState[i][j] = 0;
			}
		}

		// 버텍스 버퍼 초기화
		for (UINT i = 0; i < MAX_VERTEX_STREAMS; ++i)
		{
			m_pCurrentVertexBuffers[i] = nullptr;
			m_nCurrentVertexOffsets[i] = 0;
			m_nCurrentVertexFrequency[i] = 0;
		}

		// 변환 행렬 초기화
		for (int i = 0; i < static_cast<int>(E_TRANSFORM_TYPE::MAX); ++i)
		{
			m_matCurrentTransform[i].MakeIdentity();
		}
	}

	C_DX9_ENGINE::~C_DX9_ENGINE()
	{
		Shutdown();
	}

	//============================================================================
	// 싱글톤 접근
	//============================================================================
	C_DX9_ENGINE* C_DX9_ENGINE::GetInstance()
	{
		if (s_pInstance == nullptr)
		{
			s_pInstance = new C_DX9_ENGINE();
		}
		return s_pInstance;
	}

	void C_DX9_ENGINE::DestroyInstance()
	{
		if (s_pInstance != nullptr)
		{
			delete s_pInstance;
			s_pInstance = nullptr;
		}
	}

	//============================================================================
	// 초기화/해제
	//============================================================================
	bool C_DX9_ENGINE::Initialize(HWND _hWnd, UINT _nWidth, UINT _nHeight, bool _bWindowed)
	{
		_ENGINE_3D_CONFIG config_;
		config_.hWnd = _hWnd;
		config_.nScreenWidth = _nWidth;
		config_.nScreenHeight = _nHeight;
		config_.bWindowed = _bWindowed;

		return Initialize(config_);
	}

	bool C_DX9_ENGINE::Initialize(const _ENGINE_3D_CONFIG& _config)
	{
		if (m_bInitialized)
		{
			return true;
		}

		m_config = _config;

		// Direct3D 생성
		m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
		if (m_pD3D == nullptr)
		{
			return false;
		}

		// 디바이스 생성
		if (!CreateDevice())
		{
			m_pD3D->Release();
			m_pD3D = nullptr;
			return false;
		}

		m_bOwnDevice = true;
		m_bInitialized = true;

		// 기본 상태 초기화
		InitDeviceDefault();

		return true;
	}

	bool C_DX9_ENGINE::InitializeWithExternalDevice(LPDIRECT3DDEVICE9 _pExternalDevice, UINT _nScreenWidth, UINT _nScreenHeight)
	{
		if (m_bInitialized)
		{
			return true;
		}

		if (_pExternalDevice == nullptr)
		{
			return false;
		}

		m_pDevice = _pExternalDevice;
		m_bOwnDevice = false;

		// Direct3D 인터페이스 획득
		m_pDevice->GetDirect3D(&m_pD3D);

		// 디바이스 능력 획득
		m_pDevice->GetDeviceCaps(&m_d3dCaps);

		m_config.nScreenWidth = _nScreenWidth;
		m_config.nScreenHeight = _nScreenHeight;

		m_bInitialized = true;

		// 기본 상태 초기화
		InitDeviceDefault();

		return true;
	}

	void C_DX9_ENGINE::Shutdown()
	{
		if (!m_bInitialized)
		{
			return;
		}

		// 버텍스 버퍼 해제
		for (auto it = m_listVertexBuffers.begin(); it != m_listVertexBuffers.end(); ++it)
		{
			_DX9_VERTEX_BUFFER* pVB = *it;
			if (pVB != nullptr)
			{
				pVB->Release();
				delete pVB;
			}
		}
		m_listVertexBuffers.clear();

		// 인덱스 버퍼 해제
		for (auto it = m_listIndexBuffers.begin(); it != m_listIndexBuffers.end(); ++it)
		{
			_DX9_INDEX_BUFFER* pIB = *it;
			if (pIB != nullptr)
			{
				pIB->Release();
				delete pIB;
			}
		}
		m_listIndexBuffers.clear();

		// 텍스처 해제
		for (auto it = m_listTextures.begin(); it != m_listTextures.end(); ++it)
		{
			_DX9_TEXTURE* pTex = *it;
			if (pTex != nullptr)
			{
				pTex->Release();
				delete pTex;
			}
		}
		m_listTextures.clear();

		// 기본 텍스처 해제
		if (m_pDefaultTexture != nullptr)
		{
			m_pDefaultTexture->Release();
			delete m_pDefaultTexture;
			m_pDefaultTexture = nullptr;
		}

		if (m_pDefaultNoiseTexture != nullptr)
		{
			m_pDefaultNoiseTexture->Release();
			delete m_pDefaultNoiseTexture;
			m_pDefaultNoiseTexture = nullptr;
		}

		// 디바이스 해제
		if (m_bOwnDevice && m_pDevice != nullptr)
		{
			m_pDevice->Release();
			m_pDevice = nullptr;
		}

		// D3D 해제
		if (m_pD3D != nullptr)
		{
			m_pD3D->Release();
			m_pD3D = nullptr;
		}

		m_bInitialized = false;
	}

	bool C_DX9_ENGINE::CreateDevice()
	{
		// 프레젠트 파라미터 설정
		ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
		m_d3dpp.BackBufferWidth = m_config.nScreenWidth;
		m_d3dpp.BackBufferHeight = m_config.nScreenHeight;
		m_d3dpp.BackBufferFormat = m_config.eBackBufferFormat;
		m_d3dpp.BackBufferCount = 1;
		m_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
		m_d3dpp.MultiSampleQuality = 0;
		m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		m_d3dpp.hDeviceWindow = m_config.hWnd;
		m_d3dpp.Windowed = m_config.bWindowed ? TRUE : FALSE;
		m_d3dpp.EnableAutoDepthStencil = TRUE;
		m_d3dpp.AutoDepthStencilFormat = m_config.eDepthStencilFormat;
		m_d3dpp.Flags = 0;
		m_d3dpp.FullScreen_RefreshRateInHz = 0;
		m_d3dpp.PresentationInterval = m_config.bVSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

		// 디바이스 능력 확인
		HRESULT hr = m_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &m_d3dCaps);
		if (FAILED(hr))
		{
			return false;
		}

		// 디바이스 생성 플래그
		DWORD dwBehaviorFlags = D3DCREATE_FPU_PRESERVE;

		if (m_config.bMultiThread)
		{
			dwBehaviorFlags |= D3DCREATE_MULTITHREADED;
		}

		// 하드웨어 버텍스 프로세싱 지원 확인
		if (m_d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		{
			dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
		else
		{
			dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		// 디바이스 생성
		hr = m_pD3D->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			m_config.hWnd,
			dwBehaviorFlags,
			&m_d3dpp,
			&m_pDevice
		);

		return SUCCEEDED(hr);
	}

	void C_DX9_ENGINE::InitDeviceDefault()
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		// 기본 렌더 스테이트 설정
		m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		m_pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
		m_pDevice->SetRenderState(D3DRS_COLORVERTEX, TRUE);

		// 텍스처 스테이지 기본 설정
		m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		// 샘플러 기본 설정
		for (UINT i = 0; i < MAX_TEXTURE_STAGES; ++i)
		{
			m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			m_pDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			m_pDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			m_pDevice->SetSamplerState(i, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		}

		// 뷰포트 설정
		m_currentViewport.Set(0, 0, m_config.nScreenWidth, m_config.nScreenHeight, 0.0f, 1.0f);
		m_pDevice->SetViewport(&m_currentViewport);

		// 타이밍 초기화
		m_dwLastFPSTime = timeGetTime();
		m_dwLastFlipTime = m_dwLastFPSTime;
	}

	void C_DX9_ENGINE::ClearStates()
	{
		// 텍스처 상태 클리어
		for (UINT i = 0; i < MAX_IMAGE_UNITS; ++i)
		{
			m_pCurrentTextures[i] = nullptr;
		}

		// 버텍스 버퍼 상태 클리어
		for (UINT i = 0; i < MAX_VERTEX_STREAMS; ++i)
		{
			m_pCurrentVertexBuffers[i] = nullptr;
			m_nCurrentVertexOffsets[i] = 0;
		}

		m_pCurrentIndexBuffer = nullptr;
	}

	bool C_DX9_ENGINE::ResetDevice()
	{
		if (m_pDevice == nullptr)
		{
			return false;
		}

		// 디바이스 손실 전 처리
		OnLostDevice();

		// 프레젠트 파라미터 업데이트
		m_d3dpp.BackBufferWidth = m_config.nScreenWidth;
		m_d3dpp.BackBufferHeight = m_config.nScreenHeight;

		// 디바이스 리셋
		HRESULT hr = m_pDevice->Reset(&m_d3dpp);
		if (FAILED(hr))
		{
			return false;
		}

		// 디바이스 복구 후 처리
		OnResetDevice();

		// 기본 상태 재설정
		InitDeviceDefault();

		m_bDeviceLost = false;

		return true;
	}

	//============================================================================
	// 샘플러 인덱스 변환
	//============================================================================
	UINT C_DX9_ENGINE::GetSamplerToIndex(UINT _nSampler)
	{
		// 버텍스 텍스처 샘플러 (D3DVERTEXTEXTURESAMPLER0 ~ 3)
		if (_nSampler >= D3DVERTEXTEXTURESAMPLER0)
		{
			return _nSampler - D3DVERTEXTEXTURESAMPLER0 + MAX_TEXTURE_STAGES;
		}
		return _nSampler;
	}

	UINT C_DX9_ENGINE::GetIndexToSampler(UINT _nIndex)
	{
		if (_nIndex >= MAX_TEXTURE_STAGES)
		{
			return _nIndex - MAX_TEXTURE_STAGES + D3DVERTEXTEXTURESAMPLER0;
		}
		return _nIndex;
	}

	//============================================================================
	// 디바이스 상태/정보
	//============================================================================
	bool C_DX9_ENGINE::QueryFeature(E_QUERY_FEATURE _eFeature)
	{
		switch (_eFeature)
		{
		case E_QUERY_FEATURE::HWSKINNING:
			return (m_d3dCaps.MaxVertexBlendMatrixIndex >= 4);

		case E_QUERY_FEATURE::VERTEXTEXTURE:
			return (m_d3dCaps.VertexShaderVersion >= D3DVS_VERSION(3, 0));

		case E_QUERY_FEATURE::SHADER_MODEL_3:
			return (m_d3dCaps.VertexShaderVersion >= D3DVS_VERSION(3, 0) &&
				m_d3dCaps.PixelShaderVersion >= D3DPS_VERSION(3, 0));

		case E_QUERY_FEATURE::SHADER_MODEL_4:
			return false;  // D3D9는 SM4 미지원

		case E_QUERY_FEATURE::INSTANCING:
			return (m_d3dCaps.VertexShaderVersion >= D3DVS_VERSION(3, 0));

		case E_QUERY_FEATURE::RENDER_TO_TEXTURE:
			return true;  // D3D9는 항상 지원
		}

		return false;
	}

	_DEVICE_STATUS_ C_DX9_ENGINE::QueryStatus()
	{
		if (m_pDevice == nullptr)
		{
			return _DX9_DEVICE_DESTROY;
		}

		HRESULT hr = m_pDevice->TestCooperativeLevel();

		switch (hr)
		{
		case D3D_OK:
			return _DX9_DEVICE_OK;

		case D3DERR_DEVICELOST:
			return _DX9_DEVICE_LOST;

		case D3DERR_DEVICENOTRESET:
			return _DX9_DEVICE_RESTORED;

		default:
			return _DX9_DEVICE_DESTROY;
		}
	}

	bool C_DX9_ENGINE::ShowCursor(bool _bShow)
	{
		m_bCursorVisible = _bShow;
		if (m_pDevice != nullptr)
		{
			m_pDevice->ShowCursor(_bShow ? TRUE : FALSE);
		}
		return m_bCursorVisible;
	}

	//============================================================================
	// 씬 렌더링
	//============================================================================
	bool C_DX9_ENGINE::BeginScene()
	{
		if (m_pDevice == nullptr || m_bInScene)
		{
			return false;
		}

		// 디바이스 상태 확인
		_DEVICE_STATUS_ status_ = QueryStatus();
		if (status_ == _DX9_DEVICE_LOST)
		{
			m_bDeviceLost = true;
			return false;
		}
		else if (status_ == _DX9_DEVICE_RESTORED)
		{
			if (!ResetDevice())
			{
				return false;
			}
		}

		HRESULT hr = m_pDevice->BeginScene();
		if (SUCCEEDED(hr))
		{
			m_bInScene = true;
			m_profileCurrent.Reset();
			return true;
		}

		return false;
	}

	void C_DX9_ENGINE::EndScene()
	{
		if (m_pDevice == nullptr || !m_bInScene)
		{
			return;
		}

		m_pDevice->EndScene();
		m_bInScene = false;

		// 프로파일 정보 복사
		m_profileLast = m_profileCurrent;
	}

	void C_DX9_ENGINE::Clear(bool _bTarget, bool _bDepth, bool _bStencil,
		DWORD _dwColor, float _fDepth, DWORD _dwStencil, DWORD _nTargetIdx)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		DWORD dwFlags = 0;
		if (_bTarget)  dwFlags |= D3DCLEAR_TARGET;
		if (_bDepth)   dwFlags |= D3DCLEAR_ZBUFFER;
		if (_bStencil) dwFlags |= D3DCLEAR_STENCIL;

		m_pDevice->Clear(_nTargetIdx, nullptr, dwFlags, _dwColor, _fDepth, _dwStencil);
	}

	void C_DX9_ENGINE::Flip()
	{
		Present();
	}

	bool C_DX9_ENGINE::Present(HWND _hDestWindow, LPRECT _pDst, LPRECT _pSrc)
	{
		if (m_pDevice == nullptr)
		{
			return false;
		}

		// FPS 계산
		DWORD dwCurrentTime = timeGetTime();
		m_dwLastElapsedTime = dwCurrentTime - m_dwLastFlipTime;
		m_dwLastFlipTime = dwCurrentTime;

		m_nFrameCount++;

		// 1초마다 FPS 업데이트
		if (dwCurrentTime - m_dwLastFPSTime >= 1000)
		{
			m_fFPS = static_cast<float>(m_nFrameCount - m_nLastFPSFrameCount) * 1000.0f /
				static_cast<float>(dwCurrentTime - m_dwLastFPSTime);
			m_dwLastFPSTime = dwCurrentTime;
			m_nLastFPSFrameCount = m_nFrameCount;
		}

		HRESULT hr = m_pDevice->Present(_pSrc, _pDst, _hDestWindow, nullptr);

		return SUCCEEDED(hr);
	}

	//============================================================================
	// 렌더 타겟/깊이 버퍼 관리
	//============================================================================
	_DX9_TEXTURE* C_DX9_ENGINE::SetRenderTarget(UINT _nIndex, _DX9_TEXTURE* _pTexture, int _nSurface)
	{
		if (m_pDevice == nullptr || _nIndex >= MAX_RENDER_TARGETS)
		{
			return nullptr;
		}

		_DX9_TEXTURE* pPrevious = m_pCurrentRenderTargets[_nIndex];

		if (_pTexture != nullptr && _pTexture->pTexture != nullptr)
		{
			LPDIRECT3DSURFACE9 pSurface = nullptr;
			_pTexture->pTexture->GetSurfaceLevel(_nSurface, &pSurface);

			if (pSurface != nullptr)
			{
				m_pDevice->SetRenderTarget(_nIndex, pSurface);
				pSurface->Release();
			}
		}
		else if (_nIndex == 0)
		{
			// 기본 백버퍼로 복원
			LPDIRECT3DSURFACE9 pBackBuffer = nullptr;
			m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
			if (pBackBuffer != nullptr)
			{
				m_pDevice->SetRenderTarget(0, pBackBuffer);
				pBackBuffer->Release();
			}
		}
		else
		{
			m_pDevice->SetRenderTarget(_nIndex, nullptr);
		}

		m_pCurrentRenderTargets[_nIndex] = _pTexture;
		m_nCurrentRenderTargetSurface[_nIndex] = _nSurface;

		return pPrevious;
	}

	_DX9_TEXTURE* C_DX9_ENGINE::GetRenderTarget(UINT _nIndex) const
	{
		if (_nIndex >= MAX_RENDER_TARGETS)
		{
			return nullptr;
		}
		return m_pCurrentRenderTargets[_nIndex];
	}

	_DX9_TEXTURE* C_DX9_ENGINE::SetDepthStencilBuffer(_DX9_TEXTURE* _pTexture)
	{
		if (m_pDevice == nullptr)
		{
			return nullptr;
		}

		_DX9_TEXTURE* pPrevious = m_pCurrentDepthStencil;

		if (_pTexture != nullptr && _pTexture->pTexture != nullptr)
		{
			LPDIRECT3DSURFACE9 pSurface = nullptr;
			_pTexture->pTexture->GetSurfaceLevel(0, &pSurface);

			if (pSurface != nullptr)
			{
				m_pDevice->SetDepthStencilSurface(pSurface);
				pSurface->Release();
			}
		}
		else
		{
			// 기본 깊이 버퍼로 복원
			LPDIRECT3DSURFACE9 pDepthStencil = nullptr;
			m_pDevice->GetDepthStencilSurface(&pDepthStencil);
			if (pDepthStencil != nullptr)
			{
				m_pDevice->SetDepthStencilSurface(pDepthStencil);
				pDepthStencil->Release();
			}
		}

		m_pCurrentDepthStencil = _pTexture;

		return pPrevious;
	}

	_DX9_TEXTURE* C_DX9_ENGINE::GetDepthStencilBuffer() const
	{
		return m_pCurrentDepthStencil;
	}

	//============================================================================
	// 버텍스 버퍼 관리
	//============================================================================
	_DX9_VERTEX_BUFFER* C_DX9_ENGINE::CreateVertexBuffer(UINT _nSize, UINT _nVertexSize, const void* _pData, DWORD _dwFlags)
	{
		if (m_pDevice == nullptr)
		{
			return nullptr;
		}

		_DX9_VERTEX_BUFFER* pBuffer = new _DX9_VERTEX_BUFFER(_nVertexSize, _nSize / _nVertexSize, 0, _dwFlags);

		if (!pBuffer->Create(m_pDevice))
		{
			delete pBuffer;
			return nullptr;
		}

		// 초기 데이터 복사
		if (_pData != nullptr)
		{
			void* pLocked = nullptr;
			if (SUCCEEDED(pBuffer->pVertexBuffer->Lock(0, _nSize, &pLocked, 0)))
			{
				memcpy(pLocked, _pData, _nSize);
				pBuffer->pVertexBuffer->Unlock();
			}
		}

		m_listVertexBuffers.push_back(pBuffer);

		return pBuffer;
	}

	void C_DX9_ENGINE::DeleteVertexBuffer(_DX9_VERTEX_BUFFER* _pBuffer)
	{
		if (_pBuffer == nullptr)
		{
			return;
		}

		m_listVertexBuffers.remove(_pBuffer);

		_pBuffer->Release();
		delete _pBuffer;
	}

	void* C_DX9_ENGINE::LockVertexBuffer(_DX9_VERTEX_BUFFER* _pBuffer, DWORD _dwFlags, UINT _nOffset, UINT _nSize)
	{
		if (_pBuffer == nullptr || _pBuffer->pVertexBuffer == nullptr)
		{
			return nullptr;
		}

		void* pLocked = nullptr;
		if (SUCCEEDED(_pBuffer->pVertexBuffer->Lock(_nOffset, _nSize, &pLocked, _dwFlags)))
		{
			return pLocked;
		}

		return nullptr;
	}

	bool C_DX9_ENGINE::UnlockVertexBuffer(_DX9_VERTEX_BUFFER* _pBuffer)
	{
		if (_pBuffer == nullptr || _pBuffer->pVertexBuffer == nullptr)
		{
			return false;
		}

		return SUCCEEDED(_pBuffer->pVertexBuffer->Unlock());
	}

	void C_DX9_ENGINE::SetVertexBuffer(_DX9_VERTEX_BUFFER* _pBuffer, UINT _nStream, UINT _nOffset)
	{
		if (m_pDevice == nullptr || _nStream >= MAX_VERTEX_STREAMS)
		{
			return;
		}

		if (_pBuffer != m_pCurrentVertexBuffers[_nStream] || _nOffset != m_nCurrentVertexOffsets[_nStream])
		{
			if (_pBuffer != nullptr && _pBuffer->pVertexBuffer != nullptr)
			{
				m_pDevice->SetStreamSource(_nStream, _pBuffer->pVertexBuffer, _nOffset, _pBuffer->nStructSize);
			}
			else
			{
				m_pDevice->SetStreamSource(_nStream, nullptr, 0, 0);
			}

			m_pCurrentVertexBuffers[_nStream] = _pBuffer;
			m_nCurrentVertexOffsets[_nStream] = _nOffset;
		}
	}

	//============================================================================
	// 인덱스 버퍼 관리
	//============================================================================
	_DX9_INDEX_BUFFER* C_DX9_ENGINE::CreateIndexBuffer(UINT _nIndices, bool _b32Bit, const void* _pData, DWORD _dwFlags)
	{
		if (m_pDevice == nullptr)
		{
			return nullptr;
		}

		UINT nIndexSize = _b32Bit ? sizeof(DWORD) : sizeof(WORD);

		_DX9_INDEX_BUFFER* pBuffer = new _DX9_INDEX_BUFFER(nIndexSize, _nIndices, _dwFlags);

		if (!pBuffer->Create(m_pDevice))
		{
			delete pBuffer;
			return nullptr;
		}

		// 초기 데이터 복사
		if (_pData != nullptr)
		{
			void* pLocked = nullptr;
			if (SUCCEEDED(pBuffer->pIndexBuffer->Lock(0, _nIndices * nIndexSize, &pLocked, 0)))
			{
				memcpy(pLocked, _pData, _nIndices * nIndexSize);
				pBuffer->pIndexBuffer->Unlock();
			}
		}

		m_listIndexBuffers.push_back(pBuffer);

		return pBuffer;
	}

	void C_DX9_ENGINE::DeleteIndexBuffer(_DX9_INDEX_BUFFER* _pBuffer)
	{
		if (_pBuffer == nullptr)
		{
			return;
		}

		m_listIndexBuffers.remove(_pBuffer);

		_pBuffer->Release();
		delete _pBuffer;
	}

	void* C_DX9_ENGINE::LockIndexBuffer(_DX9_INDEX_BUFFER* _pBuffer, DWORD _dwFlags, UINT _nOffset, UINT _nSize)
	{
		if (_pBuffer == nullptr || _pBuffer->pIndexBuffer == nullptr)
		{
			return nullptr;
		}

		void* pLocked = nullptr;
		if (SUCCEEDED(_pBuffer->pIndexBuffer->Lock(_nOffset, _nSize, &pLocked, _dwFlags)))
		{
			return pLocked;
		}

		return nullptr;
	}

	bool C_DX9_ENGINE::UnlockIndexBuffer(_DX9_INDEX_BUFFER* _pBuffer)
	{
		if (_pBuffer == nullptr || _pBuffer->pIndexBuffer == nullptr)
		{
			return false;
		}

		return SUCCEEDED(_pBuffer->pIndexBuffer->Unlock());
	}

	void C_DX9_ENGINE::SetIndexBuffer(_DX9_INDEX_BUFFER* _pBuffer)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_pBuffer != m_pCurrentIndexBuffer)
		{
			if (_pBuffer != nullptr && _pBuffer->pIndexBuffer != nullptr)
			{
				m_pDevice->SetIndices(_pBuffer->pIndexBuffer);
			}
			else
			{
				m_pDevice->SetIndices(nullptr);
			}

			m_pCurrentIndexBuffer = _pBuffer;
		}
	}

	//============================================================================
	// 버텍스 포맷
	//============================================================================
	void C_DX9_ENGINE::SetFVF(DWORD _dwFVF)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_dwFVF != m_dwCurrentFVF)
		{
			m_pDevice->SetFVF(_dwFVF);
			m_dwCurrentFVF = _dwFVF;
			m_pCurrentVertexDecl = nullptr;
		}
	}

	void C_DX9_ENGINE::SetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 _pDecl)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_pDecl != m_pCurrentVertexDecl)
		{
			m_pDevice->SetVertexDeclaration(_pDecl);
			m_pCurrentVertexDecl = _pDecl;
			m_dwCurrentFVF = 0;
		}
	}

	//============================================================================
	// 텍스처 관리
	//============================================================================
	_DX9_TEXTURE* C_DX9_ENGINE::CreateTexture(UINT _nWidth, UINT _nHeight, D3DFORMAT _eFormat, DWORD _dwFlags)
	{
		if (m_pDevice == nullptr)
		{
			return nullptr;
		}

		DWORD dwUsage = 0;
		D3DPOOL pool = D3DPOOL_MANAGED;

		if (_dwFlags & TCF_RENDERTARGET)
		{
			dwUsage |= D3DUSAGE_RENDERTARGET;
			pool = D3DPOOL_DEFAULT;
		}
		if (_dwFlags & TCF_DYNAMIC)
		{
			dwUsage |= D3DUSAGE_DYNAMIC;
			pool = D3DPOOL_DEFAULT;
		}
		if (_dwFlags & TCF_DEPTHSTENCIL)
		{
			dwUsage |= D3DUSAGE_DEPTHSTENCIL;
			pool = D3DPOOL_DEFAULT;
		}
		if (_dwFlags & TCF_AUTOGENMIPMAP)
		{
			dwUsage |= D3DUSAGE_AUTOGENMIPMAP;
		}

		_DX9_TEXTURE* pTexture = new _DX9_TEXTURE(m_pDevice, _nWidth, _nHeight, _eFormat, dwUsage);
		pTexture->d3dPool = pool;

		if (!pTexture->Create())
		{
			delete pTexture;
			return nullptr;
		}

		m_listTextures.push_back(pTexture);

		return pTexture;
	}

	_DX9_TEXTURE* C_DX9_ENGINE::CreateTextureFromFile(const std::wstring& _strFilePath, E_TEXTURE_FILTER _eFilter, DWORD _dwFlags)
	{
		UNREFERENCED_PARAMETER(_eFilter);
		UNREFERENCED_PARAMETER(_dwFlags);

		if (m_pDevice == nullptr)
		{
			return nullptr;
		}

		_DX9_TEXTURE* pTexture = new _DX9_TEXTURE(m_pDevice);

		if (!pTexture->LoadTexture(_strFilePath.c_str()))
		{
			delete pTexture;
			return nullptr;
		}

		m_listTextures.push_back(pTexture);

		return pTexture;
	}

	_DX9_TEXTURE* C_DX9_ENGINE::CreateRenderTargetTexture(UINT _nWidth, UINT _nHeight, D3DFORMAT _eFormat, E_TEXTURE_FILTER _eFilter, DWORD _dwFlags)
	{
		UNREFERENCED_PARAMETER(_eFilter);
		return CreateTexture(_nWidth, _nHeight, _eFormat, _dwFlags | TCF_RENDERTARGET);
	}

	_DX9_TEXTURE* C_DX9_ENGINE::CreateDepthStencilTexture(UINT _nWidth, UINT _nHeight, D3DFORMAT _eFormat)
	{
		return CreateTexture(_nWidth, _nHeight, _eFormat, TCF_DEPTHSTENCIL);
	}

	void C_DX9_ENGINE::DeleteTexture(_DX9_TEXTURE* _pTexture)
	{
		if (_pTexture == nullptr)
		{
			return;
		}

		m_listTextures.remove(_pTexture);

		_pTexture->Release();
		delete _pTexture;
	}

	void C_DX9_ENGINE::SetTexture(UINT _nStage, _DX9_TEXTURE* _pTexture)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		UINT nIndex = GetSamplerToIndex(_nStage);
		if (nIndex >= MAX_IMAGE_UNITS)
		{
			return;
		}

		if (_pTexture != m_pCurrentTextures[nIndex])
		{
			if (_pTexture != nullptr)
			{
				m_pDevice->SetTexture(_nStage, _pTexture->pTexture);
			}
			else
			{
				m_pDevice->SetTexture(_nStage, nullptr);
			}

			m_pCurrentTextures[nIndex] = _pTexture;
		}
	}

	_DX9_TEXTURE* C_DX9_ENGINE::GetTexture(UINT _nStage) const
	{
		UINT nIndex = const_cast<C_DX9_ENGINE*>(this)->GetSamplerToIndex(_nStage);
		if (nIndex >= MAX_IMAGE_UNITS)
		{
			return nullptr;
		}
		return m_pCurrentTextures[nIndex];
	}

	bool C_DX9_ENGINE::LockTexture(_DX9_TEXTURE* _pTexture, int _nLevel, _LOCKED_RECT* _pLockedRect, const RECT* _pRect, DWORD _dwFlags)
	{
		if (_pTexture == nullptr || _pTexture->pTexture == nullptr || _pLockedRect == nullptr)
		{
			return false;
		}

		D3DLOCKED_RECT lockedRect;
		if (SUCCEEDED(_pTexture->pTexture->LockRect(_nLevel, &lockedRect, _pRect, _dwFlags)))
		{
			_pLockedRect->nPitch = lockedRect.Pitch;
			_pLockedRect->pBits = lockedRect.pBits;
			return true;
		}

		return false;
	}

	bool C_DX9_ENGINE::UnlockTexture(_DX9_TEXTURE* _pTexture, int _nLevel)
	{
		if (_pTexture == nullptr || _pTexture->pTexture == nullptr)
		{
			return false;
		}

		return SUCCEEDED(_pTexture->pTexture->UnlockRect(_nLevel));
	}

	//============================================================================
	// 텍스처 상태
	//============================================================================
	void C_DX9_ENGINE::SetTextureStageState(UINT _nStage, _TEXTURE_STAGE_STATE_TYPE_ _eType, UINT _nValue)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		UINT nIndex = GetSamplerToIndex(_nStage);
		if (nIndex >= MAX_IMAGE_UNITS)
		{
			return;
		}

		// 상태 비교 후 변경된 경우만 D3D API 호출
		if (m_nCurrentTextureStageState[nIndex][_eType] != _nValue)
		{
			m_pDevice->SetTextureStageState(_nStage, d3dTextureStageStateTypeTable[_eType], _nValue);
			m_nCurrentTextureStageState[nIndex][_eType] = _nValue;
		}
	}

	void C_DX9_ENGINE::SetTextureFilter(UINT _nSampler, E_TEXTURE_FILTER _eFilter)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		UINT nIndex = GetSamplerToIndex(_nSampler);
		if (nIndex >= MAX_IMAGE_UNITS)
		{
			return;
		}

		// 상태 비교 후 변경된 경우만 D3D API 호출
		if (_eFilter != m_eCurrentTextureFilter[nIndex])
		{
			int nFilterIndex = static_cast<int>(_eFilter);
			if (nFilterIndex >= 0 && nFilterIndex < static_cast<int>(sizeof(s_textureFilterSets) / sizeof(s_textureFilterSets[0])))
			{
				const _TEXTURE_FILTER_SET& filterSet_ = s_textureFilterSets[nFilterIndex];

				// 기존 필터와 비교하여 변경된 부분만 설정
				int nOldFilterIndex = static_cast<int>(m_eCurrentTextureFilter[nIndex]);
				const _TEXTURE_FILTER_SET& oldFilterSet_ = s_textureFilterSets[nOldFilterIndex];

				if (filterSet_.minFilter != oldFilterSet_.minFilter)
				{
					m_pDevice->SetSamplerState(_nSampler, D3DSAMP_MINFILTER, filterSet_.minFilter);
				}
				if (filterSet_.magFilter != oldFilterSet_.magFilter)
				{
					m_pDevice->SetSamplerState(_nSampler, D3DSAMP_MAGFILTER, filterSet_.magFilter);
				}
				if (filterSet_.mipFilter != oldFilterSet_.mipFilter)
				{
					m_pDevice->SetSamplerState(_nSampler, D3DSAMP_MIPFILTER, filterSet_.mipFilter);
				}
			}

			m_eCurrentTextureFilter[nIndex] = _eFilter;
		}
	}

	void C_DX9_ENGINE::SetTextureAddress(UINT _nSampler, E_TEXTURE_ADDRESS _eU, E_TEXTURE_ADDRESS _eV, E_TEXTURE_ADDRESS _eW)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		UINT nIndex = GetSamplerToIndex(_nSampler);
		if (nIndex >= MAX_IMAGE_UNITS)
		{
			return;
		}

		// 각 축별로 변경된 경우만 D3D API 호출
		if (_eU != m_eCurrentTextureAddress[nIndex][0])
		{
			m_pDevice->SetSamplerState(_nSampler, D3DSAMP_ADDRESSU, s_d3dTextureAddressModes[static_cast<int>(_eU)]);
			m_eCurrentTextureAddress[nIndex][0] = _eU;
		}

		if (_eV != m_eCurrentTextureAddress[nIndex][1])
		{
			m_pDevice->SetSamplerState(_nSampler, D3DSAMP_ADDRESSV, s_d3dTextureAddressModes[static_cast<int>(_eV)]);
			m_eCurrentTextureAddress[nIndex][1] = _eV;
		}

		if (_eW != m_eCurrentTextureAddress[nIndex][2])
		{
			m_pDevice->SetSamplerState(_nSampler, D3DSAMP_ADDRESSW, s_d3dTextureAddressModes[static_cast<int>(_eW)]);
			m_eCurrentTextureAddress[nIndex][2] = _eW;
		}
	}

	void C_DX9_ENGINE::SetTextureAddressClamp(UINT _nSampler)
	{
		SetTextureAddress(_nSampler, E_TEXTURE_ADDRESS::CLAMP, E_TEXTURE_ADDRESS::CLAMP, E_TEXTURE_ADDRESS::CLAMP);
	}

	void C_DX9_ENGINE::SetTextureAddressWrap(UINT _nSampler)
	{
		SetTextureAddress(_nSampler, E_TEXTURE_ADDRESS::WRAP, E_TEXTURE_ADDRESS::WRAP, E_TEXTURE_ADDRESS::WRAP);
	}

	void C_DX9_ENGINE::SetTextureMipmapLodBias(UINT _nStage, float _fBias)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		UINT nIndex = GetSamplerToIndex(_nStage);
		if (nIndex >= MAX_IMAGE_UNITS)
		{
			return;
		}

		// 상태 비교 후 변경된 경우만 D3D API 호출
		if (_fBias != m_fCurrentMipmapLodBias[nIndex])
		{
			m_pDevice->SetSamplerState(_nStage, D3DSAMP_MIPMAPLODBIAS, *reinterpret_cast<DWORD*>(&_fBias));
			m_fCurrentMipmapLodBias[nIndex] = _fBias;
		}
	}

	void C_DX9_ENGINE::SetTextureMaxAnisotropy(UINT _nStage, DWORD _dwValue)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		UINT nIndex = GetSamplerToIndex(_nStage);
		if (nIndex >= MAX_IMAGE_UNITS)
		{
			return;
		}

		// 상태 비교 후 변경된 경우만 D3D API 호출
		if (_dwValue != m_dwCurrentMaxAnisotropy[nIndex])
		{
			m_pDevice->SetSamplerState(_nStage, D3DSAMP_MAXANISOTROPY, _dwValue);
			m_dwCurrentMaxAnisotropy[nIndex] = _dwValue;
		}
	}

	void C_DX9_ENGINE::SetTextureBorderColor(UINT _nSampler, DWORD _dwColor)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		UINT nIndex = GetSamplerToIndex(_nSampler);
		if (nIndex >= MAX_IMAGE_UNITS)
		{
			return;
		}

		// 상태 비교 후 변경된 경우만 D3D API 호출
		if (_dwColor != m_dwCurrentTextureBorderColor[nIndex])
		{
			m_pDevice->SetSamplerState(_nSampler, D3DSAMP_BORDERCOLOR, _dwColor);
			m_dwCurrentTextureBorderColor[nIndex] = _dwColor;
		}
	}

	//============================================================================
	// 변환 행렬
	//============================================================================
	void C_DX9_ENGINE::SetTransform(E_TRANSFORM_TYPE _eType, const _DMATRIX9& _matrix)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		int nTypeIndex = static_cast<int>(_eType);
		if (nTypeIndex >= 0 && nTypeIndex < static_cast<int>(E_TRANSFORM_TYPE::MAX))
		{
			// 행렬 비교 후 변경된 경우만 D3D API 호출
			if (memcmp(&m_matCurrentTransform[nTypeIndex], &_matrix, sizeof(_DMATRIX9)) != 0)
			{
				m_pDevice->SetTransform(s_d3dTransformTypes[nTypeIndex], &_matrix);
				m_matCurrentTransform[nTypeIndex] = _matrix;
			}
		}
	}

	_DMATRIX9 C_DX9_ENGINE::GetTransform(E_TRANSFORM_TYPE _eType) const
	{
		int nTypeIndex = static_cast<int>(_eType);
		if (nTypeIndex >= 0 && nTypeIndex < static_cast<int>(E_TRANSFORM_TYPE::MAX))
		{
			return m_matCurrentTransform[nTypeIndex];
		}

		_DMATRIX9 identity;
		identity.MakeIdentity();
		return identity;
	}

	//============================================================================
	// 뷰포트
	//============================================================================
	void C_DX9_ENGINE::SetViewport(const _DVIEWPORT9& _viewport)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		// 뷰포트 비교 후 변경된 경우만 D3D API 호출
		if (memcmp(&m_currentViewport, &_viewport, sizeof(_DVIEWPORT9)) != 0)
		{
			m_currentViewport = _viewport;
			m_pDevice->SetViewport(&m_currentViewport);
		}
	}

	void C_DX9_ENGINE::SetViewport(DWORD _x, DWORD _y, DWORD _nWidth, DWORD _nHeight, float _fMinZ, float _fMaxZ)
	{
		_DVIEWPORT9 vp(_x, _y, _nWidth, _nHeight, _fMinZ, _fMaxZ);
		SetViewport(vp);
	}

	//============================================================================
	// 블렌딩 상태
	//============================================================================
	void C_DX9_ENGINE::SetBlending(E_BLEND _eSrc, E_BLEND _eDst, E_BLEND_OP _eOp)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		bool bEnable_ = (_eDst != E_BLEND::NONE);

		if (bEnable_ != m_bCurrentBlendEnable)
		{
			m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, bEnable_ ? TRUE : FALSE);
			m_bCurrentBlendEnable = bEnable_;
		}

		if (bEnable_)
		{
			if (_eSrc != m_eCurrentSrcBlend)
			{
				m_pDevice->SetRenderState(D3DRS_SRCBLEND, s_d3dBlends[static_cast<int>(_eSrc)]);
				m_eCurrentSrcBlend = _eSrc;
			}

			if (_eDst != m_eCurrentDstBlend)
			{
				m_pDevice->SetRenderState(D3DRS_DESTBLEND, s_d3dBlends[static_cast<int>(_eDst)]);
				m_eCurrentDstBlend = _eDst;
			}

			if (_eOp != m_eCurrentBlendOp)
			{
				m_pDevice->SetRenderState(D3DRS_BLENDOP, s_d3dBlendOps[static_cast<int>(_eOp)]);
				m_eCurrentBlendOp = _eOp;
			}
		}
	}

	void C_DX9_ENGINE::SetSeparateBlending(E_BLEND _eSrc, E_BLEND _eDst, E_BLEND_OP _eOp)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		bool bEnable_ = (_eDst != E_BLEND::NONE);

		if (bEnable_ != m_bCurrentSeparateBlendEnable)
		{
			m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, bEnable_ ? TRUE : FALSE);
			m_bCurrentSeparateBlendEnable = bEnable_;
		}

		if (bEnable_)
		{
			if (_eSrc != m_eCurrentSeparateSrcBlend)
			{
				m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, s_d3dBlends[static_cast<int>(_eSrc)]);
				m_eCurrentSeparateSrcBlend = _eSrc;
			}

			if (_eDst != m_eCurrentSeparateDstBlend)
			{
				m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, s_d3dBlends[static_cast<int>(_eDst)]);
				m_eCurrentSeparateDstBlend = _eDst;
			}

			if (_eOp != m_eCurrentSeparateBlendOp)
			{
				m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, s_d3dBlendOps[static_cast<int>(_eOp)]);
				m_eCurrentSeparateBlendOp = _eOp;
			}
		}
	}

	void C_DX9_ENGINE::SetBlendFactor(DWORD _dwColor)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_dwColor != m_dwCurrentBlendFactor)
		{
			m_pDevice->SetRenderState(D3DRS_BLENDFACTOR, _dwColor);
			m_dwCurrentBlendFactor = _dwColor;
		}
	}

	void C_DX9_ENGINE::SetTextureFactor(DWORD _dwColor)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_dwColor != m_dwCurrentTextureFactor)
		{
			m_pDevice->SetRenderState(D3DRS_TEXTUREFACTOR, _dwColor);
			m_dwCurrentTextureFactor = _dwColor;
		}
	}

	//============================================================================
	// 알파 테스트
	//============================================================================
	void C_DX9_ENGINE::SetAlphaTestEnable(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentAlphaTestEnable)
		{
			m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, _bEnable ? TRUE : FALSE);
			m_bCurrentAlphaTestEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetAlphaRef(DWORD _dwRef)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_dwRef != m_dwCurrentAlphaRef)
		{
			m_pDevice->SetRenderState(D3DRS_ALPHAREF, _dwRef);
			m_dwCurrentAlphaRef = _dwRef;
		}
	}

	void C_DX9_ENGINE::SetAlphaFunc(E_CMP_FUNC _eFunc)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_eFunc != m_eCurrentAlphaFunc)
		{
			m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, s_d3dCmpFuncs[static_cast<int>(_eFunc)]);
			m_eCurrentAlphaFunc = _eFunc;
		}
	}

	//============================================================================
	// 깊이/스텐실 상태
	//============================================================================
	void C_DX9_ENGINE::SetDepthFunc(E_CMP_FUNC _eFunc)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_eFunc != m_eCurrentDepthFunc)
		{
			m_pDevice->SetRenderState(D3DRS_ZFUNC, s_d3dCmpFuncs[static_cast<int>(_eFunc)]);
			m_eCurrentDepthFunc = _eFunc;
		}
	}

	void C_DX9_ENGINE::SetDepthEnable(bool _bEnable, bool _bWriteEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentDepthTestEnable)
		{
			m_pDevice->SetRenderState(D3DRS_ZENABLE, _bEnable ? D3DZB_TRUE : D3DZB_FALSE);
			m_bCurrentDepthTestEnable = _bEnable;
		}

		if (_bWriteEnable != m_bCurrentDepthWriteEnable)
		{
			m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, _bWriteEnable ? TRUE : FALSE);
			m_bCurrentDepthWriteEnable = _bWriteEnable;
		}
	}

	void C_DX9_ENGINE::SetColorWriteEnable(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentColorWriteEnable)
		{
			DWORD dwMask = _bEnable ? (D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA) : 0;
			m_pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwMask);
			m_bCurrentColorWriteEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetStencilEnable(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentStencilEnable)
		{
			m_pDevice->SetRenderState(D3DRS_STENCILENABLE, _bEnable ? TRUE : FALSE);
			m_bCurrentStencilEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetStencilTwoSide(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		// 상태 비교 후 변경된 경우만 D3D API 호출
		if (_bEnable != m_bCurrentTwoSidedStencil)
		{
			m_pDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, _bEnable ? TRUE : FALSE);
			m_bCurrentTwoSidedStencil = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetStencilRef(DWORD _dwValue)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_dwValue != m_dwCurrentStencilRef)
		{
			m_pDevice->SetRenderState(D3DRS_STENCILREF, _dwValue);
			m_dwCurrentStencilRef = _dwValue;
		}
	}

	void C_DX9_ENGINE::SetStencilMask(DWORD _dwValue)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_dwValue != m_dwCurrentStencilMask)
		{
			m_pDevice->SetRenderState(D3DRS_STENCILMASK, _dwValue);
			m_dwCurrentStencilMask = _dwValue;
		}
	}

	void C_DX9_ENGINE::SetStencilWriteMask(DWORD _dwValue)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		// 상태 비교 후 변경된 경우만 D3D API 호출
		if (_dwValue != m_dwCurrentStencilWriteMask)
		{
			m_pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, _dwValue);
			m_dwCurrentStencilWriteMask = _dwValue;
		}
	}

	void C_DX9_ENGINE::SetStencilFunc(E_CMP_FUNC _eFunc)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_eFunc != m_eCurrentStencilFunc)
		{
			m_pDevice->SetRenderState(D3DRS_STENCILFUNC, s_d3dCmpFuncs[static_cast<int>(_eFunc)]);
			m_eCurrentStencilFunc = _eFunc;
		}
	}

	void C_DX9_ENGINE::SetStencilOp(E_STENCIL_OP _ePass, E_STENCIL_OP _eFail, E_STENCIL_OP _eZFail)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_ePass != m_eCurrentStencilPassOp)
		{
			m_pDevice->SetRenderState(D3DRS_STENCILPASS, s_d3dStencilOps[static_cast<int>(_ePass)]);
			m_eCurrentStencilPassOp = _ePass;
		}

		if (_eFail != m_eCurrentStencilFailOp)
		{
			m_pDevice->SetRenderState(D3DRS_STENCILFAIL, s_d3dStencilOps[static_cast<int>(_eFail)]);
			m_eCurrentStencilFailOp = _eFail;
		}

		if (_eZFail != m_eCurrentStencilZFailOp)
		{
			m_pDevice->SetRenderState(D3DRS_STENCILZFAIL, s_d3dStencilOps[static_cast<int>(_eZFail)]);
			m_eCurrentStencilZFailOp = _eZFail;
		}
	}

	void C_DX9_ENGINE::SetStencilCCWFunc(E_CMP_FUNC _eFunc)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_eFunc != m_eCurrentStencilBackFunc)
		{
			m_pDevice->SetRenderState(D3DRS_CCW_STENCILFUNC, s_d3dCmpFuncs[static_cast<int>(_eFunc)]);
			m_eCurrentStencilBackFunc = _eFunc;
		}
	}

	void C_DX9_ENGINE::SetStencilCCWOp(E_STENCIL_OP _ePass, E_STENCIL_OP _eFail, E_STENCIL_OP _eZFail)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		// 각 상태별로 변경된 경우만 D3D API 호출
		if (_ePass != m_eCurrentStencilCCWPassOp)
		{
			m_pDevice->SetRenderState(D3DRS_CCW_STENCILPASS, s_d3dStencilOps[static_cast<int>(_ePass)]);
			m_eCurrentStencilCCWPassOp = _ePass;
		}

		if (_eFail != m_eCurrentStencilCCWFailOp)
		{
			m_pDevice->SetRenderState(D3DRS_CCW_STENCILFAIL, s_d3dStencilOps[static_cast<int>(_eFail)]);
			m_eCurrentStencilCCWFailOp = _eFail;
		}

		if (_eZFail != m_eCurrentStencilCCWZFailOp)
		{
			m_pDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL, s_d3dStencilOps[static_cast<int>(_eZFail)]);
			m_eCurrentStencilCCWZFailOp = _eZFail;
		}
	}

	//============================================================================
	// 기타 렌더 상태
	//============================================================================
	void C_DX9_ENGINE::SetCullMode(E_CULL_MODE _eMode)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_eMode != m_eCurrentCullMode)
		{
			m_pDevice->SetRenderState(D3DRS_CULLMODE, s_d3dCullModes[static_cast<int>(_eMode)]);
			m_eCurrentCullMode = _eMode;
		}
	}

	void C_DX9_ENGINE::SetFillMode(E_FILL_MODE _eMode)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_eMode != m_eCurrentFillMode)
		{
			m_pDevice->SetRenderState(D3DRS_FILLMODE, s_d3dFillModes[static_cast<int>(_eMode)]);
			m_eCurrentFillMode = _eMode;
		}
	}

	void C_DX9_ENGINE::SetClipPlanes(const D3DXPLANE* _pPlanes, int _nCount)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		DWORD dwEnable = 0;

		for (int i = 0; i < _nCount && i < 6; ++i)
		{
			m_pDevice->SetClipPlane(i, reinterpret_cast<const float*>(&_pPlanes[i]));
			dwEnable |= (1 << i);
		}

		if (dwEnable != m_dwCurrentClipPlaneEnable)
		{
			m_pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, dwEnable);
			m_dwCurrentClipPlaneEnable = dwEnable;
		}
	}

	void C_DX9_ENGINE::SetDepthBias(float _fDepthBias, float _fSlopeScale)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_fDepthBias != m_fCurrentDepthBias)
		{
			m_pDevice->SetRenderState(D3DRS_DEPTHBIAS, *reinterpret_cast<DWORD*>(&_fDepthBias));
			m_fCurrentDepthBias = _fDepthBias;
		}

		if (_fSlopeScale != m_fCurrentSlopeScaleDepthBias)
		{
			m_pDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *reinterpret_cast<DWORD*>(&_fSlopeScale));
			m_fCurrentSlopeScaleDepthBias = _fSlopeScale;
		}
	}

	void C_DX9_ENGINE::SetLighting(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentLightingEnable)
		{
			m_pDevice->SetRenderState(D3DRS_LIGHTING, _bEnable ? TRUE : FALSE);
			m_bCurrentLightingEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetAmbient(DWORD _dwColor)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_dwColor != m_dwCurrentAmbientColor)
		{
			m_pDevice->SetRenderState(D3DRS_AMBIENT, _dwColor);
			m_dwCurrentAmbientColor = _dwColor;
		}
	}

	void C_DX9_ENGINE::SetNormalizeNormals(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentNormalizeNormals)
		{
			m_pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, _bEnable ? TRUE : FALSE);
			m_bCurrentNormalizeNormals = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetFogEnable(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentFogEnable)
		{
			m_pDevice->SetRenderState(D3DRS_FOGENABLE, _bEnable ? TRUE : FALSE);
			m_bCurrentFogEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetFog(bool _bLinear, DWORD _dwColor, float _fNear, float _fFar, float _fDensity, bool _bPixel, bool _bRange)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		SetFogColor(_dwColor);

		// 포그 시작/끝 거리 비교 후 변경 시만 D3D API 호출
		if (_fNear != m_fCurrentFogNear)
		{
			m_pDevice->SetRenderState(D3DRS_FOGSTART, *reinterpret_cast<DWORD*>(&_fNear));
			m_fCurrentFogNear = _fNear;
		}

		if (_fFar != m_fCurrentFogFar)
		{
			m_pDevice->SetRenderState(D3DRS_FOGEND, *reinterpret_cast<DWORD*>(&_fFar));
			m_fCurrentFogFar = _fFar;
		}

		if (_fDensity != m_fCurrentFogDensity)
		{
			m_pDevice->SetRenderState(D3DRS_FOGDENSITY, *reinterpret_cast<DWORD*>(&_fDensity));
			m_fCurrentFogDensity = _fDensity;
		}

		// 포그 모드 변경 시만 D3D API 호출
		if (_bPixel != m_bCurrentFogPixel || _bLinear != m_bCurrentFogLinear)
		{
			if (_bPixel)
			{
				m_pDevice->SetRenderState(D3DRS_FOGTABLEMODE, _bLinear ? D3DFOG_LINEAR : D3DFOG_EXP);
				m_pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
			}
			else
			{
				m_pDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
				m_pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, _bLinear ? D3DFOG_LINEAR : D3DFOG_EXP);
			}
			m_bCurrentFogPixel = _bPixel;
			m_bCurrentFogLinear = _bLinear;
		}

		if (_bRange != m_bCurrentFogRange)
		{
			m_pDevice->SetRenderState(D3DRS_RANGEFOGENABLE, _bRange ? TRUE : FALSE);
			m_bCurrentFogRange = _bRange;
		}
	}

	void C_DX9_ENGINE::SetFogColor(DWORD _dwColor)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_dwColor != m_dwCurrentFogColor)
		{
			m_pDevice->SetRenderState(D3DRS_FOGCOLOR, _dwColor);
			m_dwCurrentFogColor = _dwColor;
		}
	}

	void C_DX9_ENGINE::SetClipping(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentClippingEnable)
		{
			m_pDevice->SetRenderState(D3DRS_CLIPPING, _bEnable ? TRUE : FALSE);
			m_bCurrentClippingEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetScissorTestEnable(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentScissorTestEnable)
		{
			m_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, _bEnable ? TRUE : FALSE);
			m_bCurrentScissorTestEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetScissorRect(const RECT* _pRect)
	{
		if (m_pDevice == nullptr || _pRect == nullptr)
		{
			return;
		}

		// 시저 렉트 비교 후 변경된 경우만 D3D API 호출
		if (memcmp(&m_rectCurrentScissor, _pRect, sizeof(RECT)) != 0)
		{
			m_rectCurrentScissor = *_pRect;
			m_pDevice->SetScissorRect(_pRect);
		}
	}

	void C_DX9_ENGINE::SetPointSize(DWORD _nSize)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		float fSize = static_cast<float>(_nSize);

		// 상태 비교 후 변경된 경우만 D3D API 호출
		if (fSize != m_fCurrentPointSize)
		{
			m_pDevice->SetRenderState(D3DRS_POINTSIZE, *reinterpret_cast<DWORD*>(&fSize));
			m_fCurrentPointSize = fSize;
		}
	}

	void C_DX9_ENGINE::SetPointSpriteEnable(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		// 상태 비교 후 변경된 경우만 D3D API 호출
		if (_bEnable != m_bCurrentPointSpriteEnable)
		{
			m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, _bEnable ? TRUE : FALSE);
			m_bCurrentPointSpriteEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetVertexBlendEnable(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentVertexBlendEnable)
		{
			m_pDevice->SetRenderState(D3DRS_VERTEXBLEND, _bEnable ? D3DVBF_1WEIGHTS : D3DVBF_DISABLE);
			m_bCurrentVertexBlendEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetIndexedVertexBlendEnable(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentIndexedVertexBlendEnable)
		{
			m_pDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, _bEnable ? TRUE : FALSE);
			m_bCurrentIndexedVertexBlendEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetSpecularEnable(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentSpecularEnable)
		{
			m_pDevice->SetRenderState(D3DRS_SPECULARENABLE, _bEnable ? TRUE : FALSE);
			m_bCurrentSpecularEnable = _bEnable;
		}
	}

	void C_DX9_ENGINE::SetColorVertex(bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		if (_bEnable != m_bCurrentColorVertexEnable)
		{
			m_pDevice->SetRenderState(D3DRS_COLORVERTEX, _bEnable ? TRUE : FALSE);
			m_bCurrentColorVertexEnable = _bEnable;
		}
	}

	//============================================================================
	// 드로우 프리미티브
	//============================================================================
	bool C_DX9_ENGINE::DrawPrimitive(E_PRIMITIVE_TYPE _eType, UINT _nStart, UINT _nPrimitiveCount)
	{
		if (m_pDevice == nullptr)
		{
			return false;
		}

		HRESULT hr = m_pDevice->DrawPrimitive(
			s_d3dPrimitiveTypes[static_cast<int>(_eType)],
			_nStart,
			_nPrimitiveCount
		);

		if (SUCCEEDED(hr))
		{
			m_profileCurrent.AddDP(_nPrimitiveCount, 1);
			return true;
		}

		return false;
	}

	bool C_DX9_ENGINE::DrawIndexedPrimitive(E_PRIMITIVE_TYPE _eType, INT _nBaseVertexIndex, UINT _nMinIndex, UINT _nNumVertices,
		UINT _nStartIndex, UINT _nPrimitiveCount)
	{
		if (m_pDevice == nullptr)
		{
			return false;
		}

		HRESULT hr = m_pDevice->DrawIndexedPrimitive(
			s_d3dPrimitiveTypes[static_cast<int>(_eType)],
			_nBaseVertexIndex,
			_nMinIndex,
			_nNumVertices,
			_nStartIndex,
			_nPrimitiveCount
		);

		if (SUCCEEDED(hr))
		{
			m_profileCurrent.AddDIP(_nPrimitiveCount, 1);
			return true;
		}

		return false;
	}

	bool C_DX9_ENGINE::DrawPrimitiveUP(E_PRIMITIVE_TYPE _eType, UINT _nPrimitiveCount, const void* _pVertexData, UINT _nVertexStride)
	{
		if (m_pDevice == nullptr || _pVertexData == nullptr)
		{
			return false;
		}

		HRESULT hr = m_pDevice->DrawPrimitiveUP(
			s_d3dPrimitiveTypes[static_cast<int>(_eType)],
			_nPrimitiveCount,
			_pVertexData,
			_nVertexStride
		);

		if (SUCCEEDED(hr))
		{
			m_profileCurrent.AddDPUP(_nPrimitiveCount);
			return true;
		}

		return false;
	}

	bool C_DX9_ENGINE::DrawIndexedPrimitiveUP(E_PRIMITIVE_TYPE _eType, UINT _nMinVertexIndex, UINT _nNumVertices, UINT _nPrimitiveCount,
		const void* _pIndexData, const void* _pVertexData, UINT _nVertexStride, bool _b32BitIndex)
	{
		if (m_pDevice == nullptr || _pIndexData == nullptr || _pVertexData == nullptr)
		{
			return false;
		}

		D3DFORMAT indexFormat = _b32BitIndex ? D3DFMT_INDEX32 : D3DFMT_INDEX16;

		HRESULT hr = m_pDevice->DrawIndexedPrimitiveUP(
			s_d3dPrimitiveTypes[static_cast<int>(_eType)],
			_nMinVertexIndex,
			_nNumVertices,
			_nPrimitiveCount,
			_pIndexData,
			indexFormat,
			_pVertexData,
			_nVertexStride
		);

		if (SUCCEEDED(hr))
		{
			m_profileCurrent.AddDIPUP(_nPrimitiveCount);
			return true;
		}

		return false;
	}

	//============================================================================
	// 인스턴싱
	//============================================================================
	void C_DX9_ENGINE::SetupForRenderInstancing(_DX9_VERTEX_BUFFER* _pInstanceBuffer, UINT _nInstanceCount, UINT _nOffset)
	{
		if (m_pDevice == nullptr || _pInstanceBuffer == nullptr)
		{
			return;
		}

		// 스트림 0: 버텍스 데이터 (인덱스당 1회)
		m_pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | _nInstanceCount);

		// 스트림 1: 인스턴스 데이터 (인스턴스당 1회)
		m_pDevice->SetStreamSourceFreq(1, static_cast<UINT>(D3DSTREAMSOURCE_INSTANCEDATA) | 1u);
		m_pDevice->SetStreamSource(1, _pInstanceBuffer->pVertexBuffer, _nOffset, _pInstanceBuffer->nStructSize);

		m_nCurrentInstanceCount = _nInstanceCount;
	}

	void C_DX9_ENGINE::RestoreForRenderInstancing()
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		// 스트림 주파수 초기화
		m_pDevice->SetStreamSourceFreq(0, 1);
		m_pDevice->SetStreamSourceFreq(1, 1);
		m_pDevice->SetStreamSource(1, nullptr, 0, 0);

		m_nCurrentInstanceCount = 0;
	}

	//============================================================================
	// 셰이더 관련
	//============================================================================
	void C_DX9_ENGINE::ShaderOff()
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		m_pDevice->SetVertexShader(nullptr);
		m_pDevice->SetPixelShader(nullptr);
	}

	//============================================================================
	// 유틸리티
	//============================================================================
	bool C_DX9_ENGINE::SaveScreenShot(const std::wstring& _strFilePath)
	{
		if (m_pDevice == nullptr)
		{
			return false;
		}

		// 백버퍼 서페이스 획득
		LPDIRECT3DSURFACE9 pBackBuffer = nullptr;
		HRESULT hr = m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
		if (FAILED(hr) || pBackBuffer == nullptr)
		{
			return false;
		}

		// 파일로 저장
		hr = D3DXSaveSurfaceToFileW(_strFilePath.c_str(), D3DXIFF_PNG, pBackBuffer, nullptr, nullptr);

		pBackBuffer->Release();

		return SUCCEEDED(hr);
	}

	void C_DX9_ENGINE::OnLostDevice()
	{
		// D3DPOOL_DEFAULT 리소스 해제
		for (auto it = m_listVertexBuffers.begin(); it != m_listVertexBuffers.end(); ++it)
		{
			_DX9_VERTEX_BUFFER* pVB = *it;
			if (pVB != nullptr)
			{
				pVB->OnLostDevice();
			}
		}

		for (auto it = m_listIndexBuffers.begin(); it != m_listIndexBuffers.end(); ++it)
		{
			_DX9_INDEX_BUFFER* pIB = *it;
			if (pIB != nullptr)
			{
				pIB->OnLostDevice();
			}
		}

		for (auto it = m_listTextures.begin(); it != m_listTextures.end(); ++it)
		{
			_DX9_TEXTURE* pTex = *it;
			if (pTex != nullptr)
			{
				pTex->OnLostDevice();
			}
		}

		ClearStates();

		m_bDeviceLost = true;
	}

	void C_DX9_ENGINE::OnResetDevice()
	{
		// D3DPOOL_DEFAULT 리소스 재생성
		for (auto it = m_listVertexBuffers.begin(); it != m_listVertexBuffers.end(); ++it)
		{
			_DX9_VERTEX_BUFFER* pVB = *it;
			if (pVB != nullptr && pVB->IsDynamic())
			{
				pVB->Create(m_pDevice);
			}
		}

		for (auto it = m_listIndexBuffers.begin(); it != m_listIndexBuffers.end(); ++it)
		{
			_DX9_INDEX_BUFFER* pIB = *it;
			if (pIB != nullptr && pIB->IsDynamic())
			{
				pIB->Create(m_pDevice);
			}
		}

		for (auto it = m_listTextures.begin(); it != m_listTextures.end(); ++it)
		{
			_DX9_TEXTURE* pTex = *it;
			if (pTex != nullptr)
			{
				pTex->OnResetDevice();
			}
		}

		m_bDeviceLost = false;
	}

	//============================================================================
	// 2D 렌더링 지원
	//============================================================================
	bool C_DX9_ENGINE::Begin2D()
	{
		if (m_pDevice == nullptr || m_bIn2DMode)
		{
			return false;
		}

		// 현재 변환 행렬 저장
		m_matSaved2DWorld = GetTransform(E_TRANSFORM_TYPE::WORLD);
		m_matSaved2DView = GetTransform(E_TRANSFORM_TYPE::VIEW);
		m_matSaved2DProj = GetTransform(E_TRANSFORM_TYPE::PROJECTION);

		// World/View를 단위행렬로 설정
		_DMATRIX9 matIdentity;
		matIdentity.MakeIdentity();
		SetTransform(E_TRANSFORM_TYPE::WORLD, matIdentity);
		SetTransform(E_TRANSFORM_TYPE::VIEW, matIdentity);

		// 직교 투영 행렬 설정 (뷰포트와 1:1 매핑)
		// 좌상단이 (0,0), 우하단이 (width, height)
		_DMATRIX9 matOrtho;
		matOrtho.SetOrthoOffCenterLH(
			static_cast<float>(m_currentViewport.X),
			static_cast<float>(m_currentViewport.X + m_currentViewport.Width),
			static_cast<float>(m_currentViewport.Y + m_currentViewport.Height),
			static_cast<float>(m_currentViewport.Y),
			m_currentViewport.MinZ,
			m_currentViewport.MaxZ
		);
		SetTransform(E_TRANSFORM_TYPE::PROJECTION, matOrtho);

		m_bIn2DMode = true;

		return true;
	}

	void C_DX9_ENGINE::End2D()
	{
		if (!m_bIn2DMode)
		{
			return;
		}

		// 이전 변환 행렬 복원
		SetTransform(E_TRANSFORM_TYPE::WORLD, m_matSaved2DWorld);
		SetTransform(E_TRANSFORM_TYPE::VIEW, m_matSaved2DView);
		SetTransform(E_TRANSFORM_TYPE::PROJECTION, m_matSaved2DProj);

		m_bIn2DMode = false;
	}

	//============================================================================
	// 라이트/머티리얼
	//============================================================================
	void C_DX9_ENGINE::SetLight(int _nIndex, const D3DLIGHT9* _pLight)
	{
		if (m_pDevice == nullptr || _pLight == nullptr)
		{
			return;
		}

		m_pDevice->SetLight(_nIndex, _pLight);
	}

	void C_DX9_ENGINE::SetLightEnable(int _nIndex, bool _bEnable)
	{
		if (m_pDevice == nullptr)
		{
			return;
		}

		m_pDevice->LightEnable(_nIndex, _bEnable ? TRUE : FALSE);
	}

	void C_DX9_ENGINE::SetMaterial(const D3DMATERIAL9* _pMaterial)
	{
		if (m_pDevice == nullptr || _pMaterial == nullptr)
		{
			return;
		}

		m_pDevice->SetMaterial(_pMaterial);
	}

} // namespace dx9
