#pragma once


#include <DarkCore/DMemory.h>
#include <DarkCore/DTypes.h>

#include "DX9Vector2.h"
#include "DX9Vector4.h"



namespace dx9
{

	struct _DX9_PRESENT_PARAMETERS
		: public D3DPRESENT_PARAMETERS
	{
		_DX9_PRESENT_PARAMETERS(long _nWidth = 0, long _nHeight = 0)
		{
			BackBufferWidth = _nWidth;
			BackBufferHeight = _nHeight;
			BackBufferFormat = D3DFMT_X8R8G8B8;
			BackBufferCount = 1;

			MultiSampleType = D3DMULTISAMPLE_NONE;
			MultiSampleQuality = 0;

			SwapEffect = D3DSWAPEFFECT_DISCARD;
			hDeviceWindow = nullptr;
			Windowed = TRUE;
			EnableAutoDepthStencil = TRUE;
			AutoDepthStencilFormat = D3DFMT_D24S8;
			Flags = 0;

			/* FullScreen_RefreshRateInHz must be zero for Windowed mode */
			FullScreen_RefreshRateInHz = 0;	// 윈도우 모드에서는 "0" 이어야한다
			PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	// D3DPRESENT_INTERVAL_ONE
		}
		void SetSize(WORD _nWidth, WORD _nHeight)
		{
			BackBufferWidth = _nWidth;
			BackBufferHeight = _nHeight;
		}
	};

	struct _DFSIZE;
	struct _DFPOINT;
	struct _DFRECT;

	struct _DFSIZE
	{
		float cx;
		float cy;
		// Constructors
		// construct an uninitialized size
		_DFSIZE() throw();
		// create from two integers
		_DFSIZE(float initCX, float initCY) throw();
		// create from another size
		_DFSIZE(SIZE initSize) throw();
		_DFSIZE(_DFSIZE *initSize) throw();
		// create from a point
		_DFSIZE(_DFPOINT initPt) throw();
		_DFSIZE(_DFPOINT* initPt) throw();
		// create from a DWORD: cx = LOWORD(dw) cy = HIWORD(dw)
		_DFSIZE(DWORD dwSize) throw();

		// Operations
		BOOL operator==(_DFSIZE size) const throw();
		BOOL operator!=(_DFSIZE size) const throw();
		void operator+=(_DFSIZE size) throw();
		void operator-=(_DFSIZE size) throw();
		void Set(float CX, float CY) throw();
		void Set(_DFSIZE * _size) throw();
		void Set(_DFRECT * _size) throw();

		// Operators returning _DFSIZE values
		_DFSIZE operator+(_DFSIZE size) const throw();
		_DFSIZE operator-(_DFSIZE size) const throw();
		_DFSIZE operator-() const throw();

		// Operators returning _DFPOINT values
		_DFPOINT operator+(_DFPOINT point) const throw();
		_DFPOINT operator-(_DFPOINT point) const throw();

	};

	/////////////////////////////////////////////////////////////////////////////
	// _DFPOINT - A 2-D point, similar to Windows _DFPOINT structure.

	struct _DFPOINT
	{
		float x, y;
		// create an uninitialized point
		_DFPOINT() throw();
		// create from two integers
		_DFPOINT(float initX, float initY) throw();
		//_DFPOINT(long initX, long initY) throw();
		// create from another point
		_DFPOINT(POINT initPt) throw();
		_DFPOINT(POINT* initPt) throw();
		_DFPOINT(dk::LPDPOINT initPt) throw();
		_DFPOINT(_DFPOINT* initPt) throw();
		// create from a size
		_DFPOINT(_DFSIZE initSize) throw();

		float GetX();
		float GetY();

		void Set(float _x, float _y);
		void Set(POINT _pt);
		void SetX(float _x);
		void SetY(float _y);

		// Operations

		// translate the point
		void Offset(float xOffset, float yOffset) throw();
		void Offset(_DFPOINT point) throw();
		void Offset(_DFSIZE size) throw();
		void SetPoint(float X, float Y) throw();

		void Scale(float ix, float iy)
		{
			x = x*ix;
			y = y*iy;
		}

