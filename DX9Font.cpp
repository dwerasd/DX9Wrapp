/**
 * @file DX9Font.cpp
 * @brief DirectX 9 고성능 폰트 시스템 구현
 * @details RealSpace3의 RFont를 대체하는 최적화된 폰트 렌더링 시스템
 */

#include "framework.h"
#include "DX9Font.h"

namespace dx9
{

//============================================================================
// 정적 멤버 초기화
//============================================================================
C_DX9_FONT_SYSTEM* C_DX9_FONT_SYSTEM::s_pInstance = nullptr;

//============================================================================
// _DX9_FONT 구현 (레거시 호환)
//============================================================================
_DX9_FONT::_DX9_FONT(
	LPDIRECT3DDEVICE9 pDevice
	, LPCWSTR _wszName
	, int _nSize
	, UINT _nWeight
	, UINT _nCharset
	, bool _bItalic
	, bool _bAntiAliased
)	: pDevice(pDevice)
	, wstrName(_wszName)
	, nSize(_nSize)
	, nWeight(_nWeight)
	, nCharset(_nCharset)
	, bItalic(_bItalic)
	, bAntiAliased(_bAntiAliased)
{
}

_DX9_FONT::~_DX9_FONT()
{
	DSAFE_RELEASE(pFont);
}

void _DX9_FONT::Create()
{
	if (nullptr == pFont)
	{
		D3DXCreateFontW(
			pDevice
			, nSize
			, 0
			, nWeight
			, 1
			, (BOOL)(bItalic)
			, nCharset
			, OUT_OUTLINE_PRECIS
			, bAntiAliased ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY
			, DEFAULT_PITCH
			, wstrName.c_str()
			, &pFont
		);
	}
}

void _DX9_FONT::Release()
{
	DSAFE_RELEASE(pFont);
}

LPD3DXFONT _DX9_FONT::GetFont() { return pFont; }

void _DX9_FONT::OnLostDevice()
{
	if (nullptr != pFont)
		pFont->OnLostDevice();
}

void _DX9_FONT::OnResetDevice()
{
	if (nullptr != pFont)
		pFont->OnResetDevice();
}

//============================================================================
// C_DX9_FONT_TEXTURE 구현
//============================================================================

C_DX9_FONT_TEXTURE::C_DX9_FONT_TEXTURE()
	: m_pDevice(nullptr)
	, m_hDC(nullptr)
	, m_hBitmapFont(nullptr)
	, m_hBitmapOutline(nullptr)
	, m_pBitsFont(nullptr)
	, m_pBitsOutline(nullptr)
	, m_pTextureFont(nullptr)
	, m_pTextureOutline(nullptr)
	, m_nCellCountX(0)
	, m_nCellCountY(0)
	, m_nTotalCells(0)
	, m_nLastUsedID(0)
	, m_pCells(nullptr)
{
}

C_DX9_FONT_TEXTURE::~C_DX9_FONT_TEXTURE()
{
	Destroy();
}

bool C_DX9_FONT_TEXTURE::Create(LPDIRECT3DDEVICE9 _pDevice)
{
	m_pDevice = _pDevice;

	m_nCellCountX = FONT_TEXTURE_SIZE / FONT_CELL_SIZE;
	m_nCellCountY = FONT_TEXTURE_SIZE / FONT_CELL_SIZE;
	m_nTotalCells = m_nCellCountX * m_nCellCountY;

	// 텍스처 생성
	HRESULT hr = m_pDevice->CreateTexture(
		FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 1,
		0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
		&m_pTextureFont, nullptr);
	if (FAILED(hr)) return false;

	hr = m_pDevice->CreateTexture(
		FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 1,
		0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
		&m_pTextureOutline, nullptr);
	if (FAILED(hr)) return false;

	// 셀 정보 배열 생성 및 LRU 리스트 초기화
	m_pCells = new _FONT_TEXTURE_CELL[m_nTotalCells];
	for (int i = 0; i < m_nTotalCells; ++i)
	{
		m_pCells[i].nID = 0;
		m_pCells[i].nIndex = i;
		m_listLRU.push_back(&m_pCells[i]);
	}

	// 모든 셀에 대해 iterator 설정
	auto it = m_listLRU.begin();
	for (int i = 0; i < m_nTotalCells; ++i, ++it)
	{
		m_pCells[i].iter = it;
	}

	// GDI 메모리 DC 생성
	m_hDC = CreateCompatibleDC(nullptr);
	if (!m_hDC) return false;

	// DIBSection 생성 (폰트용)
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = FONT_CELL_SIZE;
	bmi.bmiHeader.biHeight = -FONT_CELL_SIZE;  // Top-down DIB
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	m_hBitmapFont = CreateDIBSection(m_hDC, &bmi, DIB_RGB_COLORS, (void**)&m_pBitsFont, nullptr, 0);
	if (!m_hBitmapFont || !m_pBitsFont)
	{
		DeleteDC(m_hDC);
		m_hDC = nullptr;
		return false;
	}

	// DIBSection 생성 (외곽선용)
	m_hBitmapOutline = CreateDIBSection(m_hDC, &bmi, DIB_RGB_COLORS, (void**)&m_pBitsOutline, nullptr, 0);
	if (!m_hBitmapOutline || !m_pBitsOutline)
	{
		DeleteObject(m_hBitmapFont);
		DeleteDC(m_hDC);
		m_hDC = nullptr;
		return false;
	}

	SetMapMode(m_hDC, MM_TEXT);

	return true;
}

void C_DX9_FONT_TEXTURE::Destroy()
{
	if (m_hDC)
	{
		if (m_hBitmapFont)
		{
			DeleteObject(m_hBitmapFont);
			m_hBitmapFont = nullptr;
		}
		if (m_hBitmapOutline)
		{
			DeleteObject(m_hBitmapOutline);
			m_hBitmapOutline = nullptr;
		}
		DeleteDC(m_hDC);
		m_hDC = nullptr;
	}

	DSAFE_RELEASE(m_pTextureFont);
	DSAFE_RELEASE(m_pTextureOutline);

	if (m_pCells)
	{
		delete[] m_pCells;
		m_pCells = nullptr;
	}
	m_listLRU.clear();
}

bool C_DX9_FONT_TEXTURE::IsNeedUpdate(int _nCellIndex, int _nID)
{
	// 등록되지 않은 글자
	if (_nCellIndex == -1)
		return true;

	// 이미 같은 ID로 등록되어 있으면 LRU 갱신 후 false
	if (m_pCells[_nCellIndex].nID == _nID)
	{
		// 최근 사용된 것으로 리스트 뒤로 이동
		m_listLRU.splice(m_listLRU.end(), m_listLRU, m_pCells[_nCellIndex].iter);
		return false;
	}

	return true;
}

int C_DX9_FONT_TEXTURE::GetCharWidth(HFONT _hFont, const wchar_t* _wszChar, int _nCharLen)
{
	SIZE size;
	const HFONT hPrevFont = (HFONT)SelectObject(m_hDC, _hFont);
	GetTextExtentPoint32W(m_hDC, _wszChar, _nCharLen, &size);
	SelectObject(m_hDC, hPrevFont);
	return size.cx;
}

bool C_DX9_FONT_TEXTURE::MakeGlyphBitmap(HFONT _hFont, _FONT_GLYPH_INFO* _pInfo, const wchar_t* _wszChar, int _nCharLen, int _nOutlineWidth, float _fOutlineOpacity)
{
	const HFONT hPrevFont = (HFONT)SelectObject(m_hDC, _hFont);

	// 글자 크기 측정
	SIZE size;
	GetTextExtentPoint32W(m_hDC, _wszChar, _nCharLen, &size);

	// ABC Width 보정
	ABCFLOAT abcFirst, abcLast;
	const UINT uiCharFirst = _wszChar[0];
	const UINT uiCharLast = (_nCharLen > 1) ? _wszChar[1] : _wszChar[0];

	GetCharABCWidthsFloatW(m_hDC, uiCharFirst, uiCharFirst, &abcFirst);
	if (abcFirst.abcfA < 0.0f)
		size.cx -= (int)abcFirst.abcfA;

	GetCharABCWidthsFloatW(m_hDC, uiCharLast, uiCharLast, &abcLast);
	if (abcLast.abcfC < 0.0f)
		size.cx -= (int)abcLast.abcfC;

	const int nWidth = min(size.cx, FONT_CELL_SIZE);
	const int nHeight = min(size.cy, FONT_CELL_SIZE);

	// LRU에서 가장 오래된 셀 획득
	m_nLastUsedID++;
	_FONT_TEXTURE_CELL* pCell = m_listLRU.front();
	pCell->nID = m_nLastUsedID;

	RECT rt = { 0, 0, FONT_CELL_SIZE, FONT_CELL_SIZE };

	// 외곽선 렌더링
	if (_nOutlineWidth > 0)
	{
		HBITMAP hPrevBitmap = (HBITMAP)SelectObject(m_hDC, m_hBitmapOutline);

		FillRect(m_hDC, &rt, (HBRUSH)GetStockObject(BLACK_BRUSH));

		SetBkMode(m_hDC, TRANSPARENT);
		const int nOpacity = (int)(255.0f * min(1.0f, _fOutlineOpacity));
		SetTextColor(m_hDC, RGB(nOpacity, nOpacity, nOpacity));
		SetBkColor(m_hDC, 0x00000000);
		SetTextAlign(m_hDC, TA_TOP);

		// 8방향 외곽선 그리기 (최적화)
		DrawOutline8Dir(_wszChar, _nCharLen, _nOutlineWidth);

		UploadToTexture(m_pTextureOutline, pCell->nIndex,
			size.cx + _nOutlineWidth * 2, size.cy + _nOutlineWidth * 2, m_pBitsOutline);

		SelectObject(m_hDC, hPrevBitmap);
	}

	// 폰트 렌더링
	HBITMAP hPrevBitmap = (HBITMAP)SelectObject(m_hDC, m_hBitmapFont);

	SetBkMode(m_hDC, OPAQUE);
	SetTextColor(m_hDC, RGB(255, 255, 255));
	SetBkColor(m_hDC, 0x00000000);
	SetTextAlign(m_hDC, TA_TOP);
	FillRect(m_hDC, &rt, (HBRUSH)GetStockObject(BLACK_BRUSH));
	ExtTextOutW(m_hDC, _nOutlineWidth, _nOutlineWidth, ETO_OPAQUE, nullptr, _wszChar, _nCharLen, nullptr);

	bool bResult = UploadToTexture(m_pTextureFont, pCell->nIndex,
		size.cx + _nOutlineWidth * 2, size.cy + _nOutlineWidth * 2, m_pBitsFont);

	// LRU 갱신: 방금 사용한 셀을 뒤로 이동
	m_listLRU.splice(m_listLRU.end(), m_listLRU, m_listLRU.begin());

	// 글리프 정보 설정
	_pInfo->nTextureID = pCell->nID;
	_pInfo->nCellIndex = pCell->nIndex;
	_pInfo->nWidth = nWidth;
	_pInfo->nHeight = nHeight;

	SelectObject(m_hDC, hPrevFont);
	SelectObject(m_hDC, hPrevBitmap);

	return bResult;
}

void C_DX9_FONT_TEXTURE::DrawOutline8Dir(const wchar_t* _wszChar, int _nCharLen, int _nOutlineWidth)
{
	// 8방향 원형 패턴으로 외곽선 생성 (O(8*w) - 최적화됨)
	for (int nDist = 1; nDist <= _nOutlineWidth; ++nDist)
	{
		// 상하좌우 4방향
		ExtTextOutW(m_hDC, _nOutlineWidth, _nOutlineWidth - nDist, ETO_OPAQUE, nullptr, _wszChar, _nCharLen, nullptr);
		ExtTextOutW(m_hDC, _nOutlineWidth, _nOutlineWidth + nDist, ETO_OPAQUE, nullptr, _wszChar, _nCharLen, nullptr);
		ExtTextOutW(m_hDC, _nOutlineWidth - nDist, _nOutlineWidth, ETO_OPAQUE, nullptr, _wszChar, _nCharLen, nullptr);
		ExtTextOutW(m_hDC, _nOutlineWidth + nDist, _nOutlineWidth, ETO_OPAQUE, nullptr, _wszChar, _nCharLen, nullptr);

		// 대각선 4방향
		ExtTextOutW(m_hDC, _nOutlineWidth - nDist, _nOutlineWidth - nDist, ETO_OPAQUE, nullptr, _wszChar, _nCharLen, nullptr);
		ExtTextOutW(m_hDC, _nOutlineWidth + nDist, _nOutlineWidth - nDist, ETO_OPAQUE, nullptr, _wszChar, _nCharLen, nullptr);
		ExtTextOutW(m_hDC, _nOutlineWidth - nDist, _nOutlineWidth + nDist, ETO_OPAQUE, nullptr, _wszChar, _nCharLen, nullptr);
		ExtTextOutW(m_hDC, _nOutlineWidth + nDist, _nOutlineWidth + nDist, ETO_OPAQUE, nullptr, _wszChar, _nCharLen, nullptr);
	}
}

bool C_DX9_FONT_TEXTURE::UploadToTexture(LPDIRECT3DTEXTURE9 _pTexture, int _nCellIndex, int _nWidth, int _nHeight, DWORD* _pBits)
{
	D3DLOCKED_RECT lockedRect;
	if (FAILED(_pTexture->LockRect(0, &lockedRect, nullptr, 0)))
		return false;

	const int nCellX = _nCellIndex % m_nCellCountX;
	const int nCellY = _nCellIndex / m_nCellCountX;
	const int nPixelX = nCellX * FONT_CELL_SIZE;
	const int nPixelY = nCellY * FONT_CELL_SIZE;

	const int nUploadW = min(FONT_CELL_SIZE - 1, _nWidth);
	const int nUploadH = min(FONT_CELL_SIZE - 1, _nHeight);

	BYTE* const pDest = (BYTE*)lockedRect.pBits;
	const int nPitch = lockedRect.Pitch;

	// 픽셀 복사 (Red → Alpha 변환)
	for (int y = 0; y <= nUploadH; ++y)
	{
		DWORD* pDestRow = (DWORD*)(pDest + (nPixelY + y) * nPitch) + nPixelX;

		for (int x = 0; x <= nUploadW; ++x)
		{
			// 마지막 행/열은 투명
			if (y == nUploadH || x == nUploadW)
			{
				pDestRow[x] = 0x00000000;
			}
			else
			{
				DWORD dwPixel = _pBits[y * FONT_CELL_SIZE + x];
				// RGB 중 R 값을 Alpha로 이동 (흰 글자 -> 알파)
				dwPixel = (dwPixel << 8) | 0x00FFFFFF;
				pDestRow[x] = dwPixel;
			}
		}
	}

	_pTexture->UnlockRect(0);
	return true;
}

//============================================================================
// C_DX9_FONT_SYSTEM 구현
//============================================================================

C_DX9_FONT_SYSTEM::C_DX9_FONT_SYSTEM()
	: m_pDevice(nullptr)
	, m_nBatchCount(0)
	, m_nCurrentOutlineWidth(0)
	, m_bInFontMode(false)
{
}

C_DX9_FONT_SYSTEM::~C_DX9_FONT_SYSTEM()
{
	Finalize();
}

C_DX9_FONT_SYSTEM* C_DX9_FONT_SYSTEM::GetInstance()
{
	if (!s_pInstance)
		s_pInstance = new C_DX9_FONT_SYSTEM();
	return s_pInstance;
}

void C_DX9_FONT_SYSTEM::DestroyInstance()
{
	if (s_pInstance)
	{
		delete s_pInstance;
		s_pInstance = nullptr;
	}
}

bool C_DX9_FONT_SYSTEM::Initialize(LPDIRECT3DDEVICE9 _pDevice)
{
	if (!_pDevice)
		return false;

	m_pDevice = _pDevice;
	return m_fontTexture.Create(_pDevice);
}

void C_DX9_FONT_SYSTEM::Finalize()
{
	m_fontTexture.Destroy();

	// 폰트 파일 해제
	for (const auto& strPath : m_vFontFiles)
		RemoveFontResourceW(strPath.c_str());
	m_vFontFiles.clear();

	// 메모리 폰트 해제
	for (HANDLE hFont : m_vFontMemoryHandles)
		RemoveFontMemResourceEx(hFont);
	m_vFontMemoryHandles.clear();

	m_pDevice = nullptr;
}

bool C_DX9_FONT_SYSTEM::BeginFont()
{
	if (m_bInFontMode)
	{
		_ASSERT(!"C_DX9_FONT_SYSTEM::BeginFont() - Already in font mode");
		return false;
	}

	if (!m_pDevice)
		return false;

	// 렌더 스테이트 설정
	m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x08);
	m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	// 텍스처 스테이트 설정
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	m_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// 변환 행렬 저장 및 2D 직교 투영 설정
	m_pDevice->GetTransform(D3DTS_VIEW, (D3DMATRIX*)&m_matSavedView);
	m_pDevice->GetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&m_matSavedProj);

	D3DVIEWPORT9 vp;
	m_pDevice->GetViewport(&vp);

	D3DXMATRIX matIdentity, matOrtho;
	D3DXMatrixIdentity(&matIdentity);
	D3DXMatrixOrthoOffCenterLH(&matOrtho,
		(float)vp.X, (float)(vp.X + vp.Width),
		(float)(vp.Y + vp.Height), (float)vp.Y,
		vp.MinZ, vp.MaxZ);

	m_pDevice->SetTransform(D3DTS_WORLD, &matIdentity);
	m_pDevice->SetTransform(D3DTS_VIEW, &matIdentity);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &matOrtho);

	m_nBatchCount = 0;
	m_bInFontMode = true;

	return true;
}

