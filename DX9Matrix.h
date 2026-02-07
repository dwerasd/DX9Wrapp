#pragma once


#include <Windows.h>

#include <DarkCore/DMemory.h>

#include "DX9Def.h"
#include "DX9Vector3.h"
#include "DX9Vector4.h"



namespace dx9
{
	struct _DMATRIX9
		: public _D3DXMATRIXA16
	{
		_DMATRIX9();
		_DMATRIX9(const float* f);
		_DMATRIX9(const float(*f)[4]);
		_DMATRIX9(
			float _f11, float _f12, float _f13, float _f14,
			float _f21, float _f22, float _f23, float _f24,
			float _f31, float _f32, float _f33, float _f34,
			float _f41, float _f42, float _f43, float _f44
		);

		_DMATRIX9 operator = (const D3DXMATRIX& other);
		_DMATRIX9 operator = (D3DXMATRIX* other);
		_DMATRIX9 operator = (const _DMATRIX9& other);
		_DMATRIX9 operator = (_DMATRIX9* other);
		//_DMATRIX9 operator=(const _DMATRIX9& other);

		bool operator==(const _DMATRIX9& other);
		bool operator!=(const _DMATRIX9& other);
		_DMATRIX9& operator*=(const _DMATRIX9& other);
		_DMATRIX9 operator*(const _DMATRIX9& other);
		_DMATRIX9 operator*(float f);
		_DMATRIX9 operator-(const _DMATRIX9& mat);	// 090716, OZ
		_DMATRIX9 operator+(const _DMATRIX9& mat);	// 090716, OZ

		// transform, mvector * mmatrix 로 대체가능.
		//friend _DVECTOR3 operator * (const _DVECTOR3& v, const _DMATRIX9& tm);
		//friend _DVECTOR4 operator * (const _DVECTOR4& v, const _DMATRIX9& tm);

		void Set(float f11, float f12, float f13, float f14,
			float f21, float f22, float f23, float f24,
			float f31, float f32, float f33, float f34,
			float f41, float f42, float f43, float f44
		);
		void Set(D3DXMATRIX* v);
		void Set(D3DXMATRIX v);
		void Set(const _DMATRIX9* v);
		void Set(_DMATRIX9 *v);
		void Set(_DMATRIX9 v);
		void Clear();

		void TransformVect(_DVECTOR3& vect) const;
		void TransformVectHomogeneous(_DVECTOR3& vect) const;
		void TransformVect(const _DVECTOR3& in, _DVECTOR3& out) const;
		void TransformVect(const _DVECTOR3& in, _DVECTOR4& out) const;
		void TransformNormal(_DVECTOR3& vect) const;
		void TransformNormal(const _DVECTOR3& in, _DVECTOR3& out) const;
		//void TransformPlane(const MPlane& in, MPlane& out) const;

		void MultiplyTo(const _DMATRIX9& other, _DMATRIX9& out) const; // out = this * other;

		void MakeZero();
		void MakeIdentity();

		// 콤포넌트만 세팅
		void SetTranslation(const _DVECTOR3& trans);
		void SetInverseTranslation(const _DVECTOR3& trans);
		_DVECTOR3 GetTranslation() const;
		void SetRotationDegrees(const _DVECTOR3& rotation);
		void SetRotationRadians(const _DVECTOR3& rotation);
		void SetScale(const _DVECTOR3& scale);
		void SetScale(float s);
		void SetRotationX(float fRadian);
		void SetRotationY(float fRadian);
		void SetRotationZ(float fRadian);

		// matrix 전체를 세팅해주는 헬퍼펑션들
		void SetRotationMatrix(float x, float y, float z) { SetRotationMatrix(_DVECTOR3(x, y, z)); }
		void SetRotationMatrix(const _DVECTOR3& rotation);// 단위 : 라디안
		void SetTranslationMatrix(float x, float y, float z) { SetTranslationMatrix(_DVECTOR3(x, y, z)); }
		void SetTranslationMatrix(const _DVECTOR3& trans);
		void SetScaleMatrix(float x, float y, float z) { SetScaleMatrix(_DVECTOR3(x, y, z)); }
		void SetScaleMatrix(const _DVECTOR3& scale);
		void SetProjectionMatrixRH(float w, float h, float zNear, float zFar);
		void SetProjectionMatrixLH(float w, float h, float zNear, float zFar);
		void SetProjectionMatrixFovRH(float fFovY, float fAspectRatio, float zNear, float zFar);
		void SetProjectionMatrixFovLH(float fFovY, float fAspectRatio, float zNear, float zFar);
		void SetLookAtMatrixLH(_DVECTOR3& position, _DVECTOR3& target, _DVECTOR3& upVector);
		void SetLookAtMatrixRH(_DVECTOR3& position, _DVECTOR3& target, _DVECTOR3& upVector);
		void SetOrthoLH(float w, float h, float zn, float zf);
		void SetOrthoRH(float w, float h, float zn, float zf);
		void SetOrthoOffCenterRH(float l, float r, float b, float t, float zn, float zf);
		void SetOrthoOffCenterLH(float l, float r, float b, float t, float zn, float zf);

		void SetRotationMatrixAxis(const _DVECTOR3& axis, float radian);
		void SetRotationYawPitchRoll(float yaw, float pitch, float roll);	// 테스트 되지 않았음.
		void SetLocalMatrix(_DVECTOR3& position, _DVECTOR3& dir, _DVECTOR3& up);
		void SetLocalMatrix(_DVECTOR3& position, _DVECTOR3& dir, _DVECTOR3& up, _DVECTOR3& right);

		void SetScreenSpaceMatrix(DWORD dwScreenWidth, DWORD dwScreenHeight);

		bool GetInverse(_DMATRIX9* pOut, float* fDet = nullptr) const;
		_DMATRIX9 GetTranspose() const;

		// 행렬식(Determinant) 계산
		// @return: 행렬식 값 (역행렬 존재 여부 판단에 사용)
		float Determinant() const noexcept;

		// 행렬이 단위행렬인지 확인
		// @param _fTolerance: 허용 오차
		// @return: 단위행렬이면 true
		bool IsIdentity(float _fTolerance = 0.0001f) const noexcept;

		// 행렬에서 스케일 벡터 추출
		// @return: 스케일 벡터 (x, y, z 각 축의 스케일)
		_DVECTOR3 GetScale() const noexcept;

		// 행렬에서 회전 각도 추출 (라디안)
		// @return: 회전 벡터 (x, y, z 각 축의 회전)
		_DVECTOR3 GetRotationRadians() const noexcept;

		static const _DMATRIX9 _IDENTITY;
	};

