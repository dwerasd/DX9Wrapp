#pragma once

/**
 * @file DX9Math.h
 * @brief DX9 수학 유틸리티 함수 모음
 * 
 * 게임 및 그래픽스 프로그래밍에서 자주 사용되는 수학 함수들을 제공합니다.
 * 모든 함수는 constexpr/noexcept를 최대한 활용하여 컴파일 타임 최적화를 지원합니다.
 */

#include <cmath>
#include <cfloat>



namespace dx9
{
	//=============================================================================
	// 상수 정의
	//=============================================================================

	// 원주율 π
	constexpr float DX9_PI = 3.14159265358979323846f;
	// 2π
	constexpr float DX9_2PI = 6.28318530717958647692f;
	// π/2
	constexpr float DX9_HALF_PI = 1.57079632679489661923f;
	// π/4
	constexpr float DX9_QUARTER_PI = 0.78539816339744830962f;
	// 1/π
	constexpr float DX9_INV_PI = 0.31830988618379067154f;
	// 도(degree)를 라디안으로 변환하는 계수
	constexpr float DX9_DEG_TO_RAD = 0.01745329251994329577f;
	// 라디안을 도(degree)로 변환하는 계수
	constexpr float DX9_RAD_TO_DEG = 57.29577951308232087680f;
	// 매우 작은 값 (부동소수점 비교용)
	constexpr float DX9_EPSILON = 1e-6f;
	// 황금비
	constexpr float DX9_GOLDEN_RATIO = 1.61803398874989484820f;

	//=============================================================================
	// 기본 수학 함수
	//=============================================================================

	/**
	 * @brief 값을 지정된 범위로 제한합니다.
	 * @param _fValue: 제한할 값
	 * @param _fMin: 최소값
	 * @param _fMax: 최대값
	 * @return: _fMin과 _fMax 사이로 제한된 값
	 * 
	 * @example
	 * float f = dx9::Clamp(1.5f, 0.0f, 1.0f);  // 결과: 1.0f
	 * float g = dx9::Clamp(-0.5f, 0.0f, 1.0f); // 결과: 0.0f
	 */
	inline constexpr float Clamp(float _fValue, float _fMin, float _fMax) noexcept
	{
		return (_fValue < _fMin) ? _fMin : ((_fValue > _fMax) ? _fMax : _fValue);
	}

	/**
	 * @brief 정수 값을 지정된 범위로 제한합니다.
	 * @param _nValue: 제한할 값
	 * @param _nMin: 최소값
	 * @param _nMax: 최대값
	 * @return: _nMin과 _nMax 사이로 제한된 값
	 */
	inline constexpr int ClampInt(int _nValue, int _nMin, int _nMax) noexcept
	{
		return (_nValue < _nMin) ? _nMin : ((_nValue > _nMax) ? _nMax : _nValue);
	}

	/**
	 * @brief 값을 0~1 범위로 제한합니다.
	 * @param _fValue: 제한할 값
	 * @return: 0.0f ~ 1.0f 범위로 제한된 값
	 */
	inline constexpr float Saturate(float _fValue) noexcept
	{
		return Clamp(_fValue, 0.0f, 1.0f);
	}

	/**
	 * @brief 선형 보간 (Linear Interpolation)
	 * @param _fFrom: 시작값
	 * @param _fTo: 끝값
	 * @param _fT: 보간 계수 (0.0 ~ 1.0)
	 * @return: 보간된 값
	 * 
	 * @example
	 * float f = dx9::Lerp(0.0f, 10.0f, 0.5f);  // 결과: 5.0f
	 * float g = dx9::Lerp(0.0f, 10.0f, 0.25f); // 결과: 2.5f
	 */
	inline constexpr float Lerp(float _fFrom, float _fTo, float _fT) noexcept
	{
		return _fFrom + (_fTo - _fFrom) * _fT;
	}

