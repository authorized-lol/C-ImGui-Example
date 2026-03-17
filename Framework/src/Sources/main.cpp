#include "..\gui.h"
#include <thread>

// ---- authorized.lol API key ----
// Replace with your actual application API key from the dashboard
static constexpr const char* AUTHORIZED_LOL_API_KEY = "YOUR_API_KEY";

// Data
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
ImFontAtlas i;

static void SetRoundedCorners(HWND hwnd)
{
    DWORD preference = 2;
    DwmSetWindowAttribute(hwnd, 33, &preference, sizeof(preference));
}

void Blur(HWND WindowW)
{
    SetRoundedCorners(WindowW); //only for windows 11

    struct ACCENTPOLICY
    {
        int na;
        int nf;
        int nc;
        int nA;
    };
    struct WINCOMPATTRDATA
    {
        int na;
        PVOID pd;
        ULONG ul;
    };

    const HINSTANCE hm = LoadLibrary(L"user32.dll");
    if (hm)
    {
        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute)
        {
            ACCENTPOLICY policy = { 3, 0, 0, 0 }; // 4,0,155,0 (Acrylic blur) //  3,0,0,0 
            WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
            SetWindowCompositionAttribute(WindowW, &data);
        }
        FreeLibrary(hm);
    }
}

// Main code
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // Set authorized.lol API key
    AuthorizedLol::API_KEY = AUTHORIZED_LOL_API_KEY;

    // Initialize curl globally (must be done before any curl calls)
    curl_global_init(CURL_GLOBAL_ALL);

    wchar_t exePath[MAX_PATH];
    GetModuleFileName(nullptr, exePath, MAX_PATH);

    std::wstring exeName = std::wstring(exePath);
    size_t lastSlash = exeName.find_last_of(L"/\\");
    size_t lastDot = exeName.find_last_of(L".");
    std::wstring appName = exeName.substr(lastSlash + 1, lastDot - lastSlash - 1);

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, appName.c_str(), nullptr};
    ::RegisterClassExW(&wc);
    hwnd = CreateWindowExW(NULL, wc.lpszClassName, appName.c_str(), WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) / 2) - (window::size_max.x / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (window::size_max.y / 2), window::size_max.x, window::size_max.y, 0, 0, hInstance, 0);

    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
  
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 0.f);

    std::chrono::steady_clock::time_point loading_time;
    bool timer_started = false;

    POINT mouse;
    rc = { 0 };
    GetWindowRect(hwnd, &rc);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Initialize Fonts
    ui::initialize_fonts();
    ui::initialize_images();
    ImGuiStyle* s = &ImGui::GetStyle();

    s->WindowPadding = ImVec2(0, 0), s->WindowBorderSize = 0;
    s->ItemSpacing = ImVec2(20, 20);

    s->ScrollbarSize = 4.f;

    Blur(hwnd);

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool done = false;
    while (!done)
    {
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) { ImGui::DestroyContext(); i.ClearInputData(); break; }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame(); 

        ImGui::SetNextWindowSize(window::size_max);
        ImGui::SetNextWindowBgAlpha(ui::alpha::background);
        ImGui::Begin("authorized.lol", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
        {
            ui::alpha::background = std::clamp(ui::alpha::background + (6.f * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 1.f);
            float fade_in_speed = 2.0f;
            float fade_out_speed = 0.5f;

            if(ui::variables::logged_in) {
            ui::alpha::login_images = std::clamp(ui::alpha::login_images + (fade_in_speed * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 1.f);
            }
            else {
            ui::alpha::login_images = std::clamp(ui::alpha::login_images - (fade_out_speed * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 1.f);
            }
            // Show spinner whenever loading is active — snap to 1 immediately so there's no fade-in delay
            if (ui::variables::loading)
                ui::alpha::loading = 1.0f;
            else
                ui::alpha::loading = std::clamp(ui::alpha::loading - fade_out_speed * ImGui::GetIO().DeltaTime, 0.0f, 1.0f);

                ui::resize(window::size_max, ImVec2(680, 496));

            ui::move_window();

            ImGuiWindow* window = ImGui::GetCurrentWindow();
            ImVec2 pos(0, 0);
            ImVec2 size(pos.x + window::size_max.x, pos.y + window::size_max.y);
            ImDrawList* backgroundDrawList = ImGui::GetBackgroundDrawList();
            ImDrawList* foregroundDrawList = ImGui::GetForegroundDrawList();

            ui::render_background(backgroundDrawList);

            ui::render_login(backgroundDrawList);

            if (ui::variables::loading || ui::alpha::loading > 0.01f)
                ui::render_loading(backgroundDrawList);

            // Show main UI once logged in and loading screen has fully faded out
            if (ui::variables::logged_in && !ui::variables::loading && ui::alpha::loading <= 0.01f)
            {
                ui::render_main(backgroundDrawList);
            }

            ui::render_notification();

            ui::render_outline(backgroundDrawList);
        }
        ImGui::End();
        ImGui::EndFrame();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    curl_global_cleanup();
    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}