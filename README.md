# DX9Wrapp

DirectX 9 그래픽스 고성능 추상화 레이어 래퍼 라이브러리. 

## 개요

2D/3D 렌더링을 위한 DirectX 9 래퍼로, 디바이스 관리, 스프라이트 배칭, 폰트 렌더링, 3D 엔진 등을 제공한다.

## 주요 기능

### 디바이스 관리
- **C_DX9_DEVICE** / **C_DX9_DEVICEEX** - Direct3D9(Ex) 디바이스 래핑
- 2D 모드(Z-Buffer 비활성)와 3D 모드 지원
- Lost Device 자동 복구
- 렌더 상태 캐싱을 통한 API 호출 최소화

### 2D 스프라이트 렌더링
- **C_DX9_SPRITE_RENDERER** - 배칭 기반 고성능 스프라이트 렌더러
- 최대 4096개 스프라이트 배치 처리
- 7가지 블렌딩 모드 (Alpha, Additive, Normal, Cloud, Shadow, Opacity, Copy)
- 회전, 스케일링, 플립, 색상 변조 지원
- 반픽셀 보정

### 3D 엔진
- **C_DX9_ENGINE** - RealSpace3 호환 3D 렌더링 엔진
- World/View/Projection 변환
- 컬링, 스텐실, 텍스처 필터링 (Point ~ Anisotropic)
- 다양한 프리미티브 타입

### 폰트 시스템
- **C_DX9_FONT_SYSTEM** - 글리프 캐싱 기반 폰트 렌더링
- 1024x1024 텍스처 아틀라스, 32x32 셀 할당
- 2D/3D 텍스트 렌더링
- GDI 래스터라이징

### 수학 라이브러리
- 2D/3D/4D 벡터 연산 (`_DVECTOR2`, `_DVECTOR3`, `_DVECTOR4`)
- 4x4 행렬 연산 (`_DMATRIX9`) - 투영, LookAt, 분해 등

### 리소스 관리
- 텍스처 로딩 (PNG, JPG, DDS 등)
- 정점/인덱스 버퍼 관리
- Dynamic/Static 버퍼 자동 선택

### ImGui 통합
- Dear ImGui + ImPlot 지원
- DirectX 9 백엔드

## 빌드

### 요구사항
- Visual Studio 2019 이상 (v145 툴셋)
- Windows 10 SDK
- DirectX SDK (June 2010)
- [DarkCore](../DarkCore) 라이브러리

### 구성
| 구성 | 설명 |
|------|------|
| Debug | 디버그 빌드 |
| Release | 릴리즈 빌드 (정적 런타임) |
| ReleaseMD | 릴리즈 빌드 (동적 런타임) |

Win32 및 x64 플랫폼 지원. 출력물은 정적 라이브러리(.lib).

### 링크 라이브러리
```
d3d9.lib  d3dx9.lib  dxguid.lib  dxerr.lib
dinput8.lib  winmm.lib  dsound.lib  imm32.lib
```

## 구조

```
DX9Wrapp/
├── DX9Device / CDX9DeviceEx   디바이스 초기화 및 관리
├── DX9DeviceState             렌더 상태 캐싱
├── DX9Engine                  3D 렌더링 엔진
├── DX9Sprite / SpriteEngine   2D 스프라이트 배칭 렌더러
├── DX9SpriteCompat            레거시 호환 레이어
├── DX9Font                    폰트 렌더링 시스템
├── DX9Texture / DX9Buffer     리소스 관리
├── DX9Matrix / DX9Vector*     수학 라이브러리
├── CImGui                     ImGui 통합
└── DX9Util                    유틸리티
```

## 네임스페이스

모든 타입과 함수는 `dx9` 네임스페이스에 정의된다.