	/**
	 * @brief 역 선형 보간 (Inverse Lerp)
	 * 값이 범위 내에서 어느 위치에 있는지 0~1 사이의 값으로 반환
	 * @param _fFrom: 범위 시작값
	 * @param _fTo: 범위 끝값
	 * @param _fValue: 위치를 알고 싶은 값
	 * @return: 0.0 ~ 1.0 사이의 위치 값
	 * 
	 * @example
	 * float f = dx9::InverseLerp(0.0f, 10.0f, 5.0f);  // 결과: 0.5f
	 */
	inline float InverseLerp(float _fFrom, float _fTo, float _fValue) noexcept
	{
		const float fDenom_ = _fTo - _fFrom;
		if (fabsf(fDenom_) < DX9_EPSILON)
			return 0.0f;
		return (_fValue - _fFrom) / fDenom_;
	}

	/**
	 * @brief 값을 한 범위에서 다른 범위로 매핑합니다.
	 * @param _fValue: 변환할 값
	 * @param _fInMin: 입력 범위 최소값
	 * @param _fInMax: 입력 범위 최대값
	 * @param _fOutMin: 출력 범위 최소값
	 * @param _fOutMax: 출력 범위 최대값
	 * @return: 새 범위로 매핑된 값
	 * 
	 * @example
	 * // 0~100 범위의 값을 0~1 범위로 변환
	 * float f = dx9::Remap(50.0f, 0.0f, 100.0f, 0.0f, 1.0f);  // 결과: 0.5f
	 */
	inline float Remap(float _fValue, float _fInMin, float _fInMax, float _fOutMin, float _fOutMax) noexcept
	{
		const float fT_ = InverseLerp(_fInMin, _fInMax, _fValue);
		return Lerp(_fOutMin, _fOutMax, fT_);
	}

	/**
	 * @brief 부드러운 보간 (Hermite Smoothstep)
	 * 3차 Hermite 보간으로 부드러운 시작과 끝을 가짐
	 * @param _fEdge0: 시작 경계
	 * @param _fEdge1: 끝 경계
	 * @param _fX: 입력값
	 * @return: 부드럽게 보간된 0~1 값
	 * 
	 * @example
	 * // 페이드 효과에 활용
	 * float fAlpha = dx9::SmoothStep(0.0f, 1.0f, fTime);
	 */
	inline float SmoothStep(float _fEdge0, float _fEdge1, float _fX) noexcept
	{
		const float fT_ = Saturate(InverseLerp(_fEdge0, _fEdge1, _fX));
		return fT_ * fT_ * (3.0f - 2.0f * fT_);
	}

	/**
	 * @brief 더 부드러운 보간 (Perlin의 SmootherStep)
	 * 5차 보간으로 1차, 2차 미분도 0에서 시작하여 더욱 부드러움
	 * @param _fEdge0: 시작 경계
	 * @param _fEdge1: 끝 경계
	 * @param _fX: 입력값
	 * @return: 매우 부드럽게 보간된 0~1 값
	 */
	inline float SmootherStep(float _fEdge0, float _fEdge1, float _fX) noexcept
	{
		const float fT_ = Saturate(InverseLerp(_fEdge0, _fEdge1, _fX));
		return fT_ * fT_ * fT_ * (fT_ * (fT_ * 6.0f - 15.0f) + 10.0f);
	}

	//=============================================================================
	// 각도 변환 함수
	//=============================================================================

	/**
	 * @brief 도(degree)를 라디안으로 변환합니다.
	 * @param _fDegrees: 변환할 각도 (도)
	 * @return: 라디안 값
	 */
	inline constexpr float DegToRad(float _fDegrees) noexcept
	{
		return _fDegrees * DX9_DEG_TO_RAD;
	}

	/**
	 * @brief 라디안을 도(degree)로 변환합니다.
	 * @param _fRadians: 변환할 각도 (라디안)
	 * @return: 도 값
	 */
	inline constexpr float RadToDeg(float _fRadians) noexcept
	{
		return _fRadians * DX9_RAD_TO_DEG;
	}

