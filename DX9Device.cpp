#include "framework.h"
#include "DX9Device.h"



namespace dx9
{
#if defined(_USE_FAKE_VERTEX_)
	struct _CUSTOM_VERTEX_FOR_2D
	{
		dx9::_DVECTOR3 position; // The position	
		D3DCOLOR    color;    // The color
		FLOAT       tu, tv;   // The texture coordinates
		
		_CUSTOM_VERTEX_FOR_2D()
			: color(D3DCOLOR_ARGB(255, 255, 255, 255))
			, tu(0.0f)
			, tv(0.0f)
		{
		}
	};
#define _D3DFVF_CUSTOM_VERTEX_FOR_2D_ (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

	_CUSTOM_VERTEX_FOR_2D gs_FakeVertexBuffer[4];
#endif
	C_DX9_DEVICE::C_DX9_DEVICE(bool _bWindowMode, bool _bVerticalSync)
		: eDeviceMode(DX9_DEVICE_MODE_2D)
		, bWindowMode(_bWindowMode)
		, bVerticalSync(_bVerticalSync)	// 프레임 출력할 수 있는만큼으로 제한
		, bCursor(false)
#if defined(_USE_FAKE_VERTEX_)
		, bUseFakeVertex(false)
		, pTextureCombination(nullptr)
		, pFakeVertexBuffer(nullptr)
#endif
		, nLastDeviceStatus(0)
		, pDirect3D9(nullptr)
		, pDevice(nullptr)
		//, pSprite(nullptr)
		, bytAlphaBlend(255)
	{
		ClearDX9States();
	}

	C_DX9_DEVICE::~C_DX9_DEVICE()
	{
		Destroy();
	}
	
	void C_DX9_DEVICE::InitDeviceDefault()
	{
		currentVertexFormat = 0;
		for (size_t i = 0; i < MAX_VERTEXSTREAM; i++)			// 4
		{
			currentVertexBuffer[i] = nullptr;
			currentVertexOffset[i] = 0;
			currentFrequencyParameter[i] = 0;
		}
		currentIndexBuffer = nullptr;
		currentInstanceCount = 1;

		for (size_t i = 0; i < _countof(dwRenderState); i++)	// 256
		{
			dwRenderState[i] = 0;
		}
		for (size_t i = 0; i < _countof(dwSamplerState); i++)	// 16
		{
			dwSamplerState[i] = 0;
		}
		for (size_t i = 0; i < _countof(currentTextures); i++)	// 16
		{
			currentTextures[i] = nullptr;
			currentTextureFilter[i] = _D3DTEXF_COUNT;
			for (size_t j = 0; j < 3; j++)
			{
				currentTextureAddress[i][j] = RTADDRESS_WRAP;
			}
			for (size_t j = 0; j < _D3DTSS_MAX; j++)
			{
				currentTextureStageSettings[i][j] = _D3DTSS_MAX;
			}
			currentMaxAnisotropy[i] = 0;
			currentMipmaplodBias[i] = 0.0f;
			currentTextureBorderColor[i] = 0;
		}
		currentFVF = 0;
	}

	void C_DX9_DEVICE::ClearDX9States()
	{
		for (long i = 0; i < _countof(dwRenderState); i++)
		{
			dwRenderState[i] = 0xFFFFFFFF;
		}
		for (long i = 0; i < _countof(dwSamplerState); i++)
		{
			dwSamplerState[i] = 0xFFFFFFFF;
		}
	}

	LPDIRECT3DDEVICE9 C_DX9_DEVICE::Init(HWND _hWnd, dk::DSIZE _sizeScreen, _E_DX9_DEVICE_MODE_ _eMode)
	{
		eDeviceMode = _eMode;
		hWnd = _hWnd;
		rectRender.Set(&_sizeScreen);
		v2DisplaySize.Set((float)_sizeScreen.cx, (float)_sizeScreen.cy);
		DBGPRINT("v2DisplaySize.Set(%i, %i)", (DWORD)v2DisplaySize.x, (DWORD)v2DisplaySize.y);
		//DBGPRINT("C_DX9_DEVICE::Init(start) %i / %i", dkScreenSize.cx, dkScreenSize.cy);
		//////////////////////////////////////////////////////////////////////////
		// 여기서 "d3d9.dll"이랑 "Direct3DCreate9"를 암호화 하고, 가상화를 걸자.

		HMODULE hD3D9 = ::LoadLibraryW(L"d3d9.dll");
		typedef LPDIRECT3D9(__stdcall* Direct3DCreate9_PROC)(UINT);
		if (0 != hD3D9)
		{
			Direct3DCreate9_PROC pDirect3DCreate9 = (Direct3DCreate9_PROC)GetProcAddress(hD3D9, "Direct3DCreate9");
			DBGPRINT("[결과] pDirect3DCreate9: %x", pDirect3DCreate9);
			if (nullptr != pDirect3DCreate9)
			{
				pDirect3D9 = pDirect3DCreate9(D3D_SDK_VERSION);
				DBGPRINT("[성공] pDirect3D9: %x", pDirect3D9);
			}
		}
		//////////////////////////////////////////////////////////////////////////
		do
		{
			if (nullptr == pDirect3D9)
			{
				DBGPRINT("[실패] Direct3DCreate9");
				break;
			}
			//////////////////////////////////////////////////////////////////////////
			// D3DXEFFECTCOMPILER 리소스가 많은 메모리를 점유하기 때문에 
			// RShaderFX에서 셰이더 컴파일이 끝나면 리소스를 해제하는데
			// 이 때 레퍼런스 카운터가 0이 되어 DLL언로드, 로드가 반복이 되어 부하가 걸린다.
			// 부하 방지를 위해 초기화 할 때 D3DXCreateEffectCompiler()를 한번 호출해준다.
			// D3DXCreateEffectCompiler가 호출되면 셰이더 컴파일러 DLL이 로드 된다.

			LPD3DXEFFECTCOMPILER pEffectCompiler;
			D3DXCreateEffectCompiler("", 1, NULL, NULL, 0, &pEffectCompiler, NULL);
			//////////////////////////////////////////////////////////////////////////
			pDirect3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps);

			//DBGPRINT("MAX TEXTURE SIZE: %i / %i", d3dCaps.MaxTextureWidth, d3dCaps.MaxTextureHeight);
			D3DDEVTYPE d3dDevType = D3DDEVTYPE_HAL;	// 3D 가속기로 렌더링, D3DDEVTYPE_REF = CPU 명령셋으로 렌더링
			DWORD dwBehaviorFlags = QueryFeature(RQF_HARDWARETNL) ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			//DWORD dwBehaviorFlags = d3dCaps.VertexProcessingCaps ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			DBGPRINT("[결과] 하드웨어 버텍스 프로세싱: %s", DIS_SET(dwBehaviorFlags, D3DCREATE_HARDWARE_VERTEXPROCESSING) ? "ON" : "OFF");

			//////////////////////////////////////////////////////////////////////////
			// D3DCREATE_FPU_PRESERVE
			// 애플리케이션으로 배정밀도 부동 소수점 단위(FPU) 또는 FPU 예외를 유효하게 할 필요가 있는 것을 나타낸다.
			// Microsoft® Direct3D® 는, 불려 갈 때마다 FPU 상태를 설정한다.
			// 디폴트에서는, 파이프라인은 단정밀도를 사용한다.배정밀도가 필요한 경우는,
			// 반드시 이 플래그를 사용하는 것.이 플래그를 설정 하면, Direct3D 의 퍼포먼스가 저하한다.

			//DSET_BIT(dwBehaviorFlags, D3DCREATE_FPU_PRESERVE);	// 루아에서 double 의 범위를 쓰려면 필요한것 같다.
			// 루아를 사용하면서 위 플래그를 안쓰려면 유저데이터 형식으로 랩핑해서 루아에 넘겨줬다가 c++ 에서 받자.
			//////////////////////////////////////////////////////////////////////////
			/*
	#ifndef DEBUG_VS
			//////////////////////////////////////////////////////////////////////////
			// 순수디바이스 = SetRenderState를 사용할 수 있는 모든 Get 함수를 지원하지 않는다.(GetViewport 는 예외)
			// Get 함수를 위해 저장해둔 정보가 없어서 좀 더 빠르게 SetRenderState 가 가능하다.
			// 중복체크를 하지 않기때문에 SetRenderState 를 자주 호출하면 느려지므로 직접 관리를 해야한다.
			dwBehaviorFlags |= D3DCREATE_PUREDEVICE;
			//////////////////////////////////////////////////////////////////////////
	#else
			if (d3dDevType != D3DDEVTYPE_REF)
			{
				dwBehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
				dwBehaviorFlags &= ~D3DCREATE_PUREDEVICE;
				dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			}
	#endif
			*/
			_DX9_PRESENT_PARAMETERS d3dpp = { 0 };
			InitPresentParameters(&d3dpp);

			if (nullptr != pDirect3D9)
			{

				//D3DDISPLAYMODEEX displayModeEx = { 0 };
				//D3DDISPLAYROTATION displayRotation = D3DDISPLAYROTATION_IDENTITY;
				//pDirect3D9->GetAdapterDisplayModeEx(D3DADAPTER_DEFAULT, &displayModeEx, &displayRotation);

				//DBGPRINT("C_DX9_DEVICE::CreateDevice(시작)");
				if (FAILED(pDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, d3dDevType, hWnd, dwBehaviorFlags, &d3dpp, &pDevice)))
				{
					//DBGPRINT("C_DX9_DEVICE::CreateDevice(실패1)");
					if (DIS_SET(dwBehaviorFlags, D3DCREATE_PUREDEVICE))
					{
						//DBGPRINT("C_DX9_DEVICE::CreateDevice(실패): D3DCREATE_PUREDEVICE 제거한 후 재시도");
						DREMOVE_BIT(dwBehaviorFlags, D3DCREATE_PUREDEVICE);
						pDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, d3dDevType, hWnd, dwBehaviorFlags, &d3dpp, &pDevice);
					}
					if (pDevice)
					{
						DSAFE_RELEASE(pDirect3D9);
						//DBGPRINT("C_DX9_DEVICE::CreateDevice(실패: %x)", pDevice);
						break;
					}
				}
				DBGPRINT("[결과] C_DX9_DEVICE::CreateDevice(): %x", pDevice);
			}
