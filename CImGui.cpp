#include "framework.h"
#include "CImGui.h"



extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
C_IMGUI::C_IMGUI(bool _bVerticalSync)
	: bVerticalSync(_bVerticalSync)
	  , dkClearColor(114, 140, 153)
{

}

C_IMGUI::~C_IMGUI()
{

}

void C_IMGUI::Init_ImGui(HWND _hWnd, LPDIRECT3DDEVICE9 _pDevice, bool _bVerticalSync)
{
	hWnd = _hWnd;
	this->bVerticalSync = _bVerticalSync;
	if (!_pDevice)
	{
		const LPDIRECT3D9 g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
		if (g_pD3D)
		{
			// Create the D3DDevice
			ZeroMemory(&this->g_d3dpp, sizeof(this->g_d3dpp));
			this->g_d3dpp.Windowed = TRUE;
			this->g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			this->g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
			this->g_d3dpp.EnableAutoDepthStencil = TRUE;
			this->g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
			this->g_d3dpp.PresentationInterval = this->bVerticalSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;           // Present with vsync
			//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
			if (0 < g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &this->g_d3dpp, &this->pDevice))
			{
				DBGPRINT("[ImGui] C_IMGUI::C_IMGUI() - CreateDevice 실패");
			}
		}
	}
	else { pDevice = _pDevice; }

	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGui::StyleColorsDark();

	// Docking / Multi-Viewport ConfigFlags 는 호출자(앱) 가 결정하도록 위임.
	// 라이브러리에서 강제 ON 하면 multi-viewport 비용을 모든 의존 프로젝트가
	// 떠안게 되므로 (DX9 secondary swapchain present 등 상시 비용),
	// 앱이 INI/메뉴 토글로 제어하게 둔다.

	ImGui_ImplWin32_Init(this->hWnd);
	ImGui_ImplDX9_Init(this->pDevice);

	// 기본 폰트
	const ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("Font/AppleSDGothicNeoB.ttf", 16.0f, 0, io.Fonts->GetGlyphRangesKorean());
	io.Fonts->AddFontFromFileTTF("Fonts/DungGeunMo.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesKorean());
	//io.Fonts->AddFontFromFileTTF("Fonts/gulim.ttc", 18.0f, 0, io.Fonts->GetGlyphRangesKorean());

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowMenuButtonPosition = ImGuiDir_Right;
}