	/**
	 * @brief 각도를 -180 ~ 180 범위로 정규화합니다.
	 * @param _fDegrees: 정규화할 각도 (도)
	 * @return: -180 ~ 180 범위의 각도
	 */
	inline float NormalizeAngleDeg(float _fDegrees) noexcept
	{
		float fResult_ = fmodf(_fDegrees + 180.0f, 360.0f);
		if (fResult_ < 0.0f)
			fResult_ += 360.0f;
		return fResult_ - 180.0f;
	}

	/**
	 * @brief 각도를 -π ~ π 범위로 정규화합니다.
	 * @param _fRadians: 정규화할 각도 (라디안)
	 * @return: -π ~ π 범위의 각도
	 */
	inline float NormalizeAngleRad(float _fRadians) noexcept
	{
		float fResult_ = fmodf(_fRadians + DX9_PI, DX9_2PI);
		if (fResult_ < 0.0f)
			fResult_ += DX9_2PI;
		return fResult_ - DX9_PI;
	}

	/**
	 * @brief 두 각도 사이의 최단 거리를 계산합니다 (도 단위).
	 * @param _fFrom: 시작 각도 (도)
	 * @param _fTo: 목표 각도 (도)
	 * @return: 최단 각도 차이 (-180 ~ 180)
	 */
	inline float DeltaAngleDeg(float _fFrom, float _fTo) noexcept
	{
		return NormalizeAngleDeg(_fTo - _fFrom);
	}

	/**
	 * @brief 두 각도 사이를 부드럽게 보간합니다 (도 단위).
	 * @param _fFrom: 시작 각도 (도)
	 * @param _fTo: 목표 각도 (도)
	 * @param _fT: 보간 계수 (0.0 ~ 1.0)
	 * @return: 보간된 각도
	 */
	inline float LerpAngleDeg(float _fFrom, float _fTo, float _fT) noexcept
	{
		const float fDelta_ = DeltaAngleDeg(_fFrom, _fTo);
		return _fFrom + fDelta_ * _fT;
	}

	//=============================================================================
	// 비교 함수
	//=============================================================================

	/**
	 * @brief 두 부동소수점 값이 거의 같은지 비교합니다.
	 * @param _fA: 첫 번째 값
	 * @param _fB: 두 번째 값
	 * @param _fTolerance: 허용 오차 (기본값: DX9_EPSILON)
	 * @return: 차이가 허용 오차 이내이면 true
	 */
	inline bool Approximately(float _fA, float _fB, float _fTolerance = DX9_EPSILON) noexcept
	{
		return fabsf(_fB - _fA) <= _fTolerance;
	}

	/**
	 * @brief 값이 0에 가까운지 확인합니다.
	 * @param _fValue: 확인할 값
	 * @param _fTolerance: 허용 오차 (기본값: DX9_EPSILON)
	 * @return: 0에 가까우면 true
	 */
	inline bool IsNearlyZero(float _fValue, float _fTolerance = DX9_EPSILON) noexcept
	{
		return fabsf(_fValue) <= _fTolerance;
	}

	/**
	 * @brief 두 값 중 최소값을 반환합니다.
	 * @param _fA: 첫 번째 값
	 * @param _fB: 두 번째 값
	 * @return: 더 작은 값
	 */
	inline constexpr float Min(float _fA, float _fB) noexcept
	{
		return (_fA < _fB) ? _fA : _fB;
	}

	/**
	 * @brief 두 값 중 최대값을 반환합니다.
	 * @param _fA: 첫 번째 값
	 * @param _fB: 두 번째 값
	 * @return: 더 큰 값
	 */
	inline constexpr float Max(float _fA, float _fB) noexcept
	{
		return (_fA > _fB) ? _fA : _fB;
	}

	/**
	 * @brief 세 값 중 최소값을 반환합니다.
	 */
	inline constexpr float Min3(float _fA, float _fB, float _fC) noexcept
	{
		return Min(Min(_fA, _fB), _fC);
	}

	/**
	 * @brief 세 값 중 최대값을 반환합니다.
	 */
	inline constexpr float Max3(float _fA, float _fB, float _fC) noexcept
	{
		return Max(Max(_fA, _fB), _fC);
	}

	//=============================================================================
	// 부호 및 절대값 함수
	//=============================================================================