	inline void _DMATRIX9::MakeZero()
	{
		memset(&m, 0, sizeof(float) * 16);
	}

	inline void _DMATRIX9::MakeIdentity()
	{
		MakeZero();
		m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
	}

	inline void _DMATRIX9::SetTranslation(const _DVECTOR3& trans)
	{
		_41 = trans.x;
		_42 = trans.y;
		_43 = trans.z;
	}

	inline void _DMATRIX9::SetInverseTranslation(const _DVECTOR3& trans)
	{
		_41 = -trans.x;
		_42 = -trans.y;
		_43 = -trans.z;

	}

	inline _DVECTOR3 _DMATRIX9::GetTranslation() const
	{
		return _DVECTOR3(_41, _42, _43);
	}

	inline void _DMATRIX9::SetRotationDegrees(const _DVECTOR3& rotation)
	{
		_DVECTOR3 v;
		v.Set(rotation * (float)3.1415926535897932384626433832795 / 180.0);

		SetRotationRadians(v);
	}

	inline void _DMATRIX9::SetRotationRadians(const _DVECTOR3& rotation)
	{
		const double cr = cos(rotation.x);
		const double sr = sin(rotation.x);
		const double cp = cos(rotation.y);
		const double sp = sin(rotation.y);
		const double cy = cos(rotation.z);
		const double sy = sin(rotation.z);

		m[0][0] = float(cp * cy);
		m[0][1] = float(cp * sy);
		m[0][2] = float(-sp);

		const double srsp = sr * sp;
		const double crsp = cr * sp;

		m[1][0] = (float)(srsp * cy - cr * sy);
		m[1][1] = (float)(srsp * sy + cr * cy);
		m[1][2] = (float)(sr * cp);

		m[2][0] = (float)(crsp * cy + sr * sy);
		m[2][1] = (float)(crsp * sy - sr * cy);
		m[2][2] = (float)(cr * cp);
	}