bool C_DX9_FONT_SYSTEM::EndFont()
{
	if (!m_bInFontMode)
	{
		_ASSERT(!"C_DX9_FONT_SYSTEM::EndFont() - Not in font mode");
		return false;
	}

	FlushFont();

	// 렌더 스테이트 복원
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	m_pDevice->SetTexture(0, nullptr);

	// 변환 행렬 복원
	m_pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m_matSavedView);
	m_pDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&m_matSavedProj);

	m_bInFontMode = false;

	return true;
}

void C_DX9_FONT_SYSTEM::FlushFont()
{
	if (m_nBatchCount == 0)
		return;

	m_pDevice->SetFVF(FONT_VERTEX_FVF);

	// 외곽선 렌더링
	if (m_nCurrentOutlineWidth > 0)
	{
		m_pDevice->SetTexture(0, m_fontTexture.GetTextureOutline());
		m_pDevice->DrawIndexedPrimitiveUP(
			D3DPT_TRIANGLELIST, 0, m_nBatchCount * 4, m_nBatchCount * 2,
			m_vIndexBuffer, D3DFMT_INDEX16,
			m_vOutlineBuffer, sizeof(_FONT_VERTEX));
	}

	// 폰트 렌더링
	m_pDevice->SetTexture(0, m_fontTexture.GetTextureFont());
	m_pDevice->DrawIndexedPrimitiveUP(
		D3DPT_TRIANGLELIST, 0, m_nBatchCount * 4, m_nBatchCount * 2,
		m_vIndexBuffer, D3DFMT_INDEX16,
		m_vFontBuffer, sizeof(_FONT_VERTEX));

	m_nBatchCount = 0;
}

