#include "framework.h"
#include "DX9Buffer.h"



namespace dx9
{
	_DX9_VERTEX_BUFFER::_DX9_VERTEX_BUFFER(UINT _nStructSize, DWORD _nCreateCount, DWORD _dwFVF, DWORD _dwFlags)
		: nCreateSize(_nStructSize* _nCreateCount)
		, nStructSize(_nStructSize)
		, dwFVF(_dwFVF)
		, dwUsage(_dwFlags)
	{
		d3dPool = DIS_SET(dwUsage, D3DUSAGE_DYNAMIC) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
	}
	bool _DX9_VERTEX_BUFFER::Create(LPDIRECT3DDEVICE9 _pDevice)
	{
		const HRESULT hResult = _pDevice->CreateVertexBuffer(
			nCreateSize
			, dwUsage
			, dwFVF
			, d3dPool
			, &pVertexBuffer
			, nullptr
		);
		return hResult == D3D_OK;
	}

	void _DX9_VERTEX_BUFFER::Release()
	{
		DSAFE_RELEASE(pVertexBuffer);
	}

	void _DX9_VERTEX_BUFFER::OnLostDevice()
	{
		if (DIS_SET(dwUsage, D3DUSAGE_DYNAMIC))
		{
			DSAFE_RELEASE(pVertexBuffer);
		}
	}

	bool _DX9_VERTEX_BUFFER::IsDynamic()
	{
		return DIS_SET(dwUsage, D3DUSAGE_DYNAMIC);
	}

	LPVOID* _DX9_VERTEX_BUFFER::Lock(UINT _nLockSize, UINT _nOffetToLock)
	{
		LPVOID* pBuffer = nullptr;
		pVertexBuffer->Lock(
			_nOffetToLock
			, _nLockSize
			, (LPVOID*)&pBuffer
			, DIS_SET(dwUsage, D3DUSAGE_DYNAMIC) ? D3DLOCK_DISCARD : 0
		);
		return(pBuffer);
	}

	void _DX9_VERTEX_BUFFER::Unlock()
	{
		pVertexBuffer->Unlock();
	}

	_DX9_INDEX_BUFFER::_DX9_INDEX_BUFFER(UINT _nIndexSize, DWORD _nCreateCount, DWORD _dwFlags)
		: nCreateCount(_nCreateCount)
		  , nIndexSize(_nIndexSize)
		  , dwUsage(_dwFlags)
	{
		d3dPool = DIS_SET(dwUsage, D3DUSAGE_DYNAMIC) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
	}
	bool _DX9_INDEX_BUFFER::Create(LPDIRECT3DDEVICE9 _pDevice)
	{
		//DBGPRINT("인덱스버퍼 사이즈: %i ", nIndices * nIndexSize);
		if (_pDevice->CreateIndexBuffer(
			nIndexSize * nCreateCount
			, dwUsage
			, nIndexSize == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32
			, d3dPool
			, &pIndexBuffer
			, nullptr
		) != D3D_OK)
		{
			return false;
		}
		return true;
	}

	void _DX9_INDEX_BUFFER::Release()
	{
		DSAFE_RELEASE(pIndexBuffer);
	}
	bool _DX9_INDEX_BUFFER::IsDynamic()
	{
		return DIS_SET(dwUsage, D3DUSAGE_DYNAMIC);
	}
	void _DX9_INDEX_BUFFER::OnLostDevice()
	{
		if (DIS_SET(dwUsage, D3DUSAGE_DYNAMIC))
		{
			DSAFE_RELEASE(pIndexBuffer);
		}
	}
	LPVOID* _DX9_INDEX_BUFFER::Lock(UINT _nLockSize, UINT _nOffetToLock)
	{
		LPVOID* pBuffer = nullptr;
		pIndexBuffer->Lock(
			_nOffetToLock
			, _nLockSize
			, (LPVOID*)&pBuffer
			, DIS_SET(dwUsage, D3DUSAGE_DYNAMIC) ? D3DLOCK_DISCARD : 0
		);
		return(pBuffer);
	}

	void _DX9_INDEX_BUFFER::Unlock()
	{
		pIndexBuffer->Unlock();
	}