	inline void _DMATRIX9::SetScale(const _DVECTOR3& scale)
	{
		_11 = scale.x;
		_22 = scale.y;
		_33 = scale.z;
	}

	inline void _DMATRIX9::SetScale(float s)
	{
		_11 = _22 = _33 = s;
	}

	inline void _DMATRIX9::SetRotationMatrix(const _DVECTOR3& rotation)
	{
		MakeIdentity();
		SetRotationRadians(rotation);
	}

	inline void _DMATRIX9::SetTranslationMatrix(const _DVECTOR3& trans)
	{
		MakeIdentity();
		SetTranslation(trans);
	}

	inline void _DMATRIX9::SetScaleMatrix(const _DVECTOR3& scale)
	{
		MakeIdentity();
		SetScale(scale);
	}

	inline void _DMATRIX9::TransformVect(_DVECTOR3& vect) const
	{
		float vec[3];

		vec[0] = vect.x * _11 + vect.y * _21 + vect.z * _31 + _41;
		vec[1] = vect.x * _12 + vect.y * _22 + vect.z * _32 + _42;
		vec[2] = vect.x * _13 + vect.y * _23 + vect.z * _33 + _43;

		vect.x = vec[0];
		vect.y = vec[1];
		vect.z = vec[2];
	}

	inline void _DMATRIX9::TransformVectHomogeneous(_DVECTOR3& vect) const
	{
		float vec[3];

		vec[0] = vect.x * _11 + vect.y * _21 + vect.z * _31 + _41;
		vec[1] = vect.x * _12 + vect.y * _22 + vect.z * _32 + _42;
		vec[2] = vect.x * _13 + vect.y * _23 + vect.z * _33 + _43;
		const float invw = 1.f / (vect.x * _14 + vect.y * _24 + vect.z * _34 + _44);

		vect.x = vec[0] * invw;
		vect.y = vec[1] * invw;
		vect.z = vec[2] * invw;

	}

	inline void _DMATRIX9::TransformVect(const _DVECTOR3& in, _DVECTOR3& out) const
	{
		//assert(&in!=&out); // 이경우 에러납니다.
		out.x = in.x * _11 + in.y * _21 + in.z * _31 + _41;
		out.y = in.x * _12 + in.y * _22 + in.z * _32 + _42;
		out.z = in.x * _13 + in.y * _23 + in.z * _33 + _43;
	}

	inline void _DMATRIX9::TransformVect(const _DVECTOR3& in, _DVECTOR4& out) const
	{
		out.x = in.x * _11 + in.y * _21 + in.z * _31 + _41;
		out.y = in.x * _12 + in.y * _22 + in.z * _32 + _42;
		out.z = in.x * _13 + in.y * _23 + in.z * _33 + _43;
		out.w = in.x * _14 + in.y * _24 + in.z * _34 + _44;
	}

	inline void _DMATRIX9::TransformNormal(_DVECTOR3& vect) const
	{
		float vec[3];

		vec[0] = vect.x * _11 + vect.y * _21 + vect.z * _31;
		vec[1] = vect.x * _12 + vect.y * _22 + vect.z * _32;
		vec[2] = vect.x * _13 + vect.y * _23 + vect.z * _33;

		vect.x = vec[0];
		vect.y = vec[1];
		vect.z = vec[2];

	}

	inline void _DMATRIX9::TransformNormal(const _DVECTOR3& in, _DVECTOR3& out) const
	{
		out.x = in.x * _11 + in.y * _21 + in.z * _31;
		out.y = in.x * _12 + in.y * _22 + in.z * _32;
		out.z = in.x * _13 + in.y * _23 + in.z * _33;
	}


	inline _DVECTOR3 operator * (const _DVECTOR3& v, const _DMATRIX9& tm)
	{
		_DVECTOR3 ret;
		tm.TransformVect(v, ret);
		return ret;
	}