	/**
	 * @brief 값의 부호를 반환합니다.
	 * @param _fValue: 입력 값
	 * @return: 양수면 1.0f, 음수면 -1.0f, 0이면 0.0f
	 */
	inline constexpr float Sign(float _fValue) noexcept
	{
		return (_fValue > 0.0f) ? 1.0f : ((_fValue < 0.0f) ? -1.0f : 0.0f);
	}

	/**
	 * @brief 절대값을 반환합니다.
	 * @param _fValue: 입력 값
	 * @return: 절대값
	 */
	inline constexpr float Abs(float _fValue) noexcept
	{
		return (_fValue < 0.0f) ? -_fValue : _fValue;
	}

	/**
	 * @brief 값을 목표 방향으로 일정량 이동시킵니다.
	 * @param _fCurrent: 현재 값
	 * @param _fTarget: 목표 값
	 * @param _fMaxDelta: 최대 이동량
	 * @return: 이동된 값 (목표를 초과하지 않음)
	 * 
	 * @example
	 * // 매 프레임 0.1씩 목표로 이동
	 * fValue = dx9::MoveTowards(fValue, fTarget, 0.1f * fDeltaTime);
	 */
	inline float MoveTowards(float _fCurrent, float _fTarget, float _fMaxDelta) noexcept
	{
		const float fDiff_ = _fTarget - _fCurrent;
		if (fabsf(fDiff_) <= _fMaxDelta)
			return _fTarget;
		return _fCurrent + Sign(fDiff_) * _fMaxDelta;
	}

	//=============================================================================
	// 랩핑 및 반복 함수
	//=============================================================================

	/**
	 * @brief 값을 지정된 범위 내에서 반복시킵니다 (모듈러 연산).
	 * @param _fValue: 입력 값
	 * @param _fLength: 반복 길이 (0 ~ _fLength)
	 * @return: 0 ~ _fLength 범위로 반복된 값
	 * 
	 * @example
	 * float f = dx9::Repeat(3.5f, 2.0f);  // 결과: 1.5f
	 * float g = dx9::Repeat(-0.5f, 2.0f); // 결과: 1.5f
	 */
	inline float Repeat(float _fValue, float _fLength) noexcept
	{
		return Clamp(_fValue - floorf(_fValue / _fLength) * _fLength, 0.0f, _fLength);
	}

	/**
	 * @brief 값을 지정된 범위 내에서 핑퐁(왕복)시킵니다.
	 * @param _fValue: 입력 값 (시간 등)
	 * @param _fLength: 반복 길이
	 * @return: 0 ~ _fLength 사이를 왕복하는 값
	 * 
	 * @example
	 * // 알파값을 0~1 사이에서 왕복
	 * float fAlpha = dx9::PingPong(fTime, 1.0f);
	 */
	inline float PingPong(float _fValue, float _fLength) noexcept
	{
		const float fT_ = Repeat(_fValue, _fLength * 2.0f);
		return _fLength - fabsf(fT_ - _fLength);
	}

	//=============================================================================
	// 제곱근 및 거듭제곱 함수
	//=============================================================================

	/**
	 * @brief 빠른 역제곱근 (Fast Inverse Square Root)
	 * Quake III의 유명한 알고리즘 기반
	 * @param _fNumber: 입력 값
	 * @return: 1 / sqrt(_fNumber)의 근사값
	 * 
	 * @note 정확도는 낮지만 속도가 빠름. 정확한 값이 필요하면 1.0f / sqrtf() 사용
	 */
	inline float FastInvSqrt(float _fNumber) noexcept
	{
		const float fHalf_ = 0.5f * _fNumber;
		int i_ = *reinterpret_cast<int*>(&_fNumber);
		i_ = 0x5f3759df - (i_ >> 1);
		_fNumber = *reinterpret_cast<float*>(&i_);
		_fNumber = _fNumber * (1.5f - fHalf_ * _fNumber * _fNumber);  // 1회 뉴턴-랩슨 반복
		return _fNumber;
	}

