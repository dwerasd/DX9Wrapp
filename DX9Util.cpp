#include "framework.h"
#include "DX9Util.h"



namespace dx9
{
	bool CursorInArea(dk::LPDPOINT _pPos, LPDFRECT _pRect)
	{	// 모두 만족하면 true 리턴
		dk::DRECT rc((int)_pRect->left, (int)_pRect->top, (int)_pRect->right, (int)_pRect->bottom);
		return (_pPos->x >= rc.left					// X 좌표가 영역의 좌측보다 크거나 같다
			&& _pPos->x <= (rc.left + rc.right)	// X 좌표가 영역의 우측보다 작거나 같다
			&& _pPos->y >= rc.top						// Y 좌표가 상단보다 크거나 같다
			&& _pPos->y <= (rc.top + rc.bottom)	// Y 좌표가 하단보다 작거나 같다
			);
	}
}
