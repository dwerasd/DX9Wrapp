#pragma once


#include <DarkCore/DMemory.h>



namespace dx9
{
	// 1D vector (this odd construct is used to facilitate the transition between 1D and 2D, and the maintenance of some branches/patches)
	struct _DVECTOR1
		//: public dk::C_ALIGNED_ALLOCATION_POLICYT<16>
	{
		float   x;
		_DVECTOR1()
			: x(0.0f)
		{
		}
		_DVECTOR1(float _x)
			: x(_x)
		{
		}
	};

}