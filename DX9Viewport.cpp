#include "framework.h"
#include "DX9Viewport.h"



namespace dx9
{

	_DVIEWPORT9 _DVIEWPORT9::operator = (_DVIEWPORT9 other)
	{
		Set(other);
		return *this;
	}
	_DVIEWPORT9 _DVIEWPORT9::operator = (_DVIEWPORT9 *other)
	{
		Set(other);
		return *this;
	}
	_DVIEWPORT9 _DVIEWPORT9::operator = (DVIEWPORT other)
	{
		Set(other);
		return (*this);
	}
	_DVIEWPORT9 _DVIEWPORT9::operator = (DVIEWPORT *other)
	{
		Set(other);
		return(*this);
	}
}
