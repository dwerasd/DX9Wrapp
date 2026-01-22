#include "framework.h"
#include "DX9Matrix.h"



namespace dx9
{
	static float _matIdentity[] =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	const _DMATRIX9 _DMATRIX9::_IDENTITY(_matIdentity);

	_DMATRIX9::_DMATRIX9()
	{
		Clear();
	}
	_DMATRIX9::_DMATRIX9(const float* f)
	{
		memcpy(&_11, f, sizeof(float) * 4 * 4);
	}

	_DMATRIX9::_DMATRIX9(const float(*f)[4])
	{
		for (int i = 0; i < 4; ++i)
		{
			memcpy(&_11 + 4 * i, f[i], sizeof(float) * 4);
		}
	}
	_DMATRIX9::_DMATRIX9(
		float _f11, float _f12, float _f13, float _f14,
		float _f21, float _f22, float _f23, float _f24,
		float _f31, float _f32, float _f33, float _f34,
		float _f41, float _f42, float _f43, float _f44)
		: _D3DXMATRIXA16(
			_f11, _f12, _f13, _f14,
			_f21, _f22, _f23, _f24,
			_f31, _f32, _f33, _f34,
			_f41, _f42, _f43, _f44
		)
	{
		/*
		_11 = f11; _12 = f12; _13 = f13; _14 = f14;
		_21 = f21; _22 = f22; _23 = f23; _24 = f24;
		_31 = f31; _32 = f32; _33 = f33; _34 = f34;
		_41 = f41; _42 = f42; _43 = f43; _44 = f44;
		*/
	}