		_DFPOINT* operator=(POINT* _pPoint) throw();
		_DFPOINT* operator=(_DFPOINT* _pPoint) throw();
		BOOL operator==(_DFPOINT point) const throw();
		BOOL operator!=(_DFPOINT point) const throw();
		void operator+=(_DFSIZE size) throw();
		void operator-=(_DFSIZE size) throw();
		void operator+=(_DFPOINT point) throw();
		void operator-=(_DFPOINT point) throw();

		// Operators returning _DFPOINT values
		_DFPOINT operator+(_DFSIZE size) const throw();
		_DFPOINT operator+(_DFPOINT point) const throw();
		_DFPOINT operator+(_DFPOINT* point) const throw();
		_DFPOINT operator-(_DFSIZE size) const throw();
		_DFPOINT operator-() const throw();
		// Operators returning _DFSIZE values
		_DFPOINT operator-(_DFPOINT point) const throw();
	};

	/////////////////////////////////////////////////////////////////////////////
	// _DFRECT - A 2-D rectangle, similar to Windows _DFRECT structure.
	
	struct _DFRECT
	{
		float left;
		float top;
		float right;
		float bottom;

		// uninitialized rectangle
		_DFRECT() throw();
		// from left, top, right, and bottom
		_DFRECT(
			float l,
			float t,
			float r,
			float b) throw();
		_DFRECT(
			long l,
			long t,
			long r,
			long b) throw();
		_DFRECT(const dk::LPDRECT _pRect) throw();
		// copy constructor
		_DFRECT(const _DFRECT& srcRect) throw();

		// from a pointer to another rect
		_DFRECT(_DFRECT *lpSrcRect) throw();
		// from a point and size
		_DFRECT(_DFPOINT point, _DFSIZE size) throw();
		// from two points
		_DFRECT(_DFPOINT topLeft, _DFPOINT bottomRight) throw();

		// Attributes (in addition to _DFRECT members)

		// retrieves the width
		float Width() const throw();
		// returns the height
		float Height() const throw();
		// returns the size
		_DFSIZE Size() const throw();
		// reference to the top-left point
		_DFPOINT& TopLeft() throw();
		// reference to the bottom-right point
		_DFPOINT& BottomRight() throw();
		// const reference to the top-left point
		const _DFPOINT& TopLeft() const throw();
		// const reference to the bottom-right point
		const _DFPOINT& BottomRight() const throw();
		// the geometric center point of the rectangle
		_DFPOINT CenterPoint() const throw();
		// std::swap the left and right
		void SwapLeftRight() throw();
		static void WINAPI SwapLeftRight(_Inout_ _DFRECT *lpRect) throw();

		// returns TRUE if rectangle is at (0,0) and has no area
		BOOL IsRectNull() const throw();

		// Operations

		// set rectangle from left, top, right, and bottom
		void Set(
			float _left,
			float _top,
			float _right,
			float _bottom) throw();
		void Set(
			_DFPOINT topLeft,
			_DFPOINT bottomRight) throw();

		void Set(_DFRECT *_rect) throw();

		// empty the rectangle
		void SetRectEmpty() throw();
		// copy from another rectangle
		void CopyRect(_DFRECT *lpSrcRect) throw();
		// TRUE if exactly the same as another rectangle
		BOOL EqualRect(_DFRECT *lpRect) const throw();

		// Inflate rectangle's width and height by
		// x units to the left and right ends of the rectangle
		// and y units to the top and bottom.
		void InflateRect(
			float x,
			float y) throw();
		// Inflate rectangle's width and height by
		// size.cx units to the left and right ends of the rectangle
		// and size.cy units to the top and bottom.
		void InflateRect(_DFSIZE size) throw();
		// Inflate rectangle's width and height by moving individual sides.
		// Left side is moved to the left, right side is moved to the right,
		// top is moved up and bottom is moved down.
		void InflateRect(_DFRECT *lpRect) throw();
		void InflateRect(
			float l,
			float t,
			float r,
			float b) throw();

		// deflate the rectangle's width and height without
		// moving its top or left
		void DeflateRect(
			float x,
			float y) throw();
		void DeflateRect(_DFSIZE size) throw();
		void DeflateRect(_DFRECT *lpRect) throw();
		void DeflateRect(
			float l,
			float t,
			float r,
			float b) throw();