void C_DX9_FONT_SYSTEM::AddGlyphToBatch(
	float _fX, float _fY, float _fWidth, float _fHeight,
	float _fU0, float _fV0, float _fU1, float _fV1,
	DWORD _dwColorFont, DWORD _dwColorOutline,
	int _nOutlineWidth)
{
	if (m_nBatchCount >= FONT_MAX_BATCH)
		FlushFont();

	// 인덱스 버퍼 설정
	WORD* const pIndices = m_vIndexBuffer + m_nBatchCount * 6;
	const WORD nBaseVertex = (WORD)(m_nBatchCount * 4);
	pIndices[0] = nBaseVertex + 3;
	pIndices[1] = nBaseVertex + 0;
	pIndices[2] = nBaseVertex + 2;
	pIndices[3] = nBaseVertex + 0;
	pIndices[4] = nBaseVertex + 1;
	pIndices[5] = nBaseVertex + 2;

	// 폰트 버텍스 (직접 버퍼에 기록)
	_FONT_VERTEX* pFont = m_vFontBuffer + m_nBatchCount * 4;
	pFont[0].x = _fX;            pFont[0].y = _fY;             pFont[0].z = 0;
	pFont[1].x = _fX;            pFont[1].y = _fY + _fHeight;  pFont[1].z = 0;
	pFont[2].x = _fX + _fWidth;  pFont[2].y = _fY + _fHeight;  pFont[2].z = 0;
	pFont[3].x = _fX + _fWidth;  pFont[3].y = _fY;             pFont[3].z = 0;

	pFont[0].u = _fU0; pFont[0].v = _fV0;
	pFont[1].u = _fU0; pFont[1].v = _fV1;
	pFont[2].u = _fU1; pFont[2].v = _fV1;
	pFont[3].u = _fU1; pFont[3].v = _fV0;

	pFont[0].dwColor = pFont[1].dwColor = pFont[2].dwColor = pFont[3].dwColor = _dwColorFont;

	// 외곽선 버텍스
	if (_nOutlineWidth > 0)
	{
		_FONT_VERTEX* pOutline = m_vOutlineBuffer + m_nBatchCount * 4;
		pOutline[0] = pFont[0]; pOutline[0].dwColor = _dwColorOutline;
		pOutline[1] = pFont[1]; pOutline[1].dwColor = _dwColorOutline;
		pOutline[2] = pFont[2]; pOutline[2].dwColor = _dwColorOutline;
		pOutline[3] = pFont[3]; pOutline[3].dwColor = _dwColorOutline;
	}

	m_nCurrentOutlineWidth = _nOutlineWidth;
	m_nBatchCount++;
}

