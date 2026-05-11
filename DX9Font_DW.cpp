// DX9Font_DW.cpp: GDI 글리프 아틀라스 + D3D9 자체 batch 구현.
#include "framework.h"
#include "DX9Font_DW.h"

#include <cstdarg>
#include <cstdio>
#include <vector>


namespace dx9
{
	//============================================================================
	// 생성/소멸
	//============================================================================
	C_DX9_FONT_DW::C_DX9_FONT_DW()
		: m_pDevice(nullptr)
		, m_pTexture(nullptr)
		, m_uAtlasSize(0)
		, m_uCellSize(0)
		, m_uCellsX(0), m_uCellsY(0), m_uCellCount(0)
		, m_uFontHeight(0)
		, m_hDC(nullptr)
		, m_hFont(nullptr)
		, m_hBitmap(nullptr)
		, m_pBits(nullptr)
		, m_uNextCell(0)
		, m_pVB(nullptr)
		, m_pIB(nullptr)
		, m_uMaxQuads(0)
		, m_uQuadCount(0)
		, m_uVBOffset(0)
		, m_bRendering(false)
	{
		ZeroMemory(&m_Backup, sizeof(m_Backup));
	}


	C_DX9_FONT_DW::~C_DX9_FONT_DW()
	{
		Shutdown();
	}


