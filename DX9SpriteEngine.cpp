/**
 * @file DX9SpriteEngine.cpp
 * @brief DirectX 9 2D 스프라이트 엔진 구현
 * @author DX9Wrapp
 * @date 2026-01-20
 */

#include "framework.h"
#include "DX9SpriteEngine.h"
#include "DX9Math.h"

namespace dx9
{

//============================================================================
// 싱글톤 인스턴스
//============================================================================
C_DX9_SPRITE_ENGINE* C_DX9_SPRITE_ENGINE::s_pInstance = nullptr;

//============================================================================
// 생성자/소멸자
//============================================================================
C_DX9_SPRITE_ENGINE::C_DX9_SPRITE_ENGINE()
	: m_pD3D(nullptr)
	, m_pDevice(nullptr)
	, m_pSpriteRenderer(nullptr)
	, m_bInitialized(false)
	, m_bDeviceLost(false)
	, m_bInFrame(false)
	, m_bOwnDevice(true)
	, m_nFrameCountForFPS(0)
{
	ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
	ZeroMemory(&m_liFrequency, sizeof(m_liFrequency));
	ZeroMemory(&m_liLastFrameTime, sizeof(m_liLastFrameTime));
	ZeroMemory(&m_liLastFPSUpdate, sizeof(m_liLastFPSUpdate));
	
	QueryPerformanceFrequency(&m_liFrequency);
}

C_DX9_SPRITE_ENGINE::~C_DX9_SPRITE_ENGINE()
{
	Shutdown();
}

//============================================================================
// 싱글톤 접근
//============================================================================
C_DX9_SPRITE_ENGINE* C_DX9_SPRITE_ENGINE::GetInstance()
{
	if (s_pInstance == nullptr)
	{
		s_pInstance = new C_DX9_SPRITE_ENGINE();
	}
	return s_pInstance;
}

void C_DX9_SPRITE_ENGINE::DestroyInstance()
{
	if (s_pInstance != nullptr)
	{
		delete s_pInstance;
		s_pInstance = nullptr;
	}
}

//============================================================================
// 초기화
//============================================================================
bool C_DX9_SPRITE_ENGINE::Initialize(HWND _hWnd, UINT _nWidth, UINT _nHeight, bool _bWindowed)
{
	_ENGINE_CONFIG config;
	config.hWnd = _hWnd;
	config.nScreenWidth = _nWidth;
	config.nScreenHeight = _nHeight;
	config.bWindowed = _bWindowed;
	
	return Initialize(config);
}

bool C_DX9_SPRITE_ENGINE::Initialize(const _ENGINE_CONFIG& _config)
{
	if (m_bInitialized)
	{
		DBGPRINT("[DX9SpriteEngine] Already initialized\n");
		return true;
	}
	
	m_config = _config;
	
	// Direct3D 생성
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pD3D == nullptr)
	{
		DBGPRINT("[DX9SpriteEngine] Failed to create Direct3D9\n");
		return false;
	}
	
	// 디바이스 생성
	if (!CreateDevice())
	{
		m_pD3D->Release();
		m_pD3D = nullptr;
		return false;
	}
	
	// 스프라이트 렌더러 생성
	m_pSpriteRenderer = new C_DX9_SPRITE_RENDERER();
	if (!m_pSpriteRenderer->Initialize(m_pDevice, m_config.nScreenWidth, m_config.nScreenHeight))
	{
		DBGPRINT("[DX9SpriteEngine] Failed to initialize sprite renderer\n");
		delete m_pSpriteRenderer;
		m_pSpriteRenderer = nullptr;
		m_pDevice->Release();
		m_pDevice = nullptr;
		m_pD3D->Release();
		m_pD3D = nullptr;
		return false;
	}
	
	// 텍스처 슬롯 배열 초기화
	m_vTextures.resize(m_config.nMaxTextures);
	
	// 타이머 초기화
	QueryPerformanceCounter(&m_liLastFrameTime);
	QueryPerformanceCounter(&m_liLastFPSUpdate);
	