#if defined(LAYERED_WINDOW)
			bInitLayeredWindow = InitLayeredTexture(D3DFMT_A8R8G8B8, D3DFMT_D24S8);
			if (FALSE != ImageCreate(&imgBack))
			{

			}
#endif
			//D3DXCreateSprite(pDevice, &pSprite);

			DVIEWPORT9 vpt9(0, 0, (DWORD)v2DisplaySize.x, (DWORD)v2DisplaySize.y);
			wrappSetViewport(&vpt9);

			InitDeviceDefault();

			//////////////////////////////////////////////////////////////////////////
			// 모드에 따른 초기화
			if (DX9_DEVICE_MODE_3D == eDeviceMode)
			{
				// 3D 모드: Z-Buffer 활성화, 기본 렌더 스테이트 설정
				Init3DMode();
			}
			else
			{
				// 2D 모드: 기존 동작 유지 (Z-Buffer 비활성화)
				Init2DMode();
			}
			//////////////////////////////////////////////////////////////////////////

			// 여기에서 폰트 텍스쳐를 생성하도록 하자.

			//Begin2D();
		} while (false);
		nLastDeviceStatus = _DX9_DEVICE_OK;		// 디바이스가 사용 가능 상태
		DBGPRINT("[결과] C_DX9_DEVICE::Init() 모드: %s", (DX9_DEVICE_MODE_3D == eDeviceMode) ? "3D" : "2D");
		return(pDevice);
	}

	void C_DX9_DEVICE::Init2DMode()
	{
		// 2D 전용 모드 초기화
		// Z-Buffer 비활성화
		wrappSetRenderState(D3DRS_ZENABLE, FALSE);
		wrappSetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		// 컬링 비활성화
		wrappSetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		// 라이팅 비활성화
		wrappSetRenderState(D3DRS_LIGHTING, FALSE);

		// 알파 블렌딩 설정
		wrappSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// 알파 테스트 설정
		wrappSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		wrappSetRenderState(D3DRS_ALPHAREF, 0);
		wrappSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);

		DBGPRINT("[정보] Init2DMode() - 2D 전용 모드 초기화 완료");
	}

	void C_DX9_DEVICE::Init3DMode()
	{
		// 3D 게임용 모드 초기화 (2D GUI도 지원)
		// Z-Buffer 활성화
		wrappSetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		wrappSetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		wrappSetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

		// 기본 컬링 (시계방향 = 뒷면 제거)
		wrappSetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

		// 라이팅 (셰이더 사용 시 보통 비활성화)
		wrappSetRenderState(D3DRS_LIGHTING, FALSE);

		// 알파 블렌딩 (기본 비활성화, 필요 시 활성화)
		wrappSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// 알파 테스트 비활성화 (셰이더에서 처리)
		wrappSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

		// 스펙큘러 하이라이트
		wrappSetRenderState(D3DRS_SPECULARENABLE, FALSE);

		// 안개 (기본 비활성화)
		wrappSetRenderState(D3DRS_FOGENABLE, FALSE);

		// 노멀라이즈 (스케일링된 오브젝트용)
		wrappSetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

		// 텍스처 필터링 (Anisotropic)
		const DWORD dwMaxAniso = d3dCaps.MaxAnisotropy > 0 ? d3dCaps.MaxAnisotropy : 1;
		for (int i = 0; i < 8; ++i)
		{
			wrappSetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			wrappSetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			wrappSetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			wrappSetSamplerState(i, D3DSAMP_MAXANISOTROPY, dwMaxAniso);
		}

		// 디바이스 기능 로깅
		DBGPRINT("[정보] Init3DMode() - 3D 모드 초기화 완료");
		DBGPRINT("[정보] - VS 버전: %d.%d", D3DSHADER_VERSION_MAJOR(d3dCaps.VertexShaderVersion), D3DSHADER_VERSION_MINOR(d3dCaps.VertexShaderVersion));
		DBGPRINT("[정보] - PS 버전: %d.%d", D3DSHADER_VERSION_MAJOR(d3dCaps.PixelShaderVersion), D3DSHADER_VERSION_MINOR(d3dCaps.PixelShaderVersion));
		DBGPRINT("[정보] - 하드웨어 T&L: %s", QueryFeature(RQF_HARDWARETNL) ? "지원" : "미지원");
		DBGPRINT("[정보] - 최대 텍스처 크기: %dx%d", d3dCaps.MaxTextureWidth, d3dCaps.MaxTextureHeight);
		DBGPRINT("[정보] - 최대 Anisotropy: %d", d3dCaps.MaxAnisotropy);
	}
