# DX9Wrapp

DirectX 9 그래픽스 추상화 레이어 래퍼 라이브러리.

## 개요

2D/3D 렌더링을 위한 DirectX 9 래퍼로, 디바이스 관리, 스프라이트 배칭, 폰트 렌더링, 3D 엔진 등을 제공한다.

## 주요 기능

### 디바이스 관리
- **C_DX9_DEVICE** - 현재 프로젝트에 포함된 Direct3D9 디바이스 래퍼
- **C_DX9_DEVICEEX** - D3D9Ex 실험 소스만 존재하며 현재 프로젝트 빌드에는 포함되지 않음
- 2D 모드(Z-Buffer 비활성)와 3D 모드 지원
- Lost Device 자동 복구
- 렌더 상태 캐싱을 통한 API 호출 최소화

### 2D 스프라이트 렌더링
- **C_DX9_SPRITE_RENDERER** - 일반 `Draw()` 경로의 배칭 스프라이트 렌더러
- 배치당 최대 4,096개 스프라이트 처리
- 호환 `DrawCompat()` 경로는 호출마다 즉시 플러시
- 9가지 블렌딩 모드 (Alpha, Additive, Normal, Cloud, Shadow, Opacity, Clear, Copy, Alpha Additive)
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
- 호출자가 지정한 usage flag에 따른 Dynamic/Static 버퍼 관리

### ImGui 통합
- Dear ImGui + ImPlot 지원
- DirectX 9 백엔드

## 빌드

### 요구사항
- `DX9Wrapp.vcxproj`: v145 툴셋
- `DX9Wrapp_2019.vcxproj`: Debug/Release Win32와 Debug x64는 v142, Release x64는 v141
- Windows 10 SDK
- vcpkg MSBuild 통합과 `directxsdk` 포트
  - Win32: `directxsdk:x86-windows`, `directxsdk:x86-windows-static`
  - x64: `directxsdk:x64-windows`, `directxsdk:x64-windows-static`
- 비공개 [DarkCore](../DarkCore) 라이브러리를 DX9Wrapp과 같은 부모 디렉터리에 배치

요구 디렉터리 배치는 다음과 같다.

```
<솔루션 루트>/
├── DarkCore/
└── DX9Wrapp/
```

프로젝트는 라이브러리군의 공통 주입점으로 `$(SolutionDir)`를 사용한다. 현재 DX9Wrapp 소스가 솔루션 제공 헤더를 직접 참조하지는 않지만, 지원 빌드 경로는 소비 프로젝트의 솔루션에 포함해 빌드하는 방식이며 `.vcxproj` 직접 빌드는 지원 대상이 아니다.

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

핵심 DX9 래퍼 타입과 함수는 `dx9` 네임스페이스에 정의된다. ImGui 통합 클래스 `C_IMGUI` 등 일부 심볼은 전역 네임스페이스에 있다.