	m_bOwnDevice = true;  // 엔진이 디바이스를 소유
	m_bInitialized = true;
	DBGPRINT("[DX9SpriteEngine] Initialized successfully\n");
	
	return true;
}

bool C_DX9_SPRITE_ENGINE::InitializeWithExternalDevice(LPDIRECT3DDEVICE9 _pExternalDevice, UINT _nScreenWidth, UINT _nScreenHeight)
{
	if (m_bInitialized)
	{
		DBGPRINT("[DX9SpriteEngine] Already initialized\n");
		return true;
	}
	
	if (_pExternalDevice == nullptr)
	{
		DBGPRINT("[DX9SpriteEngine] External device is null\n");
		return false;
	}
	
	// 외부 디바이스 참조 (Release하지 않음)
	m_pDevice = _pExternalDevice;
	m_pD3D = nullptr;  // 외부 디바이스이므로 D3D 인터페이스는 사용하지 않음
	
	// 설정 저장
	m_config.nScreenWidth = _nScreenWidth;
	m_config.nScreenHeight = _nScreenHeight;
	m_config.nMaxTextures = 4096;
	
	// 스프라이트 렌더러 생성 (외부 디바이스: BeginScene/EndScene 스킵)
	m_pSpriteRenderer = new C_DX9_SPRITE_RENDERER();
	if (!m_pSpriteRenderer->Initialize(m_pDevice, _nScreenWidth, _nScreenHeight, true))  // true = 외부 디바이스
	{
		DBGPRINT("[DX9SpriteEngine] Failed to initialize sprite renderer with external device\n");
		delete m_pSpriteRenderer;
		m_pSpriteRenderer = nullptr;
		m_pDevice = nullptr;  // 외부 디바이스이므로 Release 안 함
		return false;
	}
	
	// 텍스처 슬롯 배열 초기화
	m_vTextures.resize(m_config.nMaxTextures);
	
	// 타이머 초기화
	QueryPerformanceCounter(&m_liLastFrameTime);
	QueryPerformanceCounter(&m_liLastFPSUpdate);
	
	m_bOwnDevice = false;  // 외부 디바이스 - Release하지 않음
	m_bInitialized = true;
	DBGPRINT("[DX9SpriteEngine] Initialized with external device successfully\n");
	
	return true;
}

bool C_DX9_SPRITE_ENGINE::CreateDevice()
{
	// 프레젠트 파라미터 설정
	ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
	m_d3dpp.Windowed = m_config.bWindowed;
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dpp.BackBufferFormat = m_config.bWindowed ? D3DFMT_UNKNOWN : m_config.eBackBufferFormat;
	m_d3dpp.BackBufferWidth = m_config.nScreenWidth;
	m_d3dpp.BackBufferHeight = m_config.nScreenHeight;
	m_d3dpp.EnableAutoDepthStencil = FALSE;
	m_d3dpp.PresentationInterval = m_config.bVSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	m_d3dpp.hDeviceWindow = m_config.hWnd;
	
	// 하드웨어 버텍스 처리로 디바이스 생성 시도
	HRESULT hr = m_pD3D->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		m_config.hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&m_d3dpp,
		&m_pDevice
	);
	
	// 실패 시 소프트웨어 버텍스 처리로 재시도
	if (FAILED(hr))
	{
		hr = m_pD3D->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			m_config.hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&m_d3dpp,
			&m_pDevice
		);
	}
	
	if (FAILED(hr))
	{
		DBGPRINT("[DX9SpriteEngine] Failed to create D3D device\n");
		return false;
	}
	
	return true;
}