		// translate the rectangle by moving its top and left
		void OffsetRect(
			float x,
			float y) throw();
		void OffsetRect(_DFSIZE size) throw();
		void OffsetRect(_DFPOINT point) throw();
		void NormalizeRect() throw();

		// absolute position of rectangle
		void MoveToY(float y) throw();
		void MoveToX(float x) throw();
		void MoveToXY(
			float x,
			float y) throw();
		void MoveToXY(_DFPOINT point) throw();

		// set this rectangle to intersection of two others
		BOOL IntersectRect(
			_DFRECT *lpRect1,
			_DFRECT *lpRect2) throw();

		// set this rectangle to bounding union of two others
		BOOL UnionRect(
			_DFRECT *lpRect1,
			_DFRECT *lpRect2) throw();

		// set this rectangle to minimum of two others
		BOOL SubtractRect(
			_DFRECT *lpRectSrc1,
			_DFRECT *lpRectSrc2) throw();

		// Additional Operations
		void operator=(const _DFRECT& srcRect) throw();
		BOOL operator==(const _DFRECT& rect) const throw();
		BOOL operator!=(const _DFRECT& rect) const throw();
		void operator+=(_DFRECT *lpRect) throw();
		void operator-=(_DFRECT *lpRect) throw();
	};

	// _DFSIZE
	inline _DFSIZE::_DFSIZE() throw()
	{
		cx = 0;
		cy = 0;
	}

	inline _DFSIZE::_DFSIZE(float initCX, float initCY) throw()
	{
		cx = initCX;
		cy = initCY;
	}

	inline _DFSIZE::_DFSIZE(SIZE initSize) throw()
	{
		//*(_DFSIZE*)this = initSize;
		cx = (float)initSize.cx;
		cy = (float)initSize.cy;
	}

	inline _DFSIZE::_DFSIZE(_DFSIZE *initSize) throw()
	{
		cx = initSize->cx;
		cy = initSize->cy;
	}
	inline _DFSIZE::_DFSIZE(_DFPOINT initPt) throw()
	{
		*(_DFPOINT*)this = initPt;
	}
	inline _DFSIZE::_DFSIZE(_DFPOINT* initSize) throw()
	{
		cx = initSize->x;
		cy = initSize->y;
	}
	inline _DFSIZE::_DFSIZE(DWORD dwSize) throw()
	{
		cx = (short)LOWORD(dwSize);
		cy = (short)HIWORD(dwSize);
	}

	inline BOOL _DFSIZE::operator==(_DFSIZE size) const throw()
	{
		return (cx == size.cx && cy == size.cy);
	}

	inline BOOL _DFSIZE::operator!=(_DFSIZE size) const throw()
	{
		return (cx != size.cx || cy != size.cy);
	}

	inline void _DFSIZE::operator+=(_DFSIZE size) throw()
	{
		cx += size.cx;
		cy += size.cy;
	}

	inline void _DFSIZE::operator-=(_DFSIZE size) throw()
	{
		cx -= size.cx;
		cy -= size.cy;
	}
	inline void _DFSIZE::Set(float CX, float CY) throw()
	{
		cx = CX;
		cy = CY;
	}
	inline void _DFSIZE::Set(_DFSIZE *_size) throw()
	{
		cx = _size->cx;
		cy = _size->cy;
	}
	inline void _DFSIZE::Set(_DFRECT *_rect) throw()
	{
		cx = _rect->right;
		cy = _rect->bottom;
	}

	inline _DFSIZE _DFSIZE::operator+(_DFSIZE size) const throw()
	{
		return _DFSIZE(cx + size.cx, cy + size.cy);
	}

	inline _DFSIZE _DFSIZE::operator-(_DFSIZE size) const throw()
	{
		return _DFSIZE(cx - size.cx, cy - size.cy);
	}

	inline _DFSIZE _DFSIZE::operator-() const throw()
	{
		return _DFSIZE(-cx, -cy);
	}

	inline _DFPOINT _DFSIZE::operator+(_DFPOINT point) const throw()
	{
		return _DFPOINT(cx + point.x, cy + point.y);
	}

