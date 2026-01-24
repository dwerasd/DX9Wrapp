#include "stdafx.h"
#include "CDX9DeviceEx.h"



namespace dx9
{
	typedef enum _D3DTEXTUREFILTERTYPE
	{
		D3DTEXF_NONE = 0,    // filtering disabled (valid for mip filter only)
		D3DTEXF_POINT = 1,    // nearest
		D3DTEXF_LINEAR = 2,    // linear interpolation
		D3DTEXF_ANISOTROPIC = 3,    // anisotropic
		D3DTEXF_PYRAMIDALQUAD = 6,    // 4-sample tent
		D3DTEXF_GAUSSIANQUAD = 7,    // 4-sample gaussian
		// D3D9Ex only --
#if !defined(D3D_DISABLE_9EX)
		D3DTEXF_CONVOLUTIONMONO = 8,    // Convolution filter for monochrome textures
#endif // !D3D_DISABLE_9EX
		// -- D3D9Ex only
		D3DTEXF_FORCE_DWORD = 0x7fffffff,   // force 32-bit size enum
	} D3DTEXTUREFILTERTYPE;

	// rtypes 에 정의된 texturefiltertype 에 대응
	struct _DX9_TEXTURE_FILTER_SET
	{
		D3DTEXTUREFILTERTYPE minFilter;
		D3DTEXTUREFILTERTYPE magFilter;
		D3DTEXTUREFILTERTYPE mipFilter;
	};

	const _DX9_TEXTURE_FILTER_SET d3dTextureFilterSets[] =
	{
		D3DTEXF_POINT,			D3DTEXF_POINT,		D3DTEXF_NONE,		// RTF_POINT
		D3DTEXF_LINEAR,			D3DTEXF_LINEAR,		D3DTEXF_NONE,		// RTF_LINEAR
		D3DTEXF_LINEAR,			D3DTEXF_LINEAR,		D3DTEXF_POINT,		// RTF_BILINEAR
		D3DTEXF_LINEAR,			D3DTEXF_LINEAR,		D3DTEXF_LINEAR,		// RTF_TRILINEAR
		D3DTEXF_ANISOTROPIC,	D3DTEXF_ANISOTROPIC,D3DTEXF_POINT,		// RTF_BILINEAR_ANISO
		D3DTEXF_ANISOTROPIC,	D3DTEXF_ANISOTROPIC,D3DTEXF_LINEAR,		// RTF_TRILINEAR_ANISO
		D3DTEXF_ANISOTROPIC,	D3DTEXF_LINEAR,		D3DTEXF_NONE,		// RTF_BILINEAR_ANISO_LINEAR_FOR_FONT
	};
	C_DX9_DEVICEEX::C_DX9_DEVICEEX()
		: bWindowMode(true)
		, bCursor(false)
		, bVerticalSync(false)
		, nLastDeviceStatus(0)
		, pDirect3D9Ex(nullptr)
		, pDevice(nullptr)
		, pSprite(nullptr)
		, bytAlphaBlend(255)
	{
		ClearDX9States();
	}

	C_DX9_DEVICEEX::~C_DX9_DEVICEEX()
	{
		DSAFE_RELEASE(pDevice);
		DSAFE_RELEASE(pDirect3D9Ex);
	}
	void C_DX9_DEVICEEX::ClearDX9States()
	{
		for (long i = 0; i < _countof(dwRenderState); ++i)
		{
			dwRenderState[i] = 0;
		}
		for (long i = 0; i < _countof(dwSamplerState); ++i)
		{
			dwSamplerState[i] = 0;
		}
	}