#if defined(_USE_FAKE_VERTEX_)
	void C_DX9_DEVICE::InitFakeVertex()
	{
		do
		{
			wrappSetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);		// Turn off culling
			wrappSetRenderState(D3DRS_LIGHTING, FALSE);				// Turn off D3D lighting
			wrappSetRenderState(D3DRS_ZENABLE, FALSE);				// Turn on the zbuffer
			// 알파테스트에 사용될 알파값 = TextureAlpha * TextureFactorAlpha
			// 버텍스버퍼에 색상값이 없으므로 TextureFactor를 사용한다.
			wrappSetTextureStageState(0, _D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			wrappSetTextureStageState(0, _D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
			wrappSetTextureStageState(0, _D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
			// 투명색 적용을 위한 알파테스트. (알파값이 0이면 투명색처리)
			wrappSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			wrappSetRenderState(D3DRS_ALPHAREF, 0);
			wrappSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
			// 알파 블렌딩 ON !
			wrappSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			// 알파블렌딩 함수를 반투명으로 설정.
			wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			// 컬러 셰이딩 = Texture * TextureFactor
			// 버텍스버퍼에 색상값이 없으므로 TextureFactor를 사용한다.
			wrappSetTextureStageState(0, _D3DTSS_COLOROP, D3DTOP_MODULATE);
			wrappSetTextureStageState(0, _D3DTSS_COLORARG1, D3DTA_TFACTOR);
			wrappSetTextureStageState(0, _D3DTSS_COLORARG2, D3DTA_TEXTURE);
			// 필터링 off
			// 확대 될경우엔 아무래도 필터링을 켜는것이 보기 좋다.
			// 그러나 스프라이트는 투명색과 필터링이 일어나기때문에
			// 투명색과의 경계부분이 테두리가 생길수있다.
			// 필터링을 끄면 2D스프라이트와 완.전.동.일.하게 나타난다.
			wrappSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			wrappSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

			SetScreenSize();

			

			// 삼각형팬으로 만든 정규화된 정사각형을 만들어 둡니다.
			// 이것을 변형시켜서 원하는 크기로 렌더링합니다.
			// 코딩상 편의를 위해 버텍스버퍼를 쓰지 않았습니다.
			// 버텍스 버퍼로 바꿔버리세요...ㅡ.ㅡ;;;
			// 사용자 변환이 있으므로 정규사각형을 (-0.5,-0.5)~(0.5,0.5)로하여
			// 사용자 변환을 간단하게 설정할수 있도록 해준다.
			//gs_FakeVertexBuffer[0].position = D3DXVECTOR3(-0.5f, -0.5f, 1.0f);
			//gs_FakeVertexBuffer[1].position = D3DXVECTOR3(+0.5f, -0.5f, 1.0f);
			//gs_FakeVertexBuffer[2].position = D3DXVECTOR3(+0.5f, +0.5f, 1.0f);
			//gs_FakeVertexBuffer[3].position = D3DXVECTOR3(-0.5f, +0.5f, 1.0f);
			gs_FakeVertexBuffer[0].position.Set( -0.5f, -0.5f, 1.0f);
			gs_FakeVertexBuffer[1].position.Set(+0.5f, -0.5f, 1.0f);
			gs_FakeVertexBuffer[2].position.Set(+0.5f, +0.5f, 1.0f);
			gs_FakeVertexBuffer[3].position.Set(-0.5f, +0.5f, 1.0f);
			// 버텍스 셰이더 설정.
			D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE] = { 0 };
			if (FAILED(::D3DXDeclaratorFromFVF(_D3DFVF_CUSTOM_VERTEX_FOR_2D_, Declaration)))
			{
				DBGPRINT("C_DX9_DEVICE::InitFakeVertex() - D3DXDeclaratorFromFVF 실패");
				break;
			}
			char szVertexShaderSource[] =
			{
				"vs_1_1								"
				"dcl_position v0                    "
				"mul  r0.xy, v0.xy, c0.zw			"
				"mov  r0.zw, c2.zw					"
				"dp4  r1.x, r0, c4					"
				"dp4  r1.y, r0, c5					"
				"add  r1.xy, r1.xy, c0.xy			"
				"mov  r1.zw, c2.zw					"
				"m4x4 oPos, r1, c8					"
				"mad  oT0.xy, v0.xy, c1.zw, c1.xy	"
				";\n"
				""
			};
			LPD3DXBUFFER pCode = nullptr;
			if (FAILED(::D3DXAssembleShader(szVertexShaderSource, (UINT)strlen(szVertexShaderSource), 0, NULL, NULL, &pCode, NULL)))
			{
				DBGPRINT("C_DX9_DEVICE::InitFakeVertex() - D3DXAssembleShader 실패");
				break;
			}
			LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration9 = nullptr;//2D출력용
			pDevice->CreateVertexDeclaration(Declaration, &pVertexDeclaration9);

			LPDIRECT3DVERTEXSHADER9 pVertexShader9 = nullptr;
			if (FAILED(pDevice->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &pVertexShader9)))
			{
				DBGPRINT("C_DX9_DEVICE::InitFakeVertex() - CreateVertexShader 실패");
				DSAFE_RELEASE(pCode);
				break;
			}
			DSAFE_RELEASE(pCode);

			pDevice->SetVertexDeclaration(pVertexDeclaration9);
			pDevice->SetVertexShader(pVertexShader9);

			float fVertexShaderConst[4] = { 1,1,1,1 };
			wrappSetVertexShaderConstantF(2, fVertexShaderConst, 1);
			// 조합용 텍스쳐 생성
			if (!pTextureCombination)
			{
				::D3DXCreateTexture(pDevice, 512, 512, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTextureCombination);
			}
			if (!pFakeVertexBuffer)
			{
				pDevice->CreateVertexBuffer(4 * sizeof(_CUSTOM_VERTEX_FOR_2D), 0, _D3DFVF_CUSTOM_VERTEX_FOR_2D_, D3DPOOL_DEFAULT, &pFakeVertexBuffer, NULL);
			}
			// Fill the vertex buffer. We are setting the tu and tv texture
			// coordinates, which range from 0.0 to 1.0	  

			bUseFakeVertex = true;
		} while (false);
	}
	void C_DX9_DEVICE::SetScreenSize()
	{
		// 평행투영 행렬설정.
			// 정점의 x,y에서 0.5를 빼주는것을 행렬로 만들어서 평행투영앞에 넣어버린다.
			// 이렇게 하면, 정점에 스크린(정수)좌표를 그냥 사용할 수 있다.
		D3DXMATRIX projection;
		::D3DXMatrixOrthoOffCenterLH(&projection, 0, v2DisplaySize.x, v2DisplaySize.y, 0, 0, 1);
		D3DXMATRIX offset;
		D3DXMatrixTranslation(&offset, -0.5, -0.5, 0);
		projection = offset * projection;

		::D3DXMatrixTranspose(&projection, &projection);
		wrappSetVertexShaderConstantF(8, (float*)&projection, 4);

		// 사용자 행렬은 일단 단위행렬로 설정.
		D3DXMATRIX identity;
		::D3DXMatrixIdentity(&identity);
		wrappSetVertexShaderConstantF(4, (float*)&identity, 4);
	}
	void C_DX9_DEVICE::DrawTexture2D(
		_DX9_TEXTURE* _pTexture
		, dk::DRECT _rcDisplay
		, dk::DRECT _rcSource
		, DWORD _dwColor
		, BYTE _nBlendingType
		, bool _bLighting
		, float _fAlpha
		, float _fScale
		, int _nAngle
		, bool _bInvert
	)
	{
		if (_fAlpha <= 0.0f)
			_fAlpha = 0.0f;
		if (_fAlpha >= 1.0f)
			_fAlpha = 1.0f;
		if (_fScale <= 0.0f)
			_fScale = 0.0f;

		_nAngle = _nAngle % 360;
		
		if (_fScale == 1.0f && _nAngle == 0)
		{
			wrappSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			wrappSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		}
		else
		{
			wrappSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);//D3DTEXF_LINEAR
			wrappSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);//D3DTEXF_LINEAR
		}
		switch (_nBlendingType)
		{
		case 0://그냥 찍음
			wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);		// D3DBLEND_SRCALPHA
			wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);	// D3DBLEND_INVSRCALPHA
			break;
		case 1://반투명등등
			//D3DTFN_LINEAR D3DTFN_POINT
			if (_bLighting)
			{
				wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			}
			else
			{
				wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);//D3DBLEND_SRCALPHA
				wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);//D3DBLEND_INVSRCALPHA																					
			}
			break;
		case 2://광원
			wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		case 3://구름
			wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
			wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);
			break;
		case 4://그림자
			_dwColor = D3DCOLOR_ARGB(255, 96, 96, 96);
			wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
			break;
		case 5://오퍼서티
			wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);//D3DBLEND_SRCALPHA
			wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);//D3DBLEND_INVSRCALPHA																					
			break;
		case 6://백서페이스 clear
			wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			break;
		case 7://화면 그대로 출력
			wrappSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			wrappSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			break;
		}
		
		// 디스플레이 좌표
		long x = 0, y = 0, w = 0, h = 0;
		if (1.0f == _fScale)
		{
			x = _rcDisplay.left;
			y = _rcDisplay.top;
			w = _rcDisplay.right - _rcDisplay.left;
			h = _rcDisplay.bottom - _rcDisplay.top;
		}
		else
		{
			x = _rcDisplay.left + ((_rcDisplay.right - _rcDisplay.left) / 2);
			y = _rcDisplay.top + ((_rcDisplay.bottom - _rcDisplay.top) / 2);
			x = x - long(((_rcDisplay.right - _rcDisplay.left) * _fScale) / 2);
			y = y - long(((_rcDisplay.bottom - _rcDisplay.top) * _fScale) / 2);
			w = long((_rcDisplay.right - _rcDisplay.left) * _fScale);
			h = long((_rcDisplay.bottom - _rcDisplay.top) * _fScale);
		}
		// 소스 좌표
		float tx = ((float)(_rcSource.left) / v2DisplaySize.x);
		float ty = ((float)(_rcSource.top) / v2DisplaySize.y);
		float tw = ((float)(_rcSource.right - _rcSource.left) / v2DisplaySize.x);
		float th = ((float)(_rcSource.bottom - _rcSource.top) / v2DisplaySize.y);
		if (_bInvert)
		{
			tx = ((float)(_rcSource.left) / (float)(v2DisplaySize.x)) + tw;
			tw = -((float)(_rcSource.right - _rcSource.left) / (float)(v2DisplaySize.x));
		}
		// Z축을 회전축으로 해 회전하는 행렬을 생성 한다.
		dx9::_DMATRIX9 usermatrix;
		::D3DXMatrixRotationZ(&usermatrix, _nAngle * 3.1415f / 180.0f);
		// 행렬의 전치행렬을 돌려준다.
		::D3DXMatrixTranspose(&usermatrix, &usermatrix);
		// 상수 설정
		wrappSetVertexShaderConstantF(4, (float*)&usermatrix, 4);
		// 텍스쳐를 선택하고
		wrappSetTexture(0, _pTexture);
		// 알파블랜딩으로 원하는 색으로 텍스쳐 출력, 뒤의 인수만큼 텍스쳐가 투명해지는가보다
		//wrappSetRenderState(D3DRS_TEXTUREFACTOR, _dwColor);
		
		float fVertexShaderConst[8];
		// 디스플레이 좌표
		fVertexShaderConst[0] = (float)x + w * 0.5f;
		fVertexShaderConst[1] = (float)y + h * 0.5f;
		fVertexShaderConst[2] = (float)w;
		fVertexShaderConst[3] = (float)h;
		// 소스 좌표
		fVertexShaderConst[4] = (float)tx + tw * 0.5f;
		fVertexShaderConst[5] = (float)ty + th * 0.5f;
		fVertexShaderConst[6] = (float)tw;
		fVertexShaderConst[7] = (float)th;
		// 상수 설정
		wrappSetVertexShaderConstantF(0, fVertexShaderConst, 2);
		// 그린다
		pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, gs_FakeVertexBuffer, sizeof(_CUSTOM_VERTEX_FOR_2D));
	}