	inline _DFPOINT _DFSIZE::operator-(_DFPOINT point) const throw()
	{
		return _DFPOINT(cx - point.x, cy - point.y);
	}

	// _DFPOINT
	inline _DFPOINT::_DFPOINT() throw()
	{
		x = 0;
		y = 0;
	}

	inline _DFPOINT::_DFPOINT(float initX,float initY) throw()
	{
		x = initX;
		y = initY;
	}
	/*
	inline _DFPOINT::_DFPOINT(long initX, long initY) throw()
	{
		x = (float)initX;
		y = (float)initY;
	}
	*/
	inline _DFPOINT::_DFPOINT(POINT initPt) throw()
	{
		//*(_DFPOINT*)this = initPt;
		x = (float)initPt.x;
		y = (float)initPt.y;
	}
	inline _DFPOINT::_DFPOINT(POINT* initPt) throw()
	{
		x = (float)initPt->x;
		y = (float)initPt->y;
	}
	inline _DFPOINT::_DFPOINT(dk::LPDPOINT initPt) throw()
	{
		x = (float)initPt->x;
		y = (float)initPt->y;
	}
	inline _DFPOINT::_DFPOINT(_DFPOINT* initPt) throw()
	{
		x = (float)initPt->x;
		y = (float)initPt->y;
	}

	inline _DFPOINT::_DFPOINT(_DFSIZE initSize) throw()
	{
		*(_DFSIZE*)this = initSize;
	}

	inline float _DFPOINT::GetX()
	{
		return(x);
	}
	inline float _DFPOINT::GetY()
	{
		return(y);
	}
	inline void _DFPOINT::Set(float _x, float _y)
	{
		x = _x;
		y = _y;
	}
	inline void _DFPOINT::Set(POINT _pt)
	{
		x = (float)_pt.x;
		y = (float)_pt.y;
	}
	inline void _DFPOINT::SetX(float _x)
	{
		x = _x;
	}
	inline void _DFPOINT::SetY(float _y)
	{
		y = _y;
	}

	inline void _DFPOINT::Offset(
		float xOffset,
		float yOffset) throw()
	{
		x += xOffset;
		y += yOffset;
	}

	inline void _DFPOINT::Offset(_DFPOINT point) throw()
	{
		x += point.x;
		y += point.y;
	}

	inline void _DFPOINT::Offset(_DFSIZE size) throw()
	{
		x += size.cx;
		y += size.cy;
	}

	inline void _DFPOINT::SetPoint(
		float X,
		float Y) throw()
	{
		x = X;
		y = Y;
	}
	inline _DFPOINT* _DFPOINT::operator=(POINT* _pPoint) throw()
	{
		x = (float)_pPoint->x;
		y = (float)_pPoint->y;
		return(this);
	}
	inline _DFPOINT* _DFPOINT::operator=(_DFPOINT* _pPoint) throw()
	{
		x = (float)_pPoint->x;
		y = (float)_pPoint->y;
		return(this);
	}
	inline BOOL _DFPOINT::operator==(_DFPOINT point) const throw()
	{
		return (x == point.x && y == point.y);
	}

	inline BOOL _DFPOINT::operator!=(_DFPOINT point) const throw()
	{
		return (x != point.x || y != point.y);
	}

	inline void _DFPOINT::operator+=(_DFSIZE size) throw()
	{
		x += size.cx;
		y += size.cy;
	}

	inline void _DFPOINT::operator-=(_DFSIZE size) throw()
	{
		x -= size.cx;
		y -= size.cy;
	}

	inline void _DFPOINT::operator+=(_DFPOINT point) throw()
	{
		x += point.x;
		y += point.y;
	}

	inline void _DFPOINT::operator-=(_DFPOINT point) throw()
	{
		x -= point.x;
		y -= point.y;
	}

	inline _DFPOINT _DFPOINT::operator+(_DFSIZE size) const throw()
	{
		return _DFPOINT(x + size.cx, y + size.cy);
	}
	inline _DFPOINT _DFPOINT::operator+(_DFPOINT point) const throw()
	{
		return _DFPOINT(x + point.x, y + point.y);
	}
	inline _DFPOINT _DFPOINT::operator+(_DFPOINT* point) const throw()
	{
		return _DFPOINT(x + point->x, y + point->y);
	}
	inline _DFPOINT _DFPOINT::operator-(_DFSIZE size) const throw()
	{
		return _DFPOINT(x - size.cx, y - size.cy);
	}