//============================================================================
// 종료
//============================================================================
void C_DX9_SPRITE_ENGINE::Shutdown()
{
	if (!m_bInitialized)
		return;
	
	// 텍스처 해제
	UnloadAllTextures();
	
	// 스프라이트 렌더러 해제
	if (m_pSpriteRenderer != nullptr)
	{
		m_pSpriteRenderer->Release();
		delete m_pSpriteRenderer;
		m_pSpriteRenderer = nullptr;
	}
	
	// 디바이스 해제 (소유한 경우에만)
	if (m_bOwnDevice)
	{
		if (m_pDevice != nullptr)
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
	}
	else
	{
		// 외부 디바이스: 포인터만 초기화 (Release 안 함)
		m_pDevice = nullptr;
		m_pD3D = nullptr;
	}
	
	m_bInitialized = false;
	DBGPRINT("[DX9SpriteEngine] Shutdown complete\n");
}

//============================================================================
// 화면 크기 변경
//============================================================================
bool C_DX9_SPRITE_ENGINE::Resize(UINT _nWidth, UINT _nHeight)
{
	if (!m_bInitialized)
		return false;
	
	m_config.nScreenWidth = _nWidth;
	m_config.nScreenHeight = _nHeight;
	
	ResetDevice();
	
	return true;
}

void C_DX9_SPRITE_ENGINE::ResetDevice()
{
	// 스프라이트 렌더러 디바이스 손실 처리
	if (m_pSpriteRenderer != nullptr)
	{
		m_pSpriteRenderer->OnLostDevice();
	}
	
	// 프레젠트 파라미터 업데이트
	m_d3dpp.BackBufferWidth = m_config.nScreenWidth;
	m_d3dpp.BackBufferHeight = m_config.nScreenHeight;
	
	// 디바이스 리셋
	HRESULT hr = m_pDevice->Reset(&m_d3dpp);
	if (SUCCEEDED(hr))
	{
		// 스프라이트 렌더러 복구
		if (m_pSpriteRenderer != nullptr)
		{
			m_pSpriteRenderer->OnResetDevice();
			m_pSpriteRenderer->SetScreenSize(m_config.nScreenWidth, m_config.nScreenHeight);
		}
		m_bDeviceLost = false;
	}
}

bool C_DX9_SPRITE_ENGINE::HandleDeviceLost()
{
	HRESULT hr = m_pDevice->TestCooperativeLevel();
	
	if (hr == D3DERR_DEVICELOST)
	{
		// 아직 복구 불가
		Sleep(100);
		return false;
	}
	
	if (hr == D3DERR_DEVICENOTRESET)
	{
		// 복구 가능
		ResetDevice();
	}
	
	return !m_bDeviceLost;
}

//============================================================================
// 텍스처 관리
//============================================================================
int C_DX9_SPRITE_ENGINE::LoadTexture(const std::wstring& _strFilePath, D3DCOLOR _nColorKey)
{
	// 이미 로드된 텍스처 확인
	auto it = m_mapTextureIndex.find(_strFilePath);
	if (it != m_mapTextureIndex.end())
	{
		return it->second;
	}
	
	// 빈 슬롯 찾기
	int nSlot = -1;
	for (size_t i = 0; i < m_vTextures.size(); ++i)
	{
		if (m_vTextures[i].pTexture == nullptr)
		{
			nSlot = static_cast<int>(i);
			break;
		}
	}
	
	if (nSlot < 0)
	{
		DBGPRINT("[DX9SpriteEngine] No available texture slot\n");
		return -1;
	}
	
	if (LoadTextureToSlot(nSlot, _strFilePath, _nColorKey))
	{
		return nSlot;
	}
	
	return -1;
}

