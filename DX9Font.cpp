#include "framework.h"
#include "DX9Font.h"



namespace dx9
{
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
				//, OUT_STROKE_PRECIS
				, bAntiAliased ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY
				, DEFAULT_PITCH
				//, VARIABLE_PITCH | FF_ROMAN
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
		{
			pFont->OnResetDevice();
			//Create();
		}
	}
}
