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
	typedef struct _DX9_VERTEX_FORMAT
	{
		LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration;
		UINT arrVertexSize[MAX_VERTEXSTREAM];
	} DX9_VERTEX_FORMAT, *LPDX9_VERTEX_FORMAT;

	typedef struct _DX9_VERTEX_BUFFER
		//: public IDirect3DVertexBuffer9
	{
		LPDIRECT3DVERTEXBUFFER9 pVertexBuffer;
		UINT nCreateSize;
		UINT nStructSize;
		DWORD dwFVF;
		DWORD dwUsage;
		D3DPOOL d3dPool;

		_DX9_VERTEX_BUFFER();
		_DX9_VERTEX_BUFFER(UINT _nStructSize, DWORD _nCreateCount, DWORD _dwFVF = 0, DWORD _dwFlags = 0);
			
		bool Create(LPDIRECT3DDEVICE9 _pDevice);
		void Release();
		void OnLostDevice();
		bool IsDynamic();
		LPVOID* Lock(UINT _nLockSize, UINT _nOffetToLock = 0);
		void Unlock();
	} DX9_VERTEX_BUFFER, *LPDX9_VERTEX_BUFFER;

	typedef struct _DX9_INDEX_BUFFER
	{
		LPDIRECT3DINDEXBUFFER9 pIndexBuffer;
		DWORD nCreateCount;
		UINT nIndexSize;
		DWORD dwUsage;
		D3DPOOL d3dPool;

		_DX9_INDEX_BUFFER();
		_DX9_INDEX_BUFFER(UINT _nIndexSize, DWORD _nIndices, DWORD _dwFlags = 0);
		
		bool Create(LPDIRECT3DDEVICE9 _pDevice);
		void Release();
		bool IsDynamic();
		void OnLostDevice();
		LPVOID* Lock(UINT _nLockSize, UINT _nOffetToLock = 0);
		void Unlock();
	} DX9_INDEX_BUFFER, *LPDX9_INDEX_BUFFER;

	typedef struct _VERTEX_NO_TEX_RHW
	{
		union
		{
			struct
			{
				float x, y, z, rhw;
			};
			float pos[4];
			dx9::DVECTOR4 v4;
		};
		unsigned long dwColor;

		_VERTEX_NO_TEX_RHW();
		_VERTEX_NO_TEX_RHW(float _x, float _y, unsigned long _color = 0xFFFFFFFF);
		_VERTEX_NO_TEX_RHW(float _x, float _y, float _z, unsigned long _color = 0xFFFFFFFF);
		_VERTEX_NO_TEX_RHW(float _x, float _y, float _z, float _rhw, unsigned long _color = 0xFFFFFFFF);
	} VERTEX_NO_TEX_RHW, * LPVERTEX_NO_TEX_RHW;

#define _D3DFVF_NO_TEX_RHW_	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

	typedef struct _VERTEX_TEX_RHW
		: public _VERTEX_NO_TEX_RHW
	{
		union
		{
			struct
			{
				float u, v;
			};
			float uv[2];
			dx9::DVECTOR2 v2;
		};

		_VERTEX_TEX_RHW();
		_VERTEX_TEX_RHW(float _x, float _y, float _u, float _v);
		_VERTEX_TEX_RHW(float _x, float _y, DWORD _color);
		_VERTEX_TEX_RHW(float _x, float _y, DWORD _color, float _u, float _v);
		_VERTEX_TEX_RHW(float _x, float _y, float _z, DWORD _color, float _u, float _v);
		_VERTEX_TEX_RHW(float _x, float _y, float _z, float _rhw, DWORD _color, float _u, float _v);

		_VERTEX_TEX_RHW* operator=(_In_ _VERTEX_TEX_RHW* _pPoint) throw();
		_VERTEX_TEX_RHW* operator=(_In_ const _VERTEX_TEX_RHW& _pPoint) throw();
	} VERTEX_TEX_RHW, *LPVERTEX_TEX_RHW;

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

	typedef struct _VERTEX_NO_TEX
	{
		union
		{
			struct
			{
				float x, y, z;
			};
			float pos[3];
			dx9::DVECTOR3 v3;
		};
		unsigned long dwColor;

		_VERTEX_NO_TEX();
		_VERTEX_NO_TEX(float _x, float _y, unsigned long _color = 0xFFFFFFFF);
		_VERTEX_NO_TEX(float _x, float _y, float _z, unsigned long _color = 0xFFFFFFFF);
		
	} VERTEX_NO_TEX, * LPVERTEX_NO_TEX;

#define _D3DFVF_NO_TEX_	(D3DFVF_XYZ | D3DFVF_DIFFUSE)

	typedef struct _VERTEX_TEX
		: public _VERTEX_NO_TEX
	{
		union
		{
			struct
			{
				float u, v;
			};
			float uv[2];
			dx9::DVECTOR2 v2;
		};

		_VERTEX_TEX();
		_VERTEX_TEX(float _x, float _y, float _u, float _v);
		_VERTEX_TEX(float _x, float _y, DWORD _color = 0xFFFFFFFF);
		_VERTEX_TEX(float _x, float _y, DWORD _color, float _u, float _v);
		_VERTEX_TEX(float _x, float _y, float _z, DWORD _color, float _u, float _v);

		void operator=(const _VERTEX_TEX* _src) throw();
		void operator=(const _VERTEX_TEX& _src) throw();

	} VERTEX_TEX, * LPVERTEX_TEX;

	inline void _VERTEX_TEX::operator=(const _VERTEX_TEX* _src) throw()
	{
		memcpy(this, _src, sizeof(_VERTEX_TEX));
	}
	inline void _VERTEX_TEX::operator=(const _VERTEX_TEX& _src) throw()
	{
		memcpy(this, &_src, sizeof(_VERTEX_TEX));
	}
#define _D3DFVF_TEX_	(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

	typedef struct _INDEX_LINE
	{
		unsigned short _0, _1;

		_INDEX_LINE();
		_INDEX_LINE(unsigned short _n0, unsigned short _n1);
	} INDEX_LINE, * LPINDEX_LINE;

	typedef struct _INDEX_TRIANGLE
		: public _INDEX_LINE
	{
		unsigned short _2;

		_INDEX_TRIANGLE();
		_INDEX_TRIANGLE(unsigned short _n0, unsigned short _n1, unsigned short _n2);
	} INDEX_TRIANGLE, * LPINDEX_TRIANGLE;

}