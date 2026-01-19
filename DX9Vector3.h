#pragma once


#include <Windows.h>
#include <directxsdk/d3dx9.h>

#include <DarkCore/DMemory.h>

#include "DX9Vector2.h"



namespace dx9
{
	typedef struct _DVECTOR3
		: public D3DXVECTOR3
	{
		_DVECTOR3();
		_DVECTOR3(float fx, float fy, float fz);
		_DVECTOR3(D3DXVECTOR3 _v3);
		_DVECTOR3(const D3DXVECTOR3 &_v3);
		_DVECTOR3(_DVECTOR2 _v2);
		_DVECTOR3(const _DVECTOR2 &_v2);

		_DVECTOR3 operator+() const { return _DVECTOR3(x, y, z); }
		_DVECTOR3 operator-() const { return _DVECTOR3(-x, -y, -z); }

		_DVECTOR3 operator+(const _DVECTOR3& other) const { return _DVECTOR3(x + other.x, y + other.y, z + other.z); }
		_DVECTOR3& operator+=(const _DVECTOR3& other) { x += other.x; y += other.y; z += other.z; return *this; }

		_DVECTOR3 operator-(const _DVECTOR3& other) const { return _DVECTOR3(x - other.x, y - other.y, z - other.z); }
		_DVECTOR3& operator-=(const _DVECTOR3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }

		_DVECTOR3 operator*(const _DVECTOR3& other) const { return _DVECTOR3(x * other.x, y * other.y, z * other.z); }
		_DVECTOR3& operator*=(const _DVECTOR3& other) { x *= other.x; y *= other.y; z *= other.z; return *this; }
		_DVECTOR3 operator*(const float _v) const { return _DVECTOR3(x * _v, y * _v, z * _v); }
		_DVECTOR3& operator*=(const float _v) { x *= _v; y *= _v; z *= _v; return *this; }

		_DVECTOR3 operator/(const _DVECTOR3& other) const { return _DVECTOR3(x / other.x, y / other.y, z / other.z); }
		_DVECTOR3& operator/=(const _DVECTOR3& other) { x /= other.x; y /= other.y; z /= other.z; return *this; }
		_DVECTOR3 operator/(const float _v) const { float i = (float)1.0 / _v; return _DVECTOR3(x * i, y * i, z * i); }
		_DVECTOR3& operator/=(const float _v) { float i = (float)1.0 / _v; x *= i; y *= i; z *= i; return *this; }

		bool operator<=(const _DVECTOR3& other) const { return x <= other.x && y <= other.y && z <= other.z; };
		bool operator< (const _DVECTOR3& other) const { return x < other.x&& y < other.y&& z < other.z; };
		bool operator>=(const _DVECTOR3& other) const { return x >= other.x && y >= other.y && z >= other.z; };
		bool operator> (const _DVECTOR3& other) const { return x > other.x && y > other.y && z > other.z; };
		//bool operator==(const _DVECTOR3& other) const { return other.x==x && other.y==y && other.z==z; }
		bool operator==(const _DVECTOR3& other) { return IsEqual(other); }
		bool operator!=(const _DVECTOR3& other) const { return other.x != x || other.y != y || other.z != z; }

		_DVECTOR3 operator = (_DVECTOR3 other);
		_DVECTOR3 operator = (_DVECTOR3 *other);
		_DVECTOR3 operator = (_DVECTOR2 &other);
		_DVECTOR3 operator = (const _DVECTOR2 &other);
		_DVECTOR3 operator = (_DVECTOR2 *other);
		_DVECTOR3 operator = (const _DVECTOR2 *other);
		_DVECTOR3 operator = (D3DXVECTOR3 other);
		_DVECTOR3 operator = (D3DXVECTOR3 *other);

		void Set(float fx, float fy, float fz = 0.0f)	{ x = fx; y = fy; z = fz; }
		void Set(_DVECTOR2 &v)					{ Set(v.x, v.y); }
		void Set(_DVECTOR2 *v)					{ Set(v->x, v->y); }
		void Set(const _DVECTOR2 &v)			{ Set(v.x, v.y); }
		void Set(const _DVECTOR2 *v)			{ Set(v->x, v->y); }
		void Set(_DVECTOR3 *v)					{ Set(v->x, v->y, v->z); }
		void Set(_DVECTOR3 v)					{ Set(&v); }
		void Set(D3DXVECTOR3 v)					{ Set((_DVECTOR3 *)&v); }

		void Clear() noexcept					{ x = y = z = 0.0f; }
		bool Empty() const noexcept { return(x == 0.0f && y == 0.0f && z == 0.0f); }
		float Length() const noexcept;
		float LengthSq() const noexcept;

		float DistanceTo(_DVECTOR3 *tar);		// tar까지의 거리
		float XYDistanceTo(_DVECTOR3 *tar);		// tar까지의 XY거리
		float XYDistanceToSq(_DVECTOR3 *tar);	// tar까지의 XY거리SQ
		float ZDistanceTo(_DVECTOR3 *tar);		// tar까지의 Z거리
		float DistanceToSq(_DVECTOR3 *tar);		// tar까지의 거리SQ
		float DotProduct(_DVECTOR3 *other);
		_DVECTOR3 CrossProduct(_DVECTOR3 p);
		_DVECTOR3 CrossProduct(_DVECTOR3* p);

		// 두 벡터 간 내적 계산 (const 참조 버전)
		// @param _other: 대상 벡터
		// @return: 내적 결과 (|A||B|cos(θ))
		float Dot(const _DVECTOR3& _other) const noexcept { return x * _other.x + y * _other.y + z * _other.z; }

		// 외적 계산 (const 참조 버전)
		// @param _other: 대상 벡터
		// @return: 외적 결과 벡터
		_DVECTOR3 Cross(const _DVECTOR3& _other) const noexcept { return _DVECTOR3(y * _other.z - z * _other.y, z * _other.x - x * _other.z, x * _other.y - y * _other.x); }

		_DVECTOR3 *Normalize();
		_DVECTOR3 *FastNormalize();

		// 정규화된 벡터 반환 (자기 자신은 수정하지 않음)
		// @return: 정규화된 새 벡터
		_DVECTOR3 GetNormalized() const noexcept;

		_DVECTOR3 *GetInterpolated(_DVECTOR3 *pOutput, _DVECTOR3 *other, float d);
		_DVECTOR3 *InterpolateTo(_DVECTOR3 *pOutput, _DVECTOR3 *other, float d);		// 로컬 안만들고 처리하게.. 속도 문제. 

		// 선형 보간 (Lerp) - 정적 함수 버전
		// @param _from: 시작 벡터
		// @param _to: 끝 벡터
		// @param _t: 보간 계수 (0.0 ~ 1.0)
		// @return: 보간된 벡터
		static _DVECTOR3 Lerp(const _DVECTOR3& _from, const _DVECTOR3& _to, float _t) noexcept;

		// 법선에 대한 반사 벡터 계산
		// @param _normal: 반사면의 법선 벡터 (정규화되어 있어야 함)
		// @return: 반사된 벡터
		_DVECTOR3 Reflect(const _DVECTOR3& _normal) const noexcept;

		// 다른 벡터에 투영
		// @param _other: 투영할 대상 벡터
		// @return: _other에 투영된 벡터
		_DVECTOR3 Project(const _DVECTOR3& _other) const noexcept;

		float AngleBetween(_DVECTOR3 *other);			///< 두 단위 벡터 사이의 라디안 각도 반환(방향성 없음)
		float AngleToXY(_DVECTOR3 *other);					///< XY평면상에서 other까지의 라디안 각도 반환(방향성 있음)
		float GetAngleXY();										///< XY평면상에서 x축으로부터 y축 방향으로의 라디안 각도

		//const float fToler = 0.001f;
		bool Equals(float a, float b, float tolerance = 0.001f)
		{
			return (fabs(b - a) <= tolerance) ? true : false;
		}
		bool IsEqual(const _DVECTOR3& other);

		static const _DVECTOR3 ZERO;
		static const _DVECTOR3 AXISX;
		static const _DVECTOR3 AXISY;
		static const _DVECTOR3 AXISZ;

	} DVECTOR3, *LPDVECTOR3;
}