	/**
	 * @brief 값의 제곱을 반환합니다.
	 * @param _fValue: 입력 값
	 * @return: _fValue * _fValue
	 */
	inline constexpr float Square(float _fValue) noexcept
	{
		return _fValue * _fValue;
	}

	/**
	 * @brief 값의 세제곱을 반환합니다.
	 * @param _fValue: 입력 값
	 * @return: _fValue * _fValue * _fValue
	 */
	inline constexpr float Cube(float _fValue) noexcept
	{
		return _fValue * _fValue * _fValue;
	}

	//=============================================================================
	// 이징 함수 (Easing Functions)
	//=============================================================================

	/**
	 * @brief 이차 함수 Ease-In (느리게 시작)
	 * @param _fT: 진행도 (0.0 ~ 1.0)
	 * @return: 이징된 값
	 */
	inline constexpr float EaseInQuad(float _fT) noexcept
	{
		return _fT * _fT;
	}

	/**
	 * @brief 이차 함수 Ease-Out (느리게 끝남)
	 * @param _fT: 진행도 (0.0 ~ 1.0)
	 * @return: 이징된 값
	 */
	inline constexpr float EaseOutQuad(float _fT) noexcept
	{
		return _fT * (2.0f - _fT);
	}

	/**
	 * @brief 이차 함수 Ease-In-Out (느리게 시작하고 느리게 끝남)
	 * @param _fT: 진행도 (0.0 ~ 1.0)
	 * @return: 이징된 값
	 */
	inline constexpr float EaseInOutQuad(float _fT) noexcept
	{
		return (_fT < 0.5f) ? (2.0f * _fT * _fT) : (-1.0f + (4.0f - 2.0f * _fT) * _fT);
	}

	/**
	 * @brief 삼차 함수 Ease-In
	 */
	inline constexpr float EaseInCubic(float _fT) noexcept
	{
		return _fT * _fT * _fT;
	}

	/**
	 * @brief 삼차 함수 Ease-Out
	 */
	inline constexpr float EaseOutCubic(float _fT) noexcept
	{
		const float f_ = _fT - 1.0f;
		return f_ * f_ * f_ + 1.0f;
	}

	/**
	 * @brief 삼차 함수 Ease-In-Out
	 */
	inline constexpr float EaseInOutCubic(float _fT) noexcept
	{
		return (_fT < 0.5f) ? (4.0f * _fT * _fT * _fT) : (((_fT - 1.0f) * (2.0f * _fT - 2.0f) * (2.0f * _fT - 2.0f) + 1.0f));
	}

	/**
	 * @brief 탄성(Elastic) Ease-Out - 바운스 효과
	 * @param _fT: 진행도 (0.0 ~ 1.0)
	 * @return: 이징된 값
	 */
	inline float EaseOutElastic(float _fT) noexcept
	{
		if (_fT <= 0.0f) return 0.0f;
		if (_fT >= 1.0f) return 1.0f;
		return powf(2.0f, -10.0f * _fT) * sinf((_fT * 10.0f - 0.75f) * (DX9_2PI / 3.0f)) + 1.0f;
	}

	/**
	 * @brief 바운스 Ease-Out - 공이 튀는 듯한 효과
	 * @param _fT: 진행도 (0.0 ~ 1.0)
	 * @return: 이징된 값
	 */
	inline float EaseOutBounce(float _fT) noexcept
	{
		if (_fT < 1.0f / 2.75f)
		{
			return 7.5625f * _fT * _fT;
		}
		else if (_fT < 2.0f / 2.75f)
		{
			const float fT_ = _fT - 1.5f / 2.75f;
			return 7.5625f * fT_ * fT_ + 0.75f;
		}
		else if (_fT < 2.5f / 2.75f)
		{
			const float fT_ = _fT - 2.25f / 2.75f;
			return 7.5625f * fT_ * fT_ + 0.9375f;
		}
		else
		{
			const float fT_ = _fT - 2.625f / 2.75f;
			return 7.5625f * fT_ * fT_ + 0.984375f;
		}
	}

}	// namespace dx9