long C_IMGUI::Update_ImGui()
{
	long nResult = 0;
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	do
	{
		// 메뉴 윈도우의 위치 지정
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(this->v2MainWindowSize);
		// 메뉴 윈도우 옵션
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus;
		flags |= ImGuiWindowFlags_MenuBar;
		// 메뉴 윈도우 생성
		if (ImGui::Begin("MainMenu", nullptr, flags))
		{
			// 메뉴 그리기
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu(toUtf8("파일")))
				{	// 첫번째 메뉴
					if (ImGui::MenuItem(toUtf8("최대 퍼포먼스")))
					{

					}
					if (ImGui::MenuItem(toUtf8("종료")))
					{
						nResult = -1;
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(toUtf8("실행")))
				{	// 두번째 메뉴
					if (ImGui::MenuItem(toUtf8("크레온API")))
					{
						// 서버로 크레온 실행하라고 전송
					}
					if (ImGui::MenuItem(toUtf8("이베스트API")))
					{
						// 서버로 이베스트 실행하라고 전송
					}
					if (ImGui::MenuItem(toUtf8("키움API")))
					{
						// 서버로 키움 실행하라고 전송
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(toUtf8("설정")))
				{	// 두번째 메뉴
					if (ImGui::MenuItem(toUtf8("정보")))
					{

					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(toUtf8("도움말")))
				{	// 두번째 메뉴
					if (ImGui::MenuItem(toUtf8("정보")))
					{

					}
					ImGui::EndMenu();
				}
				ImGui::Text("[ImGui] FRAME (%.1f FPS)", ImGui::GetIO().Framerate);
				ImGui::EndMenuBar();
			}
			ImGui::End();
		}
		//DBGPRINT("메뉴그린다");
		//nResult = DrawMainMenu();
		//if (nResult) { break; }
		//DBGPRINT("종목정보그린다");
		//DrawStockInfo();
		//DBGPRINT("찾은종목그린다");
		//DrawStockFound();

	} while (false);
	ImGui::EndFrame();
	return(nResult);
}

void C_IMGUI::Draw_ImGui()
{
	// Clear();
	if (!bResetDevice)
	{
		this->pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		this->pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		this->pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		HRESULT hResult = this->pDevice->Clear(
			0
			, nullptr
			, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
			, this->dkClearColor.dwColor
			, 1.0f
			, 0
		);
		//if (FAILED(hResult))
		//{
		//	DBGPRINT("[ImGui] C_DX9_DEVICE::Clear() - 실패(%d / %d)", hResult, ::GetLastError());
		//}
		if (D3D_OK == this->pDevice->BeginScene())
		{
			//
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			//
			this->pDevice->EndScene();
		}
		// Multi-Viewport — secondary OS 창들 렌더 + Present (메인 viewport Present 전후 무관, 가이드는 메인 Present 전 권장).
		// ViewportsEnable 안 켜진 경우엔 no-op 에 가깝지만 분기로 명시.
		{
			const ImGuiIO& ioRender = ImGui::GetIO();
			if (ioRender.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
		}
		hResult = this->pDevice->Present(nullptr, nullptr, 0, nullptr);
		if (0 > hResult)	// Present 가 실패했을 경우
		{
			if (D3DERR_DEVICELOST == hResult && D3DERR_DEVICENOTRESET == this->pDevice->TestCooperativeLevel())
			{
				this->g_d3dpp.BackBufferWidth = (DWORD)this->v2MainWindowSize.x;
				this->g_d3dpp.BackBufferHeight = (DWORD)this->v2MainWindowSize.y;
				ResetDevice();
			}
		}
	}
	//if (!bVerticalSync)
	//{	// 기본은 false 상태지만 빠른 연산이 필요할때는 풀고 연산을 진행한다.
	//	dk::Sleep(1);	// VerticalSync 가 false 인 상태에서 Sleep 을 호출하게 되면 FPS 가 64정도로 줄어들게 된다.
	//}
}

void C_IMGUI::Destroy_ImGui()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

void C_IMGUI::Clear(DWORD _dwColor, DWORD _dwFlags, float _fZ, DWORD _dwStencil, DWORD _dwIndex)
{
	this->pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	this->pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	this->pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	const HRESULT hResult = this->pDevice->Clear(
		_dwIndex
		, nullptr
		, _dwFlags
		, _dwColor ? _dwColor : dkClearColor.dwColor
		, _fZ
		, _dwStencil
	);
	if (FAILED(hResult))
	{
		//DBGPRINT("[ImGui] C_DX9_DEVICE::Clear() - 실패(%d / %d)", hResult, ::GetLastError());
	}
}

HRESULT C_IMGUI::BeginScene()
{
	return pDevice->BeginScene();
}

void C_IMGUI::EndScene()
{
	pDevice->EndScene();
}
void C_IMGUI::Present(HWND hDestWindowOverride, LPRECT pDst, LPRECT pSrc, RGNDATA* pDirtyRegion)
{
	const HRESULT hResult = pDevice->Present(
		pSrc
		//, pDst ? pDst : &rectRender
		, pDst
		, hDestWindowOverride
		, pDirtyRegion
	);
	if (0 > hResult)	// Present 가 실패했을 경우
	{
		if (D3DERR_DEVICELOST == hResult && D3DERR_DEVICENOTRESET == pDevice->TestCooperativeLevel())
		{
			this->g_d3dpp.BackBufferWidth = (DWORD)this->v2MainWindowSize.x;
			this->g_d3dpp.BackBufferHeight = (DWORD)this->v2MainWindowSize.y;
			ResetDevice();
		}
	}
	//if (!bVerticalSync)
	//{	// 기본은 false 상태지만 빠른 연산이 필요할때는 풀고 연산을 진행한다.
	//	dk::Sleep(1);	// VerticalSync 가 false 인 상태에서 Sleep 을 호출하게 되면 FPS 가 64정도로 줄어들게 된다.
	//}
}

void C_IMGUI::ResetDevice()
{
	this->bResetDevice = true;
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const HRESULT hResult = this->pDevice->Reset(&this->g_d3dpp);
	if (hResult == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
	this->bResetDevice = false;
}

void C_IMGUI::SetWindowSize(UINT _x, UINT _y)
{
	this->g_d3dpp.BackBufferWidth = _x;
	this->g_d3dpp.BackBufferHeight = _y;
	this->v2MainWindowSize.x = (float)_x;
	this->v2MainWindowSize.y = (float)_y;
}

bool C_IMGUI::WndProc_ImGui(HWND _hWnd, UINT _nMessage, WPARAM _wParam, LPARAM _lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(_hWnd, _nMessage, _wParam, _lParam))
	{
		return(true);
	}
	return(false);
}

// ============================================================================
// toUtf8: narrow / wide 문자열을 UTF-8 char* 로 변환 (캐시 wrapper).
// ============================================================================
// 캐시 키는 *내용 기반* (std::string / std::wstring) 만 사용한다.
//
// 과거 wide 버전에 포인터 기반 (const wchar_t*) 캐시가 있었으나 다음 trap 들로
// 제거됨:
//   1) stale 데이터: 같은 stack buffer (예: wchar_t wszBuf_[N] + swprintf) 에
//      매 프레임 다른 내용이 쓰이면 포인터 hit 으로 첫 프레임 결과가 영구 반환.
//   2) dangling pointer: 임시 std::wstring 의 c_str() 등 곧 해제될 주소가 키로
//      등록되면 캐시는 invalid pointer 를 영구 보유. 동일 주소 재할당 시
//      잘못된 hit + 메모리 오염.
//
// 비용: GUI 한글 라벨은 보통 16 wchar 이내라 std::wstring SBO 안에 들어가
// heap alloc 없이 키 생성 가능. 포인터 O(1) 대비 손실은 측정 불가 수준.
//
// thread_local: 메인 스레드 전용 가정 + 멀티스레드 안전.
// ============================================================================

LPCSTR toUtf8(LPCSTR _pszData)
{
	static thread_local std::unordered_map<std::string, std::string> g_umapNarrow;
	if (_pszData == nullptr || *_pszData == '\0') { return ""; }

	const std::string strKey_(_pszData);
	const std::unordered_map<std::string, std::string>::iterator it_ = g_umapNarrow.find(strKey_);
	if (it_ != g_umapNarrow.end()) { return it_->second.c_str(); }

	return g_umapNarrow.emplace(strKey_, dk::AnsiToUtf8(_pszData)).first->second.c_str();
}

LPCSTR toUtf8(LPCWSTR _pwszData)
{
	static thread_local std::unordered_map<std::wstring, std::string> g_umapWide;
	if (_pwszData == nullptr || *_pwszData == L'\0') { return ""; }

	const std::wstring strKey_(_pwszData);
	const std::unordered_map<std::wstring, std::string>::iterator it_ = g_umapWide.find(strKey_);
	if (it_ != g_umapWide.end()) { return it_->second.c_str(); }

	return g_umapWide.emplace(strKey_, dk::Utf16ToUtf8(_pwszData)).first->second.c_str();
}

bool bInit = false;
typedef std::unordered_map<std::string, ImVec4> UMAP_IMGUI_COLORS;
UMAP_IMGUI_COLORS umapImGuiColors;
void ColorInit()
{
	/*
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("App background", { 0x73, 0x8c, 0x99, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Text title", { 0x55, 0xff, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Text dim", { 0xaa, 0xaa, 0xaa, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Text highlight", { 0xff, 0xa2, 0x57, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Text okay", { 0x55, 0xff, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Text warning", { 0xff, 0xff, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Text error", { 0xff, 0x55, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix invalid", { 0xd9, 0x00, 0x00, 0xaa }));				// Plot stats min/max 거의 똑같은 빨간색.
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix masked", { 0x00, 0xaa, 0xff, 0xaa }));				// 하늘색보다 조금 짙음.
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix DR-only", { 0xb0, 0x00, 0xb0, 0xaa }));				// 자홍색
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix 2D", { 0x00, 0xaa, 0xaa, 0xaa }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix 3D", { 0x00, 0xd9, 0x00, 0xaa }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix 3D+DR", { 0xd9, 0xd9, 0x00, 0xaa }));					// 노랑
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix time", { 0xd0, 0xd0, 0xd0, 0xaa }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix RTK float", { 0xff, 0xff, 0x00, 0xaa }));				// 노랑
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix RTK fixed", { 0xff, 0xa0, 0x00, 0xaa }));				// 노랑
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix RTK float + DR", { 0x9e, 0x9e, 0x00, 0xaa }));		// 노랑
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Fix RTK fixed + DR", { 0x9e, 0x64, 0x00, 0xaa }));		// 노랑
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot grid major", { 0xaa, 0xaa, 0xaa, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot grid minor", { 0x55, 0x55, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot grid label", { 0xff, 0xff, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot stats min/max", { 0xaa, 0x00, 0x00, 0xff }));		// 거의 완전 빨간색.
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot stats mean", { 0xaa, 0x00, 0xaa, 0xff }));			// 자홍색
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot error ellipse", { 0xaa, 0x00, 0x00, 0xaf }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot highlight masked", { 0x55, 0xff, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot highlight OK fix", { 0xff, 0x55, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot crosshairs", { 0xff, 0x55, 0xff, 0xaf }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot crosshairs label", { 0xff, 0x55, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Plot histogram", { 0xb2, 0x81, 0x5e, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Map highlight masked", { 0x55, 0xff, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Map highlight OK fix", { 0xff, 0x55, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Map accuracy estimate", { 0x40, 0x40, 0x40, 0x55 }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Map baseline", { 0xff, 0x42, 0xf2, 0xcc })); // hsla(304 100% 63% 0.8 }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Map crosshairs", { 0xff, 0x44, 0x44, 0xaf }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Map layout debug", { 0xff, 0x00, 0x00, 0xaf }));			// 빨간색
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Map zoom rect", { 0xdd, 0xe7, 0x08, 0xa0 }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Sky view satellite", { 0x55, 0x86, 0xff, 0xa3 }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal used text", { 0x55, 0xff, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal unused text", { 0xaa, 0xaa, 0xaa, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal used", { 0x00, 0xaa, 0x00, 0xaf }));				// 밝은 녹색
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal unused", { 0xaa, 0xaa, 0xaa, 0xaf }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 0 - 5 dBHz", { 0xe0, 0x52, 0x52, 0xdf })); // hsl(  0 70% 60% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 5 - 10 dBHz", { 0xe0, 0x69, 0x52, 0xdf })); // hsl( 10 70% 60% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 10 - 15 dBHz", { 0xe0, 0x81, 0x52, 0xdf })); // hsl( 20 70% 60% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 15 - 20 dBHz", { 0xe0, 0x99, 0x52, 0xdf })); // hsl( 30 70% 60% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 20 - 25 dBHz", { 0xe0, 0xb1, 0x52, 0xdf })); // hsl( 40 70% 60% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 25 - 30 dBHz", { 0xe0, 0xc9, 0x52, 0xdf })); // hsl( 50 70% 60% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 30 - 35 dBHz", { 0xe0, 0xe0, 0x52, 0xdf })); // hsl( 60 70% 60% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 35 - 40 dBHz", { 0xb3, 0xe0, 0x85, 0xdf })); // hsl( 90 60% 70% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 40 - 45 dBHz", { 0x06, 0xf9, 0x06, 0xdf })); // hsl(120 95% 50% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 45 - 50 dBHz", { 0x13, 0xec, 0x5b, 0xdf })); // hsl(140 85% 50% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 50 - 55 dBHz", { 0x13, 0xec, 0xa4, 0xdf })); // hsl(160 85% 50% }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Signal 55 - oo dBHz", { 0x13, 0xec, 0xec, 0xdf })); // hsl(180 85% 50% }));
	// Generic colours (some from https://github.com/leiradel/ImGuiAl/blob/master/term/imguial_term.h }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour Black", { 0x00, 0x00, 0x00, 0xff }));				// 검정
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour Grey", { 0x55, 0x55, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour White", { 0xaa, 0xaa, 0xaa, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour BrightWhite", { 0xff, 0xff, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour Blue", { 0x00, 0x00, 0xaa, 0xff }));				// 진한 파랑
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour BrightBlue", { 0x55, 0x55, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour Green", { 0x00, 0xaa, 0x00, 0xff }));				// 밝은 녹색
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour BrightGreen", { 0x55, 0xff, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour Cyan", { 0x00, 0xaa, 0xaa, 0xff }));				// 하늘색보다 조금 짙음.
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour BrightCyan", { 0x55, 0xff, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour Red", { 0xaa, 0x00, 0x00, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour BrightRed", { 0xff, 0x55, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour Magenta", { 0xaa, 0x00, 0xaa, 0xff }));			// 자홍색
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour BrightMagenta", { 0xff, 0x55, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour Brown", { 0xaa, 0x55, 0x00, 0xff }));				// 갈색이어야하는데, 텍스트에서는 노란색 뜸.
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour Yellow", { 0xff, 0xff, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour Orange", { 0xa8, 0x65, 0x00, 0xff }));				// 오렌지색이 안뜨고 텍스트에서는 노란색 뜸.
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour BrightOrange", { 0xff, 0xa1, 0x14, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Colour None", { 0x00, 0x00, 0x00, 0x00 }));				// 이러면 안뜸.
	// Log window
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Log UBX messages", { 0xff, 0x80, 0x80, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Log NMEA messages", { 0xab, 0xff, 0x80, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Log RTCM3 messages", { 0xaf, 0x80, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Log NOVATEL messages", { 0x41, 0xce, 0x73, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Log GARBAGE messages", { 0x80, 0xce, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Log Epochs", { 0xff, 0xfe, 0x80, 0xff }));
	// Inf window
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Inf Debug", { 0x00, 0xaa, 0xaa, 0xff }));					// 하늘색보다 조금 짙음.
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Inf Notice", { 0xff, 0xff, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Inf Warning", { 0xff, 0xff, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Inf Error", { 0xff, 0x55, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Inf Test", { 0xff, 0x55, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Inf Other", { 0xaa, 0x55, 0x00, 0xff }));					// 텍스트에서는 노란색 뜸.
	// Debug logs
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Debug Trace", { 0xaa, 0x00, 0xaa, 0xff }));				// 텍스트 자홍색
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Debug Debug", { 0x00, 0xaa, 0xaa, 0xff }));				// 텍스트 하늘색
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Debug Print", { 0xaa, 0xaa, 0xaa, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Debug Notice", { 0xff, 0xff, 0xff, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Debug Warning", { 0xff, 0xff, 0x55, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("Debug Error", { 0xff, 0x55, 0x55, 0xff }));
	*/
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("색없음", { 0x00, 0x00, 0x00, 0x00 }));					// 이러면 안뜸.
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("검정색", { 0x00, 0x00, 0x00, 0xff }));				// 검정
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("분홍색", { 0xaa, 0x00, 0xaa, 0xff }));				// 텍스트 자홍색
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("하늘색", { 0x00, 0xaa, 0xaa, 0xff }));				// 텍스트 하늘색
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("노란색", { 0xaa, 0x55, 0x00, 0xff }));
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("파란색", { 0x00, 0x00, 0xaa, 0xff }));				// 진한 파랑
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("빨간색", { 0xff, 0x00, 0x00, 0xaf }));				// 빨간
	umapImGuiColors.insert(UMAP_IMGUI_COLORS::value_type("초록색", { 0x00, 0xaa, 0x00, 0xff }));
}