#endif
	void C_DX9_DEVICE::Destroy()
	{
		//DSAFE_DELETE(pViewport);
#if defined(_USE_FAKE_VERTEX_)
		bUseFakeVertex = false;
		DSAFE_RELEASE(pTextureCombination);
		DSAFE_RELEASE(pFakeVertexBuffer);
#endif
		DSAFE_RELEASE(pDevice);
		DSAFE_RELEASE(pDirect3D9);
	}

	_DEVICE_STATUS_ C_DX9_DEVICE::GetDeviceStatus()
	{
		//DBGPRINT("C_DX9_DEVICE::GetDeviceStatus() 계속 진행");
		_DEVICE_STATUS_ nResult = _DX9_DEVICE_OK;
		do
		{
			if (nullptr == pDevice)
			{
				nResult = _DX9_DEVICE_LOST;
				break;
			}
			//////////////////////////////////////////////////////////////////////////
			// - 디바이스가 소실되는 경우
			// 최소화 되는경우, 다른프로그램이 풀스크린모드로 생성될 경우
			// 전력관리 이벤트(WM_POWERBROADCAST)가 발생할 경우
			// 디스플레이 등록정보에서 색상모드를 변경하는 경우
			//
			HRESULT hResult = pDevice->TestCooperativeLevel();	// 항상 D3D_OK 를 리턴한다고 해서 EX로 바꾼다.
			//HRESULT hResult = pDevice->CheckDeviceState(hWnd);
			if (0 > hResult)
			{
				// If the device was lost, do not render until we get it back
				if (D3DERR_DEVICELOST == hResult)			// 디바이스가 소실되었고 아직 복구할 수 없는 상태이다.
				{
					nResult = _DX9_DEVICE_LOST;				// 이 경우 매니저를 포함한 모든 리소스를 해제한다.
				}
				// Check if the device needs to be resized.
				else if (D3DERR_DEVICENOTRESET == hResult)	// 디바이스가 소실되었지만 지금 복구할 수 있는 상태이다.
				{

					nResult = _DX9_DEVICE_RESTORED;			// 이 경우 모든 리소스를 해제하고 디바이스를 릴리즈한다.

				}
				else if (D3DERR_OUTOFVIDEOMEMORY == hResult)
				{
					nResult = _DX9_DEVICE_DESTROY;			// 메모리 시발ㅋㅋ
				}
				else
				{
					// D3D9Ex only pDevice->CheckDeviceState()
					if (D3DERR_DEVICEREMOVED == hResult)
					{
						nResult = _DX9_DEVICE_LOST;
					}
					else if (D3DERR_DEVICEHUNG == hResult)	// OS가 하드웨어 어댑터를 재설정했고, 프로그램을 종료해야할정도.
					{
						nResult = _DX9_DEVICE_DESTROY;		// 프로그램 뒤진건가보다.
					}
					else if (S_PRESENT_MODE_CHANGED == hResult)
					{
						//////////////////////////////////////////////////////////////////////////
						// CheckDeviceStats가 S_PRESENT_MODE_CHANGED를 반환하면 디스플레이 모드를 재설정하고 장치 유실을 다시 테스트하십시오.
						// 디스플레이 모드가 재설정되지 않으면 CheckDeviceLost는 S_PRESENT_MODE_CHANGED를 계속 반환하여 GPU 중단으로 인해
						// 손실 된 장치를 마스킹하여 ANGLE이 컨텍스트 손실을보고하지 못하게 할 수 있습니다.
						// 커밋 : https://code.google.com/p/angleproject/source/detail?r=1986
						//
						nResult = _DX9_DEVICE_RESTORED;
					}
					else if (S_PRESENT_OCCLUDED == hResult)	// 이걸 반환할 경우 더미프레임을 표시하고 다시 시도
					{
						//////////////////////////////////////////////////////////////////////////
						// CheckDeviceState가 S_PRESENT_OCCLUDED를 반환하면 더미 프레임을 표시하고 다시 시도하십시오.
						// 화면이 잠겨있는 동안 GPU가 중단되는 경우 glFinish가 중단되는 것을 방지하기위한 것입니다.
						// 재현하려면 GPU를 중단하기 전에 몇 초 동안 기다리도록이 샘플을 수정하십시오.
						// https://www.khronos.org/registry/webgl/conformance-suites/1.0.0/extra/lots-of-polys-example.html
						// GPU 중단을 시작한 후 화면을 빠르게 잠급니다. GPU는 재설정되지만 ANGLE은 S_PRESENT_OCCLUDED와 D3DERR_NOERROR 만 볼 수 있습니다.
						// 장치 손실 오류가 표시되지 않습니다. 화면이 잠겨있는 동안 Present를 호출하면 CheckDeviceState가 장치 손실을 반환하는 것으로 보입니다.
						// testDeviceLost에서 여분의 Present를 수행하는 것은 매번 매우 느리기 때문에 잘 작동하지 않았을 것입니다.
						// D3D 지연이 숨겨진 창에 표시되어 앱을 조절하기 때문일 수 있습니다.
						// Committed: https://code.google.com/p/angleproject/source/detail?r=2001
						//
						nResult = _DX9_DEVICE_RESTORED;
					}
				}
			}

		} while (false);
		// 디바이스 상태값 업데이트
		if (nLastDeviceStatus != nResult)
		{
			nLastDeviceStatus = nResult;
		}
		return(nResult);
	}

	bool C_DX9_DEVICE::IsCursorVisible()
	{
		return bCursor;
	}

	bool C_DX9_DEVICE::ShowCursor(bool b)
	{
		pDevice->ShowCursor(b);
		bool bPrevious = bCursor;
		bCursor = b;
		return bPrevious;
	}

	void C_DX9_DEVICE::InitPresentParameters(_DX9_PRESENT_PARAMETERS* _d3dpp)
	{
		// 모니터 사이즈로 초기화 함, 결국에는 리셋 디바이스때문에 맞춰서 쓰게 고친다.
		//_d3dpp->BackBufferWidth = rectRender.right = 3840;//::GetSystemMetrics(SM_CXSCREEN);
		//_d3dpp->BackBufferHeight = rectRender.bottom = 2160;//::GetSystemMetrics(SM_CYSCREEN);

		// dialog 사용할 경우
		//[1] SwapEffect = D3DSWAPEFFECT_DISCARD;
		//[2] BackBufferFormat = D3DFMT_X8R8G8B8; 또는 D3DFMT_X1R5G5B5, D3DFMT_R5G6B5
		//[3] Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		//D3DDevice->SetDialogBoxMode( TRUE );

		_d3dpp->Windowed = bWindowMode;
		if (false != _d3dpp->Windowed)		// 창모드라면
		{
			_D3DDISPLAYMODE d3ddm;
			if (SUCCEEDED(pDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
			{
				_d3dpp->BackBufferFormat = d3ddm.Format;
				//DBGPRINT("pDirect3D9->GetAdapterDisplayMode(%d)", d3ddm.Format);
			}
			else
			{
				//DBGPRINT("pDirect3D9->GetAdapterDisplayMode(이거왜실패?)");
			}
			_d3dpp->FullScreen_RefreshRateInHz = 0;	// 창모드라면 0이어야한다.

		}
		if (0 == _d3dpp->BackBufferFormat)	// 풀모드일 경우 반드시 들어온다.
		{
			_d3dpp->BackBufferFormat = D3DFMT_X8R8G8B8;
		}
		_d3dpp->hDeviceWindow = hWnd;  //This is our main (and only) window 
		_d3dpp->PresentationInterval = bVerticalSync ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	long C_DX9_DEVICE::CheckResourceFormat(_D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage)
	{
		D3DCAPS9 devCaps;
		pDevice->GetDeviceCaps(&devCaps);

		D3DDISPLAYMODE displayMode;
		pDirect3D9->GetAdapterDisplayMode(devCaps.AdapterOrdinal, &displayMode);

		return pDirect3D9->CheckDeviceFormat(devCaps.AdapterOrdinal, devCaps.DeviceType, displayMode.Format, dwUsage, resType, fmt);
	}

	bool C_DX9_DEVICE::QueryFeature(_DX9_QUERY_FEATURE_TYPE_ feature)
	{
		bool bResult = false;
		switch (feature)
		{
		case RQF_HARDWARETNL:
			bResult = DIS_SET(d3dCaps.DevCaps, D3DDEVCAPS_HWTRANSFORMANDLIGHT);
			//bResult = d3dCaps.VertexProcessingCaps ? true : false;
			break;
		case RQF_USERCLIPPLANE:
			bResult = (d3dCaps.MaxUserClipPlanes > 0);
			break;
		case RQF_VS11:
			bResult = (d3dCaps.VertexShaderVersion >= D3DVS_VERSION(1, 1));
			break;
		case RQF_VS20:
			bResult = (d3dCaps.VertexShaderVersion >= D3DVS_VERSION(2, 0));
			break;
		case RQF_PS10:
			bResult = (d3dCaps.PixelShaderVersion >= D3DPS_VERSION(1, 0));
			break;
		case RQF_PS20:
			bResult = (d3dCaps.PixelShaderVersion >= D3DPS_VERSION(2, 0));
			break;
		case RQF_PS30:
			bResult = (d3dCaps.PixelShaderVersion >= D3DPS_VERSION(3, 0));
			break;
			// TODO : Device에서 검사하는 걸로 수정할것
		case RQF_R32F:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_R32F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET));
			break;
		case RQF_A32B32G32R32F:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_A32B32G32R32F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET));
			break;
		case RQF_A16B16G16R16F:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_A16B16G16R16F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET));
			break;
		case RQF_R16F:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_R16F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET));
			break;
		case RQF_RGB16:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_R5G6B5, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET));
			break;
		case RQF_G16R16F:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_G16R16F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET));
			break;
		case RQF_G32R32F:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_G32R32F, D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET));
			break;
		case RQF_VERTEXTEXTURE:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_R32F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_VERTEXTEXTURE));
			break;
		case RQF_HWSHADOWMAP:
			//------------------------------------------------------------------------
			// "Hardware Shadow Support" means that shadow depth maps are automatically sampled using PCF (Percentage Closer Filtering), 
			//  hardware shadow maps are enabled by creating a texture with a depth format (D16, D24X8, D24S8),
			//  with usage DEPTHSTENCIL set.
			bResult = SUCCEEDED(CheckResourceFormat(D3DFMT_D24S8, D3DRTYPE_TEXTURE, D3DUSAGE_DEPTHSTENCIL));
			break;
		case RQF_WFOG:
			bResult = DIS_SET(d3dCaps.RasterCaps, D3DPRASTERCAPS_WFOG);
			break;
		case RQF_MRTINDEPENDENTBITDEPTHS:
			bResult = DIS_SET(d3dCaps.PrimitiveMiscCaps, D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS);
			break;
			// 텍스쳐 필터 지원 여부
			// To check if a format supports texture filter types other than D3DTEXF_POINT (which is always supported), call IDirect3D9::CheckDeviceFormat with D3DUSAGE_QUERY_FILTER.
			// SUCCEEDED 매크로는 너무 너그럽다. 무조건 S_OK만 걸러내자.
		case RQF_RGB16_RTF:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_R5G6B5, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
			break;
		case RQF_R32F_RTF:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_R32F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
			break;
		case RQF_A8R8G8B8_RTF:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_A8R8G8B8, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
			break;
		case RQF_A32B32G32R32F_RTF:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_A32B32G32R32F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
			break;
		case RQF_A16B16G16R16F_RTF:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_A16B16G16R16F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
			break;
		case RQF_R16F_RTF:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_R16F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
			break;
		case RQF_G32R32F_RTF:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_G32R32F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER));
			break;
		case RQF_MRTBLEND_R32F:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_R32F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING));
			break;
		case RQF_MRTBLEND_G16R16F:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_G16R16F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING));
			break;
		case RQF_MRTBLEND_A8R8G8B8:
			bResult = (S_OK == CheckResourceFormat(D3DFMT_A8R8G8B8, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING));
			break;
		default:
			break;
		}
		return(bResult);
	}

