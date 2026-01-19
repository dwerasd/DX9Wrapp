#pragma once


#include <DarkCore/DMemory.h>

#include "DX9Def.h"
#include "DViewport.h"



namespace dx9
{
	typedef struct _DVIEWPORT9
		: public _D3DVIEWPORT9
	{
		_DVIEWPORT9()
		{
			X = Y = Width = Height = 0;
			MinZ = 0.0f;
			MaxZ = 1.0f;
		}
		_DVIEWPORT9(long x, long y, long w, long h, float fmin = 0.0f, float fmax = 1.0f)
		{
			X = x;
			Y = y;
			Width = w;
			Height = h;
			MinZ = fmin;
			MaxZ = fmax;
		}
		_DVIEWPORT9(_D3DVIEWPORT9* p)
		{
			X = p->X;
			Y = p->Y;
			Width = p->Width;
			Height = p->Height;
			MinZ = p->MinZ;
			MaxZ = p->MaxZ;
		}
		_DVIEWPORT9(_D3DVIEWPORT9& p)
		{
			X = p.X;
			Y = p.Y;
			Width = p.Width;
			Height = p.Height;
			MinZ = p.MinZ;
			MaxZ = p.MaxZ;
		}
		_DVIEWPORT9(_D3DVIEWPORT9 _Vpt)
		{
			X = _Vpt.X;
			Y = _Vpt.Y;
			Width = _Vpt.Width;
			Height = _Vpt.Height;
			MinZ = _Vpt.MinZ;
			MaxZ = _Vpt.MaxZ;
		}
		_DVIEWPORT9(_DVIEWPORT _Vpt)
		{
			X = _Vpt.X;
			Y = _Vpt.Y;
			Width = _Vpt.Width;
			Height = _Vpt.Height;
			MinZ = _Vpt.MinZ;
			MaxZ = _Vpt.MaxZ;
		}
		_DVIEWPORT9(_DVIEWPORT *_p)
		{
			X = _p->X;
			Y = _p->Y;
			Width = _p->Width;
			Height = _p->Height;
			MinZ = _p->MinZ;
			MaxZ = _p->MaxZ;
		}

		/*
		void Set(int x, int y, int w, int h, float fmin = 0.0f, float fmax = 1.0f)
		{
			X = x;
			Y = y;
			Width = w;
			Height = h;
			MinZ = fmin;
			MaxZ = fmax;
		}
		*/
		void Set(DWORD _X, DWORD _Y, DWORD _Width, DWORD _Height, float _MinZ = 0.0f, float _MaxZ = 1.0f)
		{
			X = _X;
			Y = _Y;
			Width = _Width;
			Height = _Height;
			MinZ = _MinZ;
			MaxZ = _MaxZ;
		}
		void Set(_DVIEWPORT9 *p)
		{
			X = p->X;
			Y = p->Y;
			Width = p->Width;
			Height = p->Height;
			MinZ = p->MinZ;
			MaxZ = p->MaxZ;
		}
		void Set(_DVIEWPORT9 &p)
		{
			X = p.X;
			Y = p.Y;
			Width = p.Width;
			Height = p.Height;
			MinZ = p.MinZ;
			MaxZ = p.MaxZ;
		}
		void Set(_DVIEWPORT &_Vpt)
		{
			X = _Vpt.X;
			Y = _Vpt.Y;
			Width = _Vpt.Width;
			Height = _Vpt.Height;
			MinZ = _Vpt.MinZ;
			MaxZ = _Vpt.MaxZ;
		}
		void Set(_DVIEWPORT *_pVpt)
		{
			X = _pVpt->X;
			Y = _pVpt->Y;
			Width = _pVpt->Width;
			Height = _pVpt->Height;
			MinZ = _pVpt->MinZ;
			MaxZ = _pVpt->MaxZ;
		}
		_DVIEWPORT9 operator = (_DVIEWPORT9 other);
		_DVIEWPORT9 operator = (_DVIEWPORT9 *other);
		_DVIEWPORT9 operator = (_DVIEWPORT other);
		_DVIEWPORT9 operator = (_DVIEWPORT *other);

	} DVIEWPORT9, *LPDVIEWPORT9;
}