LPCSTR GetColorName(size_t _nIndex)
{
	if (!bInit)
	{
		ColorInit();
		bInit = true;
	}
	if (umapImGuiColors.size() > _nIndex)
	{
		size_t nIndex = 0;
		for (UMAP_IMGUI_COLORS::iterator itr = umapImGuiColors.begin(); umapImGuiColors.end() != itr; ++itr)
		{
			if (nIndex++ == _nIndex)
			{
				return itr->first.c_str();
			}
		}
	}
	return(umapImGuiColors.begin()->first.c_str());
}

ImVec4 GetColorVec4OfText(LPCSTR _pColorName)
{
	if (!bInit)
	{
		ColorInit();
		bInit = true;
	}
	if (_pColorName)
	{
		const UMAP_IMGUI_COLORS::iterator itr = umapImGuiColors.find(_pColorName);
		if (umapImGuiColors.end() != itr)
		{
			return(itr->second);
		}
	}
	return{ 0.0f, 0.0f, 0.0f, 0.0f };
}

ImU32 GetColorOfText(LPCSTR _pColorName)
{
	if (!bInit)
	{
		ColorInit();
		bInit = true;
	}
	const UMAP_IMGUI_COLORS::iterator itr = umapImGuiColors.find(_pColorName);
	if (umapImGuiColors.end() != itr)
	{
		return(IM_COL32(itr->second.x, itr->second.y, itr->second.z, itr->second.w));
	}
	return(IM_COL32(0x00, 0x00, 0x00, 0x00));
}

