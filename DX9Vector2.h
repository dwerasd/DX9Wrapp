#pragma once


#include <Windows.h>

#include <DarkCore/DMemory.h>
#include <DarkCore/DTypes.h>

#include "DX9Def.h"
//#include "DX9Types.h"



namespace dx9
{
	typedef struct _DVECTOR2
		: public D3DXVECTOR2
	{
		_DVECTOR2();
		_DVECTOR2(float fx, float fy);
		_DVECTOR2(long fx, long fy);
		_DVECTOR2(D3DXVECTOR2 _v2);
		_DVECTOR2(tagSIZE _v2);
		//_DVECTOR2(DFPOINT _v2);
		//_DVECTOR2(DFSIZE _v2);

		//_DVECTOR2 operator = (_DVECTOR2 other);
		_DVECTOR2 operator = (_DVECTOR2 &other);
		_DVECTOR2 operator = (_DVECTOR2 *other);
		_DVECTOR2 operator = (tagSIZE &other);
		_DVECTOR2 operator = (tagSIZE *other);
		_DVECTOR2 operator = (const _DVECTOR2 *other);
		_DVECTOR2 operator = (D3DXVECTOR2 other);
		_DVECTOR2 operator = (D3DXVECTOR2 *other);
		/*
		_DVECTOR2 operator = (DFPOINT other);
		_DVECTOR2 operator = (DFPOINT* other);
		_DVECTOR2 operator = (DFSIZE other);
		_DVECTOR2 operator = (DFSIZE* other);
		*/

		// casting
		//operator float* ()				{ return (float *) &x; }
		//operator const float* () const	{ return (const float *) &x; }
		/* 32 비트에서 제대로 컴파일이 되지 않아서 제거함.
		float operator[] (size_t idx) const
		{
			return (&x)[idx];
		}    // We very rarely use this [] operator, the assert overhead is fine.
		
		float& operator[] (size_t idx)
		{
			return (&x)[idx];
		}    // We very rarely use this [] operator, the assert overhead is fine.
		*/
		void Set(int _x, int _y) { x = (float)_x; y = (float)_y; }
		void Set(float _x, float _y) { x = _x; y = _y; }
		void Set(_DVECTOR2 _v) { Set(&_v); }
		void Set(_DVECTOR2 *_v) { Set(_v->x, _v->y); }
		/*
		void Set(DFPOINT _v) { Set(&_v); }
		void Set(DFPOINT* _v) { Set(_v->x, _v->y); }
		void Set(DFSIZE _v) { Set(&_v); }
		void Set(DFSIZE* _v) { Set(_v->cx, _v->cy); }
		*/
		void Set(tagSIZE _v) { Set((float)_v.cx, (float)_v.cy); }
		void Set(tagSIZE *_v) { Set((float)_v->cx, (float)_v->cy); }
		void Set(const _DVECTOR2 *_v) { Set(_v->x, _v->y); }
		void Set(D3DXVECTOR2 _v) { Set((_DVECTOR2 *)&_v); }

		void Clear() noexcept { x = y = 0.0f; }
		bool Empty() const noexcept { return(x == 0.0f && y == 0.0f); }
		float Length() const noexcept;
		float LengthSq() const noexcept;

		// 두 벡터 간 내적 계산
		// @param _other: 대상 벡터
		// @return: 내적 결과 (|A||B|cos(θ))
		float Dot(const _DVECTOR2& _other) const noexcept { return x * _other.x + y * _other.y; }

		// 2D 외적 (z축 성분만 반환, 스칼라 값)
		// @param _other: 대상 벡터
		// @return: 외적의 z 성분 (|A||B|sin(θ))
		float Cross(const _DVECTOR2& _other) const noexcept { return x * _other.y - y * _other.x; }

		// 대상 벡터까지의 거리
		// @param _other: 대상 위치 벡터
		// @return: 유클리드 거리
		float DistanceTo(const _DVECTOR2& _other) const noexcept;

		// 대상 벡터까지의 거리의 제곱 (sqrt 연산 생략으로 빠름)
		// @param _other: 대상 위치 벡터
		// @return: 거리의 제곱
		float DistanceToSq(const _DVECTOR2& _other) const noexcept;

		// 벡터 정규화 (자기 자신을 수정)
		// @return: 정규화된 자기 자신의 포인터
		_DVECTOR2* Normalize() noexcept;

		// 정규화된 벡터 반환 (자기 자신은 수정하지 않음)
		// @return: 정규화된 새 벡터
		_DVECTOR2 GetNormalized() const noexcept;

		// 선형 보간 (Lerp)
		// @param _pOutput: 결과를 저장할 벡터 포인터
		// @param _other: 목표 벡터
		// @param _t: 보간 계수 (0.0 ~ 1.0)
		// @return: 결과 벡터 포인터
		_DVECTOR2* GetInterpolated(_DVECTOR2* _pOutput, const _DVECTOR2& _other, float _t) const noexcept;

		// 법선에 대한 반사 벡터 계산
		// @param _normal: 반사면의 법선 벡터 (정규화되어 있어야 함)
		// @return: 반사된 벡터
		_DVECTOR2 Reflect(const _DVECTOR2& _normal) const noexcept;

		// 다른 벡터에 투영
		// @param _other: 투영할 대상 벡터
		// @return: _other에 투영된 벡터
		_DVECTOR2 Project(const _DVECTOR2& _other) const noexcept;

		// 수직 벡터 반환 (90도 회전)
		// @return: 시계 방향으로 90도 회전된 벡터
		_DVECTOR2 Perpendicular() const noexcept { return _DVECTOR2(y, -x); }

		// 두 벡터 사이의 각도 (라디안)
		// @param _other: 대상 벡터
		// @return: 라디안 각도
		float AngleBetween(const _DVECTOR2& _other) const noexcept;

	} DVECTOR2, *LPDVECTOR2;

