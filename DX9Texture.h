#pragma once


#include <DarkCore/DDef.h>
#include <DarkCore/DUtil.h>
#include <DarkCore/DFile.h>

#include "DX9Def.h"



namespace dx9
{
	typedef struct _VERTEX_UV
	{
		union
		{
			struct
			{
				float u1;
				float u2;
				float v1;
				float v2;
			};
			struct
			{
				float u[2];
				float v[2];
			};
		};
		
		_VERTEX_UV()
			: u1(0.0f)
			, u2(0.0f)
			, v1(0.0f)
			, v2(0.0f)

		{

		};
		_VERTEX_UV(float U1, float U2, float V1, float V2)
			: u1(U1)
			, u2(U2)
			, v1(V1)
			, v2(V2)
		{

		};
	} VERTEX_UV, * LPVERTEX_UV;

#define MAX_PIXELSAMPLER	16
#define MAX_VERTEXSAMPLER	4									// VS 3.0에서 사용하는 버텍스 샘플러 최대 수
#define MAX_IMAGEUNIT		MAX_PIXELSAMPLER+MAX_VERTEXSAMPLER	// 최대 텍스쳐 유닛 수 + VERTEXSAMPLER 수
#define RDMAPSAMPLER 256
	//Vertex Texture Sampler(0~4)
	const int RVERTEXTEXTURESAMPLER0 = (RDMAPSAMPLER + 1);
	const int RVERTEXTEXTURESAMPLER1 = (RDMAPSAMPLER + 2);
	const int RVERTEXTEXTURESAMPLER2 = (RDMAPSAMPLER + 3);
	const int RVERTEXTEXTURESAMPLER3 = (RDMAPSAMPLER + 4);

	// D3DPOOL_SYSTEMMEM
	// 보통, 3D 장치에 의해 액세스 할 수 없는 메모리.시스템 RAM 를 사용하지만, 페이징 가능한 RAM 가 줄어들 것은 없다.
	// 이러한 리소스는, 장치가 손실해도 생성 다시 할 필요가 없다.
	// 이 풀의 리소스는 잠글 수가 있어 D3DPOOL_DEFAULT 를 사용해 생성 된 메모리 리소스에 대한 IDirect3DDevice9::UpdateSurface
	// 또는 IDirect3DDevice9::UpdateTexture 처리의 전송원으로서 사용할 수 있다.

	// 애플리케이션에서는, 대부분의 정적 리소스에 대해서 D3DPOOL_MANAGED 를 사용할 필요가 있다.
	// 이것에 의해, 손실한 장치를 처리할 필요가 없어진다.
	// 관리되는 리소스는 런타임에 의해 복원된다.이것은, 특히 UMA 시스템으로 유효하다.
	// 그 이외의 동적 리소스에 대해서는, D3DPOOL_MANAGED 는 사용하지 않는 것이 좋다.
	// 실제, D3DPOOL_MANAGED 를 D3DUSAGE_DYNAMIC 와 함께 사용해, 인덱스 버퍼 및 정점 버퍼를 생성 할 수 없다.

	// 동적 텍스처의 경우, 비디오 메모리와 시스템 메모리의 텍스처의 페어를 사용해,
	// D3DPOOL_DEFAULT 를 지정해 비디오 메모리를, D3DPOOL_SYSTEMMEM 를 지정해 시스템 메모리를 할당하는 일이 있다.
	// 잠금 메서드를 사용해, 시스템 메모리 텍스처의 비트를 잠금 및 변경할 수 있다.
	// 그 후, IDirect3DDevice9::UpdateTexture 를 사용해, 비디오 메모리 텍스처를 갱신할 수 있다.

	typedef struct _DX9_TEXTURE
	{
		LPDIRECT3DTEXTURE9 pTexture;
		LPDIRECT3DDEVICE9 pDevice;
		//LPDIRECT3DSURFACE9* rtSurfaces;
		//int nRenderTargetSurfaces;
		
		D3DFORMAT d3dFormat;
		DWORD dwUsage;
		D3DPOOL d3dPool;
		UINT nMipLevels;
		UINT nWidth, nHeight;
		
		dk::C_FILE* pFile;

		//char* pBuffer;
		//UINT nFileSize;

		_DX9_TEXTURE(LPDIRECT3DDEVICE9 _pDevice);
		_DX9_TEXTURE(LPDIRECT3DDEVICE9 _pDevice, UINT _nWidth, UINT _nHeight, D3DFORMAT _d3dFormat = D3DFMT_UNKNOWN, DWORD _dwUsage = 0);

		bool Create();
		bool LoadTexture(LPCSTR _tszPath);
		bool LoadTexture(LPCWSTR _tszPath = nullptr);
		void Release();
		
		void OnLostDevice();
		void OnResetDevice();
		
		bool IsDynamic();

	} DX9_TEXTURE, * LPDX9_TEXTURE;
}