	inline _DVECTOR4 operator * (const _DVECTOR4& v, const _DMATRIX9& tm)
	{
		_DVECTOR4 vec4;

		vec4.x = v.x * tm._11 + v.y * tm._21 + v.z * tm._31 + v.w * tm._41;
		vec4.y = v.x * tm._12 + v.y * tm._22 + v.z * tm._32 + v.w * tm._42;
		vec4.z = v.x * tm._13 + v.y * tm._23 + v.z * tm._33 + v.w * tm._43;
		vec4.w = v.x * tm._14 + v.y * tm._24 + v.z * tm._34 + v.w * tm._44;
		return vec4;
	}

	inline void _DMATRIX9::SetProjectionMatrixRH(float w, float h, float zNear, float zFar)
	{
		_11 = 2 * zNear / w;
		_12 = 0;
		_13 = 0;
		_14 = 0;

		_21 = 0;
		_22 = 2 * zNear / h;
		_23 = 0;
		_24 = 0;

		_31 = 0;
		_32 = 0;
		_33 = zFar / (zNear - zFar);
		_34 = -1;

		_41 = 0;
		_42 = 0;
		_43 = zNear * zFar / (zNear - zFar);
		_44 = 0;
	}

	inline void _DMATRIX9::SetProjectionMatrixLH(float w, float h, float zNear, float zFar)
	{
		_11 = 2 * zNear / w;
		_12 = 0;
		_13 = 0;
		_14 = 0;

		_21 = 0;
		_22 = 2 * zNear / h;
		_23 = 0;
		_24 = 0;

		_31 = 0;
		_32 = 0;
		_33 = zFar / (zFar - zNear);
		_34 = 1;

		_41 = 0;
		_42 = 0;
		_43 = zNear * zFar / (zNear - zFar);
		_44 = 0;
	}

	inline void _DMATRIX9::SetProjectionMatrixFovRH(float fFovY, float fAspectRatio, float zNear, float zFar)
	{
		const float yScale = 1 / tanf(fFovY / 2);
		const float xScale = yScale / fAspectRatio;	// dx9 문서에 잘못되어있다

		_11 = xScale;
		_12 = 0;
		_13 = 0;
		_14 = 0;

		_21 = 0;
		_22 = yScale;
		_23 = 0;
		_24 = 0;

		_31 = 0;
		_32 = 0;
		_33 = zFar / (zNear - zFar);
		_34 = -1;

		_41 = 0;
		_42 = 0;
		_43 = zNear * zFar / (zNear - zFar);
		_44 = 0;
	}

	inline void _DMATRIX9::SetProjectionMatrixFovLH(float fFovY, float fAspectRatio, float zNear, float zFar)
	{
		const float yScale = 1 / tanf(fFovY / 2);
		const float xScale = yScale / fAspectRatio;	// dx9 문서에 잘못되어있다

		_11 = xScale;
		_12 = 0;
		_13 = 0;
		_14 = 0;

		_21 = 0;
		_22 = yScale;
		_23 = 0;
		_24 = 0;

		_31 = 0;
		_32 = 0;
		_33 = zFar / (zFar - zNear);
		_34 = 1;

		_41 = 0;
		_42 = 0;
		_43 = zNear * zFar / (zNear - zFar);
		_44 = 0;
	}

	inline void _DMATRIX9::SetLookAtMatrixRH(_DVECTOR3& eye, _DVECTOR3& at, _DVECTOR3& up)
	{
		_DVECTOR3 zaxis = eye - at;
		zaxis.Normalize();

		_DVECTOR3 xaxis = up.CrossProduct(&zaxis);

		xaxis.Normalize();

		_DVECTOR3 yaxis = zaxis.CrossProduct(&xaxis);
		yaxis.Normalize();

		_11 = xaxis.x;
		_12 = yaxis.x;
		_13 = zaxis.x;
		_14 = 0;

		_21 = xaxis.y;
		_22 = yaxis.y;
		_23 = zaxis.y;
		_24 = 0;

		_31 = xaxis.z;
		_32 = yaxis.z;
		_33 = zaxis.z;
		_34 = 0;

		_41 = -xaxis.DotProduct(&eye);
		_42 = -yaxis.DotProduct(&eye);
		_43 = -zaxis.DotProduct(&eye);
		_44 = 1.0f;
	}

