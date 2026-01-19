#pragma once


#include <DarkCore/DDef.h>
#include <DarkCore/DString.h>

#include "DX9Def.h"



namespace dx9
{
	struct _DX9_FONT
	{
	private:
		LPD3DXFONT pFont{ nullptr };
		LPDIRECT3DDEVICE9 pDevice{ nullptr };

		std::wstring wstrName;
		UINT nSize;
		//0, 100, 200 ... 1000
		UINT nWeight;
		UINT nCharset;
		bool bItalic;
		bool bAntiAliased;

	public:
		_DX9_FONT(
			LPDIRECT3DDEVICE9 pDevice
			, LPCWSTR _wszName
			, int _nSize
			, UINT _nWeight = FW_NORMAL
			, UINT _nCharset = HANGUL_CHARSET//DEFAULT_CHARSET
			, bool _bItalic = false
			, bool _bAntiAliased = false
		);
		virtual ~_DX9_FONT();

		void Create();
		void Release();

		LPD3DXFONT GetFont();

		void OnLostDevice();
		void OnResetDevice();
	};
}