#if defined(LAYERED_WINDOW)
	bool C_DX9_DEVICE::ImageCreate(_IMAGE* pImage)
	{
		bool bResult = false;
		if (pImage)
		{
			pImage->nWidth = sizeLayeredScreen.cx;
			pImage->nHeight = sizeLayeredScreen.cy;
			pImage->nPitch = ((sizeLayeredScreen.cx * 32 + 31) & ~31) >> 3;
			pImage->pPixels = nullptr;

			pImage->hdc = CreateCompatibleDC(0);
			if (pImage->hdc)
			{
				ZeroMemory(&pImage->info, sizeof(pImage->info));
				pImage->info.bmiHeader.biSize = sizeof(pImage->info.bmiHeader);
				pImage->info.bmiHeader.biBitCount = 32;
				pImage->info.bmiHeader.biWidth = sizeLayeredScreen.cx;
				pImage->info.bmiHeader.biHeight = -sizeLayeredScreen.cy;
				pImage->info.bmiHeader.biCompression = BI_RGB;
				pImage->info.bmiHeader.biPlanes = 1;
				pImage->hBitmap = CreateDIBSection(pImage->hdc, &pImage->info, DIB_RGB_COLORS, (LPVOID*)&pImage->pPixels, NULL, 0);

				if (pImage->hBitmap)
				{
					//DBGPRINT(_T("이미지 생성 ㅇㅋ"));
					GdiFlush();
					bResult = true;
				}
				else
				{
					ImageDestroy(pImage);
				}
			}
		}
		return bResult;
	}

	void C_DX9_DEVICE::ImageDestroy(_IMAGE* pImage)
	{
		if (pImage)
		{
			pImage->nWidth = 0;
			pImage->nHeight = 0;
			pImage->nPitch = 0;
			if (pImage->hBitmap)
			{
				DeleteObject(pImage->hBitmap);
				pImage->hBitmap = 0;
			}
			if (pImage->hdc)
			{
				DeleteDC(pImage->hdc);
				pImage->hdc = 0;
			}
			memset(&pImage->info, 0, sizeof(pImage->info));
			pImage->pPixels = 0;
		}
	}

	bool C_DX9_DEVICE::InitLayeredTexture(_D3DFORMAT format, _D3DFORMAT depthStencil)
	{
		bool bResult = false;
		//sizeLayeredScreen.SetSize(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
		sizeLayeredScreen.Set(rectRender.right, rectRender.bottom);

		HRESULT hResult = D3DXCreateTexture(pDevice, sizeLayeredScreen.cx, sizeLayeredScreen.cy, 0, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &pLayeredTexture);
		if (SUCCEEDED(hResult))
		{
			hResult = pLayeredTexture->GetSurfaceLevel(0, &pRenderTargetSurface);
		}
		if (SUCCEEDED(hResult))
		{
			hResult = pDevice->CreateDepthStencilSurface(sizeLayeredScreen.cx, sizeLayeredScreen.cy, depthStencil, D3DMULTISAMPLE_NONE, 0, TRUE, &pDepthStencilSurface, 0);
		}
		if (SUCCEEDED(hResult))
		{
			hResult = pDevice->CreateOffscreenPlainSurface(sizeLayeredScreen.cx, sizeLayeredScreen.cy, format, D3DPOOL_SYSTEMMEM, &pSurface, 0);
			bResult = true;
		}
		return bResult;
	}

	void C_DX9_DEVICE::CopyLayeredTextureImage()
	{
		if (SUCCEEDED(pDevice->GetRenderTargetData(pRenderTargetSurface, pSurface)))
		{
			D3DLOCKED_RECT rcLock = { 0 };
			if (SUCCEEDED(pSurface->LockRect(&rcLock, 0, 0)))
			{
				LPBYTE pSrc = (LPBYTE)rcLock.pBits;
				LPBYTE pDest = imgBack.pPixels;
				int srcPitch = rcLock.Pitch;
				int destPitch = imgBack.nPitch;

				if (srcPitch == destPitch)
				{
					memcpy(pDest, pSrc, destPitch * imgBack.nHeight);
				}
				else
				{
					for (long i = 0; i < imgBack.nHeight; i++)
					{
						memcpy(&pDest[destPitch * i], &pSrc[srcPitch * i], destPitch);
					}
				}
				pSurface->UnlockRect();
			}
		}
	}

	void C_DX9_DEVICE::RedrawLayeredWindow16()
	{
		HDC hDC = GetDC(hWnd);
		if (0 != hDC)
		{
			HGDIOBJ hPrevObj = SelectObject(imgBack.hdc, imgBack.hBitmap);

			dk::DPOINT ptDest;
			ClientToScreen(hWnd, &ptDest);
			//DBGPRINT("ptDest: %i / %i", ptDest.x, ptDest.y);

			dk::DPOINT ptSrc;
			dk::DSIZE sizeClient(imgBack.nWidth, imgBack.nHeight);
			BLENDFUNCTION blendFunc =
			{
				AC_SRC_OVER
				, 0
				, bytAlphaBlend
				, AC_SRC_ALPHA
			};
			UpdateLayeredWindow(hWnd, hDC, &ptDest, &sizeClient, imgBack.hdc, &ptSrc, 0, &blendFunc, ULW_ALPHA);
			SelectObject(imgBack.hdc, hPrevObj);
			ReleaseDC(hWnd, hDC);
		}
	}