bool C_DX9_SPRITE_ENGINE::LoadTextureToSlot(int _nSlot, const std::wstring& _strFilePath, D3DCOLOR _nColorKey)
{
	if (_nSlot < 0 || _nSlot >= static_cast<int>(m_vTextures.size()))
		return false;
	
	// 기존 텍스처 해제
	UnloadTexture(_nSlot);
	
	// 텍스처 정보 획득
	D3DXIMAGE_INFO imageInfo;
	if (FAILED(D3DXGetImageInfoFromFileW(_strFilePath.c_str(), &imageInfo)))
	{
		DBGPRINT("[DX9SpriteEngine] Failed to get image info\n");
		return false;
	}
	
	// 텍스처 로드
	LPDIRECT3DTEXTURE9 pTexture = nullptr;
	HRESULT hr = D3DXCreateTextureFromFileExW(
		m_pDevice,
		_strFilePath.c_str(),
		imageInfo.Width,
		imageInfo.Height,
		1,                          // MipLevels
		0,                          // Usage
		D3DFMT_A8R8G8B8,           // Format
		D3DPOOL_MANAGED,           // Pool
		D3DX_FILTER_NONE,          // Filter
		D3DX_FILTER_NONE,          // MipFilter
		_nColorKey,                // ColorKey
		nullptr,                   // pSrcInfo
		nullptr,                   // pPalette
		&pTexture
	);
	
	if (FAILED(hr) || pTexture == nullptr)
	{
		DBGPRINT("[DX9SpriteEngine] Failed to load texture\n");
		return false;
	}
	
	// 슬롯에 저장
	m_vTextures[_nSlot].pTexture = pTexture;
	m_vTextures[_nSlot].nWidth = static_cast<int>(imageInfo.Width);
	m_vTextures[_nSlot].nHeight = static_cast<int>(imageInfo.Height);
	m_vTextures[_nSlot].strFilePath = _strFilePath;
	m_vTextures[_nSlot].bManaged = true;
	
	// 인덱스 맵에 등록
	m_mapTextureIndex[_strFilePath] = _nSlot;
	
	return true;
}

int C_DX9_SPRITE_ENGINE::RegisterTexture(LPDIRECT3DTEXTURE9 _pTexture, int _nWidth, int _nHeight)
{
	// 빈 슬롯 찾기
	for (size_t i = 0; i < m_vTextures.size(); ++i)
	{
		if (m_vTextures[i].pTexture == nullptr)
		{
			SetTextureSlot(static_cast<int>(i), _pTexture, _nWidth, _nHeight);
			return static_cast<int>(i);
		}
	}
	return -1;
}

void C_DX9_SPRITE_ENGINE::SetTextureSlot(int _nSlot, LPDIRECT3DTEXTURE9 _pTexture, int _nWidth, int _nHeight)
{
	if (_nSlot < 0 || _nSlot >= static_cast<int>(m_vTextures.size()))
		return;
	
	// 기존 관리 텍스처 해제
	if (m_vTextures[_nSlot].bManaged && m_vTextures[_nSlot].pTexture != nullptr)
	{
		m_vTextures[_nSlot].pTexture->Release();
	}
	
	m_vTextures[_nSlot].pTexture = _pTexture;
	m_vTextures[_nSlot].nWidth = _nWidth;
	m_vTextures[_nSlot].nHeight = _nHeight;
	m_vTextures[_nSlot].strFilePath.clear();
	m_vTextures[_nSlot].bManaged = false;  // 외부 텍스처는 엔진이 해제하지 않음
}

void C_DX9_SPRITE_ENGINE::UnloadTexture(int _nSlot)
{
	if (_nSlot < 0 || _nSlot >= static_cast<int>(m_vTextures.size()))
		return;
	
	_TEXTURE_SLOT& slot = m_vTextures[_nSlot];
	
	// 인덱스 맵에서 제거
	if (!slot.strFilePath.empty())
	{
		m_mapTextureIndex.erase(slot.strFilePath);
	}
	
	// 관리 텍스처만 해제
	if (slot.bManaged && slot.pTexture != nullptr)
	{
		slot.pTexture->Release();
	}
	
	slot.pTexture = nullptr;
	slot.nWidth = 0;
	slot.nHeight = 0;
	slot.strFilePath.clear();
	slot.bManaged = false;
}

void C_DX9_SPRITE_ENGINE::UnloadAllTextures()
{
	for (size_t i = 0; i < m_vTextures.size(); ++i)
	{
		if (m_vTextures[i].bManaged && m_vTextures[i].pTexture != nullptr)
		{
			m_vTextures[i].pTexture->Release();
		}
		m_vTextures[i].pTexture = nullptr;
		m_vTextures[i].nWidth = 0;
		m_vTextures[i].nHeight = 0;
		m_vTextures[i].strFilePath.clear();
		m_vTextures[i].bManaged = false;
	}
	m_mapTextureIndex.clear();
}