	LPDIRECT3DDEVICE9EX C_DX9_DEVICEEX::Init(HWND _hWnd, dk::DSIZE _sizeScreen)
	{
		hWnd = _hWnd;

		v2DisplaySize.Set((float)_sizeScreen.cx, (float)_sizeScreen.cy);

		//DBGPRINT("C_DX9_DEVICEEX::Init(start) %i / %i", dkScreenSize.cx, dkScreenSize.cy);
		//////////////////////////////////////////////////////////////////////////
		// 여기서 "d3d9.dll"이랑 "Direct3DCreate9"를 암호화 하고, 가상화를 걸자.

		HMODULE hD3D9 = LoadLibraryW(L"d3d9.dll");
		typedef HRESULT(__stdcall* Direct3DCreate9Ex_PROC)(UINT, LPDIRECT3D9EX*);
		if (0 != hD3D9)
		{
			Direct3DCreate9Ex_PROC pDirect3DCreate9Ex = (Direct3DCreate9Ex_PROC)GetProcAddress(hD3D9, "Direct3DCreate9Ex");
			DBGPRINT("pDirect3DCreate9Ex: %x", pDirect3DCreate9Ex);
			if (0 != pDirect3DCreate9Ex)
			{
				pDirect3DCreate9Ex(D3D_SDK_VERSION, &pDirect3D9Ex);
				DBGPRINT("pDirect3D9Ex: %x", pDirect3D9Ex);
			}
		}
		//////////////////////////////////////////////////////////////////////////
		do
		{
			if (nullptr == pDirect3D9Ex)
			{
				DBGPRINT("Direct3DCreate9Ex 실패");
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
			pDirect3D9Ex->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps);

			D3DDEVTYPE d3dDevType = D3DDEVTYPE_HAL;	// 3D 가속기로 렌더링, D3DDEVTYPE_REF = CPU 명령셋으로 렌더링

			DWORD dwBehaviorFlags = (QueryFeature(RQF_HARDWARETNL) ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING);
			DBGPRINT("하드웨어 버텍스 프로세싱: %s", DIS_SET(dwBehaviorFlags, D3DCREATE_HARDWARE_VERTEXPROCESSING) ? "ON" : "OFF");

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
			D3DPRESENT_PARAMETERS d3dpp = { 0 };
			InitPresentParameters(&d3dpp);
			if (nullptr != pDirect3D9Ex)
			{

				//D3DDISPLAYMODEEX displayModeEx = { 0 };
				//D3DDISPLAYROTATION displayRotation = D3DDISPLAYROTATION_IDENTITY;
				//pDirect3D9Ex->GetAdapterDisplayModeEx(D3DADAPTER_DEFAULT, &displayModeEx, &displayRotation);

				DBGPRINT("C_DX9_DEVICEEX::CreateDeviceEx(시작)");
				if (FAILED(pDirect3D9Ex->CreateDeviceEx(D3DADAPTER_DEFAULT, d3dDevType, hWnd, dwBehaviorFlags, &d3dpp, 0, &pDevice)))
				{
					DBGPRINT("C_DX9_DEVICEEX::CreateDeviceEx(실패1)");
					if (DIS_SET(dwBehaviorFlags, D3DCREATE_PUREDEVICE))
					{
						DBGPRINT("C_DX9_DEVICEEX::CreateDeviceEx(실패): D3DCREATE_PUREDEVICE 제거한 후 재시도");
						DREMOVE_BIT(dwBehaviorFlags, D3DCREATE_PUREDEVICE);
						pDirect3D9Ex->CreateDeviceEx(D3DADAPTER_DEFAULT, d3dDevType, hWnd, dwBehaviorFlags, &d3dpp, 0, &pDevice);
					}
					if (0 > pDevice)
					{
						DSAFE_RELEASE(pDirect3D9Ex);
						DBGPRINT("C_DX9_DEVICEEX::CreateDeviceEx(실패: %x)", pDevice);
						break;
					}
				}
				DBGPRINT("C_DX9_DEVICEEX::CreateDeviceEx(끝): %x", pDevice);
			}
#if defined(LAYERED_WINDOW)
			bInitLayeredWindow = InitLayeredTexture(D3DFMT_A8R8G8B8, D3DFMT_D24S8);
			if (FALSE != ImageCreate(&imgBack))
			{

			}
#endif
			D3DXCreateSprite(pDevice, &pSprite);

			dx9::DVIEWPORT9 vpt9(0, 0, (DWORD)v2DisplaySize.x, (DWORD)v2DisplaySize.y);
			pDevice->SetViewport(&vpt9);

			// 여기에서 폰트 텍스쳐를 생성하도록 하자.

		} while (false);
		nLastDeviceStatus = _DX9_DEVICE_OK;		// 디바이스가 사용 가능 상태
		DBGPRINT("C_DX9_DEVICEEX::Init(end)");
		return(pDevice);
	}