	// 2D vector (half-size integer)
	typedef struct _DVECTOR2IH
		: public dk::C_ALIGNED_ALLOCATION_POLICYT<16>
	{
		short   x, y;
		_DVECTOR2IH() { x = y = 0; }
		_DVECTOR2IH(short _x, short _y) { x = _x; y = _y; }
	} DVECTOR2IH, *LPDVECTOR2IH;

}

static inline dx9::DVECTOR2 operator*(const dx9::DVECTOR2& lhs, const float rhs)              { return dx9::DVECTOR2(lhs.x*rhs, lhs.y*rhs); }
static inline dx9::DVECTOR2 operator/(const dx9::DVECTOR2& lhs, const float rhs)              { return dx9::DVECTOR2(lhs.x/rhs, lhs.y/rhs); }
static inline dx9::DVECTOR2 operator+(const dx9::DVECTOR2& lhs, const dx9::DVECTOR2& rhs)            { return dx9::DVECTOR2(lhs.x+rhs.x, lhs.y+rhs.y); }
static inline dx9::DVECTOR2 operator-(const dx9::DVECTOR2& lhs, const dx9::DVECTOR2& rhs)            { return dx9::DVECTOR2(lhs.x-rhs.x, lhs.y-rhs.y); }
static inline dx9::DVECTOR2 operator*(const dx9::DVECTOR2& lhs, const dx9::DVECTOR2& rhs)            { return dx9::DVECTOR2(lhs.x*rhs.x, lhs.y*rhs.y); }
static inline dx9::DVECTOR2 operator/(const dx9::DVECTOR2& lhs, const dx9::DVECTOR2& rhs)            { return dx9::DVECTOR2(lhs.x/rhs.x, lhs.y/rhs.y); }
static inline dx9::DVECTOR2& operator+=(dx9::DVECTOR2& lhs, const dx9::DVECTOR2& rhs)                { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline dx9::DVECTOR2& operator-=(dx9::DVECTOR2& lhs, const dx9::DVECTOR2& rhs)                { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline dx9::DVECTOR2& operator*=(dx9::DVECTOR2& lhs, const float rhs)                  { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline dx9::DVECTOR2& operator/=(dx9::DVECTOR2& lhs, const float rhs)                  { lhs.x /= rhs; lhs.y /= rhs; return lhs; }

static inline dx9::DVECTOR2 v2Min(const dx9::DVECTOR2& lhs, const dx9::DVECTOR2& rhs)
{
	return dx9::DVECTOR2(
		lhs.x < rhs.x ? lhs.x : rhs.x
		, lhs.y < rhs.y ? lhs.y : rhs.y
	);
}
static inline dx9::DVECTOR2 v2Max(const dx9::DVECTOR2& lhs, const dx9::DVECTOR2& rhs)
{
	return dx9::DVECTOR2(
		lhs.x >= rhs.x ? lhs.x : rhs.x
		, lhs.y >= rhs.y ? lhs.y : rhs.y
	);
}
static inline dx9::DVECTOR2 v2Clamp(const dx9::DVECTOR2& _v, const dx9::DVECTOR2& mn, dx9::DVECTOR2 mx)
{
	return dx9::DVECTOR2(
		(_v.x < mn.x) ? mn.x : (_v.x > mx.x) ? mx.x : _v.x
		, (_v.y < mn.y) ? mn.y : (_v.y > mx.y) ? mx.y : _v.y
	);
}

static inline dx9::DVECTOR2 v2Lerp(const dx9::DVECTOR2& a, const dx9::DVECTOR2& b, float t) noexcept
{
	return dx9::DVECTOR2(
		a.x + (b.x - a.x) * t
		, a.y + (b.y - a.y) * t
	);
}

// 두 벡터 간 거리 계산
// @param a: 시작 위치 벡터
// @param b: 끝 위치 벡터
// @return: 유클리드 거리
static inline float v2Distance(const dx9::DVECTOR2& a, const dx9::DVECTOR2& b) noexcept
{
	float dx_ = b.x - a.x;
	float dy_ = b.y - a.y;
	return sqrtf(dx_ * dx_ + dy_ * dy_);
}

// 두 벡터 간 거리의 제곱 계산 (sqrt 연산 생략으로 빠름)
// @param a: 시작 위치 벡터
// @param b: 끝 위치 벡터
// @return: 거리의 제곱
static inline float v2DistanceSq(const dx9::DVECTOR2& a, const dx9::DVECTOR2& b) noexcept
{
	float dx_ = b.x - a.x;
	float dy_ = b.y - a.y;
	return dx_ * dx_ + dy_ * dy_;
}

// 벡터 반사 계산
// @param v: 입사 벡터
// @param n: 법선 벡터 (정규화되어 있어야 함)
// @return: 반사된 벡터
static inline dx9::DVECTOR2 v2Reflect(const dx9::DVECTOR2& v, const dx9::DVECTOR2& n) noexcept
{
	float d_ = 2.0f * (v.x * n.x + v.y * n.y);
	return dx9::DVECTOR2(v.x - d_ * n.x, v.y - d_ * n.y);
}

// 벡터의 내적 계산
// @param a: 첫 번째 벡터
// @param b: 두 번째 벡터
// @return: 내적 결과
static inline float v2Dot(const dx9::DVECTOR2& a, const dx9::DVECTOR2& b) noexcept
{
	return a.x * b.x + a.y * b.y;
}

// 벡터 정규화
// @param v: 정규화할 벡터
// @return: 정규화된 벡터
static inline dx9::DVECTOR2 v2Normalize(const dx9::DVECTOR2& v) noexcept
{
	float len_ = sqrtf(v.x * v.x + v.y * v.y);
	if (len_ > 0.0f)
	{
		float invLen_ = 1.0f / len_;
		return dx9::DVECTOR2(v.x * invLen_, v.y * invLen_);
	}
	return dx9::DVECTOR2(0.0f, 0.0f);
}