bool C_DX9_FONT_SYSTEM::AddFontFromFile(const wchar_t* _wszFontPath)
{
	// 중복 체크
	for (const auto& strPath : m_vFontFiles)
	{
		if (strPath == _wszFontPath)
			return true;
	}

	if (AddFontResourceW(_wszFontPath) != 0)
	{
		m_vFontFiles.push_back(_wszFontPath);
		return true;
	}
	return false;
}

bool C_DX9_FONT_SYSTEM::AddFontFromMemory(const void* _pData, DWORD _nSize)
{
	DWORD dwCount = 0;
	HANDLE hFont = AddFontMemResourceEx(const_cast<void*>(_pData), _nSize, nullptr, &dwCount);
	if (hFont)
	{
		m_vFontMemoryHandles.push_back(hFont);
		return true;
	}
	return false;
}

//============================================================================
// C_DX9_FONT 구현
//============================================================================

C_DX9_FONT::C_DX9_FONT()
	: m_hFont(nullptr)
	, m_nFontHeight(0)
	, m_nOutlineWidth(0)
	, m_fOutlineOpacity(1.0f)
	, m_bAntiAlias(false)
{
}

C_DX9_FONT::~C_DX9_FONT()
{
	Destroy();
}

bool C_DX9_FONT::Create(
	const wchar_t* _wszFontName,
	int _nHeight,
	bool _bBold,
	bool _bItalic,
	int _nOutlineWidth,
	float _fOutlineOpacity,
	bool _bAntiAlias)
{
	_ASSERT(_nHeight <= FONT_CELL_SIZE);

	m_nFontHeight = _nHeight + _nOutlineWidth;
	m_nOutlineWidth = _nOutlineWidth;
	m_fOutlineOpacity = _fOutlineOpacity;
	m_bAntiAlias = _bAntiAlias;

	m_hFont = CreateFontW(
		-_nHeight,                          // 높이 (음수 = 문자 높이)
		0,                                  // 너비 (0 = 기본)
		0, 0,                               // 기울기
		_bBold ? FW_BOLD : FW_NORMAL,       // 굵기
		_bItalic ? TRUE : FALSE,            // 이탤릭
		FALSE, FALSE,                       // 밑줄, 취소선
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		_bAntiAlias ? ANTIALIASED_QUALITY : DEFAULT_QUALITY,
		DEFAULT_PITCH,
		_wszFontName);

	return (m_hFont != nullptr);
}