	inline void _DMATRIX9::SetLookAtMatrixLH(_DVECTOR3& eye, _DVECTOR3& at, _DVECTOR3& up)
	{
		_DVECTOR3 zaxis = at - eye;
		zaxis.Normalize();

		_DVECTOR3 xaxis = up.CrossProduct(&zaxis);
		xaxis.Normalize();

		_DVECTOR3 yaxis = zaxis.CrossProduct(&xaxis);
		yaxis.Normalize();

		_11 = xaxis.x;
		_12 = yaxis.x;
		_13 = zaxis.x;
		_14 = 0;

		_21 = xaxis.y;
		_22 = yaxis.y;
		_23 = zaxis.y;
		_24 = 0;

		_31 = xaxis.z;
		_32 = yaxis.z;
		_33 = zaxis.z;
		_34 = 0;

		_41 = -xaxis.DotProduct(&eye);
		_42 = -yaxis.DotProduct(&eye);
		_43 = -zaxis.DotProduct(&eye);
		_44 = 1.0f;
	}

	inline void _DMATRIX9::SetOrthoLH(float w, float h, float zn, float zf)
	{
		_11 = 2.0f / w;	_12 = 0.0f;		_13 = 0.0f;			_14 = 0.0f;
		_21 = 0.0f;		_22 = 2.0f / h;	_23 = 0.0f;			_24 = 0.0f;
		_31 = 0.0f;		_32 = 0.0f;		_33 = 1.0f / (zf - zn);	_34 = 0.0f;
		_41 = 0.0f;		_42 = 0.0f;		_43 = zn / (zn - zf);	_44 = 1.0f;
	}

	inline void _DMATRIX9::SetOrthoRH(float w, float h, float zn, float zf)
	{
		_11 = 2.0f / w;	_12 = 0.0f;		_13 = 0.0f;			_14 = 0.0f;
		_21 = 0.0f;		_22 = 2.0f / h;	_23 = 0.0f;			_24 = 0.0f;
		_31 = 0.0f;		_32 = 0.0f;		_33 = 1.0f / (zn - zf);	_34 = 0.0f;
		_41 = 0.0f;		_42 = 0.0f;		_43 = zn / (zn - zf);	_44 = 1.0f;
	}

	inline void _DMATRIX9::SetOrthoOffCenterRH(float l, float r, float b, float t, float zn, float zf)
	{
		_11 = 2.0f / (r - l);	_12 = 0.0f;	_13 = 0.0f;	_14 = 0.0f;
		_21 = 0.0f;	_22 = 2.0f / (t - b);	_23 = 0.0f;	_24 = 0.0f;
		_31 = 0.0f;	_32 = 0.0f;	_33 = 1.0f / (zn - zf);	_34 = 0.0f;
		_41 = (l + r) / (l - r);_42 = (b + t) / (b - t);	_43 = zn / (zn - zf);	_44 = 1.0f;
	}

	inline void _DMATRIX9::SetOrthoOffCenterLH(float l, float r, float b, float t, float zn, float zf)
	{
		_11 = 2.0f / (r - l);	_12 = 0.0f;	_13 = 0.0f;	_14 = 0.0f;
		_21 = 0.0f;	_22 = 2.0f / (t - b);	_23 = 0.0f;	_24 = 0.0f;
		_31 = 0.0f;	_32 = 0.0f;	_33 = 1.0f / (zf - zn);	_34 = 0.0f;
		_41 = (l + r) / (l - r);_42 = (b + t) / (b - t);	_43 = -zn / (zf - zn);	_44 = 1.0f;
	}

