//============================================================================
// DX9Sprite.cpp
// 고성능 2D 스프라이트 렌더러 구현
//============================================================================

#include "framework.h"
#include "DX9Sprite.h"
#include "DX9Math.h"

#include <algorithm>

namespace dx9
{
	//------------------------------------------------------------------------
	// 생성자/소멸자
	//------------------------------------------------------------------------
	C_DX9_SPRITE_RENDERER::C_DX9_SPRITE_RENDERER()
		: m_pDevice(nullptr)
		, m_pVertexBuffer(nullptr)
		, m_pIndexBuffer(nullptr)
		, m_nScreenWidth(1024)
		, m_nScreenHeight(768)
		, m_bBegun(false)
		, m_nSpriteCount(0)
		, m_nDrawCallCount(0)
	{
		m_vInstances.reserve(MAX_BATCH_SIZE);
		m_vBatches.reserve(256);
		m_vVertices.reserve(MAX_BATCH_SIZE * VERTICES_PER_SPRITE);
	}

	C_DX9_SPRITE_RENDERER::~C_DX9_SPRITE_RENDERER()
	{
		Release();
	}

	//------------------------------------------------------------------------
	// 초기화
	//------------------------------------------------------------------------
	bool C_DX9_SPRITE_RENDERER::Initialize(LPDIRECT3DDEVICE9 _pDevice, UINT _nScreenWidth, UINT _nScreenHeight)
	{
		if (_pDevice == nullptr)
			return false;

		m_pDevice = _pDevice;
		m_nScreenWidth = _nScreenWidth;
		m_nScreenHeight = _nScreenHeight;

		// 동적 버텍스 버퍼 생성
		HRESULT hr = m_pDevice->CreateVertexBuffer(
			MAX_BATCH_SIZE * VERTICES_PER_SPRITE * sizeof(_SPRITE_VERTEX),
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
			_SPRITE_VERTEX::FVF,
			D3DPOOL_DEFAULT,
			&m_pVertexBuffer,
			nullptr
		);
		if (FAILED(hr))
		{
			DBGPRINT("[DX9Sprite] Failed to create vertex buffer\n");
			return false;
		}

		// 정적 인덱스 버퍼 생성 (미리 계산된 인덱스)
		hr = m_pDevice->CreateIndexBuffer(
			MAX_BATCH_SIZE * INDICES_PER_SPRITE * sizeof(WORD),
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_DEFAULT,
			&m_pIndexBuffer,
			nullptr
		);
		if (FAILED(hr))
		{
			DBGPRINT("[DX9Sprite] Failed to create index buffer\n");
			Release();
			return false;
		}

		// 인덱스 버퍼 초기화 (쿼드 패턴: 0,1,2, 2,1,3)
		WORD* pIndices = nullptr;
		hr = m_pIndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pIndices), 0);
		if (SUCCEEDED(hr))
		{
			for (UINT i = 0; i < MAX_BATCH_SIZE; ++i)
			{
				UINT nBaseVertex = i * VERTICES_PER_SPRITE;
				UINT nBaseIndex = i * INDICES_PER_SPRITE;

				// 삼각형 1: 0-1-2 (좌상-우상-좌하)
				pIndices[nBaseIndex + 0] = static_cast<WORD>(nBaseVertex + 0);
				pIndices[nBaseIndex + 1] = static_cast<WORD>(nBaseVertex + 1);
				pIndices[nBaseIndex + 2] = static_cast<WORD>(nBaseVertex + 2);

				// 삼각형 2: 2-1-3 (좌하-우상-우하)
				pIndices[nBaseIndex + 3] = static_cast<WORD>(nBaseVertex + 2);
				pIndices[nBaseIndex + 4] = static_cast<WORD>(nBaseVertex + 1);
				pIndices[nBaseIndex + 5] = static_cast<WORD>(nBaseVertex + 3);
			}
			m_pIndexBuffer->Unlock();
		}

		// 2D 직교 투영 행렬 설정
		SetScreenSize(_nScreenWidth, _nScreenHeight);

		return true;
	}

	//------------------------------------------------------------------------
	// 해제
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::Release()
	{
		if (m_pIndexBuffer != nullptr)
		{
			m_pIndexBuffer->Release();
			m_pIndexBuffer = nullptr;
		}
		if (m_pVertexBuffer != nullptr)
		{
			m_pVertexBuffer->Release();
			m_pVertexBuffer = nullptr;
		}
		m_pDevice = nullptr;
		m_vInstances.clear();
		m_vBatches.clear();
		m_vVertices.clear();
	}

	//------------------------------------------------------------------------
	// LostDevice 처리
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::OnLostDevice()
	{
		if (m_pIndexBuffer != nullptr)
		{
			m_pIndexBuffer->Release();
			m_pIndexBuffer = nullptr;
		}
		if (m_pVertexBuffer != nullptr)
		{
			m_pVertexBuffer->Release();
			m_pVertexBuffer = nullptr;
		}
	}

	void C_DX9_SPRITE_RENDERER::OnResetDevice()
	{
		if (m_pDevice == nullptr)
			return;

		// 버퍼 재생성
		Initialize(m_pDevice, m_nScreenWidth, m_nScreenHeight);
	}

	//------------------------------------------------------------------------
	// 화면 크기 변경
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::SetScreenSize(UINT _nScreenWidth, UINT _nScreenHeight)
	{
		m_nScreenWidth = _nScreenWidth;
		m_nScreenHeight = _nScreenHeight;

		// 2D 직교 투영 행렬 (픽셀 좌표계)
		// 좌상단 (0,0) ~ 우하단 (width, height)
		D3DXMatrixOrthoOffCenterLH(
			&m_matProjection,
			0.0f,
			static_cast<float>(m_nScreenWidth),
			static_cast<float>(m_nScreenHeight),
			0.0f,
			0.0f,
			1.0f
		);
	}

	//------------------------------------------------------------------------
	// 렌더링 시작
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::Begin()
	{
		if (m_bBegun)
			return;

		m_bBegun = true;
		m_nSpriteCount = 0;
		m_nDrawCallCount = 0;
		m_vInstances.clear();
		m_vBatches.clear();
	}

	//------------------------------------------------------------------------
	// 렌더링 종료
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::End()
	{
		if (!m_bBegun)
			return;

		// 남은 스프라이트 모두 렌더링
		Flush();
		m_bBegun = false;
	}

	//------------------------------------------------------------------------
	// 배칭 강제 플러시
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::Flush()
	{
		if (m_vInstances.empty())
			return;

		// 배치 구성
		BuildBatches();

		// 렌더링
		RenderBatches();

		// 초기화
		m_vInstances.clear();
		m_vBatches.clear();
	}

	//------------------------------------------------------------------------
	// 스프라이트 그리기 (배칭 큐에 추가)
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::Draw(
		const RECT& _rcDest,
		LPDIRECT3DTEXTURE9 _pTexture,
		const RECT& _rcSrc,
		int _nTexWidth,
		int _nTexHeight,
		E_BLEND_MODE _eBlendMode,
		bool _bLighting,
		float _fAlpha,
		float _fScale,
		float _fAngle,
		DWORD _dwColor,
		bool _bInvert
	)
	{
		// 클리핑 체크
		if (_rcDest.left > static_cast<LONG>(m_nScreenWidth) + 150 ||
			_rcDest.right < -150 ||
			_rcDest.top > static_cast<LONG>(m_nScreenHeight) + 150 ||
			_rcDest.bottom < -150)
			return;

		// 유효성 체크
		if (_rcSrc.right - _rcSrc.left <= 0 || _rcSrc.bottom - _rcSrc.top <= 0)
			return;

		if (_fAlpha <= 0.0f)
			return;

		// 알파 클램핑
		_fAlpha = dx9::Clamp(_fAlpha, 0.0f, 1.0f);
		_fScale = dx9::Max(_fScale, 0.0f);

		// 배치 크기 초과 시 플러시
		if (m_vInstances.size() >= MAX_BATCH_SIZE)
			Flush();

		// 색상에 알파 적용
		BYTE nAlpha = static_cast<BYTE>(_fAlpha * 255.0f);
		DWORD dwFinalColor = (_dwColor & 0x00FFFFFF) | (static_cast<DWORD>(nAlpha) << 24);

		// 인스턴스 추가
		_SPRITE_INSTANCE instance;
		instance.pTexture = _pTexture;
		instance.rcDest = _rcDest;
		instance.rcSrc = _rcSrc;
		instance.nTexWidth = _nTexWidth;
		instance.nTexHeight = _nTexHeight;
		instance.dwColor = dwFinalColor;
		instance.fScale = _fScale;
		instance.fAngle = _fAngle * DX9_DEG_TO_RAD;  // 도 → 라디안
		instance.eBlendMode = _eBlendMode;
		instance.bInvert = _bInvert;
		instance.bLighting = _bLighting;

		m_vInstances.push_back(instance);
		++m_nSpriteCount;
	}

	//------------------------------------------------------------------------
	// Wonderking 호환 인터페이스
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::DrawCompat(
		const RECT& _rcDest,
		LPDIRECT3DTEXTURE9 _pTexture,
		const RECT& _rcSrc,
		int _nTexWidth,
		int _nTexHeight,
		int _nBlendType,
		int _nLighting,
		float _fAlpha,
		float _fScale,
		int _nAngle,
		DWORD _dwColor,
		int _nInvert
	)
	{
		E_BLEND_MODE eMode = static_cast<E_BLEND_MODE>(_nBlendType);
		
		// blendtype 0 + lighting 1 → 가산 블렌딩
		if (_nBlendType == 0 && _nLighting == 1)
			eMode = E_BLEND_MODE::BLEND_ALPHA_ADDITIVE;

		Draw(
			_rcDest,
			_pTexture,
			_rcSrc,
			_nTexWidth,
			_nTexHeight,
			eMode,
			_nLighting != 0,
			_fAlpha,
			_fScale,
			static_cast<float>(_nAngle % 360),
			_dwColor,
			_nInvert != 0
		);
	}

	//------------------------------------------------------------------------
	// 즉시 렌더링 (배칭 없이 즉시 그리기)
	// 단일 스프라이트용 - 인덱스 버퍼 사용으로 DrawPrimitiveUP 대비 일관된 성능 제공
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::DrawImmediate(
		const RECT& _rcDest,
		LPDIRECT3DTEXTURE9 _pTexture,
		const RECT& _rcSrc,
		int _nTexWidth,
		int _nTexHeight,
		E_BLEND_MODE _eBlendMode,
		bool _bLighting,
		float _fAlpha,
		float _fScale,
		float _fAngle,
		DWORD _dwColor,
		bool _bInvert
	)
	{
		// 현재 큐 플러시
		Flush();

		// 단일 스프라이트 인스턴스 생성
		_SPRITE_INSTANCE instance;
		instance.pTexture = _pTexture;
		instance.rcDest = _rcDest;
		instance.rcSrc = _rcSrc;
		instance.nTexWidth = _nTexWidth;
		instance.nTexHeight = _nTexHeight;
		
		_fAlpha = dx9::Clamp(_fAlpha, 0.0f, 1.0f);
		BYTE nAlpha = static_cast<BYTE>(_fAlpha * 255.0f);
		instance.dwColor = (_dwColor & 0x00FFFFFF) | (static_cast<DWORD>(nAlpha) << 24);
		
		instance.fScale = dx9::Max(_fScale, 0.0f);
		instance.fAngle = _fAngle * DX9_DEG_TO_RAD;
		instance.eBlendMode = _eBlendMode;
		instance.bInvert = _bInvert;
		instance.bLighting = _bLighting;

		// 버텍스 생성
		_SPRITE_VERTEX vertices[VERTICES_PER_SPRITE];
		GenerateSpriteVertices(instance, vertices);

		// 버텍스 버퍼에 단일 쿼드 업로드 (D3DLOCK_DISCARD로 빠른 쓰기)
		void* pVertexData = nullptr;
		HRESULT hr = m_pVertexBuffer->Lock(0, VERTICES_PER_SPRITE * sizeof(_SPRITE_VERTEX),
			&pVertexData, D3DLOCK_DISCARD);
		if (FAILED(hr))
			return;

		memcpy(pVertexData, vertices, VERTICES_PER_SPRITE * sizeof(_SPRITE_VERTEX));
		m_pVertexBuffer->Unlock();

		// 렌더링
		m_pDevice->BeginScene();

		// 렌더 상태 설정
		SetupBlendMode(_eBlendMode, _bLighting);

		// 변환 행렬 설정
		_DMATRIX9 matWorld, matView;
		D3DXMatrixIdentity(&matWorld);
		D3DXMatrixIdentity(&matView);
		m_pDevice->SetTransform(D3DTS_WORLD, &matWorld);
		m_pDevice->SetTransform(D3DTS_VIEW, &matView);
		m_pDevice->SetTransform(D3DTS_PROJECTION, &m_matProjection);

		// 버텍스/인덱스 버퍼 바인딩
		m_pDevice->SetFVF(_SPRITE_VERTEX::FVF);
		m_pDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(_SPRITE_VERTEX));
		m_pDevice->SetIndices(m_pIndexBuffer);
		m_pDevice->SetTexture(0, _pTexture);

		// 텍스처 상태
		m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		// 렌더 상태
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

		// 필터링
		if (instance.fScale == 1.0f && instance.fAngle == 0.0f)
		{
			m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		}
		else
		{
			m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		}

		// DrawIndexedPrimitive로 렌더링 (인덱스 버퍼 0번부터 쿼드 1개 = 삼각형 2개)
		m_pDevice->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST,
			0,                      // BaseVertexIndex
			0,                      // MinVertexIndex
			VERTICES_PER_SPRITE,    // NumVertices (4)
			0,                      // StartIndex
			2                       // PrimitiveCount (삼각형 2개)
		);

		m_pDevice->EndScene();
		++m_nDrawCallCount;
	}

	//------------------------------------------------------------------------
	// 블렌드 모드 설정
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::SetupBlendMode(E_BLEND_MODE _eMode, bool _bLighting)
	{
		m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0);
		m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);

		switch (_eMode)
		{
		case E_BLEND_MODE::BLEND_ALPHA:
			if (_bLighting)
			{
				m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			}
			else
			{
				m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			}
			break;

		case E_BLEND_MODE::BLEND_ADDITIVE:
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;

		case E_BLEND_MODE::BLEND_NORMAL:
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;

		case E_BLEND_MODE::BLEND_CLOUD:
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);
			break;

		case E_BLEND_MODE::BLEND_SHADOW:
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
			break;

		case E_BLEND_MODE::BLEND_OPACITY:
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;

		case E_BLEND_MODE::BLEND_CLEAR:
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			break;

		case E_BLEND_MODE::BLEND_COPY:
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			break;

		case E_BLEND_MODE::BLEND_ALPHA_ADDITIVE:
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;

		default:
			m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		}
	}

	//------------------------------------------------------------------------
	// 스프라이트 버텍스 생성
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::GenerateSpriteVertices(const _SPRITE_INSTANCE& _instance, _SPRITE_VERTEX* _pOutVertices)
	{
		float fDestWidth = static_cast<float>(_instance.rcDest.right - _instance.rcDest.left);
		float fDestHeight = static_cast<float>(_instance.rcDest.bottom - _instance.rcDest.top);

		// 스케일 적용
		float fScaledWidth = fDestWidth * _instance.fScale;
		float fScaledHeight = fDestHeight * _instance.fScale;

		// 중심점 계산
		float fCenterX = static_cast<float>(_instance.rcDest.left) + fDestWidth * 0.5f;
		float fCenterY = static_cast<float>(_instance.rcDest.top) + fDestHeight * 0.5f;

		// 반쪽 크기
		float fHalfW = fScaledWidth * 0.5f;
		float fHalfH = fScaledHeight * 0.5f;

		// 회전 행렬 계산
		float fCos = cosf(_instance.fAngle);
		float fSin = sinf(_instance.fAngle);

		// 4개의 코너 위치 (회전 전: 중심 기준)
		float fCorners[4][2] = {
			{ -fHalfW, -fHalfH },  // 좌상
			{  fHalfW, -fHalfH },  // 우상
			{ -fHalfW,  fHalfH },  // 좌하
			{  fHalfW,  fHalfH }   // 우하
		};

		// 회전 적용 및 위치 이동
		for (int i = 0; i < 4; ++i)
		{
			float fX = fCorners[i][0];
			float fY = fCorners[i][1];

			// 회전
			_pOutVertices[i].x = fCenterX + fX * fCos - fY * fSin;
			_pOutVertices[i].y = fCenterY + fX * fSin + fY * fCos;
			_pOutVertices[i].z = 0.0f;
			_pOutVertices[i].dwColor = _instance.dwColor;
		}

		// 텍스처 좌표 계산
		float fInvTexW = 1.0f / static_cast<float>(_instance.nTexWidth);
		float fInvTexH = 1.0f / static_cast<float>(_instance.nTexHeight);

		float fU1 = static_cast<float>(_instance.rcSrc.left) * fInvTexW;
		float fV1 = static_cast<float>(_instance.rcSrc.top) * fInvTexH;
		float fU2 = static_cast<float>(_instance.rcSrc.right) * fInvTexW;
		float fV2 = static_cast<float>(_instance.rcSrc.bottom) * fInvTexH;

		// 반전 처리
		if (_instance.bInvert)
			std::swap(fU1, fU2);

		// UV 할당
		_pOutVertices[0].u = fU1; _pOutVertices[0].v = fV1;  // 좌상
		_pOutVertices[1].u = fU2; _pOutVertices[1].v = fV1;  // 우상
		_pOutVertices[2].u = fU1; _pOutVertices[2].v = fV2;  // 좌하
		_pOutVertices[3].u = fU2; _pOutVertices[3].v = fV2;  // 우하
	}

	//------------------------------------------------------------------------
	// 배치 구성
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::BuildBatches()
	{
		if (m_vInstances.empty())
			return;

		m_vBatches.clear();
		m_vVertices.clear();
		m_vVertices.resize(m_vInstances.size() * VERTICES_PER_SPRITE);

		// 버텍스 생성
		for (size_t i = 0; i < m_vInstances.size(); ++i)
		{
			GenerateSpriteVertices(m_vInstances[i], &m_vVertices[i * VERTICES_PER_SPRITE]);
		}

		// 배치 구성 (텍스처 + 블렌드모드 + 라이팅이 같으면 배칭)
		_SPRITE_BATCH currentBatch;
		currentBatch.pTexture = m_vInstances[0].pTexture;
		currentBatch.eBlendMode = m_vInstances[0].eBlendMode;
		currentBatch.bLighting = m_vInstances[0].bLighting;
		currentBatch.nStartVertex = 0;
		currentBatch.nVertexCount = VERTICES_PER_SPRITE;
		currentBatch.nStartIndex = 0;
		currentBatch.nPrimitiveCount = 2;

		for (size_t i = 1; i < m_vInstances.size(); ++i)
		{
			const _SPRITE_INSTANCE& inst = m_vInstances[i];

			// 배치 조건 확인 (같은 텍스처, 블렌드모드, 라이팅)
			if (inst.pTexture == currentBatch.pTexture &&
				inst.eBlendMode == currentBatch.eBlendMode &&
				inst.bLighting == currentBatch.bLighting)
			{
				// 현재 배치에 추가
				currentBatch.nVertexCount += VERTICES_PER_SPRITE;
				currentBatch.nPrimitiveCount += 2;
			}
			else
			{
				// 새 배치 시작
				m_vBatches.push_back(currentBatch);

				currentBatch.pTexture = inst.pTexture;
				currentBatch.eBlendMode = inst.eBlendMode;
				currentBatch.bLighting = inst.bLighting;
				currentBatch.nStartVertex = static_cast<UINT>(i * VERTICES_PER_SPRITE);
				currentBatch.nVertexCount = VERTICES_PER_SPRITE;
				currentBatch.nStartIndex = static_cast<UINT>(i * INDICES_PER_SPRITE);
				currentBatch.nPrimitiveCount = 2;
			}
		}

		// 마지막 배치 추가
		m_vBatches.push_back(currentBatch);
	}

	//------------------------------------------------------------------------
	// 배치 렌더링
	//------------------------------------------------------------------------
	void C_DX9_SPRITE_RENDERER::RenderBatches()
	{
		if (m_vBatches.empty() || m_vVertices.empty())
			return;

		// 버텍스 버퍼 업데이트
		void* pVertexData = nullptr;
		HRESULT hr = m_pVertexBuffer->Lock(0, static_cast<UINT>(m_vVertices.size() * sizeof(_SPRITE_VERTEX)),
			&pVertexData, D3DLOCK_DISCARD);
		if (FAILED(hr))
			return;

		memcpy(pVertexData, m_vVertices.data(), m_vVertices.size() * sizeof(_SPRITE_VERTEX));
		m_pVertexBuffer->Unlock();

		// 렌더링 상태 설정
		m_pDevice->BeginScene();

		// 고정 파이프라인 설정
		_DMATRIX9 matWorld, matView;
		D3DXMatrixIdentity(&matWorld);
		D3DXMatrixIdentity(&matView);
		m_pDevice->SetTransform(D3DTS_WORLD, &matWorld);
		m_pDevice->SetTransform(D3DTS_VIEW, &matView);
		m_pDevice->SetTransform(D3DTS_PROJECTION, &m_matProjection);

		// 버텍스/인덱스 버퍼 바인딩
		m_pDevice->SetFVF(_SPRITE_VERTEX::FVF);
		m_pDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(_SPRITE_VERTEX));
		m_pDevice->SetIndices(m_pIndexBuffer);

		// 텍스처 상태 설정
		m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		// 컬링 끄기
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

		// 각 배치 렌더링
		E_BLEND_MODE eLastBlendMode = E_BLEND_MODE::BLEND_COUNT;
		bool bLastLighting = false;
		LPDIRECT3DTEXTURE9 pLastTexture = nullptr;

		for (const auto& batch : m_vBatches)
		{
			// 블렌드 모드 변경
			if (batch.eBlendMode != eLastBlendMode || batch.bLighting != bLastLighting)
			{
				SetupBlendMode(batch.eBlendMode, batch.bLighting);
				eLastBlendMode = batch.eBlendMode;
				bLastLighting = batch.bLighting;
			}

			// 텍스처 변경
			if (batch.pTexture != pLastTexture)
			{
				m_pDevice->SetTexture(0, batch.pTexture);
				pLastTexture = batch.pTexture;

				// 필터링 설정 (배치의 첫 인스턴스 기준)
				// 간단하게 POINT 필터 사용 (원본 동작과 동일)
				m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
				m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			}

			// 인덱스 버퍼로 렌더링
			m_pDevice->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST,
				0,
				batch.nStartVertex,
				batch.nVertexCount,
				batch.nStartIndex,
				batch.nPrimitiveCount
			);

			++m_nDrawCallCount;
		}

		m_pDevice->EndScene();
	}

} // namespace dx9