void C_DX9_FONT::Destroy()
{
	if (m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont = nullptr;
	}

	// 글리프 캐시 해제
	for (auto& pair_ : m_mapGlyphs)
		delete pair_.second;
	m_mapGlyphs.clear();
}

void C_DX9_FONT::DrawText2D(
	float _fX, float _fY,
	const wchar_t* _wszText,
	int _nLength,
	DWORD _dwColorFont,
	DWORD _dwColorOutline,
	float _fScale)
{
	C_DX9_FONT_SYSTEM* const pSystem = C_DX9_FONT_SYSTEM::GetInstance();

	if (!pSystem->IsInFontMode())
	{
		_ASSERT(!"C_DX9_FONT::DrawText2D() - BeginFont() not called");
		return;
	}

	if (!_wszText || _wszText[0] == 0 || _wszText[0] == L'\n')
		return;

	C_DX9_FONT_TEXTURE* const pFontTex = pSystem->GetFontTexture();
	const float fTexWidth = (float)pFontTex->GetWidth();
	const float fTexHeight = (float)pFontTex->GetHeight();

	const wchar_t* p = _wszText;
	int nCount = 0;

	while (*p != 0)
	{
		if (_nLength != -1 && nCount >= _nLength)
			break;
		if (*p == L'\n')
			break;

		// 문자 길이 결정 (서로게이트 페어 체크)
		const int nCharLen = (p[0] != 0 && p[1] != 0 && ((p[0] & 0xFC00) == 0xD800)) ? 2 : 1;
		const WORD wKey = (WORD)(*p);

		// 글리프 정보 획득
		wchar_t wszChar[3] = { p[0], (wchar_t)((nCharLen > 1) ? p[1] : 0), 0 };
		_FONT_GLYPH_INFO* const pInfo = GetGlyphInfo(wKey, wszChar, nCharLen);

		if (pInfo)
		{
			const int nWidth = min(FONT_CELL_SIZE, pInfo->nWidth);
			const int nHeight = min(FONT_CELL_SIZE, pInfo->nHeight);
			const float fW = nWidth * _fScale;
			const float fH = nHeight * _fScale;
			const float fRenderW = fW + m_nOutlineWidth * 2;
			const float fRenderH = fH + m_nOutlineWidth * 2;

			// UV 계산
			const int nCellX = pInfo->nCellIndex % pFontTex->GetCellCountX();
			const int nCellY = pInfo->nCellIndex / pFontTex->GetCellCountX();

			const float fU0 = (0.5f + nCellX * FONT_CELL_SIZE) / fTexWidth;
			const float fV0 = (0.5f + nCellY * FONT_CELL_SIZE) / fTexHeight;
			const float fU1 = (m_nOutlineWidth * 2 + 0.5f + nCellX * FONT_CELL_SIZE + nWidth) / fTexWidth;
			const float fV1 = (m_nOutlineWidth * 2 + 0.5f + nCellY * FONT_CELL_SIZE + nHeight) / fTexHeight;

			// 배치에 추가
			pSystem->AddGlyphToBatch(_fX, _fY, fRenderW, fRenderH,
				fU0, fV0, fU1, fV1,
				_dwColorFont, _dwColorOutline, m_nOutlineWidth);

			_fX += pInfo->nWidth * _fScale;
		}

		p += nCharLen;
		nCount += nCharLen;
	}
}