	inline _DMATRIX9 _DMATRIX9::GetTranspose() const
	{
		_DMATRIX9 ret;

		for (int i = 0;i < 4; ++i)
		{
			for (int j = 0;j < 4; ++j)
			{
				ret.m[j][i] = m[i][j];
			}
		}
		return ret;
	}
	/*
	inline void _DMATRIX9::TransformPlane(const MPlane& in, MPlane& out) const
	{
		_DMATRIX9 matrix;
		GetInverse(&matrix);
		matrix = matrix.GetTranspose();

		out = MPlane((float*)(_DVECTOR4((const float*)in) * matrix));



		//_DVECTOR3 aPoint = - in.Normal() * in.d;
		//TransformVect(aPoint);

		//_DVECTOR3 normal = in.Normal();
		//_DVECTOR3 origin(0,0,0);
		//TransformVect(normal);
		//TransformVect(origin);
		//normal -= origin;
		//out.SetPlane(normal,aPoint);

	}
	*/
	inline void _DMATRIX9::SetRotationMatrixAxis(const _DVECTOR3& axis, float radian)
	{
		const float c = cosf(radian);
		const float s = sinf(radian);

		const float xx = axis.x * axis.x;
		const float yy = axis.y * axis.y;
		const float zz = axis.z * axis.z;

		const float xy = axis.x * axis.y;
		const float xz = axis.x * axis.z;
		const float yz = axis.y * axis.z;

		_11 = c + (1.f - c) * xx;
		_12 = (1 - c) * xy + s * axis.z;
		_13 = (1 - c) * xz - s * axis.y;
		_14 = 0.0f;

		_21 = (1 - c) * xy - s * axis.z;
		_22 = c + (1 - c) * yy;
		_23 = (1 - c) * yz + s * axis.x;
		_24 = 0.0f;

		_31 = (1 - c) * xz + s * axis.y;
		_32 = (1 - c) * yz - s * axis.x;
		_33 = c + (1 - c) * zz;
		_34 = 0.0f;

		_41 = 0.0f;
		_42 = 0.0f;
		_43 = 0.0f;
		_44 = 1.0f;

		return;
	}

	inline void _DMATRIX9::SetRotationYawPitchRoll(float yaw, float pitch, float roll)
	{
		// Z, X, Y 축 순서로 곱한다.
		// 찾아본 결과로 Yaw가 Up벡터 축을 뜻한다.

		float cy = cosf(yaw);
		float sy = sinf(yaw);

		float cp = cosf(pitch);
		float sp = sinf(pitch);
		float cr = cosf(roll);
		float sr = sinf(roll);

		_11 = cy * cp;
		_12 = sy * cp;
		_13 = -sy;
		_14 = 0.0f;

		_21 = cy * sp * sr - sy * sr;
		_22 = sy * sp * sr + cy * cr;
		_23 = cp * sy;
		_24 = 0.0f;

		_31 = cy * sp * cr + sy * sr;
		_32 = sy * sp * cr - cy * sp;
		_33 = cy * cr;
		_34 = 0.0f;

		_41 = _42 = _43 = 0.0f;
		_44 = 1.0f;

		/*
		// 폐기된 코드
		_DMATRIX9 z, y, x;

		z.SetRotationRadians(_DVECTOR3(0,0,yaw));
		x.SetRotationRadians(_DVECTOR3(pitch, 0, 0));
		y.SetRotationRadians(_DVECTOR3(0,-roll, 0));
		*this = z * x * y;
		*/
	}

	inline void _DMATRIX9::SetLocalMatrix(_DVECTOR3& position, _DVECTOR3& dir, _DVECTOR3& up, _DVECTOR3& right)
	{
		_11 = right.x;
		_12 = right.y;
		_13 = right.z;
		_14 = 0;

		_21 = dir.x;
		_22 = dir.y;
		_23 = dir.z;
		_24 = 0;

		_31 = up.x;
		_32 = up.y;
		_33 = up.z;
		_34 = 0;

		_41 = position.x;
		_42 = position.y;
		_43 = position.z;
		_44 = 1.0f;
	}

	inline void _DMATRIX9::SetScreenSpaceMatrix(DWORD dwScreenWidth, DWORD dwScreenHeight)
	{
		MakeIdentity();
		SetTranslation(_DVECTOR3(0.5f + 0.5f / dwScreenWidth, 0.5f + 0.5f / dwScreenHeight, 0.0f));
		SetScale(_DVECTOR3(0.5f, -0.5f, 1.0f));
	}

