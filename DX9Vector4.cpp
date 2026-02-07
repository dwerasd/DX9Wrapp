#include "framework.h"
#include "DX9Vector4.h"



namespace dx9
{
	_DVECTOR4::_DVECTOR4()
	{
		Clear();
	}
	_DVECTOR4::_DVECTOR4(float _fx, float _fy, float _fz, float _fw)
	{
		Set(_fx, _fy, _fz, _fw);
	}

	_DVECTOR4 _DVECTOR4::operator = (_DVECTOR4 *other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR4 _DVECTOR4::operator = (D3DXVECTOR4 other)
	{
		Set(other);
		return (*this);
	}
	_DVECTOR4 _DVECTOR4::operator = (D3DXVECTOR4 *other)
	{
		Set((_DVECTOR4 *)other);
		return(*this);
	}

	// 벡터의 크기(길이) 반환
	// @return: sqrt(x² + y² + z² + w²)
	float _DVECTOR4::Length() const noexcept
	{
		return sqrtf(x*x + y*y + z*z+ w*w);
	}

	// 벡터 정규화 (자기 자신을 수정)
	// @return: 정규화된 자기 자신의 포인터
	_DVECTOR4* _DVECTOR4::Normalize() noexcept
	{
		const float len_ = Length();
		if (len_ > 0.0f)
		{
			const float invLen_ = 1.0f / len_;
			x *= invLen_;
			y *= invLen_;
			z *= invLen_;
			w *= invLen_;
		}
		return this;
	}

	// 정규화된 벡터 반환 (자기 자신은 수정하지 않음)
	// @return: 정규화된 새 벡터
	_DVECTOR4 _DVECTOR4::GetNormalized() const noexcept
	{
		const float len_ = Length();
		if (len_ > 0.0f)
		{
			const float invLen_ = 1.0f / len_;
			return _DVECTOR4(x * invLen_, y * invLen_, z * invLen_, w * invLen_);
		}
		return _DVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	// 선형 보간 (Lerp) - 정적 함수 버전
	// @param _from: 시작 벡터
	// @param _to: 끝 벡터
	// @param _t: 보간 계수 (0.0 ~ 1.0)
	// @return: 보간된 벡터
	_DVECTOR4 _DVECTOR4::Lerp(const _DVECTOR4& _from, const _DVECTOR4& _to, float _t) noexcept
	{
		return _DVECTOR4(
			_from.x + (_to.x - _from.x) * _t,
			_from.y + (_to.y - _from.y) * _t,
			_from.z + (_to.z - _from.z) * _t,
			_from.w + (_to.w - _from.w) * _t
		);
	}


	// Dot product of two vector4's
	float _DVECTOR4::DotProduct(_DVECTOR4 *other)
	{
		return x*other->x + y*other->y + z*other->z + w * other->w;
	}

	_DVECTOR4 *_DVECTOR4::CrossProduct(_DVECTOR4 *_pOutput, _DVECTOR4 *_p)
	{
		// 4차원 벡터는 원래 외적이 없어서 .w는 걍 서로 곱셈해버린다.
		_pOutput->Set(
			y * _p->z - z * _p->y
			, z * _p->x - x * _p->z
			, x * _p->y - y * _p->x
			, _p->w * w
		);
		return(_pOutput);
	}

	inline bool ToleranceEqual(float a, float b, float t)
	{
		return ((a > b - t) && (a < b + t));
	}

	_DVECTOR4 *_DVECTOR4::GetInterpolated(_DVECTOR4 *_pOutput, _DVECTOR4 *_other, float _d)
	{
		const float inv = 1.0f - _d;
		_pOutput->Set(
			_other->x*inv + x * _d
			, _other->y*inv + y * _d
			, _other->z*inv + z * _d
			, _other->w*inv + w * _d
		);
		return(_pOutput);
	}
}