	_VERTEX_NO_TEX_RHW::_VERTEX_NO_TEX_RHW()
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
		, rhw(1.0f)
	{
	}
	_VERTEX_NO_TEX_RHW::_VERTEX_NO_TEX_RHW(float _x, float _y, DWORD _color)
		: x(_x)
		, y(_y)
		, z(0.0f)
		, rhw(1.0f)
		, dwColor(_color)
	{
	}
	_VERTEX_NO_TEX_RHW::_VERTEX_NO_TEX_RHW(float _x, float _y, float _z, DWORD _color)
		: x(_x)
		, y(_y)
		, z(_z)
		, rhw(1.0f)
		, dwColor(_color)
	{
	}
	_VERTEX_NO_TEX_RHW::_VERTEX_NO_TEX_RHW(float _x, float _y, float _z, float _rhw, DWORD _color)
		: x(_x)
		, y(_y)
		, z(_z)
		, rhw(_rhw)
		, dwColor(_color)
	{

	}
	_VERTEX_TEX_RHW::_VERTEX_TEX_RHW()
		: u(0.0f)
		  , v(0.0f)
	{
	}
	_VERTEX_TEX_RHW::_VERTEX_TEX_RHW(float _x, float _y, float _u, float _v)
		: _VERTEX_NO_TEX_RHW(_x, _y)
		, u(_u)
		, v(_v)
	{
		//v4.Set(_x, _y, 0.0f, 1.0f);
		//dwColor = 0xFFFFFFFF;
	}
	_VERTEX_TEX_RHW::_VERTEX_TEX_RHW(float _x, float _y, DWORD _color)
		: _VERTEX_NO_TEX_RHW(_x, _y, _color)
		, u(0.0f)
		, v(0.0f)
	{
		//v4.Set(_x, _y, 0.0f, 1.0f);
		//dwColor = _color;
	}
	_VERTEX_TEX_RHW::_VERTEX_TEX_RHW(float _x, float _y, DWORD _color, float _u, float _v)
		: _VERTEX_NO_TEX_RHW(_x, _y, _color)
		, u(_u)
		, v(_v)
	{
		//v4.Set(_x, _y, 0.0f, 1.0f);
		//dwColor = _color;
	}
	_VERTEX_TEX_RHW::_VERTEX_TEX_RHW(float _x, float _y, float _z, DWORD _color, float _u, float _v)
		: _VERTEX_NO_TEX_RHW(_x, _y, _z, _color)
		, u(_u)
		, v(_v)
	{
		//v4.Set(_x, _y, _z, 1.0f);
		//dwColor = _color;
	}
	_VERTEX_TEX_RHW::_VERTEX_TEX_RHW(float _x, float _y, float _z, float _rhw, DWORD _color, float _u, float _v)
		: _VERTEX_NO_TEX_RHW(_x, _y, _z, _rhw, _color)
		, u(_u)
		, v(_v)
	{
	}

	_VERTEX_NO_TEX::_VERTEX_NO_TEX()
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
		, dwColor(0xFFFFFFFF)
	{
	}
	_VERTEX_NO_TEX::_VERTEX_NO_TEX(float _x, float _y, DWORD _color)
		: x(_x)
		, y(_y)
		, z(0.0f)
		, dwColor(_color)
	{
	}
	_VERTEX_NO_TEX::_VERTEX_NO_TEX(float _x, float _y, float _z, DWORD _color)
		: x(_x)
		, y(_y)
		, z(_z)
		, dwColor(_color)
	{
	}

	_VERTEX_TEX::_VERTEX_TEX()
		: u(0.0f)
		  , v(0.0f)
	{
	}
	_VERTEX_TEX::_VERTEX_TEX(float _x, float _y, float _u, float _v)
		: u(_u)
		, v(_v)
	{
		v3.Set(_x, _y);
		dwColor = 0xFFFFFFFF;
	}
	_VERTEX_TEX::_VERTEX_TEX(float _x, float _y, DWORD _color)
		: u(0.0f)
		, v(0.0f)
	{
		v3.Set(_x, _y);
		dwColor = _color;
	}
	_VERTEX_TEX::_VERTEX_TEX(float _x, float _y, DWORD _color, float _u, float _v)
		: u(_u)
		, v(_v)
	{
		v3.Set(_x, _y);
		dwColor = _color;
	}
	_VERTEX_TEX::_VERTEX_TEX(float _x, float _y, float _z, DWORD _color, float _u, float _v)
		: u(_u)
		, v(_v)
	{
		v3.Set(_x, _y, _z);
		dwColor = _color;
	}

	_INDEX_LINE::_INDEX_LINE()
		: _0(0)
		, _1(1)
	{
	}
	_INDEX_LINE::_INDEX_LINE(WORD _n0, WORD _n1)
		: _0(_n0)
		, _1(_n1)
	{
	}
	/*
	void _INDEX_LINE::Set(WORD _n0, WORD _n1)
	{
		_0 = _n0;
		_1 = _n1;
	}
	*/
	_INDEX_TRIANGLE::_INDEX_TRIANGLE()
		: _2(2)
	{
	}
	_INDEX_TRIANGLE::_INDEX_TRIANGLE(WORD _n0, WORD _n1, WORD _n2)
		: _2(_n2)
	{
		_0 = _n0;
		_1 = _n1;
	}
	/*
	void _INDEX_TRIANGLE::Set(WORD _n0, WORD _n1, WORD _n2)
	{
		_0 = _n0;
		_1 = _n1;
		_2 = _n2;
	}
	*/
}