const _TEXTURE_SLOT* C_DX9_SPRITE_ENGINE::GetTextureInfo(int _nSlot) const
{
	if (_nSlot < 0 || _nSlot >= static_cast<int>(m_vTextures.size()))
		return nullptr;
	
	if (m_vTextures[_nSlot].pTexture == nullptr)
		return nullptr;
	
	return &m_vTextures[_nSlot];
}

//============================================================================
// 프레임 관리
//============================================================================
bool C_DX9_SPRITE_ENGINE::BeginFrame(D3DCOLOR _dwClearColor)
{
	if (!m_bInitialized)
		return false;
	
	// 디바이스 손실 처리 (소유한 디바이스만)
	if (m_bOwnDevice && m_bDeviceLost)
	{
		if (!HandleDeviceLost())
			return false;
	}
	
	// 프레임 타이밍 계산
	LARGE_INTEGER liCurrentTime;
	QueryPerformanceCounter(&liCurrentTime);
	
	m_stats.fFrameTime = static_cast<float>(liCurrentTime.QuadPart - m_liLastFrameTime.QuadPart) 
		/ static_cast<float>(m_liFrequency.QuadPart) * 1000.0f;
	m_liLastFrameTime = liCurrentTime;
	
	// FPS 계산 (1초마다 갱신)
	++m_nFrameCountForFPS;
	float fTimeSinceLastFPSUpdate = static_cast<float>(liCurrentTime.QuadPart - m_liLastFPSUpdate.QuadPart)
		/ static_cast<float>(m_liFrequency.QuadPart);
	
	if (fTimeSinceLastFPSUpdate >= 1.0f)
	{
		m_stats.fFPS = static_cast<float>(m_nFrameCountForFPS) / fTimeSinceLastFPSUpdate;
		m_nFrameCountForFPS = 0;
		m_liLastFPSUpdate = liCurrentTime;
	}
	
	// 화면 클리어 (소유한 디바이스만 - 외부 디바이스는 외부에서 Clear 담당)
	if (m_bOwnDevice)
	{
		m_pDevice->Clear(0, nullptr, D3DCLEAR_TARGET, _dwClearColor, 1.0f, 0);
	}
	
	// 스프라이트 배칭 시작
	if (m_pSpriteRenderer != nullptr)
	{
		m_pSpriteRenderer->Begin();
	}
	
	m_bInFrame = true;
	++m_stats.nFrameCount;
	
	return true;
}

void C_DX9_SPRITE_ENGINE::EndFrame()
{
	if (!m_bInFrame)
		return;
	
	// 스프라이트 배칭 종료 (플러시)
	if (m_pSpriteRenderer != nullptr)
	{
		m_pSpriteRenderer->End();
		
		// 통계 업데이트
		m_stats.nDrawCallsPerFrame = m_pSpriteRenderer->GetDrawCallCount();
		m_stats.nSpritesPerFrame = m_pSpriteRenderer->GetSpriteCount();
	}
	
	m_bInFrame = false;
}

bool C_DX9_SPRITE_ENGINE::Present()
{
	// 외부 디바이스 시 Present는 외부에서 담당
	if (!m_bOwnDevice)
	{
		return true;
	}
	
	HRESULT hr = m_pDevice->Present(nullptr, nullptr, nullptr, nullptr);
	
	if (hr == D3DERR_DEVICELOST)
	{
		m_bDeviceLost = true;
		return false;
	}
	
	return SUCCEEDED(hr);
}

//============================================================================
// 스프라이트 그리기 - 간단한 인터페이스
//============================================================================
void C_DX9_SPRITE_ENGINE::DrawSprite(int _nTexSlot, float _fX, float _fY, float _fAlpha)
{
	const _TEXTURE_SLOT* pSlot = GetTextureInfo(_nTexSlot);
	if (pSlot == nullptr)
		return;
	
	DrawSprite(_nTexSlot, _fX, _fY, static_cast<float>(pSlot->nWidth), static_cast<float>(pSlot->nHeight), _fAlpha);
}