	inline bool _DMATRIX9::GetInverse(_DMATRIX9* pOut, float* fDet) const
	{
		/// The inverse is calculated using Cramers rule.
		/// If no inverse exists then 'false' is returned.

		const _DMATRIX9& pMatrix = *this;

		float d = (pMatrix._11 * pMatrix._22 - pMatrix._12 * pMatrix._21) * (pMatrix._33 * pMatrix._44 - pMatrix._34 * pMatrix._43) - (pMatrix._11 * pMatrix._23 - pMatrix._13 * pMatrix._21) * (pMatrix._32 * pMatrix._44 - pMatrix._34 * pMatrix._42)
			+ (pMatrix._11 * pMatrix._24 - pMatrix._14 * pMatrix._21) * (pMatrix._32 * pMatrix._43 - pMatrix._33 * pMatrix._42) + (pMatrix._12 * pMatrix._23 - pMatrix._13 * pMatrix._22) * (pMatrix._31 * pMatrix._44 - pMatrix._34 * pMatrix._41)
			- (pMatrix._12 * pMatrix._24 - pMatrix._14 * pMatrix._22) * (pMatrix._31 * pMatrix._43 - pMatrix._33 * pMatrix._41) + (pMatrix._13 * pMatrix._24 - pMatrix._14 * pMatrix._23) * (pMatrix._31 * pMatrix._42 - pMatrix._32 * pMatrix._41);

		if (fDet)
			*fDet = d;

		if (d == 0.f)
			return false;

		d = 1.f / d;

		_DMATRIX9& out = *pOut;

		out._11 = d * (pMatrix._22 * (pMatrix._33 * pMatrix._44 - pMatrix._34 * pMatrix._43) + pMatrix._23 * (pMatrix._34 * pMatrix._42 - pMatrix._32 * pMatrix._44) + pMatrix._24 * (pMatrix._32 * pMatrix._43 - pMatrix._33 * pMatrix._42));
		out._12 = d * (pMatrix._32 * (pMatrix._13 * pMatrix._44 - pMatrix._14 * pMatrix._43) + pMatrix._33 * (pMatrix._14 * pMatrix._42 - pMatrix._12 * pMatrix._44) + pMatrix._34 * (pMatrix._12 * pMatrix._43 - pMatrix._13 * pMatrix._42));
		out._13 = d * (pMatrix._42 * (pMatrix._13 * pMatrix._24 - pMatrix._14 * pMatrix._23) + pMatrix._43 * (pMatrix._14 * pMatrix._22 - pMatrix._12 * pMatrix._24) + pMatrix._44 * (pMatrix._12 * pMatrix._23 - pMatrix._13 * pMatrix._22));
		out._14 = d * (pMatrix._12 * (pMatrix._24 * pMatrix._33 - pMatrix._23 * pMatrix._34) + pMatrix._13 * (pMatrix._22 * pMatrix._34 - pMatrix._24 * pMatrix._32) + pMatrix._14 * (pMatrix._23 * pMatrix._32 - pMatrix._22 * pMatrix._33));
		out._21 = d * (pMatrix._23 * (pMatrix._31 * pMatrix._44 - pMatrix._34 * pMatrix._41) + pMatrix._24 * (pMatrix._33 * pMatrix._41 - pMatrix._31 * pMatrix._43) + pMatrix._21 * (pMatrix._34 * pMatrix._43 - pMatrix._33 * pMatrix._44));
		out._22 = d * (pMatrix._33 * (pMatrix._11 * pMatrix._44 - pMatrix._14 * pMatrix._41) + pMatrix._34 * (pMatrix._13 * pMatrix._41 - pMatrix._11 * pMatrix._43) + pMatrix._31 * (pMatrix._14 * pMatrix._43 - pMatrix._13 * pMatrix._44));
		out._23 = d * (pMatrix._43 * (pMatrix._11 * pMatrix._24 - pMatrix._14 * pMatrix._21) + pMatrix._44 * (pMatrix._13 * pMatrix._21 - pMatrix._11 * pMatrix._23) + pMatrix._41 * (pMatrix._14 * pMatrix._23 - pMatrix._13 * pMatrix._24));
		out._24 = d * (pMatrix._13 * (pMatrix._24 * pMatrix._31 - pMatrix._21 * pMatrix._34) + pMatrix._14 * (pMatrix._21 * pMatrix._33 - pMatrix._23 * pMatrix._31) + pMatrix._11 * (pMatrix._23 * pMatrix._34 - pMatrix._24 * pMatrix._33));
		out._31 = d * (pMatrix._24 * (pMatrix._31 * pMatrix._42 - pMatrix._32 * pMatrix._41) + pMatrix._21 * (pMatrix._32 * pMatrix._44 - pMatrix._34 * pMatrix._42) + pMatrix._22 * (pMatrix._34 * pMatrix._41 - pMatrix._31 * pMatrix._44));
		out._32 = d * (pMatrix._34 * (pMatrix._11 * pMatrix._42 - pMatrix._12 * pMatrix._41) + pMatrix._31 * (pMatrix._12 * pMatrix._44 - pMatrix._14 * pMatrix._42) + pMatrix._32 * (pMatrix._14 * pMatrix._41 - pMatrix._11 * pMatrix._44));
		out._33 = d * (pMatrix._44 * (pMatrix._11 * pMatrix._22 - pMatrix._12 * pMatrix._21) + pMatrix._41 * (pMatrix._12 * pMatrix._24 - pMatrix._14 * pMatrix._22) + pMatrix._42 * (pMatrix._14 * pMatrix._21 - pMatrix._11 * pMatrix._24));
		out._34 = d * (pMatrix._14 * (pMatrix._22 * pMatrix._31 - pMatrix._21 * pMatrix._32) + pMatrix._11 * (pMatrix._24 * pMatrix._32 - pMatrix._22 * pMatrix._34) + pMatrix._12 * (pMatrix._21 * pMatrix._34 - pMatrix._24 * pMatrix._31));
		out._41 = d * (pMatrix._21 * (pMatrix._33 * pMatrix._42 - pMatrix._32 * pMatrix._43) + pMatrix._22 * (pMatrix._31 * pMatrix._43 - pMatrix._33 * pMatrix._41) + pMatrix._23 * (pMatrix._32 * pMatrix._41 - pMatrix._31 * pMatrix._42));
		out._42 = d * (pMatrix._31 * (pMatrix._13 * pMatrix._42 - pMatrix._12 * pMatrix._43) + pMatrix._32 * (pMatrix._11 * pMatrix._43 - pMatrix._13 * pMatrix._41) + pMatrix._33 * (pMatrix._12 * pMatrix._41 - pMatrix._11 * pMatrix._42));
		out._43 = d * (pMatrix._41 * (pMatrix._13 * pMatrix._22 - pMatrix._12 * pMatrix._23) + pMatrix._42 * (pMatrix._11 * pMatrix._23 - pMatrix._13 * pMatrix._21) + pMatrix._43 * (pMatrix._12 * pMatrix._21 - pMatrix._11 * pMatrix._22));
		out._44 = d * (pMatrix._11 * (pMatrix._22 * pMatrix._33 - pMatrix._23 * pMatrix._32) + pMatrix._12 * (pMatrix._23 * pMatrix._31 - pMatrix._21 * pMatrix._33) + pMatrix._13 * (pMatrix._21 * pMatrix._32 - pMatrix._22 * pMatrix._31));

		return true;

	}