	//============================================================================
	// 버퍼 생성/해제 (atlas + VB + IB)
	//============================================================================
	bool C_DX9_FONT_DW::createBuffers_()
	{
		if (!m_pDevice) return false;

		HRESULT hr = m_pDevice->CreateTexture(
			m_uAtlasSize, m_uAtlasSize, 1,
			0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
			&m_pTexture, nullptr);
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] CreateTexture(%ux%u) 실패: 0x%08X",
				m_uAtlasSize, m_uAtlasSize, hr);
			return false;
		}

		// 아틀라스 0 초기화 (alpha 0 검정).
		D3DLOCKED_RECT lr_{};
		if (SUCCEEDED(m_pTexture->LockRect(0, &lr_, nullptr, 0)))
		{
			uint8_t* const pDst_ = static_cast<uint8_t*>(lr_.pBits);
			for (uint32_t y = 0; y < m_uAtlasSize; ++y)
			{
				ZeroMemory(pDst_ + y * lr_.Pitch, m_uAtlasSize * 4);
			}
			m_pTexture->UnlockRect(0);
		}

		const UINT uVBSize_ = sizeof(_DX9_FONT_DW_VERTEX) * 4u * m_uMaxQuads;
		hr = m_pDevice->CreateVertexBuffer(
			uVBSize_,
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
			DX9_FONT_DW_FVF, D3DPOOL_DEFAULT,
			&m_pVB, nullptr);
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] CreateVertexBuffer 실패: 0x%08X", hr);
			return false;
		}

		const UINT uIBSize_ = sizeof(uint16_t) * 6u * m_uMaxQuads;
		hr = m_pDevice->CreateIndexBuffer(
			uIBSize_, D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16, D3DPOOL_MANAGED,
			&m_pIB, nullptr);
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] CreateIndexBuffer 실패: 0x%08X", hr);
			return false;
		}

		void* pIdx_ = nullptr;
		hr = m_pIB->Lock(0, 0, &pIdx_, 0);
		if (FAILED(hr)) return false;
		uint16_t* const pI_ = static_cast<uint16_t*>(pIdx_);
		for (uint32_t i = 0; i < m_uMaxQuads; ++i)
		{
			const uint16_t uBase_ = static_cast<uint16_t>(i * 4u);
			pI_[i * 6 + 0] = uBase_ + 0;
			pI_[i * 6 + 1] = uBase_ + 1;
			pI_[i * 6 + 2] = uBase_ + 2;
			pI_[i * 6 + 3] = uBase_ + 0;
			pI_[i * 6 + 4] = uBase_ + 2;
			pI_[i * 6 + 5] = uBase_ + 3;
		}
		m_pIB->Unlock();
		return true;
	}


	void C_DX9_FONT_DW::releaseBuffers_()
	{
		if (m_pVB) { m_pVB->Release(); m_pVB = nullptr; }
		if (m_pIB) { m_pIB->Release(); m_pIB = nullptr; }
		if (m_pTexture) { m_pTexture->Release(); m_pTexture = nullptr; }
	}


	//============================================================================
	// 초기화
	//============================================================================
	bool C_DX9_FONT_DW::Initialize(LPDIRECT3DDEVICE9 _pDevice,
		const wchar_t* _pFaceName,
		uint32_t _uFontHeight,
		bool _bBold,
		uint32_t _uCellSize,
		uint32_t _uAtlasSize,
		uint32_t _uMaxQuads)
	{
		if (!_pDevice || !_pFaceName || _uFontHeight == 0 || _uMaxQuads == 0)
			return false;
		if (_uCellSize == 0 || _uAtlasSize == 0) return false;
		if (_uCellSize > _uAtlasSize || (_uAtlasSize % _uCellSize) != 0) return false;

		m_pDevice = _pDevice;
		m_uAtlasSize = _uAtlasSize;
		m_uCellSize = _uCellSize;
		m_uCellsX = m_uAtlasSize / m_uCellSize;
		m_uCellsY = m_uAtlasSize / m_uCellSize;
		m_uCellCount = m_uCellsX * m_uCellsY;
		m_uFontHeight = _uFontHeight;
		m_uMaxQuads = _uMaxQuads;

		// GDI DC + DIBSection.
		m_hDC = ::CreateCompatibleDC(nullptr);
		if (!m_hDC) { DBGPRINT(L"[DX9FontDW] CreateCompatibleDC 실패"); return false; }

		BITMAPINFO bi{};
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = static_cast<LONG>(m_uCellSize);
		bi.bmiHeader.biHeight = -static_cast<LONG>(m_uCellSize);
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biCompression = BI_RGB;

		void* pBits_ = nullptr;
		m_hBitmap = ::CreateDIBSection(m_hDC, &bi, DIB_RGB_COLORS, &pBits_, nullptr, 0);
		if (!m_hBitmap)
		{
			DBGPRINT(L"[DX9FontDW] CreateDIBSection 실패");
			::DeleteDC(m_hDC); m_hDC = nullptr;
			return false;
		}
		m_pBits = static_cast<uint32_t*>(pBits_);
		::SelectObject(m_hDC, m_hBitmap);

		m_hFont = ::CreateFontW(
			-static_cast<int>(m_uFontHeight), 0, 0, 0,
			_bBold ? FW_BOLD : FW_NORMAL,
			FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			_pFaceName);
		if (!m_hFont)
		{
			DBGPRINT(L"[DX9FontDW] CreateFontW 실패: face=%ls height=%u",
				_pFaceName, _uFontHeight);
			::DeleteObject(m_hBitmap); m_hBitmap = nullptr;
			::DeleteDC(m_hDC); m_hDC = nullptr;
			return false;
		}
		::SelectObject(m_hDC, m_hFont);

		::SetTextColor(m_hDC, RGB(255, 255, 255));
		::SetBkColor(m_hDC, RGB(0, 0, 0));
		::SetBkMode(m_hDC, OPAQUE);

		if (!createBuffers_())
		{
			releaseBuffers_();
			::DeleteObject(m_hFont); m_hFont = nullptr;
			::DeleteObject(m_hBitmap); m_hBitmap = nullptr;
			::DeleteDC(m_hDC); m_hDC = nullptr;
			m_pDevice = nullptr;
			return false;
		}

		m_uNextCell = 0;
		m_uQuadCount = 0;
		m_uVBOffset = 0;
		m_mapGlyphs.clear();
		m_mapGlyphs.reserve(m_uCellCount);

		DBGPRINT(L"[DX9FontDW] 초기화: %ux%u atlas, %ux%u cell, %u slots, %ls %ups %s, maxQuads=%u",
			m_uAtlasSize, m_uAtlasSize, m_uCellSize, m_uCellSize, m_uCellCount,
			_pFaceName, _uFontHeight, _bBold ? L"Bold" : L"Regular", m_uMaxQuads);
		return true;
	}


	void C_DX9_FONT_DW::Shutdown()
	{
		releaseBuffers_();
		m_mapGlyphs.clear();
		if (m_hFont)    { ::DeleteObject(m_hFont);    m_hFont = nullptr; }
		if (m_hBitmap)  { ::DeleteObject(m_hBitmap);  m_hBitmap = nullptr; }
		if (m_hDC)      { ::DeleteDC(m_hDC);          m_hDC = nullptr; }
		m_pBits = nullptr;
		m_pDevice = nullptr;
		m_uNextCell = 0;
		m_bRendering = false;
	}


	//============================================================================
	// 디바이스 lost/reset (DEFAULT 풀 VB 재생성)
	//============================================================================
	void C_DX9_FONT_DW::OnLostDevice()
	{
		if (m_pVB) { m_pVB->Release(); m_pVB = nullptr; }
	}


	bool C_DX9_FONT_DW::OnResetDevice()
	{
		if (!m_pDevice || m_pVB) return true;

		const UINT uVBSize_ = sizeof(_DX9_FONT_DW_VERTEX) * 4u * m_uMaxQuads;
		const HRESULT hr = m_pDevice->CreateVertexBuffer(
			uVBSize_,
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
			DX9_FONT_DW_FVF, D3DPOOL_DEFAULT,
			&m_pVB, nullptr);
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] OnResetDevice CreateVertexBuffer 실패: 0x%08X", hr);
			return false;
		}
		m_uVBOffset = 0;
		return true;
	}


	//============================================================================
	// 글리프 추출 (캐시 미스 시 GDI 렌더 + atlas 부분 업로드)
	//============================================================================
	const _DX9_FONT_DW_GLYPH* C_DX9_FONT_DW::getGlyph_(wchar_t _wChar)
	{
		const auto it = m_mapGlyphs.find(_wChar);
		if (it != m_mapGlyphs.end()) return &it->second;
		if (!m_hFont || !m_pTexture) return nullptr;
		if (m_uNextCell >= m_uCellCount) return nullptr;

		// GDI 렌더 — OPAQUE + 흰색 글자.
		const RECT rcCell_ = { 0, 0,
			static_cast<LONG>(m_uCellSize), static_cast<LONG>(m_uCellSize) };
		::ExtTextOutW(m_hDC, 1, 1, ETO_OPAQUE | ETO_CLIPPED,
			&rcCell_, &_wChar, 1, nullptr);

		SIZE sz_{};
		::GetTextExtentPoint32W(m_hDC, &_wChar, 1, &sz_);

		// BGRA (DIBSection) → A8R8G8B8 (D3D9): Red → Alpha 변환.
		const uint32_t uPixels_ = m_uCellSize * m_uCellSize;
		std::vector<uint32_t> vCell_(uPixels_);
		for (uint32_t i = 0; i < uPixels_; ++i)
		{
			const uint32_t uGDI_ = m_pBits[i];
			const uint8_t uRed_ = static_cast<uint8_t>((uGDI_ >> 16) & 0xFFu);
			vCell_[i] = (static_cast<uint32_t>(uRed_) << 24) | 0x00FFFFFFu;
		}

		// 셀 위치.
		const uint32_t uCellX_ = m_uNextCell % m_uCellsX;
		const uint32_t uCellY_ = m_uNextCell / m_uCellsX;

		// atlas 부분 업로드 (LockRect 영역 지정).
		RECT rcLock_ = {
			static_cast<LONG>(uCellX_ * m_uCellSize),
			static_cast<LONG>(uCellY_ * m_uCellSize),
			static_cast<LONG>((uCellX_ + 1u) * m_uCellSize),
			static_cast<LONG>((uCellY_ + 1u) * m_uCellSize)
		};
		D3DLOCKED_RECT lr_{};
		HRESULT hr = m_pTexture->LockRect(0, &lr_, &rcLock_, 0);
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] atlas LockRect 실패: 0x%08X", hr);
			return nullptr;
		}
		for (uint32_t y = 0; y < m_uCellSize; ++y)
		{
			std::memcpy(
				static_cast<uint8_t*>(lr_.pBits) + y * lr_.Pitch,
				vCell_.data() + y * m_uCellSize,
				m_uCellSize * 4);
		}
		m_pTexture->UnlockRect(0);

		_DX9_FONT_DW_GLYPH glyph_{};
		glyph_.m_uCellIndex = static_cast<uint16_t>(m_uNextCell);
		glyph_.m_uWidth = static_cast<uint8_t>(sz_.cx > 255 ? 255 : sz_.cx);
		glyph_.m_uHeight = static_cast<uint8_t>(sz_.cy > 255 ? 255 : sz_.cy);

		const auto r_ = m_mapGlyphs.emplace(_wChar, glyph_);
		++m_uNextCell;
		return &r_.first->second;
	}


	//============================================================================
	// 상태 저장/복원 (BeginRender/EndRender 격리)
	//============================================================================
	void C_DX9_FONT_DW::saveState_()
	{
		m_pDevice->GetFVF(&m_Backup.dwFVF);
		m_pDevice->GetVertexShader(&m_Backup.pVS);
		m_pDevice->GetPixelShader(&m_Backup.pPS);
		m_pDevice->GetTexture(0, &m_Backup.pTex0);
		m_pDevice->GetIndices(&m_Backup.pIB);
		m_pDevice->GetStreamSource(0, &m_Backup.pVB, &m_Backup.uVBOffset, &m_Backup.uVBStride);

		m_pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &m_Backup.aRS[0]);
		m_pDevice->GetRenderState(D3DRS_SRCBLEND,         &m_Backup.aRS[1]);
		m_pDevice->GetRenderState(D3DRS_DESTBLEND,        &m_Backup.aRS[2]);
		m_pDevice->GetRenderState(D3DRS_ALPHATESTENABLE,  &m_Backup.aRS[3]);
		m_pDevice->GetRenderState(D3DRS_ZENABLE,          &m_Backup.aRS[4]);
		m_pDevice->GetRenderState(D3DRS_ZWRITEENABLE,     &m_Backup.aRS[5]);
		m_pDevice->GetRenderState(D3DRS_LIGHTING,         &m_Backup.aRS[6]);
		m_pDevice->GetRenderState(D3DRS_CULLMODE,         &m_Backup.aRS[7]);
		m_pDevice->GetRenderState(D3DRS_FOGENABLE,        &m_Backup.aRS[8]);
		m_pDevice->GetRenderState(D3DRS_STENCILENABLE,    &m_Backup.aRS[9]);
		m_pDevice->GetRenderState(D3DRS_SCISSORTESTENABLE,&m_Backup.aRS[10]);
		m_pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &m_Backup.aRS[11]);

		m_pDevice->GetTextureStageState(0, D3DTSS_COLOROP,   &m_Backup.aTSS[0]);
		m_pDevice->GetTextureStageState(0, D3DTSS_COLORARG1, &m_Backup.aTSS[1]);
		m_pDevice->GetTextureStageState(0, D3DTSS_COLORARG2, &m_Backup.aTSS[2]);
		m_pDevice->GetTextureStageState(0, D3DTSS_ALPHAOP,   &m_Backup.aTSS[3]);
		m_pDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &m_Backup.aTSS[4]);
		m_pDevice->GetTextureStageState(0, D3DTSS_ALPHAARG2, &m_Backup.aTSS[5]);

		m_pDevice->GetSamplerState(0, D3DSAMP_MINFILTER, &m_Backup.aSamp[0]);
		m_pDevice->GetSamplerState(0, D3DSAMP_MAGFILTER, &m_Backup.aSamp[1]);
		m_pDevice->GetSamplerState(0, D3DSAMP_MIPFILTER, &m_Backup.aSamp[2]);
		m_pDevice->GetSamplerState(0, D3DSAMP_ADDRESSU,  &m_Backup.aSamp[3]);
		m_pDevice->GetSamplerState(0, D3DSAMP_ADDRESSV,  &m_Backup.aSamp[4]);
	}


	void C_DX9_FONT_DW::restoreState_()
	{
		m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,  m_Backup.aRS[0]);
		m_pDevice->SetRenderState(D3DRS_SRCBLEND,          m_Backup.aRS[1]);
		m_pDevice->SetRenderState(D3DRS_DESTBLEND,         m_Backup.aRS[2]);
		m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,   m_Backup.aRS[3]);
		m_pDevice->SetRenderState(D3DRS_ZENABLE,           m_Backup.aRS[4]);
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE,      m_Backup.aRS[5]);
		m_pDevice->SetRenderState(D3DRS_LIGHTING,          m_Backup.aRS[6]);
		m_pDevice->SetRenderState(D3DRS_CULLMODE,          m_Backup.aRS[7]);
		m_pDevice->SetRenderState(D3DRS_FOGENABLE,         m_Backup.aRS[8]);
		m_pDevice->SetRenderState(D3DRS_STENCILENABLE,     m_Backup.aRS[9]);
		m_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, m_Backup.aRS[10]);
		m_pDevice->SetRenderState(D3DRS_COLORWRITEENABLE,  m_Backup.aRS[11]);

		m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP,   m_Backup.aTSS[0]);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, m_Backup.aTSS[1]);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, m_Backup.aTSS[2]);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   m_Backup.aTSS[3]);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, m_Backup.aTSS[4]);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, m_Backup.aTSS[5]);

		m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, m_Backup.aSamp[0]);
		m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, m_Backup.aSamp[1]);
		m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, m_Backup.aSamp[2]);
		m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU,  m_Backup.aSamp[3]);
		m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV,  m_Backup.aSamp[4]);

		m_pDevice->SetVertexShader(m_Backup.pVS);
		m_pDevice->SetPixelShader(m_Backup.pPS);
		m_pDevice->SetFVF(m_Backup.dwFVF);
		m_pDevice->SetTexture(0, m_Backup.pTex0);
		m_pDevice->SetIndices(m_Backup.pIB);
		m_pDevice->SetStreamSource(0, m_Backup.pVB, m_Backup.uVBOffset, m_Backup.uVBStride);

		if (m_Backup.pVS) { m_Backup.pVS->Release(); m_Backup.pVS = nullptr; }
		if (m_Backup.pPS) { m_Backup.pPS->Release(); m_Backup.pPS = nullptr; }
		if (m_Backup.pTex0) { m_Backup.pTex0->Release(); m_Backup.pTex0 = nullptr; }
		if (m_Backup.pIB)  { m_Backup.pIB->Release();  m_Backup.pIB = nullptr; }
		if (m_Backup.pVB)  { m_Backup.pVB->Release();  m_Backup.pVB = nullptr; }
	}


	void C_DX9_FONT_DW::setupState_()
	{
		m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		m_pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFu);

		m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		m_pDevice->SetVertexShader(nullptr);
		m_pDevice->SetPixelShader(nullptr);
		m_pDevice->SetFVF(DX9_FONT_DW_FVF);
		m_pDevice->SetTexture(0, m_pTexture);
		m_pDevice->SetIndices(m_pIB);
		m_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(_DX9_FONT_DW_VERTEX));
	}


	//============================================================================
	// BeginRender / EndRender
	//============================================================================
	bool C_DX9_FONT_DW::BeginRender()
	{
		if (m_bRendering || !IsInitialized() || !m_pVB) return false;
		saveState_();
		setupState_();
		m_uQuadCount = 0;
		m_uVBOffset = 0;
		m_bRendering = true;
		return true;
	}


	void C_DX9_FONT_DW::EndRender()
	{
		if (!m_bRendering) return;
		flushBatch_();
		restoreState_();
		m_bRendering = false;
	}


	void C_DX9_FONT_DW::appendQuad_(float _fX, float _fY, float _fW, float _fH,
		float _fU0, float _fV0, float _fU1, float _fV1, D3DCOLOR _dwColor)
	{
		if (!m_bRendering || !m_pVB) return;
		if (m_uQuadCount >= m_uMaxQuads) flushBatch_();

		const float fX0_ = _fX - 0.5f;
		const float fY0_ = _fY - 0.5f;
		const float fX1_ = _fX + _fW - 0.5f;
		const float fY1_ = _fY + _fH - 0.5f;

		_DX9_FONT_DW_VERTEX v_[4];
		v_[0] = { fX0_, fY0_, 0.0f, 1.0f, _dwColor, _fU0, _fV0 };
		v_[1] = { fX1_, fY0_, 0.0f, 1.0f, _dwColor, _fU1, _fV0 };
		v_[2] = { fX1_, fY1_, 0.0f, 1.0f, _dwColor, _fU1, _fV1 };
		v_[3] = { fX0_, fY1_, 0.0f, 1.0f, _dwColor, _fU0, _fV1 };

		const UINT uOffsetBytes_ = m_uVBOffset * sizeof(_DX9_FONT_DW_VERTEX);
		const UINT uSizeBytes_ = sizeof(_DX9_FONT_DW_VERTEX) * 4u;
		const DWORD dwLockFlag_ = (m_uVBOffset == 0) ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE;

		void* pData_ = nullptr;
		if (SUCCEEDED(m_pVB->Lock(uOffsetBytes_, uSizeBytes_, &pData_, dwLockFlag_)))
		{
			std::memcpy(pData_, v_, uSizeBytes_);
			m_pVB->Unlock();
		}

		m_uVBOffset += 4;
		++m_uQuadCount;
	}


	void C_DX9_FONT_DW::flushBatch_()
	{
		if (m_uQuadCount == 0) return;
		m_pDevice->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST, 0, 0,
			m_uQuadCount * 4u, 0, m_uQuadCount * 2u);
		m_uQuadCount = 0;
		m_uVBOffset = 0;
	}


	//============================================================================
	// 텍스트 렌더링
	//============================================================================
	void C_DX9_FONT_DW::RenderText(float _fX, float _fY, const wchar_t* _pText,
		D3DCOLOR _dwColor, float _fScale)
	{
		if (!_pText || !m_bRendering || !IsInitialized()) return;

		const float fTexInv_ = 1.0f / static_cast<float>(m_uAtlasSize);
		const float fQuadSize_ = static_cast<float>(m_uCellSize) * _fScale;
		const float fSpaceAdvance_ = static_cast<float>(m_uFontHeight) * 0.4f * _fScale;
		const float fFallbackAdvance_ = static_cast<float>(m_uFontHeight) * 0.5f * _fScale;

		float fCurX_ = _fX;

		for (const wchar_t* p = _pText; *p != L'\0'; ++p)
		{
			const wchar_t wc_ = *p;
			if (wc_ == L'\n' || wc_ == L'\r') continue;
			if (wc_ == L' ') { fCurX_ += fSpaceAdvance_; continue; }

			const _DX9_FONT_DW_GLYPH* const pG_ = getGlyph_(wc_);
			if (!pG_) { fCurX_ += fFallbackAdvance_; continue; }

			const uint32_t uCellX_ = pG_->m_uCellIndex % m_uCellsX;
			const uint32_t uCellY_ = pG_->m_uCellIndex / m_uCellsX;
			const float fU0_ = static_cast<float>(uCellX_ * m_uCellSize) * fTexInv_;
			const float fV0_ = static_cast<float>(uCellY_ * m_uCellSize) * fTexInv_;
			const float fU1_ = fU0_ + static_cast<float>(m_uCellSize) * fTexInv_;
			const float fV1_ = fV0_ + static_cast<float>(m_uCellSize) * fTexInv_;

			appendQuad_(fCurX_, _fY, fQuadSize_, fQuadSize_, fU0_, fV0_, fU1_, fV1_, _dwColor);

			fCurX_ += static_cast<float>(pG_->m_uWidth) * _fScale;
		}
	}


	void C_DX9_FONT_DW::RenderTextFmt(float _fX, float _fY, D3DCOLOR _dwColor, float _fScale,
		const wchar_t* _pFmt, ...)
	{
		if (!_pFmt) return;
		wchar_t szBuf_[512];
		va_list ap_;
		va_start(ap_, _pFmt);
		::vswprintf_s(szBuf_, _pFmt, ap_);
		va_end(ap_);
		RenderText(_fX, _fY, szBuf_, _dwColor, _fScale);
	}


	float C_DX9_FONT_DW::MeasureText(const wchar_t* _pText, float _fScale)
	{
		if (!_pText || !IsInitialized()) return 0.0f;

		const float fSpaceAdvance_ = static_cast<float>(m_uFontHeight) * 0.4f * _fScale;
		const float fFallbackAdvance_ = static_cast<float>(m_uFontHeight) * 0.5f * _fScale;

		float fW_ = 0.0f;
		for (const wchar_t* p = _pText; *p != L'\0'; ++p)
		{
			const wchar_t wc_ = *p;
			if (wc_ == L'\n' || wc_ == L'\r') continue;
			if (wc_ == L' ') { fW_ += fSpaceAdvance_; continue; }
			const _DX9_FONT_DW_GLYPH* const pG_ = getGlyph_(wc_);
			if (!pG_) { fW_ += fFallbackAdvance_; continue; }
			fW_ += static_cast<float>(pG_->m_uWidth) * _fScale;
		}
		return fW_;
	}

} // namespace dx9