void C_DX9_SPRITE_ENGINE::DrawSprite(int _nTexSlot, float _fX, float _fY, float _fWidth, float _fHeight, float _fAlpha)
{
	const _TEXTURE_SLOT* pSlot = GetTextureInfo(_nTexSlot);
	if (pSlot == nullptr)
		return;
	
	RECT rcDest = { 
		static_cast<LONG>(_fX), 
		static_cast<LONG>(_fY), 
		static_cast<LONG>(_fX + _fWidth), 
		static_cast<LONG>(_fY + _fHeight) 
	};
	
	RECT rcSrc = { 0, 0, pSlot->nWidth, pSlot->nHeight };
	
	DrawSprite(_nTexSlot, rcDest, rcSrc, _fAlpha);
}

void C_DX9_SPRITE_ENGINE::DrawSprite(int _nTexSlot, const RECT& _rcDest, const RECT& _rcSrc, float _fAlpha)
{
	DrawSpriteEx(_nTexSlot, _rcDest, _rcSrc, E_BLEND_MODE::BLEND_ALPHA, _fAlpha);
}

//============================================================================
// 스프라이트 그리기 - 상세 인터페이스
//============================================================================
void C_DX9_SPRITE_ENGINE::DrawSpriteEx(
	int _nTexSlot,
	const RECT& _rcDest,
	const RECT& _rcSrc,
	E_BLEND_MODE _eBlendMode,
	float _fAlpha,
	float _fScale,
	float _fAngle,
	D3DCOLOR _dwColor,
	bool _bInvert
)
{
	if (m_pSpriteRenderer == nullptr)
		return;
	
	const _TEXTURE_SLOT* pSlot = GetTextureInfo(_nTexSlot);
	if (pSlot == nullptr)
		return;
	
	m_pSpriteRenderer->Draw(
		_rcDest,
		pSlot->pTexture,
		_rcSrc,
		pSlot->nWidth,
		pSlot->nHeight,
		_eBlendMode,
		false,
		_fAlpha,
		_fScale,
		_fAngle,
		_dwColor,
		_bInvert
	);
}

void C_DX9_SPRITE_ENGINE::DrawSpriteCompat(
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
)
{
	if (m_pSpriteRenderer == nullptr)
		return;
	
	const _TEXTURE_SLOT* pSlot = GetTextureInfo(_nTexSlot);
	if (pSlot == nullptr)
		return;
	
	m_pSpriteRenderer->DrawCompat(
		_rcDest,
		pSlot->pTexture,
		_rcSrc,
		pSlot->nWidth,
		pSlot->nHeight,
		_nBlendType,
		_nLighting,
		_fAlpha,
		_fScale,
		_nAngle,
		_dwColor,
		_nInvert
	);
}

//============================================================================
// 배칭 제어
//============================================================================
void C_DX9_SPRITE_ENGINE::Flush()
{
	if (m_pSpriteRenderer != nullptr)
	{
		m_pSpriteRenderer->Flush();
	}
}

