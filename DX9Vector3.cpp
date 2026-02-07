#include "framework.h"
#include "DX9Vector3.h"



namespace dx9
{
	const _DVECTOR3 _DVECTOR3::ZERO(0.0f, 0.0f, 0.0f);
	const _DVECTOR3 _DVECTOR3::AXISX(1.0f, 0.0f, 0.0f);
	const _DVECTOR3 _DVECTOR3::AXISY(0.0f, 1.0f, 0.0f);
	const _DVECTOR3 _DVECTOR3::AXISZ(0.0f, 0.0f, 1.0f);

	/*
	_DVECTOR3 *_DVECTOR3::operator=(MSVector2 *other)
	{
		MSVector2 other2 = other;
		x = other2.X();
		y = other2.Y();
		z = 0.0f;
		return *this;
	}
	
	bool _DVECTOR3::IsEqual(_DVECTOR3 *other)
	{
		return (MMath::Equals(x, other->x)
			& *MMath::Equals(y, other->y)
			& *MMath::Equals(z, other->z));
	}
	*/
	_DVECTOR3::_DVECTOR3()
	{
		Clear();
	}
	_DVECTOR3::_DVECTOR3(float fx, float fy, float fz)
	{
		Set(fx, fy, fz);
	}
	_DVECTOR3::_DVECTOR3(D3DXVECTOR3 _v3)
	{
		Set(_v3);
	}
	_DVECTOR3::_DVECTOR3(const D3DXVECTOR3 &_v3)
	{
		Set(_v3);
	}
	_DVECTOR3::_DVECTOR3(_DVECTOR2 _v2)
	{
		Set(_v2);
	}
	_DVECTOR3::_DVECTOR3(const _DVECTOR2 &_v2)
	{
		Set(_v2);
	}
	_DVECTOR3 _DVECTOR3::operator = (const _DVECTOR3& other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR3 _DVECTOR3::operator = (_DVECTOR3 *other)
	{
		Set(other);
		return *this;
	}

	_DVECTOR3 _DVECTOR3::operator = (_DVECTOR2 &other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR3 _DVECTOR3::operator = (const _DVECTOR2 &other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR3 _DVECTOR3::operator = (_DVECTOR2 *other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR3 _DVECTOR3::operator = (const _DVECTOR2 *other)
	{
		Set(other);
		return *this;
	}
	_DVECTOR3 _DVECTOR3::operator = (D3DXVECTOR3 other)
	{
		Set(other);
		return (*this);
	}
	_DVECTOR3 _DVECTOR3::operator = (D3DXVECTOR3 *other)
	{
		Set((_DVECTOR3 *)other);
		return(*this);
	}

	// 벡터의 크기(길이) 반환
	// @return: sqrt(x² + y² + z²)
	float _DVECTOR3::Length() const noexcept
	{
		return sqrtf(x*x + y*y + z*z);
	}

	// 벡터의 크기(길이)의 제곱 반환 (sqrt 연산 생략으로 빠름)
	// @return: x² + y² + z²
	float _DVECTOR3::LengthSq() const noexcept
	{
		return (x*x + y*y + z*z);
	}

	float _DVECTOR3::DistanceTo(_DVECTOR3 *tar)
	{
		const float x1 = tar->x - x;
		const float y1 = tar->y - y;
		const float z1 = tar->z - z;

		return sqrtf(x1*x1 + y1*y1 + z1*z1);
	}

	float _DVECTOR3::XYDistanceTo(_DVECTOR3 *tar)
	{
		const float x1 = tar->x - x;
		const float y1 = tar->y - y;

		return sqrtf(x1*x1 + y1*y1);
	}

	float _DVECTOR3::XYDistanceToSq(_DVECTOR3 *tar)
	{
		const float x1 = tar->x - x;
		const float y1 = tar->y - y;

		return (x1*x1 + y1*y1);
	}

	float _DVECTOR3::ZDistanceTo(_DVECTOR3 *tar)
	{
		const float z1 = tar->z - z;

		return z1;
	}

	float _DVECTOR3::DistanceToSq(_DVECTOR3 *tar)
	{
		const float x1 = tar->x - x;
		const float y1 = tar->y - y;
		const float z1 = tar->z - z;

		return (x1*x1 + y1*y1 + z1*z1);
	}

	float _DVECTOR3::DotProduct(_DVECTOR3 *other)
	{
		return x*other->x + y*other->y + z*other->z;
	}

	_DVECTOR3 _DVECTOR3::CrossProduct(_DVECTOR3 p)
	{
		return(_DVECTOR3(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x));
	}

	_DVECTOR3 _DVECTOR3::CrossProduct(_DVECTOR3* p)
	{
		return(_DVECTOR3(y * p->z - z * p->y, z * p->x - x * p->z, x * p->y - y * p->x));
	}

	inline bool ToleranceEqual(float a, float b, float t)
	{
		return ((a > b - t) && (a < b + t));
	}

	_DVECTOR3 *_DVECTOR3::Normalize()
	{
		float scale = Length();

		if (scale == 0 || ToleranceEqual(scale, 0.0f, .001f))
			return this; // can't normalize astd::std::vector of zero magnitude

		scale = 1.0f / scale;
		x *= scale;
		y *= scale;
		z *= scale;
		return this;
	}
	_DVECTOR3 *_DVECTOR3::FastNormalize()
	{
		float fx = LengthSq();
		// fast invert-sqrtf code
		const float vhalf = 0.5f*fx;
		int i = *(int*)&fx;
		i = 0x5f3759df - (i >> 1);
		fx = *(float*)&i;
		fx = fx*(1.5f - vhalf*fx*fx);
		// 
		x *= fx;
		y *= fx;
		z *= fx;
		return this;
	}

	_DVECTOR3 *_DVECTOR3::GetInterpolated(_DVECTOR3 *pOutput, _DVECTOR3 *other, float d)
	{
		const float inv = 1.0f - d;
		pOutput->Set(other->x*inv + x*d, other->y*inv + y*d, other->z*inv + z*d);
		return(pOutput);
	}
	_DVECTOR3 *_DVECTOR3::InterpolateTo(_DVECTOR3 *pOutput, _DVECTOR3 *other, float d)		// 로컬 안만들고 처리하게.. 속도 문제. 
	{
		pOutput->Set((other->x - x)*d + x, (other->y - y)*d + y, (other->z - z)*d + z);
		return(pOutput);
	}

	float _DVECTOR3::AngleBetween(_DVECTOR3 *other)
	{
		float lenProduct = Length() * other->Length();

		// Divide by zero check
		if (lenProduct < 1e-6f)
			lenProduct = 1e-6f;

		float fDot = this->DotProduct(other) / lenProduct;

		if (fDot < -1.0f) fDot = -1.0f;
		if (fDot > 1.0f) fDot = 1.0f;
		return acosf(fDot);
	}

	float _DVECTOR3::AngleToXY(_DVECTOR3 *other)
	{
		if (Empty() || other->Empty())
		{
			return(0);
		}
		
		_DVECTOR3 *b = other;

		z = 0.0f;
		Normalize();
		b->z = 0.0f;
		b->Normalize();

		const float aa = GetAngleXY();

		const float _x = (float)(b->x*cosf(aa) + b->y*sinf(aa));
		const float _y = (float)(b->x*(-sinf(aa)) + b->y*cosf(aa));

		_DVECTOR3 ret = _DVECTOR3(_x, _y, 0.0f);
		return ret.GetAngleXY();
	}

	float _DVECTOR3::GetAngleXY()
	{
		if (x >= 1.0f)
			return 0.0f;
		if (x <= -1.0f)
			return -3.1415926535f;

		return (float)(y > 0 ? acos(x) : -acos(x));
	}

	bool _DVECTOR3::IsEqual(const _DVECTOR3& other)
	{
		return (Equals(x, other.x)
			&& Equals(y, other.y)
			&& Equals(z, other.z)
		);
	}

	// 정규화된 벡터 반환 (자기 자신은 수정하지 않음)
	// @return: 정규화된 새 벡터
	_DVECTOR3 _DVECTOR3::GetNormalized() const noexcept
	{
		const float len_ = Length();
		if (len_ > 0.0f)
		{
			const float invLen_ = 1.0f / len_;
			return _DVECTOR3(x * invLen_, y * invLen_, z * invLen_);
		}
		return _DVECTOR3(0.0f, 0.0f, 0.0f);
	}

	// 선형 보간 (Lerp) - 정적 함수 버전
	// @param _from: 시작 벡터
	// @param _to: 끝 벡터
	// @param _t: 보간 계수 (0.0 ~ 1.0)
	// @return: 보간된 벡터
	_DVECTOR3 _DVECTOR3::Lerp(const _DVECTOR3& _from, const _DVECTOR3& _to, float _t) noexcept
	{
		return _DVECTOR3(
			_from.x + (_to.x - _from.x) * _t,
			_from.y + (_to.y - _from.y) * _t,
			_from.z + (_to.z - _from.z) * _t
		);
	}

	// 법선에 대한 반사 벡터 계산
	// @param _normal: 반사면의 법선 벡터 (정규화되어 있어야 함)
	// @return: 반사된 벡터
	_DVECTOR3 _DVECTOR3::Reflect(const _DVECTOR3& _normal) const noexcept
	{
		const float d_ = 2.0f * Dot(_normal);
		return _DVECTOR3(x - d_ * _normal.x, y - d_ * _normal.y, z - d_ * _normal.z);
	}

	// 다른 벡터에 투영
	// @param _other: 투영할 대상 벡터
	// @return: _other에 투영된 벡터
	_DVECTOR3 _DVECTOR3::Project(const _DVECTOR3& _other) const noexcept
	{
		const float lenSq_ = _other.LengthSq();
		if (lenSq_ > 0.0f)
		{
			const float scale_ = Dot(_other) / lenSq_;
			return _DVECTOR3(_other.x * scale_, _other.y * scale_, _other.z * scale_);
		}
		return _DVECTOR3(0.0f, 0.0f, 0.0f);
	}
}