bool C_DX9_FONT::DrawText3D(
	const D3DXMATRIX& _matWorld,
	const wchar_t* _wszText,
	int _nLength,
	DWORD _dwColor)
{
	C_DX9_FONT_SYSTEM* const pSystem = C_DX9_FONT_SYSTEM::GetInstance();

	if (!pSystem->IsInFontMode())
	{
		_ASSERT(!"C_DX9_FONT::DrawText3D() - BeginFont() not called");
		return false;
	}

	// 텍스트 크기 측정
	const SIZE textSize = GetTextSize(_wszText, _nLength);

	// 중앙 정렬
	D3DXMATRIX matCentered = _matWorld;
	matCentered._41 -= textSize.cx * 0.5f;

	const LPDIRECT3DDEVICE9 pDevice = pSystem->GetDevice();

	pDevice->SetTransform(D3DTS_WORLD, &matCentered);
	pDevice->SetTransform(D3DTS_PROJECTION, &pSystem->GetSavedProj());

	pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	DrawText2D(0, 0, _wszText, _nLength, _dwColor, 0xFF000000, 1.0f);

	pSystem->FlushFont();

	return true;
}

bool C_DX9_FONT::DrawText3D(
	const D3DXVECTOR3& _vWorldPos,
	const wchar_t* _wszText,
	int _nLength,
	DWORD _dwColor)
{
	C_DX9_FONT_SYSTEM* const pSystem = C_DX9_FONT_SYSTEM::GetInstance();

	if (!pSystem->IsInFontMode())
	{
		_ASSERT(!"C_DX9_FONT::DrawText3D() - BeginFont() not called");
		return false;
	}

	const LPDIRECT3DDEVICE9 pDevice = pSystem->GetDevice();

	// 뷰포트 획득
	D3DVIEWPORT9 vp;
	pDevice->GetViewport(&vp);

	// 월드 → 스크린 변환
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	const D3DXMATRIX& matView = pSystem->GetSavedView();
	const D3DXMATRIX& matProj = pSystem->GetSavedProj();

	D3DXVECTOR3 vScreen;
	D3DXVec3Project(&vScreen, &_vWorldPos, &vp, &matProj, &matView, &matWorld);

	// 카메라 뒤에 있으면 출력하지 않음
	if (vScreen.z <= 0.0f || vScreen.z >= 1.0f)
		return false;

	// 텍스트 크기 측정 (투영 성공 시에만)
	const SIZE textSize = GetTextSize(_wszText, _nLength);

	// 중앙 정렬
	const float fX = vScreen.x - textSize.cx * 0.5f;
	const float fY = vScreen.y;

	// 그림자 + 본문
	DrawText2D(fX + 1.0f, fY + 1.0f, _wszText, _nLength, 0xA0000000, 0xFF000000, 1.0f);
	DrawText2D(fX, fY, _wszText, _nLength, _dwColor, 0xFF000000, 1.0f);

	return true;
}