	inline void _DMATRIX9::SetLocalMatrix(_DVECTOR3& position, _DVECTOR3& dir, _DVECTOR3& up)
	{
		_DVECTOR3 NRight = dir.CrossProduct(&up);

		if (NRight == _DVECTOR3::ZERO)
		{
			NRight = dir.CrossProduct(_DVECTOR3(0, 1, 0));
		}

		_DVECTOR3 NDir = dir;
		//_DVECTOR3 NUp = CrossProduct(NRight, dir);
		_DVECTOR3 NUp = NRight.CrossProduct(&dir);

		// 회전 값만 있는 행렬을 만들기 위해서 각 축성분을 Normalize해줘야 한다.
		NRight.Normalize();
		NDir.Normalize();
		NUp.Normalize();

		SetLocalMatrix(position, NDir, NUp, NRight);
	}

	inline void _DMATRIX9::SetRotationX(float fRadian)
	{
		float c = cosf(fRadian);
		float s = sinf(fRadian);

		MakeIdentity();
		_22 = c;	_23 = s;
		_32 = -s;	_33 = c;
	}

	inline void _DMATRIX9::SetRotationY(float fRadian)
	{
		float c = cosf(fRadian);
		float s = sinf(fRadian);

		MakeIdentity();
		_11 = c;	_13 = -s;
		_31 = s;	_33 = c;
	}

	inline void _DMATRIX9::SetRotationZ(float fRadian)
	{
		float c = cosf(fRadian);
		float s = sinf(fRadian);

		MakeIdentity();
		_11 = c;	_12 = s;
		_21 = -s;	_22 = c;
	}
}
