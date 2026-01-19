#pragma once


#include <Windows.h>

#include <DarkCore/DMemory.h>



namespace dx9
{
	struct _DVIEWPORT
	{
		DWORD       X;
		DWORD       Y;            /* Viewport Top left */
		DWORD       Width;
		DWORD       Height;       /* Viewport Dimensions */
		float       MinZ;         /* Min/max of clip Volume */
		float       MaxZ;

		_DVIEWPORT()
		{
			X = Y = Width = Height = 0;
			MinZ = 0.0f;
			MaxZ = 1.0f;
		}
		_DVIEWPORT(long x, long y, long w, long h, float fmin = 0.0f, float fmax = 1.0f)
		{
			X = x;
			Y = y;
			Width = w;
			Height = h;
			MinZ = fmin;
			MaxZ = fmax;
		}
		_DVIEWPORT(_DVIEWPORT *p)
		{
			X = p->X;
			Y = p->Y;
			Width = p->Width;
			Height = p->Height;
			MinZ = p->MinZ;
			MaxZ = p->MaxZ;
		}
		_DVIEWPORT(const _DVIEWPORT &p)
		{
			X = p.X;
			Y = p.Y;
			Width = p.Width;
			Height = p.Height;
			MinZ = p.MinZ;
			MaxZ = p.MaxZ;
		}
		void Set(long x, long y, long w, long h, float fmin = 0.0f, float fmax = 1.0f)
		{
			X = x;
			Y = y;
			Width = w;
			Height = h;
			MinZ = fmin;
			MaxZ = fmax;
		}
		void Set(DWORD _X, DWORD _Y, DWORD _Width, DWORD _Height, float _MinZ = 0.0f, float _MaxZ = 1.0f)
		{
			X = _X;
			Y = _Y;
			Width = _Width;
			Height = _Height;
			MinZ = _MinZ;
			MaxZ = _MaxZ;
		}
		void Set(_DVIEWPORT *p)
		{
			X = p->X;
			Y = p->Y;
			Width = p->Width;
			Height = p->Height;
			MinZ = p->MinZ;
			MaxZ = p->MaxZ;
		}
		void Set(const _DVIEWPORT &p)
		{
			X = p.X;
			Y = p.Y;
			Width = p.Width;
			Height = p.Height;
			MinZ = p.MinZ;
			MaxZ = p.MaxZ;
		}
	};
}