#define _IMGUI_COLOR_LOG_MSGUBX_            IM_COL32(0xff, 0x80, 0x80, 0xff)
#define _IMGUI_COLOR_LOG_MSGUBX             IM_COL32(0xff, 0x80, 0x80, 0xff)
#define _IMGUI_COLOR_LOG_MSGNMEA            IM_COL32(0xab, 0xff, 0x80, 0xff)
#define _IMGUI_COLOR_LOG_MSGRTCM3           IM_COL32(0xaf, 0x80, 0xff, 0xff)
#define _IMGUI_COLOR_LOG_MSGNOVATEL         IM_COL32(0x41, 0xce, 0x73, 0xff)
#define _IMGUI_COLOR_LOG_MSGGARBAGE         IM_COL32(0x80, 0xce, 0xff, 0xff)
#define _IMGUI_COLOR_LOG_EPOCH              IM_COL32(0xff, 0xfe, 0x80, 0xff)



/*
#define GUI_SETTINGS_COLOURS(_P_) \
	_P_("App background",        APP_BACKGROUND,               IM_COL32(0x73, 0x8c, 0x99, 0xff)) \
	_P_("Text title",            TEXT_TITLE,                   IM_COL32(0x55, 0xff, 0xff, 0xff)) \
	_P_("Text dim",              TEXT_DIM,                     IM_COL32(0xaa, 0xaa, 0xaa, 0xff)) \
	_P_("Text highlight",        TEXT_HIGHLIGHT,               IM_COL32(0xff, 0xa2, 0x57, 0xff)) \
	_P_("Text okay",             TEXT_OK,                      IM_COL32(0x55, 0xff, 0x55, 0xff)) \
	_P_("Text warning",          TEXT_WARNING,                 IM_COL32(0xff, 0xff, 0x55, 0xff)) \
	_P_("Text error",            TEXT_ERROR,                   IM_COL32(0xff, 0x55, 0x55, 0xff)) \
	_P_("Fix invalid",           FIX_INVALID,                  IM_COL32(0xd9, 0x00, 0x00, 0xaa)) \
	_P_("Fix masked",            FIX_MASKED,                   IM_COL32(0x00, 0xaa, 0xff, 0xaa)) \
	_P_("Fix DR-only",           FIX_DRONLY,                   IM_COL32(0xb0, 0x00, 0xb0, 0xaa)) \
	_P_("Fix 2D",                FIX_S2D,                      IM_COL32(0x00, 0xaa, 0xaa, 0xaa)) \
	_P_("Fix 3D",                FIX_S3D,                      IM_COL32(0x00, 0xd9, 0x00, 0xaa)) \
	_P_("Fix 3D+DR",             FIX_S3D_DR,                   IM_COL32(0xd9, 0xd9, 0x00, 0xaa)) \
	_P_("Fix time",              FIX_TIME,                     IM_COL32(0xd0, 0xd0, 0xd0, 0xaa)) \
	_P_("Fix RTK float",         FIX_RTK_FLOAT,                IM_COL32(0xff, 0xff, 0x00, 0xaa)) \
	_P_("Fix RTK fixed",         FIX_RTK_FIXED,                IM_COL32(0xff, 0xa0, 0x00, 0xaa)) \
	_P_("Fix RTK float + DR",    FIX_RTK_FLOAT_DR,             IM_COL32(0x9e, 0x9e, 0x00, 0xaa)) \
	_P_("Fix RTK fixed + DR",    FIX_RTK_FIXED_DR,             IM_COL32(0x9e, 0x64, 0x00, 0xaa)) \
	_P_("Plot grid major",       PLOT_GRID_MAJOR,              IM_COL32(0xaa, 0xaa, 0xaa, 0xff)) \
	_P_("Plot grid minor",       PLOT_GRID_MINOR,              IM_COL32(0x55, 0x55, 0x55, 0xff)) \
	_P_("Plot grid label",       PLOT_GRID_LABEL,              IM_COL32(0xff, 0xff, 0xff, 0xff)) \
	_P_("Plot stats min/max",    PLOT_STATS_MINMAX,            IM_COL32(0xaa, 0x00, 0x00, 0xff)) \
	_P_("Plot stats mean",       PLOT_STATS_MEAN,              IM_COL32(0xaa, 0x00, 0xaa, 0xff)) \
	_P_("Plot error ellipse",    PLOT_ERR_ELL,                 IM_COL32(0xaa, 0x00, 0x00, 0xaf)) \
	_P_("Plot highlight masked", PLOT_FIX_HL_MASKED,           IM_COL32(0x55, 0xff, 0xff, 0xff)) \
	_P_("Plot highlight OK fix", PLOT_FIX_HL_OK,               IM_COL32(0xff, 0x55, 0x55, 0xff)) \
	_P_("Plot crosshairs",       PLOT_FIX_CROSSHAIRS,          IM_COL32(0xff, 0x55, 0xff, 0xaf)) \
	_P_("Plot crosshairs label", PLOT_FIX_CROSSHAIRS_LABEL,    IM_COL32(0xff, 0x55, 0xff, 0xff)) \
	_P_("Plot histogram",        PLOT_HISTOGRAM,               IM_COL32(0xb2, 0x81, 0x5e, 0xff)) \
	_P_("Map highlight masked",  PLOT_MAP_HL_MASKED,           IM_COL32(0x55, 0xff, 0xff, 0xff)) \
	_P_("Map highlight OK fix",  PLOT_MAP_HL_OK,               IM_COL32(0xff, 0x55, 0x55, 0xff)) \
	_P_("Map accuracy estimate", PLOT_MAP_ACC_EST,             IM_COL32(0x40, 0x40, 0x40, 0x55)) \
	_P_("Map baseline",          PLOT_MAP_BASELINE,            IM_COL32(0xff, 0x42, 0xf2, 0xcc)) \ // hsla(304, 100%, 63%, 0.8)
	_P_("Map crosshairs",        MAP_CROSSHAIRS,               IM_COL32(0xff, 0x44, 0x44, 0xaf)) \
	_P_("Map layout debug",      MAP_DEBUG,                    IM_COL32(0xff, 0x00, 0x00, 0xaf)) \
	_P_("Map zoom rect",         MAP_ZOOM_RECT,                IM_COL32(0xdd, 0xe7, 0x08, 0xa0)) \
	_P_("Sky view satellite",    SKY_VIEW_SAT,                 IM_COL32(0x55, 0x86, 0xff, 0xa3)) \
	_P_("Signal used (text)",    SIGNAL_USED_TEXT,             IM_COL32(0x55, 0xff, 0x55, 0xff)) \
	_P_("Signal unused (text)",  SIGNAL_UNUSED_TEXT,           IM_COL32(0xaa, 0xaa, 0xaa, 0xff)) \
	_P_("Signal used",           SIGNAL_USED,                  IM_COL32(0x00, 0xaa, 0x00, 0xaf)) \
	_P_("Signal unused",         SIGNAL_UNUSED,                IM_COL32(0xaa, 0xaa, 0xaa, 0xaf)) \
	_P_("Signal 0 - 5 dBHz",     SIGNAL_00_05,                 IM_COL32(0xe0, 0x52, 0x52, 0xdf)) \ // hsl(  0, 70%, 60%)
	_P_("Signal 5 - 10 dBHz",    SIGNAL_05_10,                 IM_COL32(0xe0, 0x69, 0x52, 0xdf)) \ // hsl( 10, 70%, 60%)
	_P_("Signal 10 - 15 dBHz",   SIGNAL_10_15,                 IM_COL32(0xe0, 0x81, 0x52, 0xdf)) \ // hsl( 20, 70%, 60%)
	_P_("Signal 15 - 20 dBHz",   SIGNAL_15_20,                 IM_COL32(0xe0, 0x99, 0x52, 0xdf)) \ // hsl( 30, 70%, 60%)
	_P_("Signal 20 - 25 dBHz",   SIGNAL_20_25,                 IM_COL32(0xe0, 0xb1, 0x52, 0xdf)) \ // hsl( 40, 70%, 60%)
	_P_("Signal 25 - 30 dBHz",   SIGNAL_25_30,                 IM_COL32(0xe0, 0xc9, 0x52, 0xdf)) \ // hsl( 50, 70%, 60%)
	_P_("Signal 30 - 35 dBHz",   SIGNAL_30_35,                 IM_COL32(0xe0, 0xe0, 0x52, 0xdf)) \ // hsl( 60, 70%, 60%)
	_P_("Signal 35 - 40 dBHz",   SIGNAL_35_40,                 IM_COL32(0xb3, 0xe0, 0x85, 0xdf)) \ // hsl( 90, 60%, 70%)
	_P_("Signal 40 - 45 dBHz",   SIGNAL_40_45,                 IM_COL32(0x06, 0xf9, 0x06, 0xdf)) \ // hsl(120, 95%, 50%)
	_P_("Signal 45 - 50 dBHz",   SIGNAL_45_50,                 IM_COL32(0x13, 0xec, 0x5b, 0xdf)) \ // hsl(140, 85%, 50%)
	_P_("Signal 50 - 55 dBHz",   SIGNAL_50_55,                 IM_COL32(0x13, 0xec, 0xa4, 0xdf)) \ // hsl(160, 85%, 50%)
	_P_("Signal 55 - oo dBHz",   SIGNAL_55_OO,                 IM_COL32(0x13, 0xec, 0xec, 0xdf)) \ // hsl(180, 85%, 50%)
	// Generic colours (some from https://github.com/leiradel/ImGuiAl/blob/master/term/imguial_term.h)
	_P_("Colour Black",          C_BLACK,                      IM_COL32(0x00, 0x00, 0x00, 0xff)) \
	_P_("Colour Grey",           C_GREY,                       IM_COL32(0x55, 0x55, 0x55, 0xff)) \
	_P_("Colour White",          C_WHITE,                      IM_COL32(0xaa, 0xaa, 0xaa, 0xff)) \
	_P_("Colour BrightWhite",    C_BRIGHTWHITE,                IM_COL32(0xff, 0xff, 0xff, 0xff)) \
	_P_("Colour Blue",           C_BLUE,                       IM_COL32(0x00, 0x00, 0xaa, 0xff)) \
	_P_("Colour BrightBlue",     C_BRIGHTBLUE,                 IM_COL32(0x55, 0x55, 0xff, 0xff)) \
	_P_("Colour Green",          C_GREEN,                      IM_COL32(0x00, 0xaa, 0x00, 0xff)) \
	_P_("Colour BrightGreen",    C_BRIGHTGREEN,                IM_COL32(0x55, 0xff, 0x55, 0xff)) \
	_P_("Colour Cyan",           C_CYAN,                       IM_COL32(0x00, 0xaa, 0xaa, 0xff)) \
	_P_("Colour BrightCyan",     C_BRIGHTCYAN,                 IM_COL32(0x55, 0xff, 0xff, 0xff)) \
	_P_("Colour Red",            C_RED,                        IM_COL32(0xaa, 0x00, 0x00, 0xff)) \
	_P_("Colour BrightRed",      C_BRIGHTRED,                  IM_COL32(0xff, 0x55, 0x55, 0xff)) \
	_P_("Colour Magenta",        C_MAGENTA,                    IM_COL32(0xaa, 0x00, 0xaa, 0xff)) \
	_P_("Colour BrightMagenta",  C_BRIGHTMAGENTA,              IM_COL32(0xff, 0x55, 0xff, 0xff)) \
	_P_("Colour Brown",          C_BROWN,                      IM_COL32(0xaa, 0x55, 0x00, 0xff)) \
	_P_("Colour Yellow",         C_YELLOW,                     IM_COL32(0xff, 0xff, 0x55, 0xff)) \
	_P_("Colour Orange",         C_ORANGE,                     IM_COL32(0xa8, 0x65, 0x00, 0xff)) \
	_P_("Colour BrightOrange",   C_BRIGHTORANGE,               IM_COL32(0xff, 0xa1, 0x14, 0xff)) \
	_P_("Colour None",           C_NONE,                       IM_COL32(0x00, 0x00, 0x00, 0x00)) \
	// Log window
	_P_("Log UBX messages",      LOG_MSGUBX,                   IM_COL32(0xff, 0x80, 0x80, 0xff)) \
	_P_("Log NMEA messages",     LOG_MSGNMEA,                  IM_COL32(0xab, 0xff, 0x80, 0xff)) \
	_P_("Log RTCM3 messages",    LOG_MSGRTCM3,                 IM_COL32(0xaf, 0x80, 0xff, 0xff)) \
	_P_("Log NOVATEL messages",  LOG_MSGNOVATEL,               IM_COL32(0x41, 0xce, 0x73, 0xff)) \
	_P_("Log GARBAGE messages",  LOG_MSGGARBAGE,               IM_COL32(0x80, 0xce, 0xff, 0xff)) \
	_P_("Log Epochs",            LOG_EPOCH,                    IM_COL32(0xff, 0xfe, 0x80, 0xff)) \
	// Inf window
	_P_("Inf Debug",             INF_DEBUG,                    IM_COL32(0x00, 0xaa, 0xaa, 0xff)) \
	_P_("Inf Notice",            INF_NOTICE,                   IM_COL32(0xff, 0xff, 0xff, 0xff)) \
	_P_("Inf Warning",           INF_WARNING,                  IM_COL32(0xff, 0xff, 0x55, 0xff)) \
	_P_("Inf Error",             INF_ERROR,                    IM_COL32(0xff, 0x55, 0x55, 0xff)) \
	_P_("Inf Test",              INF_TEST,                     IM_COL32(0xff, 0x55, 0xff, 0xff)) \
	_P_("Inf Other",             INF_OTHER,                    IM_COL32(0xaa, 0x55, 0x00, 0xff)) \
	// Debug logs
	_P_("Debug Trace",           DEBUG_TRACE,                  IM_COL32(0xaa, 0x00, 0xaa, 0xff)) \
	_P_("Debug Debug",           DEBUG_DEBUG,                  IM_COL32(0x00, 0xaa, 0xaa, 0xff)) \
	_P_("Debug Print",           DEBUG_PRINT,                  IM_COL32(0xaa, 0xaa, 0xaa, 0xff)) \
	_P_("Debug Notice",          DEBUG_NOTICE,                 IM_COL32(0xff, 0xff, 0xff, 0xff)) \
	_P_("Debug Warning",         DEBUG_WARNING,                IM_COL32(0xff, 0xff, 0x55, 0xff)) \
	_P_("Debug Error",           DEBUG_ERROR,                  IM_COL32(0xff, 0x55, 0x55, 0xff))
*/