#endif

	_DX9_VERTEX_BUFFER* C_DX9_DEVICE::wrappCreateVertexBuffer(UINT _nStructSize, DWORD _nCreateCount, DWORD _dwFVF, DWORD _dwFlags, LPVOID _pData)
	{
		_DX9_VERTEX_BUFFER* pDX9VertexBuffer = new _DX9_VERTEX_BUFFER(_nStructSize, _nCreateCount, _dwFVF, _dwFlags);
		UINT nSize = _nStructSize * _nCreateCount;
		
		if (pDX9VertexBuffer->Create(pDevice))
		{
			if (nullptr != _pData)
			{
				LPVOID pDest = nullptr;
				HRESULT hResult = pDX9VertexBuffer->pVertexBuffer->Lock(
					0
					, nSize
					, &pDest
					, DIS_SET(_dwFlags, D3DUSAGE_DYNAMIC) ? D3DLOCK_DISCARD : 0
				);
				if (SUCCEEDED(hResult))
				{
					memcpy_s(pDest, nSize, _pData, nSize);
					pDX9VertexBuffer->pVertexBuffer->Unlock();
				}
			}
			// 여기에서 맵에 넣고
			listDX9VertexBuffers.push_back(pDX9VertexBuffer);
		}
		return(pDX9VertexBuffer);
	}

	void C_DX9_DEVICE::DeleteVertexBuffer(_DX9_VERTEX_BUFFER* _pDX9VertexBuffer)
	{
		_pDX9VertexBuffer->Release();	// 디바이스 해제하고
		for (std::list<_DX9_VERTEX_BUFFER*>::const_iterator i = listDX9VertexBuffers.begin(); i != listDX9VertexBuffers.end();++i)
		{
			if (*i == _pDX9VertexBuffer)
			{
				listDX9VertexBuffers.erase(i);		// 리스트에서 제거하고
				DSAFE_DELETE(_pDX9VertexBuffer);	// 메모리 할당을 해제한다.
				break;								// 목적을 달성했으니 반복문을 끝낸다.
			}
		}
	}
	
	_DX9_INDEX_BUFFER* C_DX9_DEVICE::wrappCreateIndexBuffer(UINT _nCreateCount, DWORD _dwFlags, LPVOID _pData)
	{
		_DX9_INDEX_BUFFER* pDX9IndexBuffer = new _DX9_INDEX_BUFFER(sizeof(WORD), _nCreateCount, _dwFlags);

		UINT nSize = sizeof(WORD) * _nCreateCount;
		if (pDX9IndexBuffer->Create(pDevice))
		{
			if (nullptr != _pData)
			{
				LPVOID pDest = nullptr;
				HRESULT hResult = pDX9IndexBuffer->pIndexBuffer->Lock(
					0
					, nSize
					, &pDest
					, DIS_SET(_dwFlags, D3DUSAGE_DYNAMIC) ? D3DLOCK_DISCARD : 0
				);
				if (SUCCEEDED(hResult) )
				{
					memcpy_s(pDest, nSize, _pData, nSize);
					pDX9IndexBuffer->pIndexBuffer->Unlock();
				}
			}
			// 여기에서 맵에 넣고
			listDX9IndexBuffers.push_back(pDX9IndexBuffer);
		}
		return(pDX9IndexBuffer);
	}

	void C_DX9_DEVICE::DeleteIndexBuffer(_DX9_INDEX_BUFFER* _pDX9IndexBuffer)
	{
		_pDX9IndexBuffer->Release();	// 디바이스 해제하고
		for (std::list<_DX9_INDEX_BUFFER*>::const_iterator i = listDX9IndexBuffers.begin(); i != listDX9IndexBuffers.end();++i)
		{
			if (*i == _pDX9IndexBuffer)
			{
				listDX9IndexBuffers.erase(i);		// 리스트에서 제거하고
				DSAFE_DELETE(_pDX9IndexBuffer);		// 메모리 할당을 해제한다.
				break;								// 목적을 달성했으니 반복문을 끝낸다.
			}
		}
	}
	_DX9_TEXTURE* C_DX9_DEVICE::wrappCreateTexture(LPCWSTR pFile)
	{
		//DBGPRINT("wrappCreateTexture()");
		_DX9_TEXTURE* pDX9Texture = new _DX9_TEXTURE(pDevice);
		pDX9Texture->LoadTexture(pFile);
		//DBGPRINT("wrappCreateTexture(Create Start)");
		listDX9Textures.push_back(pDX9Texture);

		return(pDX9Texture);
	}
	_DX9_TEXTURE* C_DX9_DEVICE::wrappCreateTexture(DWORD _nWidth, DWORD _nHeight, _D3DFORMAT _d3dFormat, DWORD _dwFlags)
	{
		//DBGPRINT("wrappCreateTexture()");
		_DX9_TEXTURE* pDX9Texture = new _DX9_TEXTURE(
			pDevice
			, _nWidth
			, _nHeight
			, _d3dFormat
			, _dwFlags
		);
		//DBGPRINT("wrappCreateTexture(Create Start)");
		pDX9Texture->Create();
		listDX9Textures.push_back(pDX9Texture);

		return(pDX9Texture);
	}
	void C_DX9_DEVICE::DeleteTexture(_DX9_TEXTURE* _pDX9Texture)
	{
		if (_pDX9Texture)
		{
			DSAFE_RELEASE(_pDX9Texture);	// 디바이스 해제하고
			for (std::list<_DX9_TEXTURE*>::const_iterator i = listDX9Textures.begin(); i != listDX9Textures.end(); ++i)
			{
				if (*i == _pDX9Texture)
				{
					listDX9Textures.erase(i);		// 리스트에서 제거하고
					DSAFE_DELETE(_pDX9Texture);		// 메모리 할당을 해제한다.
					break;								// 목적을 달성했으니 반복문을 끝낸다.
				}
			}
		}
	}

	_DX9_FONT* C_DX9_DEVICE::wrappCreateFont(LPCWSTR _wszName, int _nSize, UINT _nWeight, UINT _nCharset, bool _bItalic, bool _bAntiAliased)
	{
		DBGPRINT("pDevice: %x", pDevice);
		_DX9_FONT* pDX9Font = new _DX9_FONT(
			pDevice 
			, _wszName
			, _nSize
			, _nWeight
			, _nCharset
			, _bItalic
			, _bAntiAliased
		);
		pDX9Font->Create();
		listDX9Fonts.push_back(pDX9Font);
		return(pDX9Font);
	}

	void C_DX9_DEVICE::wrappDeleteFont(_DX9_FONT* _pDX9Font)
	{
		_pDX9Font->Release();	// 디바이스 해제하고
		for (std::list<_DX9_FONT*>::const_iterator i = listDX9Fonts.begin(); i != listDX9Fonts.end();++i)
		{
			if (*i == _pDX9Font)
			{
				listDX9Fonts.erase(i);		// 리스트에서 제거하고
				DSAFE_DELETE(_pDX9Font);	// 메모리 할당을 해제한다.
				break;						// 목적을 달성했으니 반복문을 끝낸다.
			}
		}
	}

	HRESULT C_DX9_DEVICE::wrappBeginScene()
	{
		return pDevice->BeginScene();
	}

	void C_DX9_DEVICE::wrappEndScene()
	{
		pDevice->EndScene();
	}

	void C_DX9_DEVICE::wrappClear(DWORD _dwColor, DWORD _dwFlags, float _fZ, DWORD _dwStencil, DWORD _dwIndex)
	{
#if defined(LAYERED_WINDOW)
		pDevice->SetDepthStencilSurface(pDepthStencilSurface);
		pDevice->SetRenderTarget(0, pRenderTargetSurface);
#endif
		HRESULT hResult = pDevice->Clear(
			_dwIndex
			, NULL
			, _dwFlags
			, _dwColor
			, _fZ
			, _dwStencil
		);
		if (FAILED(hResult))
		{
			DBGPRINT("[실패] C_DX9_DEVICE::wrappClear(): %d / %d", hResult, ::GetLastError());
		}
	}

	HRESULT C_DX9_DEVICE::wrappPresent(HWND hDestWindowOverride, LPRECT pDst, LPRECT pSrc, RGNDATA* pDirtyRegion)
	{
		//DBGPRINT("C_DX9_DEVICE::Present(%x)", pDevice);
#if defined(LAYERED_WINDOW)
		CopyLayeredTextureImage();
		RedrawLayeredWindow16();
#endif
		//dk::DRECT rcDisplay{ (LONG)v2DisplayPos.x, (LONG)v2DisplayPos.y, (LONG)v2DisplaySize.x, (LONG)v2DisplaySize.y };
		//------------------------------------------------------------------------
		// 후면 버퍼를 보이는 버퍼로, 호출하면 fps가 60정도로 된다.
		HRESULT hResult = pDevice->Present(
			pSrc
			//, pDst ? pDst : &rectRender
			, pDst
			, hDestWindowOverride
			, pDirtyRegion
		);
		if (0 > hResult)	// Present 가 실패했을 경우
		{
			DBGPRINT("Present 실패");
			_DEVICE_STATUS_ nStatus = GetDeviceStatus();
			if (_DX9_DEVICE_RESTORED == nStatus)		// 복구 가능한가?
			{
				DBGPRINT("복구 가능, 시도");
				ResetDevice();
				DBGPRINT("복구 시도, 완료");
			}
			else if (_DX9_DEVICE_DESTROY == nStatus)	// 디바이스가 복구 불가능
			{
				// 프로그램을 종료하도록 하자
				DBGPRINT("복구 불가능");
			}
		}
		// 펑션을 실행하고 나면 이전세팅이 무효화되는 문제가 있어서 삽입
		pDevice->SetIndices(0);
		pDevice->SetStreamSource(0, 0, 0, 0);
		return(hResult);
	}

	UINT C_DX9_DEVICE::GetSamplerNumberToSaveIndex(UINT _nStage)
	{
		// 0 ~ 15 or RVERTEXTEXTURESAMPLER0 ~ RVERTEXTEXTURESAMPLER3
		if (_nStage >= RVERTEXTEXTURESAMPLER0)
		{
			return _nStage - RVERTEXTEXTURESAMPLER0 + MAX_IMAGEUNIT - MAX_VERTEXSAMPLER;
		}
		return _nStage;
	}

	UINT C_DX9_DEVICE::GetSaveIndexToSamplerNumber(UINT _nIndex)
	{
		if (_nIndex >= MAX_IMAGEUNIT - MAX_VERTEXSAMPLER)
		{
			return _nIndex - (MAX_IMAGEUNIT - MAX_VERTEXSAMPLER) + RVERTEXTEXTURESAMPLER0;
		}
		return _nIndex;
	}

	void C_DX9_DEVICE::wrappSetTexture(int nStage, _DX9_TEXTURE* _pDX9Texture)
	{
		/*
		UINT nSaveIndex = GetSamplerNumberToSaveIndex(nStage);
		
		// 중복된 텍스쳐 변환 생략
		if (currentTextures[nSaveIndex] == _pDX9Texture)
		{
			return;
		}
		currentTextures[nSaveIndex] = _pDX9Texture;
		*/
		pDevice->SetTexture(nStage, _pDX9Texture ? _pDX9Texture->pTexture : 0);
	}

	HRESULT C_DX9_DEVICE::wrappDrawIndexedPrimitive(D3DPRIMITIVETYPE _PrimitiveType, INT _nBaseVertexIndex, UINT _nMinVertexIndex, UINT _nNumVertices, UINT _nStartIndex, UINT _nPrimCount)
	{
		return pDevice->DrawIndexedPrimitive(_PrimitiveType, _nBaseVertexIndex, _nMinVertexIndex, _nNumVertices, _nStartIndex, _nPrimCount);
	}

	void C_DX9_DEVICE::wrappSetRenderState(D3DRENDERSTATETYPE _State, DWORD _Value)
	{
		// 상태를 저장해놓고 같은 상태라면 다시 호출하지 않는다. 문제가 발생하는지는 확인해봐야함.
		if (dwRenderState[_State] != _Value)
		{
			pDevice->SetRenderState(_State, _Value);
			dwRenderState[_State] = _Value;
		}
		
	}
	void C_DX9_DEVICE::wrappSetAlphaRef(DWORD dwRef)
	{
		if (currentAlphaRef != dwRef)
		{
			currentAlphaRef = dwRef;
		}
		pDevice->SetRenderState(D3DRS_ALPHAREF, dwRef);
	}

	void C_DX9_DEVICE::wrappSetAlphaFunc(_D3D_CMP_FUNC Func)
	{
		if (currentAlphaFunc != Func)
		{
			currentAlphaFunc = Func;
		}
		pDevice->SetRenderState(D3DRS_ALPHAFUNC, d3dCmpFuncTable[Func]);
	}

	void C_DX9_DEVICE::wrappSetSamplerState(DWORD _Sampler, D3DSAMPLERSTATETYPE _Type, DWORD _Value)
	{
		if (dwSamplerState[_Type] != _Value)
		{
			dwSamplerState[_Type] = _Value;
		}
		pDevice->SetSamplerState(_Sampler, _Type, _Value);
	}
	void C_DX9_DEVICE::wrappSetFVF(DWORD _fvf)
	{
		if (_fvf != currentFVF)
		{
			currentFVF = _fvf;
			currentVertexFormat = 0;
		}
		pDevice->SetFVF(_fvf);
		
	}
	void C_DX9_DEVICE::wrappSetTextureStageState(int nStage, _TEXTURE_STAGE_STATE_TYPE_ nStageStateType, unsigned int value)
	{
		DWORD dwValue = value;
		/*
		unsigned int nSaveIndex = GetSamplerNumberToSaveIndex(nStage);
		if (currentTextureStageSettings[nSaveIndex][nStageStateType] != value)
		{
			currentTextureStageSettings[nSaveIndex][nStageStateType] = value;
			if (nStageStateType == RTSS_COLOROP || nStageStateType == RTSS_ALPHAOP)
			{
				dwValue = d3dTextureOPTable[value];
			}
		}
		*/
		if (_D3DTSS_COLOROP == nStageStateType || _D3DTSS_ALPHAOP == nStageStateType)
		{
			dwValue = d3dTextureOPTable[value];
		}
		pDevice->SetTextureStageState(nStage, d3dTextureStageStateTypeTable[nStageStateType], dwValue);
	}
	void C_DX9_DEVICE::ShaderOff()
	{
		pDevice->SetVertexShader(NULL);
		pDevice->SetPixelShader(NULL);
	}
	void C_DX9_DEVICE::wrappSetVertexBuffer(_DX9_VERTEX_BUFFER* _pDX9VertexBuffer, int _nStream, UINT _nOffset)
	{
		pDevice->SetStreamSource(
			_nStream
			, _pDX9VertexBuffer->pVertexBuffer
			, _nOffset
			, _pDX9VertexBuffer->nStructSize
		);
		currentVertexBuffer[_nStream] = _pDX9VertexBuffer;
		currentVertexOffset[_nStream] = _nOffset;
		/*
		if (_pDX9VertexBuffer != currentVertexBuffer[_nStream]
			|| _nOffset != currentVertexOffset[_nStream]
			)
		{
			if (nullptr == _pDX9VertexBuffer)
			{
				DBGPRINT("SetStreamSource(여기오면안돼)");
				pDevice->SetStreamSource(_nStream, NULL, 0, 0);
			}
			else
			{
				//listDX9VertexBuffers.
				pDevice->SetStreamSource(
					_nStream
					, _pDX9VertexBuffer->pVertexBuffer
					, _nOffset
					, _pDX9VertexBuffer->nStructSize
				);
			}
			currentVertexBuffer[_nStream] = _pDX9VertexBuffer;
			currentVertexOffset[_nStream] = _nOffset;
		}
		*/
	}

	void C_DX9_DEVICE::wrappSetIndexBuffer(_DX9_INDEX_BUFFER* _pDX9IndexBuffer)
	{
		pDevice->SetIndices(_pDX9IndexBuffer ? _pDX9IndexBuffer->pIndexBuffer : 0);
		currentIndexBuffer = _pDX9IndexBuffer;
		/*
		if (_pDX9IndexBuffer != currentIndexBuffer)
		{
			//DBGPRINT("SetIndices()");
			pDevice->SetIndices(_pDX9IndexBuffer ? _pDX9IndexBuffer->pIndexBuffer : 0);
			currentIndexBuffer = _pDX9IndexBuffer;
		}
		*/
	}

	void C_DX9_DEVICE::wrappSetVertexShaderConstantF(UINT _StartRegister, const float* _pConstantData, UINT _Vector4fCount)
	{
		pDevice->SetVertexShaderConstantF(_StartRegister, _pConstantData, _Vector4fCount);
	}

	void C_DX9_DEVICE::wrappSetViewport(dx9::LPDVIEWPORT9 _pViewport)
	{
		dx9Viewport.Set(_pViewport);
		pDevice->SetViewport(&dx9Viewport);
	}

	void C_DX9_DEVICE::wrappSetViewport(DWORD _x, DWORD _y, DWORD _nWidth, DWORD _nHeight, float _fMinZ, float _fMaxZ)
	{
		dx9Viewport.Set(_x, _y, _nWidth, _nHeight, _fMinZ, _fMaxZ);
		pDevice->SetViewport(&dx9Viewport);
	}

	dx9::LPDVIEWPORT9 C_DX9_DEVICE::GetViewport()
	{
		return &dx9Viewport;
	}

	void C_DX9_DEVICE::wrappSetTextureFilter(int nSampler, _TEXTURE_FILTER_TYPE type)
	{
		UINT nSaveIndex = GetSamplerNumberToSaveIndex(nSampler);

		if (type != currentTextureFilter[nSaveIndex])
		{
			if (type < _D3DTEXF_COUNT)
			{
				/// _TEXTURE_FILTER_TYPE은 min,mag,mip을 뭉뚱그려 관리된다. _TEXTURE_FILTER_TYPE 내에서도 각각 중복되는 것이 있는지를 체크하여 셋팅하여야 함.
				if (d3dTextureFilterSets[type].minFilter != d3dTextureFilterSets[currentTextureFilter[nSaveIndex]].minFilter)
				{
					pDevice->SetSamplerState(nSampler, D3DSAMP_MINFILTER, d3dTextureFilterSets[type].minFilter);
				}
				if (d3dTextureFilterSets[type].magFilter != d3dTextureFilterSets[currentTextureFilter[nSaveIndex]].magFilter)
				{
					pDevice->SetSamplerState(nSampler, D3DSAMP_MAGFILTER, d3dTextureFilterSets[type].magFilter);
				}
				if (d3dTextureFilterSets[type].mipFilter != d3dTextureFilterSets[currentTextureFilter[nSaveIndex]].mipFilter)
				{
					pDevice->SetSamplerState(nSampler, D3DSAMP_MIPFILTER, d3dTextureFilterSets[type].mipFilter);
				}
			}
			currentTextureFilter[nSaveIndex] = type;
		}
	}

	void C_DX9_DEVICE::wrappSetTransform(_SETTREANSFORM_TYPE type, const _DMATRIX9* matrix)
	{
		D3DTRANSFORMSTATETYPE d3dtransformtypes[] =
		{
			D3DTS_WORLD
			, D3DTS_VIEW
			, D3DTS_PROJECTION
			, D3DTS_TEXTURE0
			, D3DTS_TEXTURE1
			, D3DTS_TEXTURE2
			, D3DTS_TEXTURE3
			, D3DTS_TEXTURE4
			, D3DTS_TEXTURE5
			, D3DTS_TEXTURE6
			, D3DTS_TEXTURE7
		};
		pDevice->SetTransform(d3dtransformtypes[type], (D3DMATRIX*)matrix);
		currentTransform[type].Set(matrix);
	}

	_DMATRIX9 C_DX9_DEVICE::wrappGetTransform(_SETTREANSFORM_TYPE type) const
	{
		return currentTransform[type];
	}

	void C_DX9_DEVICE::OnShowCursor(bool b)
	{
		pDevice->ShowCursor(b);
	}

	long C_DX9_DEVICE::GetStatus()
	{
		if (nullptr != pDevice)
		{
			// D3DERR_DEVICELOST = If the device was lost, do not render until we get it back
			// D3DERR_DEVICENOTRESET = Check if the device needs to be resized.
			return pDevice->TestCooperativeLevel();
		}
		return(0);
	}

	bool C_DX9_DEVICE::ResetDevice(dk::LPDSIZE _pSize)
	{
		if (nullptr != pDevice)
		{
			nLastDeviceStatus = _DX9_DEVICE_RESTORED;

			OnLostDevice();

			_DX9_PRESENT_PARAMETERS d3dpp(nullptr != _pSize ? _pSize->cx : 0, nullptr != _pSize ? _pSize->cy : 0);
			InitPresentParameters(&d3dpp);
			HRESULT hr = pDevice->Reset(&d3dpp);
			if (hr == D3DERR_INVALIDCALL)
			{
				DBGPRINT(L"D3DERR_INVALIDCALL <-- 여기 오면 안됨");
			}
			else
			{
				dk::DRECT rect;
				::GetClientRect(hWnd, &rect);
				v2DisplaySize.Set((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
				
				InitDeviceDefault();
				RestoreDevice();

				nLastDeviceStatus = _DX9_DEVICE_OK;	// 디바이스가 사용 가능 상태
				return(true);
			}
		}
		return(false);
	}

	void C_DX9_DEVICE::OnLostDevice()
	{
		//DSAFE_RELEASE(pFrameBuffer);
		//DSAFE_RELEASE(pDepthStencilBuffer);
#if defined(_USE_FAKE_VERTEX_)
		// 조합용 텍스쳐와 버텍스버퍼 해제
		DSAFE_RELEASE(pTextureCombination);
		DSAFE_RELEASE(pFakeVertexBuffer);
#endif
		ClearDX9States();	// 저장된 상태 제거
		DSAFE_RELEASE(pStateBlock);

		// 모든 텍스쳐 제거
		for (std::list<_DX9_TEXTURE*>::const_iterator i = listDX9Textures.begin(); i != listDX9Textures.end();++i)
		{
			_DX9_TEXTURE* pDX9Texture = *i;
			if (nullptr == pDX9Texture->pTexture)			// 이미 제거 되었는가?
			{
				listDX9Textures.erase(i);					// 그렇다면 리스트에서 제거
			}
			pDX9Texture->OnLostDevice();
		}

		// 모든 버텍스버퍼 해제
		for (std::list<_DX9_VERTEX_BUFFER*>::const_iterator i = listDX9VertexBuffers.begin(); i != listDX9VertexBuffers.end();++i)
		{
			_DX9_VERTEX_BUFFER* pDX9VertexBuffer = *i;
			if (nullptr == pDX9VertexBuffer->pVertexBuffer)		// 이미 제거 되었는가?
			{
				listDX9VertexBuffers.erase(i);					// 그렇다면 리스트에서 제거
			}
			if (D3DPOOL_DEFAULT == pDX9VertexBuffer->d3dPool)	// D3DPOOL_DEFAULT 는 리셋시 디바이스가 해제되기 때문에 제거하자
			{
				DSAFE_RELEASE(pDX9VertexBuffer->pVertexBuffer);	// 해제하고 복구를 위해 리스트에는 그대로 놔둔다
			}
		}
		// 모든 인덱스버퍼 해제
		for (std::list<_DX9_INDEX_BUFFER*>::const_iterator i = listDX9IndexBuffers.begin(); i != listDX9IndexBuffers.end();++i)
		{
			_DX9_INDEX_BUFFER* pDX9IndexBuffer = *i;
			if (nullptr == pDX9IndexBuffer->pIndexBuffer)		// 이미 제거 되었는가?
			{
				listDX9IndexBuffers.erase(i);					// 그렇다면 리스트에서 제거
			}
			if (D3DPOOL_DEFAULT == pDX9IndexBuffer->d3dPool)	// D3DPOOL_DEFAULT 는 리셋시 디바이스가 해제되기 때문에 제거하자
			{
				DSAFE_RELEASE(pDX9IndexBuffer->pIndexBuffer);	// 해제하고 복구를 위해 리스트에는 그대로 놔둔다
			}
		}

		// 모든 폰트 제거
		for (std::list<_DX9_FONT*>::const_iterator i = listDX9Fonts.begin(); i != listDX9Fonts.end();++i)
		{
			_DX9_FONT* pDX9Font = *i;
			if (nullptr == pDX9Font->GetFont())			// 이미 제거 되었는가?
			{
				listDX9Fonts.erase(i);					// 그렇다면 리스트에서 제거
			}
			pDX9Font->OnLostDevice();
		}

		// 모든 셰이더 제거


		// D3D 쿼리 있을래나 몰라 아무튼 모두 제거


		//pSprite->OnLostDevice();
	}

	void C_DX9_DEVICE::RestoreDevice()
	{
		// 디폴트라서 해제되었던 텍스쳐를 다시 로드한다.
#if defined(_USE_FAKE_VERTEX_)
		if (bUseFakeVertex)
		{
			if (pTextureCombination)
			{
				DBGPRINT("C_DX9_DEVICE::RestoreDevice() - pTextureCombination 가 있음");
				pTextureCombination->Release();
				pTextureCombination = nullptr;
			}
			// 조합용 텍스처 생성
			::D3DXCreateTexture(pDevice, 512, 512, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTextureCombination);
			if (pFakeVertexBuffer)
			{	
				DBGPRINT("C_DX9_DEVICE::RestoreDevice() - pFakeVertexBuffer 가 있음");
				pFakeVertexBuffer->Release();
				pFakeVertexBuffer = nullptr;
			}
			// 2D 용 버텍스버퍼 생성
			pDevice->CreateVertexBuffer(4 * sizeof(_CUSTOM_VERTEX_FOR_2D), 0, _D3DFVF_CUSTOM_VERTEX_FOR_2D_, D3DPOOL_DEFAULT, &pFakeVertexBuffer, NULL);
		}
#endif
		// 여기에서 폰트 텍스쳐 등을 세팅하자.
		/*
		if (GetFont())
		GetFont()->OnResetDevice();
		*/
		//pSprite->OnResetDevice();

		// 모든 텍스쳐 복구
		for (std::list<_DX9_TEXTURE*>::const_iterator i = listDX9Textures.begin(); i != listDX9Textures.end();++i)
		{
			_DX9_TEXTURE* pDX9Texture = *i;
			pDX9Texture->OnResetDevice();
		}

		// 모든 버텍스버퍼 복구
		for (std::list<_DX9_VERTEX_BUFFER*>::const_iterator i = listDX9VertexBuffers.begin(); i != listDX9VertexBuffers.end();++i)
		{
			_DX9_VERTEX_BUFFER* pDX9VertexBuffer = *i;
			if (D3DPOOL_DEFAULT == pDX9VertexBuffer->d3dPool)	// D3DPOOL_DEFAULT 는 리셋시 디바이스가 해제되기 때문에 제거하자
			{
				if (nullptr == pDX9VertexBuffer->pVertexBuffer)		// 이미 제거 되었는가?
				{
					pDX9VertexBuffer->Create(pDevice);
				}
			}
		}
		// 모든 인덱스버퍼 복구
		for (std::list<_DX9_INDEX_BUFFER*>::const_iterator i = listDX9IndexBuffers.begin(); i != listDX9IndexBuffers.end();++i)
		{
			_DX9_INDEX_BUFFER* pDX9IndexBuffer = *i;
			if (D3DPOOL_DEFAULT == pDX9IndexBuffer->d3dPool)	// D3DPOOL_DEFAULT 는 리셋시 디바이스가 해제되기 때문에 제거하자
			{
				if (nullptr == pDX9IndexBuffer->pIndexBuffer)	// 제거 되었는가?
				{
					// 복구하자
					pDX9IndexBuffer->Create(pDevice);
				}
			}
		}

		// 모든 폰트 복구
		for (std::list<_DX9_FONT*>::const_iterator i = listDX9Fonts.begin(); i != listDX9Fonts.end();++i)
		{
			_DX9_FONT* pDX9Font = *i;
			pDX9Font->OnResetDevice();
		}


		//pDevice->GetRenderTarget(0, &pFrameBuffer);
		//pDevice->GetDepthStencilSurface(&pDepthStencilBuffer);

		//SetViewport(0, 0, GetScreenWidth(), GetScreenHeight());	// ViewPort 갱신
		DVIEWPORT9 vpt9(0, 0, (DWORD)v2DisplaySize.x, (DWORD)v2DisplaySize.y);
		wrappSetViewport(&vpt9);
#if defined(_USE_FAKE_VERTEX_)
		SetScreenSize();
#endif
	}

	void C_DX9_DEVICE::Begin2D()
	{
		DSAFE_RELEASE(pStateBlock);
		pDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock);	// DX9 의 상태를 백업
		
		pDevice->SetPixelShader(NULL);
		pDevice->SetVertexShader(NULL);

		pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);			// 컬링 기능을 끈다. 삼각형의 앞면, 뒷면을 모두 렌더링한다.
		pDevice->SetRenderState(D3DRS_LIGHTING, false);					// 정점에 색깔값이 있으므로 광원기능을 끈다.
		pDevice->SetRenderState(D3DRS_ZENABLE, false);
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);			// 알파블랜딩 켜고
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
		pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		pDevice->SetRenderState(D3DRS_FOGENABLE, false);

		pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

		// 뷰포트 백업
		pDevice->GetTransform(D3DTS_WORLD, &matWorld);
		pDevice->GetTransform(D3DTS_VIEW, &matView);
		pDevice->GetTransform(D3DTS_PROJECTION, &matProjection);

		// Orthogonal Projection
		DVIEWPORT9 vp(0, 0, (DWORD)v2DisplaySize.x, (DWORD)v2DisplaySize.y);
		pDevice->SetViewport(&vp);

		// 단일 뷰포트의 경우 x, y는 0이다.
		float fLeft = v2DisplayPos.x + 0.5f;
		float fRight = v2DisplayPos.x + v2DisplaySize.x + 0.5f;
		float fTop = v2DisplayPos.y + 0.5f;
		float fBottom = v2DisplayPos.y + v2DisplaySize.y + 0.5f;

		// <d3dx9.h> 또는 <DirectXMath.h>를 사용할 수 있는지 여부에 관계없이 다음의 함수에 의존하지 않음.
		// D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
		_DMATRIX9 matIdentity(
			1.0f, 0.0f, 0.0f, 0.0f
			, 0.0f, 1.0f, 0.0f, 0.0f
			, 0.0f, 0.0f, 1.0f, 0.0f
			, 0.0f, 0.0f, 0.0f, 1.0f
		);
		_DMATRIX9 matProj(
			2.0f / (fRight - fLeft), 0.0f, 0.0f, 0.0f
			, 0.0f, 2.0f / (fTop - fBottom), 0.0f, 0.0f
			, 0.0f, 0.0f, 0.5f, 0.0f
			, (fLeft + fRight) / (fLeft - fRight), (fTop + fBottom) / (fBottom - fTop), 0.5f, 1.0f
		);

		pDevice->SetTransform(D3DTS_WORLD, &matIdentity);
		pDevice->SetTransform(D3DTS_VIEW, &matIdentity);
		pDevice->SetTransform(D3DTS_PROJECTION, &matProj);
	}

	void C_DX9_DEVICE::End2D()
	{
		// 뷰포트 복구
		pDevice->SetTransform(D3DTS_WORLD, &matWorld);
		pDevice->SetTransform(D3DTS_VIEW, &matView);
		pDevice->SetTransform(D3DTS_PROJECTION, &matProjection);

		// DX9 상태 복구
		//pStateBlock->Apply();
		if (nullptr != pStateBlock)
		{
			pStateBlock->Apply();
			pStateBlock->Release();
			pStateBlock = nullptr;
		}
	}
}
