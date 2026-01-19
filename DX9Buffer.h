#pragma once


#include <DarkCore/DDef.h>

#include "DX9Def.h"
#include "DX9Vector2.h"
#include "DX9Vector3.h"
#include "DX9Vector4.h"


inline constexpr size_t MAX_VERTEXSTREAM = 4;
inline constexpr size_t MAX_RENDERTARGET = 4;
namespace dx9
{
	struct _DX9_VERTEX_FORMAT
	{
		LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration;
		UINT arrVertexSize[MAX_VERTEXSTREAM];
	};

	struct _DX9_VERTEX_BUFFER
		//: public IDirect3DVertexBuffer9
	{
		LPDIRECT3DVERTEXBUFFER9 pVertexBuffer{ nullptr };
		UINT nCreateSize{ 0 };
		UINT nStructSize{ 0 };
		DWORD dwFVF{ 0 };
		DWORD dwUsage{ 0 };
		D3DPOOL d3dPool{ D3DPOOL_MANAGED };		// 매니저는 자동관리라서 디폴트로 한다

		_DX9_VERTEX_BUFFER() = default;
		_DX9_VERTEX_BUFFER(UINT _nStructSize, DWORD _nCreateCount, DWORD _dwFVF = 0, DWORD _dwFlags = 0);

		bool Create(LPDIRECT3DDEVICE9 _pDevice);
		void Release();
		void OnLostDevice();
		bool IsDynamic();
		LPVOID* Lock(UINT _nLockSize, UINT _nOffetToLock = 0);
		void Unlock();
	};

	struct _DX9_INDEX_BUFFER
	{
		LPDIRECT3DINDEXBUFFER9 pIndexBuffer{ nullptr };
		DWORD nCreateCount{ 0 };
		UINT nIndexSize{ 0 };
		DWORD dwUsage{ 0 };
		D3DPOOL d3dPool{ D3DPOOL_MANAGED };	// 매니저는 자동관리라서 디폴트로 한다

		_DX9_INDEX_BUFFER() = default;
		_DX9_INDEX_BUFFER(UINT _nIndexSize, DWORD _nIndices, DWORD _dwFlags = 0);

		bool Create(LPDIRECT3DDEVICE9 _pDevice);
		void Release();
		bool IsDynamic();
		void OnLostDevice();
		LPVOID* Lock(UINT _nLockSize, UINT _nOffetToLock = 0);
		void Unlock();
	};

	struct _VERTEX_NO_TEX_RHW
	{
		union
		{
			struct
			{
				float x, y, z, rhw;
			};
			float pos[4];
			dx9::_DVECTOR4 v4;
		};
		DWORD dwColor{ 0xFFFFFFFF };

		_VERTEX_NO_TEX_RHW();
		_VERTEX_NO_TEX_RHW(float _x, float _y, DWORD _color = 0xFFFFFFFF);
		_VERTEX_NO_TEX_RHW(float _x, float _y, float _z, DWORD _color = 0xFFFFFFFF);
		_VERTEX_NO_TEX_RHW(float _x, float _y, float _z, float _rhw, DWORD _color = 0xFFFFFFFF);
	};

#define _D3DFVF_NO_TEX_RHW_	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

	struct _VERTEX_TEX_RHW
		: public _VERTEX_NO_TEX_RHW
	{
		union
		{
			struct
			{
				float u, v;
			};
			float uv[2];
			dx9::_DVECTOR2 v2;
		};

		_VERTEX_TEX_RHW();
		_VERTEX_TEX_RHW(float _x, float _y, float _u, float _v);
		_VERTEX_TEX_RHW(float _x, float _y, DWORD _color);
		_VERTEX_TEX_RHW(float _x, float _y, DWORD _color, float _u, float _v);
		_VERTEX_TEX_RHW(float _x, float _y, float _z, DWORD _color, float _u, float _v);
		_VERTEX_TEX_RHW(float _x, float _y, float _z, float _rhw, DWORD _color, float _u, float _v);

		_VERTEX_TEX_RHW* operator=(_In_ _VERTEX_TEX_RHW* _pPoint) throw();
		_VERTEX_TEX_RHW* operator=(_In_ const _VERTEX_TEX_RHW& _pPoint) throw();
	};

	inline _VERTEX_TEX_RHW* _VERTEX_TEX_RHW::operator=(_In_ _VERTEX_TEX_RHW* _pPoint) throw()
	{
		v4.Set(_pPoint->v4);
		v2.Set(_pPoint->v2);

		//v4 = _pPoint->v4;
		//v2 = _pPoint->v2;
		return(this);
	}
	inline _VERTEX_TEX_RHW* _VERTEX_TEX_RHW::operator=(_In_ const _VERTEX_TEX_RHW& _pPoint) throw()
	{
		v4.Set(_pPoint.v4);
		v2.Set(_pPoint.v2);

		//v4 = _pPoint.v4;
		//v2 = _pPoint.v2;
		return(this);
	}
#define _D3DFVF_TEX_RHW_	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

	struct _VERTEX_NO_TEX
	{
		union
		{
			struct
			{
				float x, y, z;
			};
			float pos[3];
			dx9::_DVECTOR3 v3;
		};
		DWORD dwColor;

		_VERTEX_NO_TEX();
		_VERTEX_NO_TEX(float _x, float _y, DWORD _color = 0xFFFFFFFF);
		_VERTEX_NO_TEX(float _x, float _y, float _z, DWORD _color = 0xFFFFFFFF);
		
	};

#define _D3DFVF_NO_TEX_	(D3DFVF_XYZ | D3DFVF_DIFFUSE)

	struct _VERTEX_TEX
		: public _VERTEX_NO_TEX
	{
		union
		{
			struct
			{
				float u, v;
			};
			float uv[2];
			dx9::_DVECTOR2 v2;
		};

		_VERTEX_TEX();
		_VERTEX_TEX(float _x, float _y, float _u, float _v);
		_VERTEX_TEX(float _x, float _y, DWORD _color = 0xFFFFFFFF);
		_VERTEX_TEX(float _x, float _y, DWORD _color, float _u, float _v);
		_VERTEX_TEX(float _x, float _y, float _z, DWORD _color, float _u, float _v);

		void operator=(const _VERTEX_TEX* _src) throw();
		void operator=(const _VERTEX_TEX& _src) throw();

	};

	inline void _VERTEX_TEX::operator=(const _VERTEX_TEX* _src) throw()
	{
		memcpy(this, _src, sizeof(_VERTEX_TEX));
	}
	inline void _VERTEX_TEX::operator=(const _VERTEX_TEX& _src) throw()
	{
		memcpy(this, &_src, sizeof(_VERTEX_TEX));
	}
#define _D3DFVF_TEX_	(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

	struct _INDEX_LINE
	{
		WORD _0, _1;

		_INDEX_LINE();
		_INDEX_LINE(WORD _n0, WORD _n1);
	};

	struct _INDEX_TRIANGLE
		: public _INDEX_LINE
	{
		WORD _2;

		_INDEX_TRIANGLE();
		_INDEX_TRIANGLE(WORD _n0, WORD _n1, WORD _n2);
	};

}