	inline _DFPOINT _DFPOINT::operator-() const throw()
	{
		return _DFPOINT(-x, -y);
	}
	inline _DFPOINT _DFPOINT::operator-(_DFPOINT point) const throw()
	{
		return _DFPOINT(x - point.x, y - point.y);
	}

	// _DFRECT
	inline _DFRECT::_DFRECT() throw()
	{
		left = 0;
		top = 0;
		right = 0;
		bottom = 0;
	}

	inline _DFRECT::_DFRECT(
		float l,
		float t,
		float r,
		float b) throw()
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}

	inline _DFRECT::_DFRECT(
		long l,
		long t,
		long r,
		long b) throw()
	{
		left = (float)l;
		top = (float)t;
		right = (float)r;
		bottom = (float)b;
	}
	inline _DFRECT::_DFRECT(const dk::LPDRECT _pRect) throw()
	{
		left = (float)_pRect->left;
		top = (float)_pRect->top;
		right = (float)_pRect->right;
		bottom = (float)_pRect->bottom;
	}
	inline _DFRECT::_DFRECT(const _DFRECT& srcRect) throw()
	{
		left = srcRect.left;
		top = srcRect.top;
		right = srcRect.right;
		bottom = srcRect.bottom;
	}

	inline _DFRECT::_DFRECT(_DFRECT *lpSrcRect) throw()
	{
		left = lpSrcRect->left;
		top = lpSrcRect->top;
		right = lpSrcRect->right;
		bottom = lpSrcRect->bottom;
	}

	inline _DFRECT::_DFRECT(
		_DFPOINT point,
		_DFSIZE size) throw()
	{
		right = (left = point.x) + size.cx;
		bottom = (top = point.y) + size.cy;
	}

	inline _DFRECT::_DFRECT(
		_DFPOINT topLeft,
		_DFPOINT bottomRight) throw()
	{
		left = topLeft.x;
		top = topLeft.y;
		right = bottomRight.x;
		bottom = bottomRight.y;
	}

	inline float _DFRECT::Width() const throw()
	{
		return right - left;
	}

	inline float _DFRECT::Height() const throw()
	{
		return bottom - top;
	}

	inline _DFSIZE _DFRECT::Size() const throw()
	{
		return _DFSIZE(right - left, bottom - top);
	}

	inline _DFPOINT& _DFRECT::TopLeft() throw()
	{
		return *((_DFPOINT*)this);
	}

	inline _DFPOINT& _DFRECT::BottomRight() throw()
	{
		return *((_DFPOINT*)this + 1);
	}

	inline const _DFPOINT& _DFRECT::TopLeft() const throw()
	{
		return *((_DFPOINT*)this);
	}

	inline const _DFPOINT& _DFRECT::BottomRight() const throw()
	{
		return *((_DFPOINT*)this + 1);
	}

	inline _DFPOINT _DFRECT::CenterPoint() const throw()
	{
		return _DFPOINT((left + right) / 2, (top + bottom) / 2);
	}

	inline void _DFRECT::SwapLeftRight() throw()
	{
		SwapLeftRight((_DFRECT *)this);
	}

	inline void WINAPI _DFRECT::SwapLeftRight(_Inout_ _DFRECT *lpRect) throw()
	{
		float temp = lpRect->left;
		lpRect->left = lpRect->right;
		lpRect->right = temp;
	}

	inline BOOL _DFRECT::IsRectNull() const throw()
	{
		return (left == 0 && right == 0 && top == 0 && bottom == 0);
	}

	inline void _DFRECT::Set(
		float _left,
		float _top,
		float _right,
		float _bottom) throw()
	{
		left = _left;
		top = _top;
		right = _right;
		bottom = _bottom;
	}

	inline void _DFRECT::Set(
		_DFPOINT topLeft,
		_DFPOINT bottomRight) throw()
	{
		left = topLeft.x;
		top = topLeft.y;
		right = bottomRight.x;
		bottom = bottomRight.y;
	}

	inline void _DFRECT::Set(_DFRECT *_rect) throw()
	{
		left = _rect->left;
		top = _rect->top;
		right = _rect->right;
		bottom = _rect->bottom;
	}

	inline void _DFRECT::CopyRect(_DFRECT *lpSrcRect) throw()
	{
		left = lpSrcRect->left;
		top = lpSrcRect->top;
		right = lpSrcRect->right;
		bottom = lpSrcRect->bottom;
	}

	inline void _DFRECT::MoveToY(float y) throw()
	{
		bottom = Height() + y;
		top = y;
	}

	inline void _DFRECT::MoveToX(float x) throw()
	{
		right = Width() + x;
		left = x;
	}

	inline void _DFRECT::MoveToXY(
		float x,
		float y) throw()
	{
		MoveToX(x);
		MoveToY(y);
	}

	inline void _DFRECT::MoveToXY(_DFPOINT pt) throw()
	{
		MoveToX(pt.x);
		MoveToY(pt.y);
	}

	inline void _DFRECT::operator=(const _DFRECT& srcRect) throw()
	{
		left = srcRect.left;
		top = srcRect.top;
		right = srcRect.right;
		bottom = srcRect.bottom;
	}

	inline BOOL _DFRECT::operator==(const _DFRECT& rect) const throw()
	{
		if (left != rect.left
			|| top != rect.top
			|| right != rect.right
			|| bottom != rect.bottom
			)
		{
			return(false);	// == 하나라도 다르면 false 리턴
		}
		return(true);
	}

	inline BOOL _DFRECT::operator!=(const _DFRECT& rect) const throw()
	{
		if (left != rect.left
			|| top != rect.top
			|| right != rect.right
			|| bottom != rect.bottom
			)
		{
			return(true);	// != 하나라도 다르면 true 리턴
		}
		return(false);
	}

	inline void _DFRECT::operator+=(_DFRECT *lpRect) throw()
	{
		InflateRect(lpRect);
	}

	inline void _DFRECT::operator-=(_DFRECT *lpRect) throw()
	{
		DeflateRect(lpRect);
	}

	inline void _DFRECT::NormalizeRect() throw()
	{
		float nTemp;
		if (left > right)
		{
			nTemp = left;
			left = right;
			right = nTemp;
		}
		if (top > bottom)
		{
			nTemp = top;
			top = bottom;
			bottom = nTemp;
		}
	}

	inline void _DFRECT::InflateRect(_DFRECT *lpRect) throw()
	{
		left -= lpRect->left;
		top -= lpRect->top;
		right += lpRect->right;
		bottom += lpRect->bottom;
	}

	inline void _DFRECT::InflateRect(
		float l,
		float t,
		float r,
		float b) throw()
	{
		left -= l;
		top -= t;
		right += r;
		bottom += b;
	}

	inline void _DFRECT::DeflateRect(_DFRECT *lpRect) throw()
	{
		left += lpRect->left;
		top += lpRect->top;
		right -= lpRect->right;
		bottom -= lpRect->bottom;
	}

	inline void _DFRECT::DeflateRect(
		float l,
		float t,
		float r,
		float b) throw()
	{
		left += l;
		top += t;
		right -= r;
		bottom -= b;
	}
	// 2D axis aligned bounding-box
	// NB: we can't rely on dk::_DVECTOR2 math operators being available here
	struct _DV2RECT
	{
		dx9::_DVECTOR2      Min;    // Upper-left
		dx9::_DVECTOR2      Max;    // Lower-right

		_DV2RECT()
			: Min(FLT_MAX, FLT_MAX)
			, Max(-FLT_MAX, -FLT_MAX)
		{
		}
		_DV2RECT(const dx9::_DVECTOR2& min, const dx9::_DVECTOR2& max)
			: Min(min)
			, Max(max)
		{
		}
		_DV2RECT(const dx9::_DVECTOR4& v)
			:Min(v.x, v.y)
			, Max(v.z, v.w)
		{
		}
		_DV2RECT(float _left, float _top, float _right, float _bottom)
			:Min(_left, _top)
			, Max(_right, _bottom)
		{
		}

		_DV2RECT operator = (const _DV2RECT &other)
		{
			Set(other);
			return *this;
		}
		_DV2RECT operator = (_DV2RECT *other)
		{
			Set(other);
			return *this;
		}
		_DV2RECT operator = (const _DV2RECT *other)
		{
			Set(other);
			return *this;
		}

		void Set(float _left, float _top, float _right, float _bottom)
		{
			Min.x = _left;
			Min.y = _top;
			Max.x = _right;
			Max.y = _bottom;
		}
		void Set(const _DV2RECT &_vrc)
		{
			Min.x = _vrc.Min.x;
			Min.y = _vrc.Min.y;
			Max.x = _vrc.Max.x;
			Max.y = _vrc.Max.y;
		}
		void Set(_DV2RECT *_vrc)
		{
			Min.x = _vrc->Min.x;
			Min.y = _vrc->Min.y;
			Max.x = _vrc->Max.x;
			Max.y = _vrc->Max.y;
		}
		void Set(const _DV2RECT *_vrc)
		{
			Min.x = _vrc->Min.x;
			Min.y = _vrc->Min.y;
			Max.x = _vrc->Max.x;
			Max.y = _vrc->Max.y;
		}
		dx9::_DVECTOR2 GetCenter() const
		{
			return dx9::_DVECTOR2((Min.x + Max.x) * 0.5f, (Min.y + Max.y) * 0.5f);
		}
		dx9::_DVECTOR2 GetSize() const
		{
			return dx9::_DVECTOR2(Max.x - Min.x, Max.y - Min.y);
		}
		float GetWidth() const
		{
			return Max.x - Min.x;
		}
		float GetHeight() const
		{
			return Max.y - Min.y;
		}
		dx9::_DVECTOR2 GetTL() const	// Top-Min.x
		{
			return Min;
		}
		dx9::_DVECTOR2 GetTR() const	// Top-Max.x
		{
			return dx9::_DVECTOR2(Max.x, Min.y);
		}
		dx9::_DVECTOR2 GetBL() const	// Bottom-Min.x
		{
			return dx9::_DVECTOR2(Min.x, Max.y);
		}
		dx9::_DVECTOR2 GetBR() const	// Bottom-Max.x
		{
			return Max;
		}
		bool Contains(const dx9::_DVECTOR2& p) const
		{
			return p.x >= Min.x && p.y >= Min.y && p.x < Max.x && p.y < Max.y;
		}
		bool Contains(const _DV2RECT& r) const
		{
			return r.Min.x >= Min.x && r.Min.y >= Min.y && r.Max.x <= Max.x && r.Max.y <= Max.y;
		}
		bool Overlaps(const _DV2RECT& r) const
		{
			return r.Min.y <  Max.y && r.Max.y >  Min.y && r.Min.x <  Max.x && r.Max.x >  Min.x;
		}
		void Add(const dx9::_DVECTOR2& p)
		{
			if (Min.x > p.x)
				Min.x = p.x;

			if (Min.y > p.y)
				Min.y = p.y;

			if (Max.x < p.x)
				Max.x = p.x;

			if (Max.y < p.y)
				Max.y = p.y;
		}
		void Add(const _DV2RECT& r)
		{
			if (Min.x > r.Min.x)
				Min.x = r.Min.x;

			if (Min.y > r.Min.y)
				Min.y = r.Min.y;

			if (Max.x < r.Max.x)
				Max.x = r.Max.x;

			if (Max.y < r.Max.y)
				Max.y = r.Max.y;
		}
		void Expand(const float amount)
		{
			Min.x -= amount;
			Min.y -= amount;
			Max.x += amount;
			Max.y += amount;
		}
		void Expand(const dx9::_DVECTOR2& amount)
		{
			Min.x -= amount.x;
			Min.y -= amount.y;
			Max.x += amount.x;
			Max.y += amount.y;
		}
		void Translate(const dx9::_DVECTOR2& d)
		{
			Min.x += d.x;
			Min.y += d.y;
			Max.x += d.x;
			Max.y += d.y;
		}
		void TranslateX(float dx)
		{
			Min.x += dx;
			Max.x += dx;
		}
		void TranslateY(float dy)
		{
			Min.y += dy;
			Max.y += dy;
		}
		void ClipWith(const _DV2RECT& r)
		{
			Min = v2Max(Min, r.Min);
			Max = v2Min(Max, r.Max);
		}                   // Simple version, may lead to an inverted rectangle, which is fine for Contains/Overlaps test but not for display.
		void ClipWithFull(const _DV2RECT& r)
		{
			Min = v2Clamp(Min, r.Min, r.Max);
			Max = v2Clamp(Max, r.Min, r.Max);
		} // Full version, ensure both points are fully clipped.

		// 각 좌표값을 정수로 내림
		// 렌더링 시 픽셀 정렬을 위해 사용
		void Floor()
		{
			Min.x = floorf(Min.x);
			Min.y = floorf(Min.y);
			Max.x = floorf(Max.x);
			Max.y = floorf(Max.y);
		}

		// 각 좌표값을 정수로 올림
		// 렌더링 시 픽셀 정렬을 위해 사용
		void Ceil()
		{
			Min.x = ceilf(Min.x);
			Min.y = ceilf(Min.y);
			Max.x = ceilf(Max.x);
			Max.y = ceilf(Max.y);
		}

		// 각 좌표값을 반올림
		// 렌더링 시 픽셀 정렬을 위해 사용
		void Round()
		{
			Min.x = floorf(Min.x + 0.5f);
			Min.y = floorf(Min.y + 0.5f);
			Max.x = floorf(Max.x + 0.5f);
			Max.y = floorf(Max.y + 0.5f);
		}

		// 사각형이 역전되었는지 확인 (Min > Max)
		bool IsInverted() const
		{
			return Min.x > Max.x || Min.y > Max.y;
		}

		// 사각형이 비어있는지 확인 (너비 또는 높이가 0 이하)
		bool IsEmpty() const
		{
			return Min.x >= Max.x || Min.y >= Max.y;
		}

		// 사각형을 기본값(FLT_MAX, -FLT_MAX)으로 초기화
		void Reset()
		{
			Min.x = Min.y = FLT_MAX;
			Max.x = Max.y = -FLT_MAX;
		}

	};

	enum _DX9_QUERY_FEATURE_TYPE_
	{
		RQF_HARDWARETNL = 0,
		RQF_USERCLIPPLANE,
		RQF_WFOG,
		RQF_VS11,
		RQF_VS20,
		RQF_PS10,
		RQF_PS11,
		RQF_PS20,
		RQF_PS30,
		RQF_R32F,
		RQF_A32B32G32R32F,
		RQF_A16B16G16R16F,
		RQF_R16F,
		RQF_RGB16,
		RQF_G16R16F,
		RQF_G32R32F,
		RQF_VERTEXTEXTURE,				// VS30의 Vertex Texture 기능이 지원 되는가
		RQF_HWSHADOWMAP,				// 새도우맵에서 하드웨어 PCF (Percentage Closer Filtering) 샘플링 지원
		RQF_MRTINDEPENDENTBITDEPTHS,	// 일부 카드에서 R32F 포멧에 대해 POST PIXEL SHADER BLENDING(디더링, 알파 테스트, 포그(안개:fog), 블렌드, 래스터 처리, 마스킹)이 안먹힘. 그에대한 지원 유무 http://telnet.or.kr/sec_directx/index.html?init_mode=api_contents_read&api_no=60
		// 텍스쳐 필터 지원 여부
		// To check if a format supports texture filter types other than D3DTEXF_POINT (which is always supported), call IDirect3D9::CheckDeviceFormat with D3DUSAGE_QUERY_FILTER.
		RQF_RGB16_RTF,
		RQF_R32F_RTF,
		RQF_A8R8G8B8_RTF,					// RFMT_A8R8G8B8
		RQF_A32B32G32R32F_RTF,
		RQF_A16B16G16R16F_RTF,
		RQF_R16F_RTF,
		RQF_G32R32F_RTF,
		RQF_MRTBLEND_R32F,
		RQF_MRTBLEND_G16R16F,
		RQF_MRTBLEND_A8R8G8B8,
	};
}
