#include "framework.h"
#include "DX9Vector2.h"



namespace dx9
{
	/*
	_DVECTOR2 *_DVECTOR2::operator=(MSVector2 *other)
	{
		MSVector2 other2 = other;
		x = other2.X();
		y = other2.Y();
		z = 0.0f;
		return *this;
	}

	bool _DVECTOR2::IsEqual(_DVECTOR2 *other)
	{
		return (MMath::Equals(x, other->x)
			& *MMath::Equals(y, other->y)
			& *MMath::Equals(z, other->z));
	}
	*/
	_DVECTOR2::_DVECTOR2()
	{
		Clear();
	}
	_DVECTOR2::_DVECTOR2(float fx, float fy)
	{
		Set(fx, fy);
	}
	_DVECTOR2::_DVECTOR2(long fx, long fy)
	{
		Set((float)fx, (float)fy);
	}
	_DVECTOR2::_DVECTOR2(D3DXVECTOR2 _v2)
	{
		Set(_v2.x, _v2.y);
	}
	_DVECTOR2::_DVECTOR2(tagSIZE _v2)
	{
		Set(_v2);
	}
	/*
	_DVECTOR2::_DVECTOR2(DFPOINT _v2)
	{
		Set(_v2);
	}
	_DVECTOR2::_DVECTOR2(DFSIZE _v2)
	{
		Set(_v2);
	}
	*/
	/*
	_DVECTOR2 _DVECTOR2::operator = (_DVECTOR2 other)
	{
		Set(other);
		return *this;
	}
	*/
	_DVECTOR2 _DVECTOR2::operator = (_DVECTOR2 &other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR2 _DVECTOR2::operator = (_DVECTOR2 *other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR2 _DVECTOR2::operator = (tagSIZE &other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR2 _DVECTOR2::operator = (tagSIZE *other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR2 _DVECTOR2::operator = (const _DVECTOR2 *other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR2 _DVECTOR2::operator = (D3DXVECTOR2 other)
	{
		Set(other);
		return (*this);
	}
	_DVECTOR2 _DVECTOR2::operator = (D3DXVECTOR2 *other)
	{
		Set((_DVECTOR2 *)other);
		return(*this);
	}
	/*
	_DVECTOR2 _DVECTOR2::operator = (DFPOINT other)
	{
		Set((_DVECTOR2*)other);
		return(*this);
	}
	_DVECTOR2 _DVECTOR2::operator = (DFPOINT* other)
	{
		Set((_DVECTOR2*)other);
		return(*this);
	}
	_DVECTOR2 _DVECTOR2::operator = (DFSIZE other)
	{
		Set((_DVECTOR2*)other);
		return(*this);
	}
	_DVECTOR2 _DVECTOR2::operator = (DFSIZE* other)
	{
		Set((_DVECTOR2*)other);
		return(*this);
	}
	*/
	// 벡터의 크기(길이) 반환
	// @return: sqrt(x² + y²)
	float _DVECTOR2::Length() const noexcept
	{
		return sqrtf(x*x + y * y);
	}

	// 벡터의 크기(길이)의 제곱 반환 (sqrt 연산 생략으로 빠름)
	// @return: x² + y²
	float _DVECTOR2::LengthSq() const noexcept
	{
		return (x*x + y * y);
	}

	// 대상 벡터까지의 거리
	// @param _other: 대상 위치 벡터
	// @return: 유클리드 거리
	float _DVECTOR2::DistanceTo(const _DVECTOR2& _other) const noexcept
	{
		float dx_ = _other.x - x;
		float dy_ = _other.y - y;
		return sqrtf(dx_ * dx_ + dy_ * dy_);
	}

	// 대상 벡터까지의 거리의 제곱 (sqrt 연산 생략으로 빠름)
	// @param _other: 대상 위치 벡터
	// @return: 거리의 제곱
	float _DVECTOR2::DistanceToSq(const _DVECTOR2& _other) const noexcept
	{
		float dx_ = _other.x - x;
		float dy_ = _other.y - y;
		return dx_ * dx_ + dy_ * dy_;
	}

	// 벡터 정규화 (자기 자신을 수정)
	// @return: 정규화된 자기 자신의 포인터
	_DVECTOR2* _DVECTOR2::Normalize() noexcept
	{
		float len_ = Length();
		if (len_ > 0.0f)
		{
			float invLen_ = 1.0f / len_;
			x *= invLen_;
			y *= invLen_;
		}
		return this;
	}

	// 정규화된 벡터 반환 (자기 자신은 수정하지 않음)
	// @return: 정규화된 새 벡터
	_DVECTOR2 _DVECTOR2::GetNormalized() const noexcept
	{
		float len_ = Length();
		if (len_ > 0.0f)
		{
			float invLen_ = 1.0f / len_;
			return _DVECTOR2(x * invLen_, y * invLen_);
		}
		return _DVECTOR2(0.0f, 0.0f);
	}

	// 선형 보간 (Lerp)
	// @param _pOutput: 결과를 저장할 벡터 포인터
	// @param _other: 목표 벡터
	// @param _t: 보간 계수 (0.0 ~ 1.0)
	// @return: 결과 벡터 포인터
	_DVECTOR2* _DVECTOR2::GetInterpolated(_DVECTOR2* _pOutput, const _DVECTOR2& _other, float _t) const noexcept
	{
		float inv_ = 1.0f - _t;
		_pOutput->Set(_other.x * inv_ + x * _t, _other.y * inv_ + y * _t);
		return _pOutput;
	}

	// 법선에 대한 반사 벡터 계산
	// @param _normal: 반사면의 법선 벡터 (정규화되어 있어야 함)
	// @return: 반사된 벡터
	_DVECTOR2 _DVECTOR2::Reflect(const _DVECTOR2& _normal) const noexcept
	{
		float d_ = 2.0f * Dot(_normal);
		return _DVECTOR2(x - d_ * _normal.x, y - d_ * _normal.y);
	}

	// 다른 벡터에 투영
	// @param _other: 투영할 대상 벡터
	// @return: _other에 투영된 벡터
	_DVECTOR2 _DVECTOR2::Project(const _DVECTOR2& _other) const noexcept
	{
		float lenSq_ = _other.LengthSq();
		if (lenSq_ > 0.0f)
		{
			float scale_ = Dot(_other) / lenSq_;
			return _DVECTOR2(_other.x * scale_, _other.y * scale_);
		}
		return _DVECTOR2(0.0f, 0.0f);
	}

	// 두 벡터 사이의 각도 (라디안)
	// @param _other: 대상 벡터
	// @return: 라디안 각도
	float _DVECTOR2::AngleBetween(const _DVECTOR2& _other) const noexcept
	{
		float lenProduct_ = Length() * _other.Length();
		if (lenProduct_ < 1e-6f)
			lenProduct_ = 1e-6f;

		float fDot_ = Dot(_other) / lenProduct_;
		if (fDot_ < -1.0f) fDot_ = -1.0f;
		if (fDot_ > 1.0f) fDot_ = 1.0f;
		return acosf(fDot_);
	}
}