	_DMATRIX9 _DMATRIX9::operator = (const D3DXMATRIX other)
	{
		Set(other);
		return *this;
	}
	_DMATRIX9 _DMATRIX9::operator = (D3DXMATRIX *other)
	{
		Set((_DMATRIX9 *)other);
		return *this;
	}
	_DMATRIX9 _DMATRIX9::operator = (const _DMATRIX9 other)
	{
		Set(other);
		return *this;
	}
	_DMATRIX9 _DMATRIX9::operator = (_DMATRIX9 *other)
	{
		Set(other);
		return *this;
	}
	/*
	inline _DMATRIX9 _DMATRIX9::operator=(const _DMATRIX9& other)
	{
#if defined(_WIN64)
		memcpy(&m, &other.m, sizeof(float) * 16);
#else
		__asm
		{
			mov esi, other;
			mov edi, this;
			movdqu xmm0, xmmword ptr[esi];
			movdqu xmm1, xmmword ptr[esi + 0x10];
			movdqu xmm2, xmmword ptr[esi + 0x20];
			movdqu xmm3, xmmword ptr[esi + 0x30];

			movdqu xmmword ptr[edi], xmm0;
			movdqu xmmword ptr[edi + 0x10], xmm1;
			movdqu xmmword ptr[edi + 0x20], xmm2;
			movdqu xmmword ptr[edi + 0x30], xmm3;
		}
#endif
		return *this;
	}
	*/
	inline bool _DMATRIX9::operator==(const _DMATRIX9& other)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (_DMATRIX9::m[i][j] != other.m[i][j])
				{
					return false;
				}
			}
		}
		return true;
	}

	inline bool _DMATRIX9::operator!=(const _DMATRIX9& other)
	{
		return !(*this == other);
	}

	inline void _DMATRIX9::MultiplyTo(const _DMATRIX9& other, _DMATRIX9& mOut) const // out = this * other;
	{
#if defined(_WIN64)
		const _DMATRIX9& m1 = *this, & m2 = other;
#define CALCCOMPONENT(i,j) mOut.m[i][j] = m1.m[i][0]*m2.m[0][j] + m1.m[i][1]*m2.m[1][j] + m1.m[i][2]*m2.m[2][j] + m1.m[i][3]*m2.m[3][j];
		CALCCOMPONENT(0, 0) CALCCOMPONENT(0, 1) CALCCOMPONENT(0, 2) CALCCOMPONENT(0, 3)
			CALCCOMPONENT(1, 0) CALCCOMPONENT(1, 1) CALCCOMPONENT(1, 2) CALCCOMPONENT(1, 3)
			CALCCOMPONENT(2, 0) CALCCOMPONENT(2, 1) CALCCOMPONENT(2, 2) CALCCOMPONENT(2, 3)
			CALCCOMPONENT(3, 0) CALCCOMPONENT(3, 1) CALCCOMPONENT(3, 2) CALCCOMPONENT(3, 3)
#else
		__asm
		{
			mov		   eax, mOut		// dst
			mov		   ecx, other	// src1
			mov		   edx, this		// src2

			movups	 xmm0, xmmword ptr[ecx]	  	   // xmm0 = src1[00, 01, 02, 03]
			movups	 xmm1, xmmword ptr[ecx + 0x10]	   // xmm1 = src1[04, 05, 06, 07]
			movups	 xmm2, xmmword ptr[ecx + 0x20]	   // xmm2 = src1[08, 09, 10, 11]
			movups	 xmm3, xmmword ptr[ecx + 0x30]	   // xmm3 = src1[12, 13, 14, 15]

			movss	  xmm7, dword ptr[edx]		   // xmm7 = src2[00, xx, xx, xx]
			movss	  xmm4, dword ptr[edx + 0x4]	  // xmm4 = src2[01, xx, xx, xx]
			movss	  xmm5, dword ptr[edx + 0x8]	  // xmm5 = src2[02, xx, xx, xx]
			movss	  xmm6, dword ptr[edx + 0xc]	  	   // xmm6 = src2[03, xx, xx, xx]

			shufps	 xmm7, xmm7, 0x0	  	   // xmm7 = src2[00, 00, 00, 00]
			shufps	 xmm4, xmm4, 0x0	  	   // xmm4 = src2[01, 01, 01, 01]
			shufps	 xmm5, xmm5, 0x0	  	   // xmm5 = src2[02, 02, 02, 02]
			shufps	 xmm6, xmm6, 0x0	  	   // xmm6 = src2[03, 03, 03, 03]

			mulps	  xmm7, xmm0	   	   	   // xmm7 *= xmm0
			mulps	  xmm4, xmm1	   	   	   // xmm4 *= xmm1
			mulps	  xmm5, xmm2	   	   	   // xmm5 *= xmm2
			mulps	  xmm6, xmm3	   	   	   // xmm6 *= xmm3

			addps	  xmm7, xmm4	   	   	   // xmm7 += xmm4
			addps	  xmm7, xmm5	   	   	   // xmm7 += xmm5
			addps	  xmm7, xmm6	   	   	   // xmm7 += xmm6

			movups	 xmmword ptr[eax], xmm7		   // eax = xmm7

			movss	  xmm7, dword ptr[edx + 0x10]	 // xmm7 = src2[04, xx, xx, xx]
			movss	  xmm4, dword ptr[edx + 0x14]	 // xmm4 = src2[05, xx, xx, xx]
			movss	  xmm5, dword ptr[edx + 0x18]	 // xmm5 = src2[06, xx, xx, xx]
			movss	  xmm6, dword ptr[edx + 0x1c]	 // xmm6 = src2[07, xx, xx, xx]

			shufps	 xmm7, xmm7, 0x0	  	   // xmm7 = src2[04, 04, 04, 04]
			shufps	 xmm4, xmm4, 0x0	  	   // xmm4 = src2[05, 05, 05, 05]
			shufps	 xmm5, xmm5, 0x0	  	   // xmm5 = src2[06, 06, 06, 06]
			shufps	 xmm6, xmm6, 0x0	  	   // xmm6 = src2[07, 07, 07, 07]

			mulps	  xmm7, xmm0	   	   	   // xmm7 *= xmm0
			mulps	  xmm4, xmm1	   	   	   // xmm4 *= xmm1
			mulps	  xmm5, xmm2	   	   	   // xmm5 *= xmm2
			mulps	  xmm6, xmm3	   	   	   // xmm6 *= xmm3

			addps	  xmm7, xmm4	   	   	   // xmm7 += xmm4
			addps	  xmm7, xmm5	   	   	   // xmm7 += xmm5
			addps	  xmm7, xmm6	   	   	   // xmm7 += xmm6

			movups	 xmmword ptr[eax + 0x10], xmm7	 // eax = xmm7

			movss	  xmm7, dword ptr[edx + 0x20]	 // xmm7 = src2[08, xx, xx, xx]
			movss	  xmm4, dword ptr[edx + 0x24]	 // xmm4 = src2[09, xx, xx, xx]
			movss	  xmm5, dword ptr[edx + 0x28]	 // xmm5 = src2[10, xx, xx, xx]
			movss	  xmm6, dword ptr[edx + 0x2c]	 // xmm6 = src2[11, xx, xx, xx]

			shufps	 xmm7, xmm7, 0x0	  	   // xmm7 = src2[08, 08, 08, 08]
			shufps	 xmm4, xmm4, 0x0	  	   // xmm4 = src2[09, 09, 09, 09]
			shufps	 xmm5, xmm5, 0x0	  	   // xmm5 = src2[10, 10, 10, 10]
			shufps	 xmm6, xmm6, 0x0	  	   // xmm6 = src2[11, 11, 11, 11]

			mulps	  xmm7, xmm0	   	   	   // xmm7 *= xmm0
			mulps	  xmm4, xmm1	   	   	   // xmm4 *= xmm1
			mulps	  xmm5, xmm2	   	   	   // xmm5 *= xmm2
			mulps	  xmm6, xmm3	   	   	   // xmm6 *= xmm3

			addps	  xmm7, xmm4	   	   	   // xmm7 += xmm4
			addps	  xmm7, xmm5	   	   	   // xmm7 += xmm5
			addps	  xmm7, xmm6	   	   	   // xmm7 += xmm6

			movups	 xmmword ptr[eax + 0x20], xmm7	 // eax = xmm7

			movss	  xmm7, dword ptr[edx + 0x30]	 // xmm7 = src2[12, xx, xx, xx]
			movss	  xmm4, dword ptr[edx + 0x34]	 // xmm4 = src2[13, xx, xx, xx]
			movss	  xmm5, dword ptr[edx + 0x38]	 // xmm5 = src2[14, xx, xx, xx]
			movss	  xmm6, dword ptr[edx + 0x3c]	 // xmm6 = src2[15, xx, xx, xx]

			shufps	 xmm7, xmm7, 0x0	  	   // xmm7 = src2[12, 12, 12, 12]
			shufps	 xmm4, xmm4, 0x0	  	   // xmm4 = src2[13, 13, 13, 13]
			shufps	 xmm5, xmm5, 0x0	  	   // xmm5 = src2[14, 14, 14, 14]
			shufps	 xmm6, xmm6, 0x0	  	   // xmm6 = src2[15, 15, 15, 15]

			mulps	  xmm7, xmm0	   	   	   // xmm7 *= xmm0
			mulps	  xmm4, xmm1	   	   	   // xmm4 *= xmm1
			mulps	  xmm5, xmm2	   	   	   // xmm5 *= xmm2
			mulps	  xmm6, xmm3	   	   	   // xmm6 *= xmm3

			addps	  xmm7, xmm4	   	   	   // xmm7 += xmm4
			addps	  xmm7, xmm5	   	   	   // xmm7 += xmm5
			addps	  xmm7, xmm6	   	   	   // xmm7 += xmm6

			movups	 xmmword ptr[eax + 0x30], xmm7	 // eax = xmm7
		}
#endif
	}

	_DMATRIX9& _DMATRIX9::operator*=(const _DMATRIX9& other)
	{
		_DMATRIX9 newMatrix;
		MultiplyTo(other, newMatrix);
		memcpy(&m, &newMatrix.m, sizeof(float) * 16);
		return *this;
	}

	_DMATRIX9 _DMATRIX9::operator*(const _DMATRIX9& other)
	{
		_DMATRIX9 tmtrx;
		MultiplyTo(other, tmtrx);
		return tmtrx;

	}

	_DMATRIX9 _DMATRIX9::operator*(float f)
	{
		_DMATRIX9 mat;
		for (int i = 0;i < 4; ++i)
		{
			for (int j = 0;j < 4; ++j)
			{
				mat.m[i][j] = m[i][j] * f;
			}
		}
		return mat;
	}

	_DMATRIX9 _DMATRIX9::operator-(const _DMATRIX9& mat)	// 090716, OZ
	{
		return _DMATRIX9(
			_11 - mat._11, _12 - mat._12, _13 - mat._13, _14 - mat._14,
			_21 - mat._21, _22 - mat._22, _23 - mat._23, _24 - mat._24,
			_31 - mat._31, _32 - mat._32, _33 - mat._33, _34 - mat._34,
			_41 - mat._41, _42 - mat._42, _43 - mat._43, _44 - mat._44
		);
	}

	_DMATRIX9 _DMATRIX9::operator+(const _DMATRIX9& mat)	// 090716, OZ
	{
		return _DMATRIX9(
			_11 + mat._11, _12 + mat._12, _13 + mat._13, _14 + mat._14,
			_21 + mat._21, _22 + mat._22, _23 + mat._23, _24 + mat._24,
			_31 + mat._31, _32 + mat._32, _33 + mat._33, _34 + mat._34,
			_41 + mat._41, _42 + mat._42, _43 + mat._43, _44 + mat._44
		);
	}
	void _DMATRIX9::Set(
		float f11, float f12, float f13, float f14,
		float f21, float f22, float f23, float f24,
		float f31, float f32, float f33, float f34,
		float f41, float f42, float f43, float f44)
	{
		_11 = f11; _12 = f12; _13 = f13; _14 = f14;
		_21 = f21; _22 = f22; _23 = f23; _24 = f24;
		_31 = f31; _32 = f32; _33 = f33; _34 = f34;
		_41 = f41; _42 = f42; _43 = f43; _44 = f44;
	}
	
	void _DMATRIX9::Set(D3DXMATRIX* v)
	{
		Set(v->_11, v->_12, v->_13, v->_14
			, v->_21, v->_22, v->_23, v->_24
			, v->_31, v->_32, v->_33, v->_34
			, v->_41, v->_42, v->_43, v->_44
		);
	}
	void _DMATRIX9::Set(D3DXMATRIX v)
	{
		Set(&v);
	}
	void _DMATRIX9::Set(const _DMATRIX9* v)
	{
		Set(v->_11, v->_12, v->_13, v->_14
			, v->_21, v->_22, v->_23, v->_24
			, v->_31, v->_32, v->_33, v->_34
			, v->_41, v->_42, v->_43, v->_44
		);
	}
	void _DMATRIX9::Set(_DMATRIX9 *v)
	{
		Set(v->_11, v->_12, v->_13, v->_14
			, v->_21, v->_22, v->_23, v->_24
			, v->_31, v->_32, v->_33, v->_34
			, v->_41, v->_42, v->_43, v->_44
		);
	}
	void _DMATRIX9::Set(_DMATRIX9 v)
	{
		Set(&v);
	}
	
	void _DMATRIX9::Clear()
	{
		/*
		Set(0.0f, 0.0f, 0.0f, 0.0f
			, 0.0f, 0.0f, 0.0f, 0.0f
			, 0.0f, 0.0f, 0.0f, 0.0f
			, 0.0f, 0.0f, 0.0f, 0.0f
		);
		*/
		_11 = _12 = _13 = _14 = _21 = _22 = _23 = _24 = _31 = _32 = _33 = _34 = _41 = _42 = _43 = _44 = 0.0f;
	}

	// 행렬식(Determinant) 계산
	// 4x4 행렬의 행렬식을 라플라스 전개(Laplace Expansion)로 계산
	// @return: 행렬식 값 (역행렬 존재 여부 판단에 사용, 0이면 역행렬 없음)
	float _DMATRIX9::Determinant() const noexcept
	{
		// 2x2 소행렬식 계산 (행 2,3의 열 조합)
		const float f2x2_01_ = _31 * _42 - _32 * _41;	// 열 0,1
		const float f2x2_02_ = _31 * _43 - _33 * _41;	// 열 0,2
		const float f2x2_03_ = _31 * _44 - _34 * _41;	// 열 0,3
		const float f2x2_12_ = _32 * _43 - _33 * _42;	// 열 1,2
		const float f2x2_13_ = _32 * _44 - _34 * _42;	// 열 1,3
		const float f2x2_23_ = _33 * _44 - _34 * _43;	// 열 2,3

		// 3x3 소행렬식 계산 (행 1,2,3의 여인수)
		// C00: 1행 0열 여인수 (열 1,2,3 사용)
		const float fCofactor00_ = _22 * f2x2_23_ - _23 * f2x2_13_ + _24 * f2x2_12_;
		// C01: 1행 1열 여인수 (열 0,2,3 사용)
		const float fCofactor01_ = _21 * f2x2_23_ - _23 * f2x2_03_ + _24 * f2x2_02_;
		// C02: 1행 2열 여인수 (열 0,1,3 사용)
		const float fCofactor02_ = _21 * f2x2_13_ - _22 * f2x2_03_ + _24 * f2x2_01_;
		// C03: 1행 3열 여인수 (열 0,1,2 사용)
		const float fCofactor03_ = _21 * f2x2_12_ - _22 * f2x2_02_ + _23 * f2x2_01_;

		// 최종 행렬식: 0행에 대한 라플라스 전개
		// det = a00*C00 - a01*C01 + a02*C02 - a03*C03
		return _11 * fCofactor00_ - _12 * fCofactor01_ + _13 * fCofactor02_ - _14 * fCofactor03_;
	}

	// 행렬이 단위행렬인지 확인
	// @param _fTolerance: 허용 오차
	// @return: 단위행렬이면 true
	bool _DMATRIX9::IsIdentity(float _fTolerance) const noexcept
	{
		// 대각 요소는 1에 가까워야 함
		if (fabsf(_11 - 1.0f) > _fTolerance) return false;
		if (fabsf(_22 - 1.0f) > _fTolerance) return false;
		if (fabsf(_33 - 1.0f) > _fTolerance) return false;
		if (fabsf(_44 - 1.0f) > _fTolerance) return false;

		// 비대각 요소는 0에 가까워야 함
		if (fabsf(_12) > _fTolerance) return false;
		if (fabsf(_13) > _fTolerance) return false;
		if (fabsf(_14) > _fTolerance) return false;
		if (fabsf(_21) > _fTolerance) return false;
		if (fabsf(_23) > _fTolerance) return false;
		if (fabsf(_24) > _fTolerance) return false;
		if (fabsf(_31) > _fTolerance) return false;
		if (fabsf(_32) > _fTolerance) return false;
		if (fabsf(_34) > _fTolerance) return false;
		if (fabsf(_41) > _fTolerance) return false;
		if (fabsf(_42) > _fTolerance) return false;
		if (fabsf(_43) > _fTolerance) return false;

		return true;
	}

	// 행렬에서 스케일 벡터 추출
	// @return: 스케일 벡터 (x, y, z 각 축의 스케일)
	_DVECTOR3 _DMATRIX9::GetScale() const noexcept
	{
		// 각 축 벡터의 길이가 스케일
		float fScaleX_ = sqrtf(_11 * _11 + _12 * _12 + _13 * _13);
		float fScaleY_ = sqrtf(_21 * _21 + _22 * _22 + _23 * _23);
		float fScaleZ_ = sqrtf(_31 * _31 + _32 * _32 + _33 * _33);
		return _DVECTOR3(fScaleX_, fScaleY_, fScaleZ_);
	}

	// 행렬에서 회전 각도 추출 (라디안)
	// 참고: 스케일이 적용된 행렬에서는 스케일을 먼저 제거해야 정확한 값을 얻을 수 있음
	// @return: 회전 벡터 (x, y, z 각 축의 회전)
	_DVECTOR3 _DMATRIX9::GetRotationRadians() const noexcept
	{
		_DVECTOR3 v3Scale_ = GetScale();

		// 스케일 제거
		float f11_ = _11 / v3Scale_.x;
		float f21_ = _21 / v3Scale_.y;
		float f31_ = _31 / v3Scale_.z;
		float f32_ = _32 / v3Scale_.z;
		float f33_ = _33 / v3Scale_.z;

		// ZYX 오일러 각도 추출
		float fRotY_ = asinf(-f31_);
		float fRotX_ = 0.0f;
		float fRotZ_ = 0.0f;

		if (fabsf(f31_) < 0.9999f)
		{
			fRotX_ = atan2f(f32_, f33_);
			fRotZ_ = atan2f(f21_, f11_);
		}
		else
		{
			// Gimbal lock 상황
			fRotX_ = atan2f(-_23 / v3Scale_.y, _22 / v3Scale_.y);
			fRotZ_ = 0.0f;
		}

		return _DVECTOR3(fRotX_, fRotY_, fRotZ_);
	}

}