	void C_DX9_DEVICEEX::Destroy()
	{
		//DSAFE_DELETE(pViewport);
		DSAFE_RELEASE(pDevice);
		DSAFE_RELEASE(pDirect3D9Ex);
	}

	_DEVICE_STATUS_ C_DX9_DEVICEEX::GetDeviceStatus()
	{
		//DBGPRINT("C_DX9_DEVICEEX::GetDeviceStatus() 계속 진행");
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
			//HRESULT hResult = pDevice->TestCooperativeLevel();	// 항상 D3D_OK 를 리턴한다고 해서 EX로 바꾼다.
			HRESULT hResult = pDevice->CheckDeviceState(hWnd);
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

	bool C_DX9_DEVICEEX::IsCursorVisible()
	{
		return bCursor;
	}

	bool C_DX9_DEVICEEX::ShowCursor(bool b)
	{
		pDevice->ShowCursor(b);
		bool bPrevious = bCursor;
		bCursor = b;
		return bPrevious;
	}

	void C_DX9_DEVICEEX::InitPresentParameters(D3DPRESENT_PARAMETERS* _d3dpp)
	{
		// 모니터 사이즈로 초기화 함.
		_d3dpp->BackBufferWidth = rectRender.right = 3840;//::GetSystemMetrics(SM_CXSCREEN);
		_d3dpp->BackBufferHeight = rectRender.bottom = 2160;//::GetSystemMetrics(SM_CYSCREEN);
		//_d3dpp->BackBufferWidth = (UINT)v2DisplaySize.x;
		//_d3dpp->BackBufferHeight = (UINT)v2DisplaySize.y;

		//_d3dpp->BackBufferWidth = 1024;
		//_d3dpp->BackBufferHeight = 768;

		_d3dpp->Windowed = bWindowMode;

		if (false != _d3dpp->Windowed)		// 창모드라면
		{
			_D3DDISPLAYMODE d3ddm;
			if (SUCCEEDED(pDirect3D9Ex->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
			{
				_d3dpp->BackBufferFormat = d3ddm.Format;
			}
		}
		if (0 == _d3dpp->BackBufferFormat)	// 풀모드일 경우 반드시 들어온다.
		{
			_d3dpp->BackBufferFormat = D3DFMT_X8R8G8B8;
		}
		_d3dpp->BackBufferCount = 1;  //We only need a single back buffer 
		_d3dpp->SwapEffect = D3DSWAPEFFECT_DISCARD;
		_d3dpp->hDeviceWindow = hWnd;  //This is our main (and only) window 
		//_d3dpp->EnableAutoDepthStencil = TRUE;
		_d3dpp->AutoDepthStencilFormat = D3DFMT_D24S8;

		_d3dpp->PresentationInterval = bVerticalSync ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	long C_DX9_DEVICEEX::CheckResourceFormat(D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage)
	{
		D3DCAPS9 devCaps;
		pDevice->GetDeviceCaps(&devCaps);

		D3DDISPLAYMODE displayMode;
		pDirect3D9Ex->GetAdapterDisplayMode(devCaps.AdapterOrdinal, &displayMode);

		return pDirect3D9Ex->CheckDeviceFormat(devCaps.AdapterOrdinal, devCaps.DeviceType, displayMode.Format, dwUsage, resType, fmt);
	}

	bool C_DX9_DEVICEEX::QueryFeature(_DX9_QUERY_FEATURE_TYPE_ feature)
	{
		bool bResult = false;
		switch (feature)
		{
		case RQF_HARDWARETNL:
			bResult = DIS_SET(d3dcaps.DevCaps, D3DDEVCAPS_HWTRANSFORMANDLIGHT);
			break;
		case RQF_USERCLIPPLANE:
			bResult = (d3dcaps.MaxUserClipPlanes > 0);
			break;
		case RQF_VS11:
			bResult = (d3dcaps.VertexShaderVersion >= D3DVS_VERSION(1, 1));
			break;
		case RQF_VS20:
			bResult = (d3dcaps.VertexShaderVersion >= D3DVS_VERSION(2, 0));
			break;
		case RQF_PS10:
			bResult = (d3dcaps.PixelShaderVersion >= D3DPS_VERSION(1, 0));
			break;
		case RQF_PS20:
			bResult = (d3dcaps.PixelShaderVersion >= D3DPS_VERSION(2, 0));
			break;
		case RQF_PS30:
			bResult = (d3dcaps.PixelShaderVersion >= D3DPS_VERSION(3, 0));
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
			bResult = DIS_SET(d3dcaps.RasterCaps, D3DPRASTERCAPS_WFOG);
			break;
		case RQF_MRTINDEPENDENTBITDEPTHS:
			bResult = DIS_SET(d3dcaps.PrimitiveMiscCaps, D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS);
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
		return false;
	}

#if defined(LAYERED_WINDOW)
	BOOL C_DX9_DEVICEEX::ImageCreate(_IMAGE* pImage)
	{
		BOOL bResult = FALSE;
		if (pImage)
		{
			pImage->nWidth = sizeLayeredScreen.cx;
			pImage->nHeight = sizeLayeredScreen.cy;
			pImage->nPitch = ((sizeLayeredScreen.cx * 32 + 31) & ~31) >> 3;
			pImage->pPixels = nullptr;

			pImage->hdc = CreateCompatibleDC(NULL);
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
					bResult = TRUE;
				}
				else
				{
					ImageDestroy(pImage);
				}
			}
		}
		return bResult;
	}

	void C_DX9_DEVICEEX::ImageDestroy(_IMAGE* pImage)
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

	BOOL C_DX9_DEVICEEX::InitLayeredTexture(D3DFORMAT format, D3DFORMAT depthStencil)
	{
		BOOL bResult = FALSE;
		//sizeLayeredScreen.SetSize(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
		sizeLayeredScreen = dkScreenSize;

		HRESULT hResult = D3DXCreateTexture(pDevice, sizeLayeredScreen.cx, sizeLayeredScreen.cy, 0, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &pLayeredTexture);
		if (SUCCEEDED(hResult))	// 만약 실패라면
		{
			hResult = pLayeredTexture->GetSurfaceLevel(0, &pRenderTargetSurface);
		}
		if (SUCCEEDED(hResult))	// 이걸 안한다
		{
			hResult = pDevice->CreateDepthStencilSurface(sizeLayeredScreen.cx, sizeLayeredScreen.cy, depthStencil, D3DMULTISAMPLE_NONE, 0, TRUE, &pDepthStencilSurface, 0);
		}
		if (SUCCEEDED(hResult)) // 맨위에꺼 실패면 또 안한다
		{
			hResult = pDevice->CreateOffscreenPlainSurface(sizeLayeredScreen.cx, sizeLayeredScreen.cy, format, D3DPOOL_SYSTEMMEM, &pSurface, 0);
			bResult = TRUE;
		}
		return bResult;
	}

	void C_DX9_DEVICEEX::CopyLayeredTextureImage()
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
					for (long i = 0; i < imgBack.nHeight; ++i)
					{
						memcpy(&pDest[destPitch * i], &pSrc[srcPitch * i], destPitch);
					}
				}
				pSurface->UnlockRect();
			}
		}
	}

	void C_DX9_DEVICEEX::RedrawLayeredWindow16()
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
	HRESULT C_DX9_DEVICEEX::BeginScene()
	{
		return pDevice->BeginScene();
	}

	void C_DX9_DEVICEEX::EndScene()
	{
		pDevice->EndScene();
	}

	void C_DX9_DEVICEEX::Clear(DWORD _dwFlags, DWORD _dwColor, float _fZ, DWORD _dwStencil, DWORD _dwIndex)
	{
#if defined(LAYERED_WINDOW)
		pDevice->SetDepthStencilSurface(pDepthStencilSurface);
		pDevice->SetRenderTarget(0, pRenderTargetSurface);
#endif
		pDevice->Clear(
			_dwIndex
			, NULL
			, _dwFlags
			, _dwColor
			, _fZ
			, _dwStencil
		);
	}

	void C_DX9_DEVICEEX::Present(HWND hDestWindowOverride, LPRECT pDst, LPRECT pSrc, RGNDATA* pDirtyRegion)
	{
		//DBGPRINT("C_DX9_DEVICEEX::Present(%x)", pDevice);
#if defined(LAYERED_WINDOW)
		CopyLayeredTextureImage();
		RedrawLayeredWindow16();
#endif
		//dk::DRECT rcDisplay{ (LONG)v2DisplayPos.x, (LONG)v2DisplayPos.y, (LONG)v2DisplaySize.x, (LONG)v2DisplaySize.y };
		//------------------------------------------------------------------------
		// 후면 버퍼를 보이는 버퍼로, 호출하면 fps가 60정도로 된다.
		HRESULT hResult = pDevice->Present(
			pSrc
			, pDst ? pDst : &rectRender
			, hDestWindowOverride
			, pDirtyRegion
		);
		if (0 > hResult)	// Present 가 실패했을 경우
		{
			DBGPRINT("Present 실패");
			_DEVICE_STATUS_ nStatus = GetDeviceStatus();
			if (_DX9_DEVICE_RESTORED == nStatus)		// 복구 가능한가?
			{
				DBGPRINT("복구가능");
				ResetDevice();
				DBGPRINT("복구완료");
			}
			else if (_DX9_DEVICE_DESTROY == nStatus)	// 디바이스가 복구 불가능
			{
				// 프로그램을 종료하도록 하자

			}
		}
		// 펑션을 실행하고 나면 이전세팅이 무효화되는 문제가 있어서 삽입
		pDevice->SetIndices(0);
		pDevice->SetStreamSource(0, 0, 0, 0);
	}

	HRESULT C_DX9_DEVICEEX::DrawIndexedPrimitive(D3DPRIMITIVETYPE _PrimitiveType, INT _nBaseVertexIndex, UINT _nMinVertexIndex, UINT _nNumVertices, UINT _nStartIndex, UINT _nPrimCount)
	{
		return pDevice->DrawIndexedPrimitive(_PrimitiveType, _nBaseVertexIndex, _nMinVertexIndex, _nNumVertices, _nStartIndex, _nPrimCount);
	}

	void C_DX9_DEVICEEX::SetRenderState(D3DRENDERSTATETYPE _State, DWORD _Value)
	{
		// 상태를 저장해놓고 같은 상태라면 다시 호출하지 않는다. 문제가 발생하는지는 확인해봐야함.
		if (dwRenderState[_State] != _Value)
		{
			pDevice->SetRenderState(_State, _Value);
			dwRenderState[_State] = _Value;
		}
	}
	void C_DX9_DEVICEEX::SetSamplerState(DWORD _Sampler, D3DSAMPLERSTATETYPE _Type, DWORD _Value)
	{
		if (dwSamplerState[_Type] != _Value)
		{
			pDevice->SetSamplerState(_Sampler, _Type, _Value);
			dwSamplerState[_Type] = _Value;
		}

	}
	void C_DX9_DEVICEEX::OnShowCursor(bool b)
	{
		pDevice->ShowCursor(b);
	}

	long C_DX9_DEVICEEX::GetStatus()
	{
		if (nullptr != pDevice)
		{
			// D3DERR_DEVICELOST = If the device was lost, do not render until we get it back
			// D3DERR_DEVICENOTRESET = Check if the device needs to be resized.
			return pDevice->TestCooperativeLevel();
		}
		return S_FALSE;
	}

	bool C_DX9_DEVICEEX::ResetDevice(dk::LPDSIZE _pSize)
	{
		//DBGPRINT("C_DX9_DEVICEEX::ResetDevice");
		if (nullptr != pDevice)
		{
			// 저장된 상태 제거
			ClearDX9States();

			// 마우스가 클릭 상태인지 확인한다.

			// 마우스가 UP 상태일때 아래를 실행한다.

			//ImGui_ImplDX9_InvalidateDeviceObjects();	// 버텍스, 인덱스, 폰트텍스쳐 제거
			OnLostDevice();

			if (nullptr != _pSize)
			{
				v2DisplaySize.Set((float)_pSize->cx, (float)_pSize->cy);
			}
			D3DPRESENT_PARAMETERS d3dpp = { 0 };
			InitPresentParameters(&d3dpp);

			HRESULT hr = pDevice->Reset(&d3dpp);
			if (hr == D3DERR_INVALIDCALL)
			{
				DBGPRINT(L"D3DERR_INVALIDCALL <-- 여기 오면 안됨");
			}
			else
			{
				OnResetDevice();
				//ImGui_ImplDX9_CreateDeviceObjects();	// 폰트텍스쳐 생성
				//Init(hWnd, &dkScreenSize);	// C_DEVICE

				nLastDeviceStatus = _DX9_DEVICE_OK;	// 디바이스가 사용 가능 상태
				return(true);
			}
		}
		return(false);
	}

	void C_DX9_DEVICEEX::OnLostDevice()
	{
		// 여기에서 폰트 텍스쳐 들도 리셋하자
		/*
		if (GetFont())
		{
			GetFont()->OnLostDevice();
		}
		*/
		//pLine->OnLostDevice();
	}

	void C_DX9_DEVICEEX::OnResetDevice()
	{
		// 여기에서 폰트 텍스쳐 등을 세팅하자.
		/*
		if (GetFont())
		GetFont()->OnResetDevice();
		*/
		//pLine->OnResetDevice();
	}

	void C_DX9_DEVICEEX::Begin2D()
	{
		DSAFE_RELEASE(pDX9StateBlock);

		pDevice->CreateStateBlock(D3DSBT_ALL, &pDX9StateBlock);	// DX9 의 상태를 백업
		// 뷰포트 백업

		pDevice->GetTransform(D3DTS_WORLD, &matWorld);
		pDevice->GetTransform(D3DTS_VIEW, &matView);
		pDevice->GetTransform(D3DTS_PROJECTION, &matProjection);

		// Orthogonal Projection
		dx9::DVIEWPORT9 vp(0, 0, (DWORD)v2DisplaySize.x, (DWORD)v2DisplaySize.y);
		pDevice->SetViewport(&vp);

		// 단일 뷰포트의 경우 x, y는 0이다.
		float fLeft = v2DisplayPos.x + 0.5f;
		float fRight = v2DisplayPos.x + v2DisplaySize.x + 0.5f;
		float fTop = v2DisplayPos.y + 0.5f;
		float fBottom = v2DisplayPos.y + v2DisplaySize.y + 0.5f;

		// <d3dx9.h> 또는 <DirectXMath.h>를 사용할 수 있는지 여부에 관계없이 다음의 함수에 의존하지 않음.
		// D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
		dx9::DMATRIX9 matIdentity(
			1.0f, 0.0f, 0.0f, 0.0f
			, 0.0f, 1.0f, 0.0f, 0.0f
			, 0.0f, 0.0f, 1.0f, 0.0f
			, 0.0f, 0.0f, 0.0f, 1.0f
		);
		dx9::DMATRIX9 matProj(
			2.0f / (fRight - fLeft), 0.0f, 0.0f, 0.0f
			, 0.0f, 2.0f / (fTop - fBottom), 0.0f, 0.0f
			, 0.0f, 0.0f, 0.5f, 0.0f
			, (fLeft + fRight) / (fLeft - fRight), (fTop + fBottom) / (fBottom - fTop), 0.5f, 1.0f
		);

		pDevice->SetTransform(D3DTS_WORLD, &matIdentity);
		pDevice->SetTransform(D3DTS_VIEW, &matIdentity);
		pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

		pDevice->SetPixelShader(NULL);
		pDevice->SetVertexShader(NULL);

		pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);		// 컬링 기능을 끈다. 삼각형의 앞면, 뒷면을 모두 렌더링한다.
		pDevice->SetRenderState(D3DRS_LIGHTING, false);				// 정점에 색깔값이 있으므로 광원기능을 끈다.
		pDevice->SetRenderState(D3DRS_ZENABLE, false);
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
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
	}

	void C_DX9_DEVICEEX::End2D()
	{
		// 뷰포트 복구
		pDevice->SetTransform(D3DTS_WORLD, &matWorld);
		pDevice->SetTransform(D3DTS_VIEW, &matView);
		pDevice->SetTransform(D3DTS_PROJECTION, &matProjection);

		// DX9 상태 복구
		if (nullptr != pDX9StateBlock)
		{
			pDX9StateBlock->Apply();
			pDX9StateBlock->Release();
			pDX9StateBlock = nullptr;
		}
	}

}