//============================================================================
// 렌더 스테이트 저장
// 호스트(Wonderking) 렌더 스테이트를 백업하여 DX9Wrapp 렌더링 후 복원 가능
//============================================================================
bool C_DX9_SPRITE_ENGINE::SaveRenderStates()
{
	if (m_pDevice == nullptr)
	{
		DBGPRINT(L"[DX9SpriteEngine] SaveRenderStates: 디바이스 없음");
		return false;
	}
	
	_RENDER_STATE_BACKUP& backup = m_stRenderStateBackup;
	
	//------------------------------------------------------------------------
	// 렌더 스테이트 저장
	//------------------------------------------------------------------------
	m_pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &backup.dwAlphaBlendEnable);
	m_pDevice->GetRenderState(D3DRS_SRCBLEND, &backup.dwSrcBlend);
	m_pDevice->GetRenderState(D3DRS_DESTBLEND, &backup.dwDestBlend);
	m_pDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &backup.dwAlphaTestEnable);
	m_pDevice->GetRenderState(D3DRS_ALPHAREF, &backup.dwAlphaRef);
	m_pDevice->GetRenderState(D3DRS_ALPHAFUNC, &backup.dwAlphaFunc);
	m_pDevice->GetRenderState(D3DRS_CULLMODE, &backup.dwCullMode);
	m_pDevice->GetRenderState(D3DRS_LIGHTING, &backup.dwLighting);
	m_pDevice->GetRenderState(D3DRS_ZENABLE, &backup.dwZEnable);
	m_pDevice->GetRenderState(D3DRS_ZWRITEENABLE, &backup.dwZWriteEnable);
	m_pDevice->GetRenderState(D3DRS_FOGENABLE, &backup.dwFogEnable);
	m_pDevice->GetRenderState(D3DRS_STENCILENABLE, &backup.dwStencilEnable);
	
	//------------------------------------------------------------------------
	// 텍스처 스테이지 상태 저장 (Stage 0)
	//------------------------------------------------------------------------
	m_pDevice->GetTextureStageState(0, D3DTSS_COLOROP, &backup.dwColorOp);
	m_pDevice->GetTextureStageState(0, D3DTSS_COLORARG1, &backup.dwColorArg1);
	m_pDevice->GetTextureStageState(0, D3DTSS_COLORARG2, &backup.dwColorArg2);
	m_pDevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &backup.dwAlphaOp);
	m_pDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &backup.dwAlphaArg1);
	m_pDevice->GetTextureStageState(0, D3DTSS_ALPHAARG2, &backup.dwAlphaArg2);
	
	//------------------------------------------------------------------------
	// 샘플러 상태 저장 (Stage 0)
	//------------------------------------------------------------------------
	m_pDevice->GetSamplerState(0, D3DSAMP_MAGFILTER, &backup.dwMagFilter);
	m_pDevice->GetSamplerState(0, D3DSAMP_MINFILTER, &backup.dwMinFilter);
	m_pDevice->GetSamplerState(0, D3DSAMP_MIPFILTER, &backup.dwMipFilter);
	m_pDevice->GetSamplerState(0, D3DSAMP_ADDRESSU, &backup.dwAddressU);
	m_pDevice->GetSamplerState(0, D3DSAMP_ADDRESSV, &backup.dwAddressV);
	
	//------------------------------------------------------------------------
	// 변환 행렬 저장
	//------------------------------------------------------------------------
	m_pDevice->GetTransform(D3DTS_WORLD, &backup.matWorld);
	m_pDevice->GetTransform(D3DTS_VIEW, &backup.matView);
	m_pDevice->GetTransform(D3DTS_PROJECTION, &backup.matProjection);
	
	//------------------------------------------------------------------------
	// 텍스처, 셰이더, FVF 저장
	//------------------------------------------------------------------------
	m_pDevice->GetTexture(0, reinterpret_cast<IDirect3DBaseTexture9**>(&backup.pTexture0));
	m_pDevice->GetVertexShader(&backup.pVS);
	m_pDevice->GetPixelShader(&backup.pPS);
	m_pDevice->GetFVF(&backup.dwFVF);
	
	//------------------------------------------------------------------------
	// 버텍스/인덱스 버퍼 저장
	//------------------------------------------------------------------------
	m_pDevice->GetStreamSource(0, &backup.pVB, &backup.nVBOffset, &backup.nVBStride);
	m_pDevice->GetIndices(&backup.pIB);
	
	backup.bValid = true;
	
	DBGPRINT(L"[DX9SpriteEngine] SaveRenderStates 완료: AlphaBlend=%d, Src=%d, Dest=%d, FVF=0x%08X",
		backup.dwAlphaBlendEnable, backup.dwSrcBlend, backup.dwDestBlend, backup.dwFVF);
	
	return true;
}

