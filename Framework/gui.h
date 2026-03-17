#pragma once
#include <d3d11.h>
#include <windows.h>
#include <dwmapi.h>
#include <string>
#include <D3DX11tex.h>
#pragma comment(lib, "D3DX11.lib")
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <imgui_freetype.h>
#include <map>
#include <algorithm>
#include <cmath>
#include <vector>
#include <sstream>
#include <fstream>
#include <tlhelp32.h>
#define CURL_STATICLIB
#include <curl/curl.h>
#pragma comment(lib, "freetype64.lib")  
#include "font_awesome.h"
#include "skStr.h"
#include "item.h"
#include "imspinner.h"
#include <Lmcons.h> 
#include <chrono>
#include <atomic>
#include "authorized_lol.h"

inline ID3D11Device* g_pd3dDevice = nullptr;
inline ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
inline IDXGISwapChain* g_pSwapChain = nullptr;
inline UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
inline ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

#define size_of IM_ARRAYSIZE

inline HWND hwnd;
inline RECT rc;

namespace window {
	inline ImVec2 size_max = { 0, 0 };
	const float rounding = 10.f;
    inline ImVec2 previous_size = size_max;
    inline float speed = 8.f;
}

struct Fonts {
    ImFont* montserrat_semibold[4];
    ImFont* tab_icon;
    ImFont* widget_icon;
    ImFont* notify_font;
    ImFont* font_awesome;
    ImFont* motherboards;
    ImFont* buttons_icon;
}; inline Fonts font;

namespace images
{
    inline ID3D11ShaderResourceView* rust;
    inline ID3D11ShaderResourceView* eac;
    inline ID3D11ShaderResourceView* valorant;
    inline ID3D11ShaderResourceView* vanguard;
    inline ID3D11ShaderResourceView* pubg;
    inline ID3D11ShaderResourceView* be;
    inline ID3D11ShaderResourceView* logo;
};

namespace ui
{
    void move_window();
    void resize(ImVec2& size, const ImVec2& target);
    void initialize_fonts();
    void initialize_images();

    // render background
    void render_background(ImDrawList* drawlist);

    // render login
    void render_login(ImDrawList* drawlist);

    // render outline
    void render_outline(ImDrawList* drawlist);

    // render loading
    void render_loading(ImDrawList* drawlist);

    void render_spoof_loading(ImDrawList* drawlist, ImVec2 window_size, float progress, const std::vector<std::string>& options);

    // render main
    void render_main(ImDrawList* drawlist);

    // add notification
    void add_notification(const std::string& icon, const std::string& msg, const ImVec4& icon_color);

    void images_alpha_transition();

    // render notification
    void render_notification();

    // render minimize & close
    void minimize_close(ImDrawList* drawlist);

    namespace items
    {
        bool input_text(const char* label, ImVec2 pos, ImVec2 Size, char buf[], size_t buf_size, ImGuiInputTextFlags flag);
        bool checkbox(const char* label, ImVec2 pos, ImVec2 size, bool* active, int offset_x = 0);
        bool button(const char* label, ImVec2 pos, ImVec2 size);
    };

    namespace variables
    {
        inline std::atomic<bool> logged_in        { false };
        inline bool              resize_main       = false;
        static float             login_fade_timer  = 0.f;
        inline float             time_base         = 0.f;
        inline std::chrono::steady_clock::time_point loading_time;
        inline bool              timer_started     = false;
        inline bool              spoof_loading     = false;
        inline bool              was_loading       = false;
        inline std::atomic<bool> loading           { false };
        inline float             loading_anim      = 0.0f;
        inline int               selected_tab      = 0;
        inline bool              spinner_alpha_zero = false;
        inline std::string       g_selected_motherboard = "";
        inline int               g_selected_button = 0;

        // authorized.lol auth state
        inline std::atomic<bool> auth_in_progress  { false };
        inline std::atomic<bool> auth_failed        { false };
        inline std::string       auth_error_msg     = "";
    };

    namespace colors
    {
        namespace loader
        {
        inline ImVec4 background = ImColor(11, 14, 19);
        inline ImVec4 background_dark = ImColor(9, 12, 17);
        inline ImVec4 outline = ImColor(36, 36, 36);
        inline ImVec4 outline_white = ImColor(255, 255, 255, 40);
        inline ImVec4 main = ImColor(2, 71, 253); 
        inline ImVec4 child = ImColor(21, 22, 23, 200);
        inline ImVec4 text_disabled = ImColor(93, 93, 93);
        inline ImVec4 text = ImColor(255, 255, 255);
        };

        namespace input_text
        {
            inline ImVec4 frame_bg = ImColor(21, 24, 29);
            inline ImVec4 text_disabled = ImColor(120, 110, 100);
            inline ImVec4 text_selected_bg = ImColor(1, 98, 213, 20);

            inline ImVec4 background = ImColor(21, 24, 29); 
            inline ImVec4 background_hovered = ImColor(21, 24, 29);
            inline ImVec4 background_active = ImColor(21, 24, 29);

            inline ImVec4 border = ImColor(255, 255, 255,0);
            inline ImVec4 border_active = ui::colors::loader::main;

            inline ImVec4 render_selection = ImColor(1, 98, 213, 80);

            inline ImVec4 text = ImColor(235, 230, 225);
            inline ImVec4 text_active = ImColor(255, 255, 255);
            inline ImVec4 text_hovered = ImColor(255, 245, 235);
        };

        namespace checkbox
        {
            // Fondo basado en input_text::background
            inline ImVec4 bg = loader::child;
            inline ImVec4 bg_hovered = loader::child;
            inline ImVec4 bg_active = loader::child;
            inline ImVec4 bg_disabled = loader::child;

            // C�rculo basado en input_text::frame_bg
            inline ImVec4 circle = loader::main;
            inline ImVec4 circle_hovered = loader::main;
            inline ImVec4 circle_active = loader::main;
            inline ImVec4 circle_disabled = ImColor(130, 130, 130, 255);

            inline ImVec4 border = input_text::border;
            inline ImVec4 border_active = loader::main;

            // Texto basado en input_text
            inline ImVec4 text = ImColor(235, 230, 225, 180);       // input_text::text con alpha ~70%
            inline ImVec4 text_hovered = input_text::text_hovered;
            inline ImVec4 text_active = input_text::text_active;
        };

        namespace button
        {
            inline ImVec4 background = loader::main;
            inline ImVec4 background_light = ImVec4(
                ImMin(loader::main.x * 1.25f, 1.0f),  // R
                ImMin(loader::main.y * 1.25f, 1.0f),  // G
                ImMin(loader::main.z * 1.25f, 1.0f),  // B
                loader::main.w                      // Alpha igual
            );
            inline ImVec4 outline_background = loader::outline_white;
        }
    };

    namespace alpha
    {
        inline float background = 0.f;
        inline float logged_in = 0.f;
        inline float loading = 0.f;
        inline float login_images = 0.f;

        inline float rust_alpha = 0.f;
        inline float eac_alpha = 0.f;

        inline float vanguard_alpha = 0.f;
        inline float valorant_alpha = 0.f;

        inline float be_alpha = 0.f;
        inline float pubg_alpha = 0.f;
    };
}
