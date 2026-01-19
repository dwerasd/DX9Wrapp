#include "framework.h"
#include "DX9Texture.h"



namespace dx9
{
	_DX9_TEXTURE::_DX9_TEXTURE(LPDIRECT3DDEVICE9 _pDevice)
		: pTexture(nullptr)
		, pDevice(_pDevice)
		, d3dFormat(D3DFMT_UNKNOWN)
		, dwUsage(0)
		, d3dPool(D3DPOOL_MANAGED)		// 매니저는 자동관리라서 디폴트로 한다
		, nMipLevels(1)
		, nWidth(0)
		, nHeight(0)
		, pFile(nullptr)
	{
		
	}
	//THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle
	_DX9_TEXTURE::_DX9_TEXTURE(LPDIRECT3DDEVICE9 _pDevice, UINT _nWidth, UINT _nHeight, D3DFORMAT _d3dFormat, DWORD _dwUsage)
		: pTexture(nullptr)
		, pDevice(_pDevice)
		, d3dFormat(_d3dFormat)
		, dwUsage(_dwUsage)
		, nMipLevels(1)
		, nWidth(_nWidth)
		, nHeight(_nHeight)
		, pFile(nullptr)
	{
		if (DIS_SET(dwUsage, D3DUSAGE_DYNAMIC)
			|| DIS_SET(dwUsage, D3DUSAGE_RENDERTARGET)
			)
		{
			//DBGPRINT("D3DPOOL_DEFAULT");
			d3dPool = D3DPOOL_DEFAULT;
		}
		else
		{
			//DBGPRINT("D3DPOOL_SYSTEMMEM");
			d3dPool = D3DPOOL_SYSTEMMEM;	// 빈 텍스쳐 생성이다.
		}
	}
	bool _DX9_TEXTURE::Create()
	{
		//UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle
		if (0 < nWidth)
		{
			DBGPRINT("_DX9_TEXTURE::Create() %i / %i / %i / %i", nWidth, nHeight, nMipLevels, d3dFormat);
			HRESULT hResult = pDevice->CreateTexture(
				nWidth
				, nHeight
				, nMipLevels
				, dwUsage
				, d3dFormat
				, d3dPool
				, &pTexture
				, 0
			);
			if (SUCCEEDED(hResult))
			{
				return(true);
			}
		}
		return(false);
	}
	bool _DX9_TEXTURE::LoadTexture(LPCSTR _tszPath)
	{
		std::string strPath = dk::GetCurrentDirectoryA();
		strPath += +_tszPath;
		
		if (nullptr == pFile)
		{
			pFile = new dk::C_FILE(strPath.c_str());
		}
		else
		{
			pFile->Init(strPath.c_str());
		}
		D3DXIMAGE_INFO d3dImageInfo;
		HRESULT hResult = D3DXGetImageInfoFromFileInMemory(pFile->GetFileData(), pFile->GetReadSize(), &d3dImageInfo);
		if (SUCCEEDED(hResult))
		{
			DBGPRINT("image size -> Width: %i / Height %i (pFile->GetReadSize(): %i)", d3dImageInfo.Width, d3dImageInfo.Height, pFile->GetReadSize());
			//rcSource.left = rcSource.top = 0;
			//rcSource.right = d3dImageInfo.Width;
			//rcSource.bottom = d3dImageInfo.Height;
			hResult = D3DXCreateTextureFromFileInMemoryEx(
				pDevice
				, pFile->GetFileData()
				, pFile->GetReadSize()
				, d3dImageInfo.Width
				, d3dImageInfo.Height
				, nMipLevels
				, D3DUSAGE_AUTOGENMIPMAP
				, D3DFMT_UNKNOWN
				, D3DPOOL_MANAGED
				//, D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR
				//, D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR
				, D3DX_DEFAULT
				, D3DX_DEFAULT
				, D3DCOLOR_ARGB(255, 0, 0, 0)
				, &d3dImageInfo
				, NULL
				, &pTexture
			);
		}
		/* 이걸 쓰면 이미지가 2의 제곱이어야만 한다.
		D3DXCreateTextureFromFileExA(
			pDevice
			, _tszPath
			, 0
			, 0
			, 0
			, 0
			, D3DFMT_UNKNOWN
			, D3DPOOL_MANAGED
			, D3DX_DEFAULT
			, D3DX_DEFAULT
			, 0
			, &d3dImageInfo
			, nullptr
			, &pTexture
		);
		*/
		if (nullptr != pTexture)
		{
			D3DSURFACE_DESC descTexture;
			pTexture->GetLevelDesc(0, &descTexture);

			d3dFormat = descTexture.Format;
			dwUsage = descTexture.Usage;
			d3dPool = descTexture.Pool;
			nWidth = descTexture.Width;
			nHeight = descTexture.Height;
			nMipLevels = d3dImageInfo.MipLevels;

			return(true);
		}
		else
		{
			DSAFE_DELETE(pFile);	// 실패할 경우에만 메모리 해제를 한다.

			std::wstringstream sStream;
			sStream << L"Failed to load texture: " << _tszPath;
			MessageBoxW(0, sStream.str().c_str(), L"_DX9_TEXTURE::LoadTexture( ... )", 0);
		}
		return(false);
	}

