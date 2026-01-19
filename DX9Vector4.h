#pragma once


#include <Windows.h>

#include <DarkCore/DMemory.h>

#include "DX9Def.h"



namespace dx9
{
	struct _DVECTOR4
		: public D3DXVECTOR4
	{
		_DVECTOR4();
		_DVECTOR4(float _fx, float _fy, float _fz, float _fw);

		_DVECTOR4 operator = (_DVECTOR4* other);
		_DVECTOR4 operator = (D3DXVECTOR4 other);
		_DVECTOR4 operator = (D3DXVECTOR4* other);

		_DVECTOR4& operator*=(const float _v) { x *= _v; y *= _v; z *= _v; w *= _v; return *this; }
		_DVECTOR4 operator/(const float _v) const { const float i = (float)1.0 / _v; return _DVECTOR4(x * i, y * i, z * i, w * i); }
		_DVECTOR4& operator/=(const float _v) { const float i = (float)1.0 / _v; x *= i; y *= i; z *= i; w *= i; return *this; }
		_DVECTOR4& operator += (const _DVECTOR4& _v) { x += _v.x; y += _v.y; z += _v.z; w += _v.w; return *this; }
		_DVECTOR4& operator -= (const _DVECTOR4& _v) { x -= _v.x; y -= _v.y; z -= _v.z; w -= _v.w; return *this; }
		bool operator!=(const _DVECTOR4& other) const { return other.x != x || other.y != y || other.z != z || other.w != w; }

		void Set(float _fx, float _fy, float _fz, float _fw) { x = _fx; y = _fy; z = _fz; w = _fw; }
		void Set(_DVECTOR4* _v) { Set(_v->x, _v->y, _v->z, _v->w); }
		void Set(_DVECTOR4 _v) { Set(&_v); }
		void Set(D3DXVECTOR4 _v) { Set((_DVECTOR4*)&_v); }

		void Clear() noexcept { x = y = z = w = 0.0f; }
		bool Empty() const noexcept { return(x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f); }
		float Length() const noexcept;
		float LengthSq() const noexcept { return x * x + y * y + z * z + w * w; }

		// 두 벡터 간 내적 계산 (const 참조 버전)
		// @param _other: 대상 벡터
		// @return: 내적 결과
		float Dot(const _DVECTOR4& _other) const noexcept { return x * _other.x + y * _other.y + z * _other.z + w * _other.w; }

		float DotProduct(_DVECTOR4* other);
		_DVECTOR4* CrossProduct(_DVECTOR4* pOutput, _DVECTOR4* p);
		_DVECTOR4* GetInterpolated(_DVECTOR4* pOutput, _DVECTOR4* other, float d);

		// 선형 보간 (Lerp) - 정적 함수 버전
		// @param _from: 시작 벡터
		// @param _to: 끝 벡터
		// @param _t: 보간 계수 (0.0 ~ 1.0)
		// @return: 보간된 벡터
		static _DVECTOR4 Lerp(const _DVECTOR4& _from, const _DVECTOR4& _to, float _t) noexcept;

		// 벡터 정규화 (자기 자신을 수정)
		// @return: 정규화된 자기 자신의 포인터
		_DVECTOR4* Normalize() noexcept;

		// 정규화된 벡터 반환 (자기 자신은 수정하지 않음)
		// @return: 정규화된 새 벡터
		_DVECTOR4 GetNormalized() const noexcept;

	};
}

inline dx9::_DVECTOR4 operator+(const dx9::_DVECTOR4& lhs, const dx9::_DVECTOR4& rhs) { return dx9::_DVECTOR4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
inline dx9::_DVECTOR4 operator-(const dx9::_DVECTOR4& lhs, const dx9::_DVECTOR4& rhs) { return dx9::_DVECTOR4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
inline dx9::_DVECTOR4 operator*(const dx9::_DVECTOR4& lhs, const dx9::_DVECTOR4& rhs) { return dx9::_DVECTOR4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