int C_DX9_FONT::GetTextWidth(const wchar_t* _wszText, int _nLength)
{
	if (!_wszText || _wszText[0] == 0)
		return 0;

	int nWidth = 0;
	const wchar_t* p = _wszText;
	int nCount = 0;

	while (*p != 0)
	{
		if (_nLength != -1 && nCount >= _nLength)
			break;

		const int nCharLen = (p[0] != 0 && p[1] != 0 && ((p[0] & 0xFC00) == 0xD800)) ? 2 : 1;
		const WORD wKey = (WORD)(*p);

		wchar_t wszChar[3] = { p[0], (wchar_t)((nCharLen > 1) ? p[1] : 0), 0 };
		_FONT_GLYPH_INFO* const pInfo = GetGlyphInfo(wKey, wszChar, nCharLen);

		if (pInfo)
			nWidth += pInfo->nWidth;

		p += nCharLen;
		nCount += nCharLen;
	}

	return nWidth;
}

SIZE C_DX9_FONT::GetTextSize(const wchar_t* _wszText, int _nLength)
{
	SIZE size = { 0, m_nFontHeight };

	if (!_wszText || _wszText[0] == 0)
		return size;

	if (_nLength == -1)
		_nLength = (int)wcslen(_wszText);

	C_DX9_FONT_TEXTURE* const pFontTex = C_DX9_FONT_SYSTEM::GetInstance()->GetFontTexture();

	// 폰트 선택 후 크기 측정
	const HFONT hPrevFont = (HFONT)SelectObject(pFontTex->GetDC(), m_hFont);
	GetTextExtentPoint32W(pFontTex->GetDC(), _wszText, _nLength, &size);
	SelectObject(pFontTex->GetDC(), hPrevFont);

	return size;
}

