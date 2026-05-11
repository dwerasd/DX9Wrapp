// DX9Font_DW.cpp: DirectWrite 기반 DX9 폰트 시스템 구현.
#include "framework.h"
#include "DX9Font_DW.h"

#include <cstdarg>
#include <cstdio>
#include <vector>


namespace dx9
{
	//============================================================================
	// 정적 멤버 (DirectWrite 공유 자원).
	//============================================================================
	Microsoft::WRL::ComPtr<IDWriteFactory3>          C_DX9_FONT_DW::s_pFactory;
	Microsoft::WRL::ComPtr<IDWriteFontSetBuilder1>   C_DX9_FONT_DW::s_pSetBuilder;
	Microsoft::WRL::ComPtr<IDWriteFontCollection1>   C_DX9_FONT_DW::s_pCustomCollection;
	bool                                              C_DX9_FONT_DW::s_bCustomDirty = false;


	bool C_DX9_FONT_DW::ensureFactory_()
	{
		if (s_pFactory) return true;

		IDWriteFactory* pBase_ = nullptr;
		HRESULT hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pBase_));
		if (FAILED(hr) || !pBase_)
		{
			DBGPRINT(L"[DX9FontDW] DWriteCreateFactory 실패: 0x%08X", hr);
			return false;
		}

		hr = pBase_->QueryInterface(IID_PPV_ARGS(s_pFactory.GetAddressOf()));
		pBase_->Release();
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] IDWriteFactory3 미지원: 0x%08X (Win10+ 필요)", hr);
			return false;
		}

		Microsoft::WRL::ComPtr<IDWriteFontSetBuilder> pBaseBuilder_;
		hr = s_pFactory->CreateFontSetBuilder(pBaseBuilder_.GetAddressOf());
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] CreateFontSetBuilder 실패: 0x%08X", hr);
			s_pFactory.Reset();
			return false;
		}
		hr = pBaseBuilder_.As(&s_pSetBuilder);
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] IDWriteFontSetBuilder1 미지원: 0x%08X (Win10 1709+)", hr);
			s_pFactory.Reset();
			return false;
		}
		return true;
	}


	bool C_DX9_FONT_DW::ensureCustomCollection_()
	{
		if (!s_bCustomDirty && s_pCustomCollection) return true;
		if (!s_pFactory || !s_pSetBuilder) return false;

		Microsoft::WRL::ComPtr<IDWriteFontSet> pSet_;
		HRESULT hr = s_pSetBuilder->CreateFontSet(pSet_.GetAddressOf());
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] CreateFontSet 실패: 0x%08X", hr);
			return false;
		}

		s_pCustomCollection.Reset();
		hr = s_pFactory->CreateFontCollectionFromFontSet(pSet_.Get(),
			s_pCustomCollection.GetAddressOf());
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] CreateFontCollectionFromFontSet 실패: 0x%08X", hr);
			return false;
		}

		s_bCustomDirty = false;
		return true;
	}


	bool C_DX9_FONT_DW::RegisterFontFile(const wchar_t* _pPath)
	{
		if (!_pPath || !*_pPath) return false;
		if (!ensureFactory_()) return false;

		Microsoft::WRL::ComPtr<IDWriteFontFile> pFile_;
		HRESULT hr = s_pFactory->CreateFontFileReference(_pPath, nullptr,
			pFile_.GetAddressOf());
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] CreateFontFileReference 실패: %ls (0x%08X)", _pPath, hr);
			return false;
		}

		BOOL bSupported_ = FALSE;
		DWRITE_FONT_FILE_TYPE eFileType_ = DWRITE_FONT_FILE_TYPE_UNKNOWN;
		DWRITE_FONT_FACE_TYPE eFaceType_ = DWRITE_FONT_FACE_TYPE_UNKNOWN;
		UINT32 uFaceCount_ = 0;
		hr = pFile_->Analyze(&bSupported_, &eFileType_, &eFaceType_, &uFaceCount_);
		if (FAILED(hr) || !bSupported_ || uFaceCount_ == 0)
		{
			DBGPRINT(L"[DX9FontDW] 폰트 파일 미지원: %ls", _pPath);
			return false;
		}

		hr = s_pSetBuilder->AddFontFile(pFile_.Get());
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] AddFontFile 실패: %ls (0x%08X)", _pPath, hr);
			return false;
		}

		s_bCustomDirty = true;
		DBGPRINT(L"[DX9FontDW] 폰트 등록: %ls (faces=%u)", _pPath, uFaceCount_);
		return true;
	}


	void C_DX9_FONT_DW::CleanupStatic()
	{
		s_pCustomCollection.Reset();
		s_pSetBuilder.Reset();
		s_pFactory.Reset();
		s_bCustomDirty = false;
	}


	//============================================================================
	// 생성/소멸
	//============================================================================
	C_DX9_FONT_DW::C_DX9_FONT_DW()
		: m_fEmSize(0.0f)
		, m_fAscent(0.0f)
		, m_fDescent(0.0f)
		, m_fLineGap(0.0f)
		, m_uDesignUnitsPerEm(0)
		, m_pDevice(nullptr)
		, m_pTexture(nullptr)
		, m_uAtlasSize(0)
		, m_uShelfX(0), m_uShelfY(0), m_uShelfH(0)
		, m_bAtlasFull(false)
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
	// 페이스 검색 (custom → system)
	//============================================================================
	static bool findFontFace_DW_(IDWriteFactory3* _pFactory,
		IDWriteFontCollection1* _pCustom,
		const wchar_t* _pFaceName,
		bool _bBold, bool _bItalic,
		IDWriteFontFace** _ppOutFace)
	{
		const DWRITE_FONT_WEIGHT eWeight_ = _bBold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
		const DWRITE_FONT_STYLE eStyle_ = _bItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
		const DWRITE_FONT_STRETCH eStretch_ = DWRITE_FONT_STRETCH_NORMAL;

		auto tryCollection_ = [&](IDWriteFontCollection* _pColl) -> bool
		{
			if (!_pColl) return false;
			UINT32 uIndex_ = 0;
			BOOL bExists_ = FALSE;
			HRESULT hr = _pColl->FindFamilyName(_pFaceName, &uIndex_, &bExists_);
			if (FAILED(hr) || !bExists_) return false;

			Microsoft::WRL::ComPtr<IDWriteFontFamily> pFamily_;
			hr = _pColl->GetFontFamily(uIndex_, pFamily_.GetAddressOf());
			if (FAILED(hr)) return false;

			Microsoft::WRL::ComPtr<IDWriteFont> pFont_;
			hr = pFamily_->GetFirstMatchingFont(eWeight_, eStretch_, eStyle_, pFont_.GetAddressOf());
			if (FAILED(hr)) return false;

			hr = pFont_->CreateFontFace(_ppOutFace);
			return SUCCEEDED(hr);
		};

		if (tryCollection_(_pCustom)) return true;

		Microsoft::WRL::ComPtr<IDWriteFontCollection> pSys_;
		if (FAILED(_pFactory->GetSystemFontCollection(pSys_.GetAddressOf(), FALSE)))
			return false;
		return tryCollection_(pSys_.Get());
	}


	//============================================================================
	// 버퍼 생성/해제 (atlas + VB + IB).
	//============================================================================
	bool C_DX9_FONT_DW::createBuffers_()
	{
		if (!m_pDevice) return false;

		// Atlas 텍스처 (MANAGED — 디바이스 lost/reset 자동 보존).
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

		// Atlas 초기화 (검정 투명 0x00000000).
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

		// Vertex buffer (DYNAMIC | WRITEONLY, DEFAULT 풀).
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

		// Index buffer (정적 — 0,1,2 / 0,2,3 패턴 반복, MANAGED).
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

		// 인덱스 채우기 (한 번만).
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
		bool _bItalic,
		uint32_t _uAtlasSize,
		uint32_t _uMaxQuads)
	{
		if (!_pDevice || !_pFaceName || _uFontHeight == 0 || _uAtlasSize == 0 || _uMaxQuads == 0)
			return false;

		if (!ensureFactory_()) return false;
		ensureCustomCollection_();

		if (!findFontFace_DW_(s_pFactory.Get(), s_pCustomCollection.Get(),
			_pFaceName, _bBold, _bItalic, m_pFace.GetAddressOf()))
		{
			DBGPRINT(L"[DX9FontDW] 폰트 페이스 찾을 수 없음: %ls (bold=%d italic=%d)",
				_pFaceName, _bBold ? 1 : 0, _bItalic ? 1 : 0);
			return false;
		}

		// 메트릭.
		DWRITE_FONT_METRICS fm_{};
		m_pFace->GetMetrics(&fm_);
		m_uDesignUnitsPerEm = fm_.designUnitsPerEm;
		m_fEmSize = static_cast<float>(_uFontHeight);
		const float fUnitScale_ = m_fEmSize / static_cast<float>(fm_.designUnitsPerEm);
		m_fAscent = static_cast<float>(fm_.ascent) * fUnitScale_;
		m_fDescent = static_cast<float>(fm_.descent) * fUnitScale_;
		m_fLineGap = static_cast<float>(fm_.lineGap) * fUnitScale_;

		m_pDevice = _pDevice;
		m_uAtlasSize = _uAtlasSize;
		m_uMaxQuads = _uMaxQuads;

		if (!createBuffers_())
		{
			releaseBuffers_();
			m_pFace.Reset();
			m_pDevice = nullptr;
			return false;
		}

		m_uShelfX = 1;
		m_uShelfY = 1;
		m_uShelfH = 0;
		m_bAtlasFull = false;
		m_uQuadCount = 0;
		m_uVBOffset = 0;
		m_mapGlyphs.clear();
		m_mapGlyphs.reserve(512);

		DBGPRINT(L"[DX9FontDW] 초기화: %ls %ups %s%s atlas=%u maxQuads=%u ascent=%.1f",
			_pFaceName, _uFontHeight,
			_bBold ? L"Bold " : L"",
			_bItalic ? L"Italic" : L"Regular",
			m_uAtlasSize, m_uMaxQuads, m_fAscent);
		return true;
	}


	void C_DX9_FONT_DW::Shutdown()
	{
		releaseBuffers_();
		m_mapGlyphs.clear();
		m_pFace.Reset();
		m_pDevice = nullptr;
		m_uShelfX = m_uShelfY = m_uShelfH = 0;
		m_bAtlasFull = false;
		m_bRendering = false;
	}


	//============================================================================
	// 디바이스 lost/reset (DEFAULT 풀 VB 재생성).
	//============================================================================
	void C_DX9_FONT_DW::OnLostDevice()
	{
		if (m_pVB) { m_pVB->Release(); m_pVB = nullptr; }
		// atlas (MANAGED) 와 IB (MANAGED) 는 보존.
	}


	bool C_DX9_FONT_DW::OnResetDevice()
	{
		if (!m_pDevice || m_pVB) return true;	// 이미 살아있거나 미초기화.

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
	// 글리프 추출 (캐시 미스 시 DirectWrite + atlas LockRect 부분 업로드).
	//============================================================================
	const _DX9_FONT_DW_GLYPH* C_DX9_FONT_DW::getGlyph_(wchar_t _wChar)
	{
		const auto it = m_mapGlyphs.find(_wChar);
		if (it != m_mapGlyphs.end())
			return &it->second;
		if (!m_pFace || !m_pTexture) return nullptr;

		const UINT32 uCode_ = static_cast<UINT32>(_wChar);
		UINT16 uGlyphIdx_ = 0;
		HRESULT hr = m_pFace->GetGlyphIndicesW(&uCode_, 1, &uGlyphIdx_);
		if (FAILED(hr)) return nullptr;

		const float fUnitScale_ = m_fEmSize / static_cast<float>(m_uDesignUnitsPerEm);

		DWRITE_GLYPH_METRICS gm_{};
		hr = m_pFace->GetDesignGlyphMetrics(&uGlyphIdx_, 1, &gm_, FALSE);
		if (FAILED(hr)) return nullptr;
		const float fAdvance_ = static_cast<float>(gm_.advanceWidth) * fUnitScale_;

		FLOAT fGlyphAdvance_ = 0.0f;
		DWRITE_GLYPH_OFFSET offset_{};
		DWRITE_GLYPH_RUN run_{};
		run_.fontFace = m_pFace.Get();
		run_.fontEmSize = m_fEmSize;
		run_.glyphCount = 1;
		run_.glyphIndices = &uGlyphIdx_;
		run_.glyphAdvances = &fGlyphAdvance_;
		run_.glyphOffsets = &offset_;
		run_.isSideways = FALSE;
		run_.bidiLevel = 0;

		Microsoft::WRL::ComPtr<IDWriteGlyphRunAnalysis> pAnalysis_;
		hr = s_pFactory->CreateGlyphRunAnalysis(&run_, 1.0f, nullptr,
			DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC,
			DWRITE_MEASURING_MODE_NATURAL,
			0.0f, 0.0f,
			pAnalysis_.GetAddressOf());
		if (FAILED(hr) || !pAnalysis_) return nullptr;

		RECT bounds_{};
		hr = pAnalysis_->GetAlphaTextureBounds(DWRITE_TEXTURE_ALIASED_1x1, &bounds_);
		if (FAILED(hr)) return nullptr;

		const int32_t iBmpW_ = bounds_.right - bounds_.left;
		const int32_t iBmpH_ = bounds_.bottom - bounds_.top;

		_DX9_FONT_DW_GLYPH glyph_{};
		glyph_.m_fAdvance = fAdvance_;

		if (iBmpW_ <= 0 || iBmpH_ <= 0)
		{
			const auto r_ = m_mapGlyphs.emplace(_wChar, glyph_);
			return &r_.first->second;
		}

		// Shelf packing.
		if (m_uShelfX + static_cast<uint32_t>(iBmpW_) + 1u > m_uAtlasSize)
		{
			m_uShelfY += m_uShelfH + 1u;
			m_uShelfX = 1;
			m_uShelfH = 0;
		}
		if (m_uShelfY + static_cast<uint32_t>(iBmpH_) + 1u > m_uAtlasSize)
		{
			m_bAtlasFull = true;
			const auto r_ = m_mapGlyphs.emplace(_wChar, glyph_);
			return &r_.first->second;
		}

		const uint32_t uAtlasX_ = m_uShelfX;
		const uint32_t uAtlasY_ = m_uShelfY;

		// 알파 비트맵 추출.
		std::vector<uint8_t> vAlpha_(static_cast<size_t>(iBmpW_) * static_cast<size_t>(iBmpH_));
		hr = pAnalysis_->CreateAlphaTexture(DWRITE_TEXTURE_ALIASED_1x1, &bounds_,
			vAlpha_.data(), static_cast<UINT32>(vAlpha_.size()));
		if (FAILED(hr)) return nullptr;

		// D3D9 A8R8G8B8 (little-endian 메모리 순서 = BGRA): RGB=흰색, A=알파.
		// 단일 픽셀 = 0xAAFFFFFF (alpha-premultiplied 아님, modulate 와 blend 로 처리).
		RECT rcLock_ = { static_cast<LONG>(uAtlasX_), static_cast<LONG>(uAtlasY_),
			static_cast<LONG>(uAtlasX_ + iBmpW_), static_cast<LONG>(uAtlasY_ + iBmpH_) };
		D3DLOCKED_RECT lr_{};
		hr = m_pTexture->LockRect(0, &lr_, &rcLock_, 0);
		if (FAILED(hr))
		{
			DBGPRINT(L"[DX9FontDW] atlas LockRect 실패: 0x%08X", hr);
			return nullptr;
		}
		for (int32_t y = 0; y < iBmpH_; ++y)
		{
			uint32_t* const pRow_ = reinterpret_cast<uint32_t*>(
				static_cast<uint8_t*>(lr_.pBits) + y * lr_.Pitch);
			const uint8_t* const pSrc_ = vAlpha_.data() + y * iBmpW_;
			for (int32_t x = 0; x < iBmpW_; ++x)
			{
				pRow_[x] = (static_cast<uint32_t>(pSrc_[x]) << 24) | 0x00FFFFFFu;
			}
		}
		m_pTexture->UnlockRect(0);

		// Shelf advance.
		m_uShelfX += static_cast<uint32_t>(iBmpW_) + 1u;
		if (static_cast<uint32_t>(iBmpH_) > m_uShelfH)
			m_uShelfH = static_cast<uint32_t>(iBmpH_);

		glyph_.m_sOffsetX = static_cast<int16_t>(bounds_.left);
		glyph_.m_sOffsetY = static_cast<int16_t>(bounds_.top);
		glyph_.m_uBmpWidth = static_cast<uint16_t>(iBmpW_);
		glyph_.m_uBmpHeight = static_cast<uint16_t>(iBmpH_);
		glyph_.m_uAtlasX = static_cast<uint16_t>(uAtlasX_);
		glyph_.m_uAtlasY = static_cast<uint16_t>(uAtlasY_);

		const auto r_ = m_mapGlyphs.emplace(_wChar, glyph_);
		return &r_.first->second;
	}


	//============================================================================
	// 상태 저장/복원 (BeginRender/EndRender).
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

		// GetXxx 가 AddRef 했으므로 Release.
		if (m_Backup.pVS) { m_Backup.pVS->Release(); m_Backup.pVS = nullptr; }
		if (m_Backup.pPS) { m_Backup.pPS->Release(); m_Backup.pPS = nullptr; }
		if (m_Backup.pTex0) { m_Backup.pTex0->Release(); m_Backup.pTex0 = nullptr; }
		if (m_Backup.pIB)  { m_Backup.pIB->Release();  m_Backup.pIB = nullptr; }
		if (m_Backup.pVB)  { m_Backup.pVB->Release();  m_Backup.pVB = nullptr; }
	}


	void C_DX9_FONT_DW::setupState_()
	{
		// 2D 알파 블렌딩 (atlas 의 ARGB 텍스처 × diffuse tint).
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
		m_uVBOffset = 0;	// 매 frame DISCARD 패턴
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


	//============================================================================
	// 글리프 quad 추가 (VB 에 4 vertex append, DISCARD/NOOVERWRITE 패턴).
	//============================================================================
	void C_DX9_FONT_DW::appendQuad_(float _fX, float _fY, float _fW, float _fH,
		float _fU0, float _fV0, float _fU1, float _fV1, D3DCOLOR _dwColor)
	{
		if (!m_bRendering || !m_pVB) return;

		// 배치 가득 — 즉시 flush 후 재시작.
		if (m_uQuadCount >= m_uMaxQuads)
		{
			flushBatch_();
		}

		// XYZRHW 의 -0.5 픽셀 보정 (DX9 텍셀-픽셀 매핑 규칙).
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

		// stream offset 은 0 — appendQuad_ 가 시작 슬롯 0 부터 채움. 다음 batch 의 첫 quad 가 DISCARD.
		m_pDevice->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST,
			0,					// BaseVertexIndex
			0,					// MinIndex
			m_uQuadCount * 4u,	// NumVertices
			0,					// StartIndex
			m_uQuadCount * 2u	// PrimitiveCount (2 triangle per quad)
		);

		m_uQuadCount = 0;
		m_uVBOffset = 0;	// DISCARD 재시작
	}


	//============================================================================
	// 텍스트 렌더링
	//============================================================================
	void C_DX9_FONT_DW::RenderText(float _fX, float _fY, const wchar_t* _pText,
		D3DCOLOR _dwColor, float _fScale)
	{
		if (!_pText || !m_bRendering || !IsInitialized()) return;

		const float fAtlasInv_ = 1.0f / static_cast<float>(m_uAtlasSize);
		const float fBaselineY_ = _fY + m_fAscent * _fScale;
		float fPenX_ = _fX;

		for (const wchar_t* p = _pText; *p != L'\0'; ++p)
		{
			const wchar_t wc_ = *p;
			if (wc_ == L'\n' || wc_ == L'\r') continue;

			const _DX9_FONT_DW_GLYPH* const pG_ = getGlyph_(wc_);
			if (!pG_) continue;

			if (pG_->m_uBmpWidth > 0 && pG_->m_uBmpHeight > 0)
			{
				const float fQX_ = fPenX_ + static_cast<float>(pG_->m_sOffsetX) * _fScale;
				const float fQY_ = fBaselineY_ + static_cast<float>(pG_->m_sOffsetY) * _fScale;
				const float fQW_ = static_cast<float>(pG_->m_uBmpWidth) * _fScale;
				const float fQH_ = static_cast<float>(pG_->m_uBmpHeight) * _fScale;
				const float fU0_ = static_cast<float>(pG_->m_uAtlasX) * fAtlasInv_;
				const float fV0_ = static_cast<float>(pG_->m_uAtlasY) * fAtlasInv_;
				const float fU1_ = static_cast<float>(pG_->m_uAtlasX + pG_->m_uBmpWidth) * fAtlasInv_;
				const float fV1_ = static_cast<float>(pG_->m_uAtlasY + pG_->m_uBmpHeight) * fAtlasInv_;

				appendQuad_(fQX_, fQY_, fQW_, fQH_, fU0_, fV0_, fU1_, fV1_, _dwColor);
			}

			fPenX_ += pG_->m_fAdvance * _fScale;
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

		float fW_ = 0.0f;
		for (const wchar_t* p = _pText; *p != L'\0'; ++p)
		{
			const wchar_t wc_ = *p;
			if (wc_ == L'\n' || wc_ == L'\r') continue;
			const _DX9_FONT_DW_GLYPH* const pG_ = getGlyph_(wc_);
			if (!pG_) continue;
			fW_ += pG_->m_fAdvance * _fScale;
		}
		return fW_;
	}

} // namespace dx9