//============================================================================
// 렌더 스테이트 복원
// SaveRenderStates()로 저장한 호스트 상태를 복원
//============================================================================
bool C_DX9_SPRITE_ENGINE::RestoreRenderStates()
{
	if (m_pDevice == nullptr)
	{
		DBGPRINT(L"[DX9SpriteEngine] RestoreRenderStates: 디바이스 없음");
		return false;
	}
	
	const _RENDER_STATE_BACKUP& backup = m_stRenderStateBackup;
	
	if (!backup.bValid)
	{
		DBGPRINT(L"[DX9SpriteEngine] RestoreRenderStates: 유효한 백업 없음");
		return false;
	}
	
	//------------------------------------------------------------------------
	// 렌더 스테이트 복원
	//------------------------------------------------------------------------
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, backup.dwAlphaBlendEnable);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, backup.dwSrcBlend);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, backup.dwDestBlend);
	m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, backup.dwAlphaTestEnable);
	m_pDevice->SetRenderState(D3DRS_ALPHAREF, backup.dwAlphaRef);
	m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, backup.dwAlphaFunc);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, backup.dwCullMode);
	m_pDevice->SetRenderState(D3DRS_LIGHTING, backup.dwLighting);
	m_pDevice->SetRenderState(D3DRS_ZENABLE, backup.dwZEnable);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, backup.dwZWriteEnable);
	m_pDevice->SetRenderState(D3DRS_FOGENABLE, backup.dwFogEnable);
	m_pDevice->SetRenderState(D3DRS_STENCILENABLE, backup.dwStencilEnable);
	
	//------------------------------------------------------------------------
	// 텍스처 스테이지 상태 복원 (Stage 0)
	//------------------------------------------------------------------------
	m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, backup.dwColorOp);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, backup.dwColorArg1);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, backup.dwColorArg2);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, backup.dwAlphaOp);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, backup.dwAlphaArg1);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, backup.dwAlphaArg2);
	
	//------------------------------------------------------------------------
	// 샘플러 상태 복원 (Stage 0)
	//------------------------------------------------------------------------
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, backup.dwMagFilter);
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, backup.dwMinFilter);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, backup.dwMipFilter);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, backup.dwAddressU);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, backup.dwAddressV);
	
	//------------------------------------------------------------------------
	// 변환 행렬 복원
	//------------------------------------------------------------------------
	m_pDevice->SetTransform(D3DTS_WORLD, &backup.matWorld);
	m_pDevice->SetTransform(D3DTS_VIEW, &backup.matView);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &backup.matProjection);
	
	//------------------------------------------------------------------------
	// 텍스처, 셰이더, FVF 복원
	//------------------------------------------------------------------------
	m_pDevice->SetTexture(0, backup.pTexture0);
	m_pDevice->SetVertexShader(backup.pVS);
	m_pDevice->SetPixelShader(backup.pPS);
	m_pDevice->SetFVF(backup.dwFVF);
	
	//------------------------------------------------------------------------
	// 버텍스/인덱스 버퍼 복원
	//------------------------------------------------------------------------
	m_pDevice->SetStreamSource(0, backup.pVB, backup.nVBOffset, backup.nVBStride);
	m_pDevice->SetIndices(backup.pIB);
	
	//------------------------------------------------------------------------
	// COM 참조 해제 (GetXXX에서 AddRef된 것들)
	//------------------------------------------------------------------------
	if (backup.pTexture0 != nullptr)
		backup.pTexture0->Release();
	if (backup.pVS != nullptr)
		backup.pVS->Release();
	if (backup.pPS != nullptr)
		backup.pPS->Release();
	if (backup.pVB != nullptr)
		backup.pVB->Release();
	if (backup.pIB != nullptr)
		backup.pIB->Release();
	
	DBGPRINT(L"[DX9SpriteEngine] RestoreRenderStates 완료");
	
	// 백업 무효화 (한 번만 복원 가능)
	m_stRenderStateBackup.bValid = false;
	
	return true;
}

} // namespace dx9