_FONT_GLYPH_INFO* C_DX9_FONT::GetGlyphInfo(WORD _wKey, const wchar_t* _wszChar, int _nCharLen)
{
	C_DX9_FONT_TEXTURE* const pFontTex = C_DX9_FONT_SYSTEM::GetInstance()->GetFontTexture();

	auto it = m_mapGlyphs.find(_wKey);

	if (it == m_mapGlyphs.end())
	{
		// 캐시 미스: 새 글리프 생성
		if (m_mapGlyphs.size() >= FONT_MAX_CELLS)
		{
			// 캐시 가득 참 - 첫 번째 항목 삭제
			delete m_mapGlyphs.begin()->second;
			m_mapGlyphs.erase(m_mapGlyphs.begin());
		}

		_FONT_GLYPH_INFO* pInfo = new _FONT_GLYPH_INFO();
		pFontTex->MakeGlyphBitmap(m_hFont, pInfo, _wszChar, _nCharLen, m_nOutlineWidth, m_fOutlineOpacity);
		m_mapGlyphs[_wKey] = pInfo;
		return pInfo;
	}
	else
	{
		// 캐시 히트: 갱신 필요 여부 확인
		_FONT_GLYPH_INFO* pInfo = it->second;
		if (pFontTex->IsNeedUpdate(pInfo->nCellIndex, pInfo->nTextureID))
		{
			pFontTex->MakeGlyphBitmap(m_hFont, pInfo, _wszChar, _nCharLen, m_nOutlineWidth, m_fOutlineOpacity);
		}
		return pInfo;
	}
}

}  // namespace dx9