	bool _DX9_TEXTURE::LoadTexture(LPCWSTR _tszPath)
	{
		std::wstring wstrPath = dk::GetCurrentDirectoryW();
		if (nullptr != _tszPath)	// 파일경로가 파라미터로 들어왔을 경우에만
		{
			wstrPath += +_tszPath;
			DBGPRINT(L"_DX9_TEXTURE::LoadTexture(%s)", wstrPath.c_str());
			/*
			if (dk::FileExists(wstrPath.c_str()))
			{
				DBGPRINT("파일 있음");
			}
			*/
			if (nullptr == pFile)
			{
				pFile = new dk::C_FILE(wstrPath.c_str());
			}
			else
			{
				pFile->Init(wstrPath.c_str());
			}
		}
		if (nullptr != pFile)
		{
			D3DXIMAGE_INFO d3dImageInfo;
			HRESULT hResult = D3DXGetImageInfoFromFileInMemory(pFile->GetFileData(), pFile->GetReadSize(), &d3dImageInfo);
			if (SUCCEEDED(hResult))
			{
				//DBGPRINT("image size -> Width: %i / Height %i (pFile->GetReadSize(): %i)", d3dImageInfo.Width, d3dImageInfo.Height, pFile->GetReadSize());
				//rcSource.left = rcSource.top = 0;
				//rcSource.right = d3dImageInfo.Width;
				//rcSource.bottom = d3dImageInfo.Height;
				hResult = D3DXCreateTextureFromFileInMemoryEx(
					pDevice
					, pFile->GetFileData()
					, pFile->GetReadSize()
					, d3dImageInfo.Width
					, d3dImageInfo.Height
					, nMipLevels
					, D3DUSAGE_AUTOGENMIPMAP
					, D3DFMT_UNKNOWN
					, D3DPOOL_MANAGED
					//, D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR
					//, D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR
					, D3DX_DEFAULT
					, D3DX_DEFAULT
					, D3DCOLOR_ARGB(255, 0, 0, 0)
					, &d3dImageInfo
					, NULL
					, &pTexture
				);
			}
			switch (hResult)
			{
			case D3DERR_NOTAVAILABLE:
				DBGPRINT("D3DERR_NOTAVAILABLE");
				break;
			case D3DERR_OUTOFVIDEOMEMORY:
				DBGPRINT("D3DERR_OUTOFVIDEOMEMORY");
				break;
			case D3DERR_INVALIDCALL:
				DBGPRINT("D3DERR_INVALIDCALL");
				break;
			case D3DXERR_INVALIDDATA:
				DBGPRINT("D3DXERR_INVALIDDATA");
				break;
			case E_OUTOFMEMORY:
				DBGPRINT("E_OUTOFMEMORY");
				break;
			default:
				//DBGPRINT("읽기 성공 %i", hResult);
				break;
			}
			if (nullptr != pTexture)
			{
				D3DSURFACE_DESC descTexture;
				pTexture->GetLevelDesc(0, &descTexture);

				d3dFormat = descTexture.Format;
				dwUsage = descTexture.Usage;
				d3dPool = descTexture.Pool;
				nWidth = descTexture.Width;
				nHeight = descTexture.Height;
				return(true);
			}
			else
			{
				DBGPRINT("pTexture: %x", pTexture);
				std::wstringstream sStream;
				sStream << L"Failed to load texture: " << wstrPath.c_str();
				MessageBoxW(0, sStream.str().c_str(), L"_DX9_TEXTURE::LoadTextureEx( ... )", 0);
			}
		}
		return(false);
	}

	void _DX9_TEXTURE::Release()
	{
		DSAFE_RELEASE(pTexture);
	}
	void _DX9_TEXTURE::OnLostDevice()
	{
		if (D3DPOOL_DEFAULT == d3dPool)	// D3DPOOL_DEFAULT 는 리셋시 디바이스가 해제되기 때문에 제거하자
		{
			// Surfaces 가 있으면 전부 해제하고
			/*
			if (nullptr != rtSurfaces)
			{
				for (int i = 0;i < nRenderTargetSurfaces;++i)
				{
					DSAFE_RELEASE(rtSurfaces[i]);
				}
				SAFE_DELETE(rtSurfaces);
			}
			*/
			//DBGPRINT("텍스쳐 해제한다");
			DSAFE_RELEASE(pTexture);
		}
	}

	void _DX9_TEXTURE::OnResetDevice()
	{
		if (D3DPOOL_DEFAULT == d3dPool)	// D3DPOOL_DEFAULT 는 해제되었을테니 복구해야한다
		{
			if (nullptr == pTexture)
			{
				// 파일에서 읽은 경우
				if (nullptr != pFile)
				{
					if (NO_ERROR == LoadTexture())
					{

					}
				}
				else if (0 < nWidth)
				{
					// 빈 텍스쳐를 복구해야하는 경우

					// 렌더타켓이나 다이나믹은 default 로 생성
					/*
					if (DIS_SET(dwUsage, D3DUSAGE_RENDERTARGET) || DIS_SET(dwUsage, D3DUSAGE_DYNAMIC))
					{
						// RenderTarget과 Dynamic은 동시에 지정 불가능
						if ()
					}
					*/
					// 다 저장되어 있으니 그냥 호출하면 될거 같다.
					//DBGPRINT("텍스쳐 생성한다");
					pDevice->CreateTexture(
						nWidth
						, nHeight
						, nMipLevels
						, dwUsage
						, d3dFormat
						, d3dPool
						, &pTexture
						, 0
					);
					//DBGPRINT("텍스쳐 생성했다");
				}
			}
		}
	}

	bool _DX9_TEXTURE::IsDynamic()
	{
		return DIS_SET(dwUsage, D3DUSAGE_DYNAMIC);
	}
}