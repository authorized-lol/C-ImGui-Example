#include "gui.h"
#include "fonts.h"
#include "images.h"
#include <thread>

static char license_key[256] = {};
static bool remember_me = false;
static bool tpm_bypass = false;
static bool arp_fix = false;

float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

std::string to_lower(const std::string& str) {
	std::string lower_str = str;
	std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return lower_str;
}

void ui::move_window()
{
	static int last_width = static_cast<int>(window::size_max.x);
	static int last_height = static_cast<int>(window::size_max.y);

	RECT rc;
	GetWindowRect(hwnd, &rc);

	int current_width = rc.right - rc.left;
	int current_height = rc.bottom - rc.top;

	int center_x = rc.left + (current_width / 2);
	int center_y = rc.top + (current_height / 2);

	int new_width = static_cast<int>(std::round(window::size_max.x));
	int new_height = static_cast<int>(std::round(window::size_max.y));

	int new_left = center_x - (new_width / 2);
	int new_top = center_y - (new_height / 2);

	ImVec2 imguiPos = ImGui::GetWindowPos();
	new_left += static_cast<int>(std::round(imguiPos.x));
	new_top += static_cast<int>(std::round(imguiPos.y));

	MoveWindow(hwnd, new_left, new_top, new_width, new_height, TRUE);

	last_width = new_width;
	last_height = new_height;

	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
}

void ui::resize(ImVec2& size, const ImVec2& target)
{
	size.x = ImLerp(size.x, target.x, window::speed * ImGui::GetIO().DeltaTime);
	size.y = ImLerp(size.y, target.y, window::speed * ImGui::GetIO().DeltaTime);
}

void ui::initialize_fonts()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImFontConfig montserrat_cfg;
	montserrat_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;
	montserrat_cfg.GlyphExtraSpacing.x = -1.0f;
	ImFontConfig cfg;

	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true; icons_config.OversampleH = 1; icons_config.OversampleV = 1;

	font.montserrat_semibold[0] = io.Fonts->AddFontFromMemoryTTF(montserrat_semibold, sizeof(montserrat_semibold), 32, &montserrat_cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.montserrat_semibold[1] = io.Fonts->AddFontFromMemoryTTF(montserrat_semibold, sizeof(montserrat_semibold), 16, &montserrat_cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.montserrat_semibold[2] = io.Fonts->AddFontFromMemoryTTF(montserrat_semibold, sizeof(montserrat_semibold), 14, &montserrat_cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.montserrat_semibold[3] = io.Fonts->AddFontFromMemoryTTF(montserrat_semibold, sizeof(montserrat_semibold), 16, &montserrat_cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.buttons_icon = io.Fonts->AddFontFromMemoryTTF(button_icon, sizeof(button_icon), 16, &cfg, io.Fonts->GetGlyphRangesCyrillic());

	font.motherboards = io.Fonts->AddFontFromMemoryTTF(motherboards, sizeof(motherboards), 40, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.widget_icon = io.Fonts->AddFontFromMemoryTTF(widget_icon, sizeof(widget_icon), 16, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.notify_font = io.Fonts->AddFontFromMemoryTTF(notify_font, sizeof(notify_font), 14, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.tab_icon = io.Fonts->AddFontFromMemoryTTF(tab_icon, sizeof(tab_icon), 18, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.font_awesome = io.Fonts->AddFontFromMemoryCompressedTTF(fa6_solid_compressed_data, fa6_solid_compressed_size, 18.0f, &icons_config, icons_ranges);
	io.FontDefault = font.montserrat_semibold[3];

}

struct CarouselImage {
	ImTextureID texture;
	float position_offset = 0.0f;
	float current_offset = 0.0f;
	float target_offset = 0.0f;
	float scale = 1.0f;
	float alpha = 1.0f;
	ImVec4 current_color = ImVec4(1, 1, 1, 1); // RGBA (normalized)
};

std::vector<CarouselImage> carousel_images;

void ui::initialize_images()
{
	D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
	if (images::rust == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, rust_login, sizeof(rust_login), &info, pump, &images::rust, 0);
	if (images::eac == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, eac_logo, sizeof(eac_logo), &info, pump, &images::eac, 0);
	if (images::vanguard == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, vanguard_icon, sizeof(vanguard_icon), &info, pump, &images::vanguard, 0);
	if (images::valorant == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, valorant_bg, sizeof(valorant_bg), &info, pump, &images::valorant, 0);
	if (images::be == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, be_logo, sizeof(be_logo), &info, pump, &images::be, 0);
	if (images::pubg == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, pubg_bg, sizeof(pubg_bg), &info, pump, &images::pubg, 0);
	if (images::logo == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo, sizeof(logo), &info, pump, &images::logo, 0);

	carousel_images = {
   { images::rust, 0, -2.0f, -2.0f},
	{ images::valorant, 1, -1.0f, -1.0f },
	{ images::pubg,  2,  0.0f,  0.0f }
	};
}

struct Notification {
	std::string message;
	std::string icon;
	float alpha = 0.0f;
	float timer = 0.0f;
	bool fading_out = false;
	float duration = 3.0f;
	float target_y = 0.0f;
	float current_y = 0.0f;
	ImVec4 bg_color = ui::colors::loader::background_dark;
	ImVec4 icon_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
};

std::vector<Notification> notifications;

/*
EXAMPLES:
ui::add_notification("A", "Notification 1 triggered!", ImVec4(0.56f, 0.93f, 0.56f, 1.0f)); OK
ui::add_notification("B", "Notification 2 triggered!", ImVec4(0.98f, 0.95f, 0.57f, 1.0f)); WARNING
ui::add_notification("C", "Notification 3 triggered!", ImVec4(0.68f, 0.85f, 0.90f, 1.0f)); INFO
ui::add_notification("D", "Notification 4 triggered!", ImVec4(0.99f, 0.60f, 0.60f, 1.0f)); ERROR
*/

void ui::add_notification(const std::string& icon, const std::string& msg, const ImVec4& icon_color) {
	const int max_notifications = 2;

	if ((int)notifications.size() >= max_notifications) {
		notifications[0].fading_out = true;
	}

	notifications.push_back({
		msg, icon, 0.0f, 0.0f, false, 3.0f, 0.0f, 0.0f,
		ImVec4(0.12f, 0.12f, 0.12f, 1.0f), icon_color
		});
}

void ui::render_notification() {
	const float padding = 14.0f;
	const float right_padding = 10.0f;
	const ImVec2 viewport_pos = ImGui::GetMainViewport()->WorkPos;
	const ImVec2 viewport_size = ImGui::GetMainViewport()->Size;

	float total_offset = 0.0f;

	for (int i = (int)notifications.size() - 1; i >= 0; --i) {
		Notification& note = notifications[i];
		float delta = ImGui::GetIO().DeltaTime;
		note.timer += delta;

		// Handle fading in/out
		if (!note.fading_out) {
			note.alpha = ImClamp(note.alpha + delta * 3.0f, 0.0f, 1.0f);
			if (note.timer >= note.duration)
				note.fading_out = true;
		}
		else {
			note.alpha = ImClamp(note.alpha - delta * 2.0f, 0.0f, 1.0f);
		}

		if (note.fading_out && note.alpha <= 0.0f) {
			notifications.erase(notifications.begin() + i);
			continue;
		}

		// Calculate sizes
		ImGui::PushFont(font.notify_font);
		ImVec2 icon_size = ImGui::CalcTextSize(note.icon.c_str());
		ImGui::PopFont();

		ImGui::PushFont(font.montserrat_semibold[2]);
		ImVec2 text_size = ImGui::CalcTextSize(note.message.c_str());
		ImGui::PopFont();

		ImVec2 box_size(icon_size.x + text_size.x + 30.0f + right_padding, ImMax(icon_size.y, text_size.y) + 10.0f);

		// Top-right stacking
		float target_y = viewport_pos.y + padding + total_offset;

		if (note.current_y == 0.0f)
			note.current_y = target_y;

		note.target_y = target_y;
		note.current_y += (note.target_y - note.current_y) * delta * 10.0f;

		ImVec2 pos(viewport_pos.x + viewport_size.x - box_size.x - padding, note.current_y);

		// Draw background and border
		ImDrawList* draw = ImGui::GetForegroundDrawList();
		draw->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + box_size.x, pos.y + box_size.y), ImColor(note.bg_color.x, note.bg_color.y, note.bg_color.z, note.alpha), 6.0f);
		draw->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + box_size.x, pos.y + box_size.y), ImColor(ui::colors::loader::outline.x, ui::colors::loader::outline.y, ui::colors::loader::outline.z, note.alpha), 6.0f);

		// Draw icon
		ImGui::PushFont(font.notify_font);
		draw->AddText(
			ImVec2(pos.x + 10, pos.y + 4),
			ImColor(note.icon_color.x, note.icon_color.y, note.icon_color.z, note.alpha),
			note.icon.c_str()
		);
		ImGui::PopFont();

		// Vertical line
		float line_x = pos.x + 15 + icon_size.x + 5.0f;
		float line_y_start = pos.y + 5.0f;
		float line_y_end = pos.y + box_size.y - 5.0f;
		draw->AddLine(
			ImVec2(line_x - 1, line_y_start),
			ImVec2(line_x - 1, line_y_end),
			ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, note.alpha * 0.3f)),
			1.0f
		);

		// Draw message
		ImGui::PushFont(font.montserrat_semibold[2]);
		draw->AddText(
			ImVec2(line_x + 8, pos.y + 4),
			ImColor(1.0f, 1.0f, 1.0f, note.alpha),
			note.message.c_str()
		);
		ImGui::PopFont();

		// Update offset for next notification
		total_offset += box_size.y + 5.0f;
	}
}

void ui::render_background(ImDrawList* drawlist)
{
	ImVec2 pos(0, 0);
	ImVec2 size(pos.x + window::size_max.x, pos.y + window::size_max.y);

	drawlist->AddRectFilled(
		pos,
		size,
		IM_COL32(
			ui::colors::loader::background.x * 255,
			ui::colors::loader::background.y * 255,
			ui::colors::loader::background.z * 255,
			ui::alpha::background * 240
		),
		window::rounding,
		ImDrawFlags_RoundCornersAll);

}

void ui::images_alpha_transition() {
	float speed_clamp = 600.f;
	switch (ui::variables::selected_tab)
	{
	case 0:
		ui::alpha::rust_alpha = std::clamp(ui::alpha::rust_alpha + (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 120.f);
		ui::alpha::eac_alpha = std::clamp(ui::alpha::eac_alpha + (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 255.f);

		ui::alpha::valorant_alpha = std::clamp(ui::alpha::valorant_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 120.f);
		ui::alpha::vanguard_alpha = std::clamp(ui::alpha::vanguard_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 255.f);

		ui::alpha::be_alpha = std::clamp(ui::alpha::be_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 255.f);
		ui::alpha::pubg_alpha = std::clamp(ui::alpha::pubg_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 120.f);
		break;
	case 1:
		ui::alpha::rust_alpha = std::clamp(ui::alpha::rust_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 120.f);
		ui::alpha::eac_alpha = std::clamp(ui::alpha::eac_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 255.f);

		ui::alpha::valorant_alpha = std::clamp(ui::alpha::valorant_alpha + (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 120.f);
		ui::alpha::vanguard_alpha = std::clamp(ui::alpha::vanguard_alpha + (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 255.f);

		ui::alpha::be_alpha = std::clamp(ui::alpha::be_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 255.f);
		ui::alpha::pubg_alpha = std::clamp(ui::alpha::pubg_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 120.f);
		break;
	case 2:
		ui::alpha::rust_alpha = std::clamp(ui::alpha::rust_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 120.f);
		ui::alpha::eac_alpha = std::clamp(ui::alpha::eac_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 255.f);

		ui::alpha::valorant_alpha = std::clamp(ui::alpha::valorant_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 120.f);
		ui::alpha::vanguard_alpha = std::clamp(ui::alpha::vanguard_alpha - (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 255.f);

		ui::alpha::be_alpha = std::clamp(ui::alpha::be_alpha + (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 255.f);
		ui::alpha::pubg_alpha = std::clamp(ui::alpha::pubg_alpha + (speed_clamp * ImGui::GetIO().DeltaTime * 1.f), 0.0f, 120.f);
		break;
	default:
		break;
	}
}

void ui::render_login(ImDrawList* drawlist)
{
	if (ui::variables::logged_in && ui::variables::login_fade_timer <= 0.0f)
		return;

	// Hide login panel entirely while loading screen is active or auth succeeded
	if (ui::variables::loading || ui::alpha::loading > 0.01f || ui::variables::logged_in)
		return;

	ImVec2 pos(0, 0);
	ImVec2 size(window::size_max.x, window::size_max.y);
	ImGuiIO& io = ImGui::GetIO();

	if (ui::variables::logged_in)
	{
		ui::variables::login_fade_timer -= io.DeltaTime * 2.0f;
		ui::variables::login_fade_timer = max(ui::variables::login_fade_timer, 0.0f);
	}
	else
	{
		ui::variables::login_fade_timer += io.DeltaTime;
	}

	float left_width = 340;
	ImVec2 left_min(pos.x, pos.y);
	ImVec2 left_max(pos.x + left_width, pos.y + size.y);

	float element_gap = 0.25f;
	int index = 0;

	ui::variables::time_base = ui::variables::login_fade_timer;
	float fade_speed_left = 2.0f;

	auto get_fade_alpha = [&](int i) -> float {
		float delay = i * element_gap;
		float a = (ui::variables::time_base - delay) * fade_speed_left;
		return std::clamp(a, 0.0f, 1.0f);
		};

	float alpha_spoofer = get_fade_alpha(index++);
	float alpha_welcome = get_fade_alpha(index++);
	float alpha_signin = get_fade_alpha(index++);
	float alpha_license = get_fade_alpha(index++);
	float alpha_buttons = get_fade_alpha(index++);
	float alpha_support = get_fade_alpha(index++);

	if (alpha_spoofer <= 0.0f && ui::variables::logged_in) 
	{
		return;
	}


	ImGui::PushFont(font.montserrat_semibold[0]);
	std::string spoofer_text = ImGui::GetCurrentWindow()->Name;
	ImVec2 spoofer_size = ImGui::CalcTextSize(spoofer_text.c_str());
	ImVec2 spoofer_pos(left_min.x + left_width * 0.5f - spoofer_size.x * 0.5f, left_min.y + 40);

	ImVec4 color_start = ui::colors::loader::main;
	ImVec4 color_end = ImVec4(color_start.x * 0.8f, color_start.y * 0.8f, color_start.z * 0.8f, color_start.w);

	float xs = spoofer_pos.x;
	for (size_t i = 0; i < spoofer_text.size(); i++)
	{
		float t = i / (float)spoofer_text.size();
		ImVec4 col = ImLerp(color_end, color_start, t);
		col.w *= alpha_spoofer;
		drawlist->AddText(ImVec2(xs, spoofer_pos.y), ImColor(col), std::string(1, spoofer_text[i]).c_str());
		xs += ImGui::CalcTextSize(std::string(1, spoofer_text[i]).c_str()).x;
	}
	ImGui::PopFont();

	ImGui::PushFont(font.montserrat_semibold[1]);
	std::string welcome_text = "Welcome";
	ImVec2 welcome_size = ImGui::CalcTextSize(welcome_text.c_str());
	ImVec2 welcome_pos(left_min.x + left_width * 0.5f - welcome_size.x * 0.5f, spoofer_pos.y + spoofer_size.y + 30);
	drawlist->AddText(welcome_pos, IM_COL32(180, 180, 180, (int)(alpha_welcome * 255)), welcome_text.c_str());
	ImGui::PopFont();

	ImGui::PushFont(font.montserrat_semibold[0]);
	std::string signin_now_text = "Sign in Now";
	ImVec2 signin_now_size = ImGui::CalcTextSize(signin_now_text.c_str());
	ImVec2 signin_now_pos(left_min.x + left_width * 0.5f - signin_now_size.x * 0.5f, welcome_pos.y + welcome_size.y + 10);
	drawlist->AddText(signin_now_pos, IM_COL32(255, 255, 255, (int)(alpha_signin * 255)), signin_now_text.c_str());
	ImGui::PopFont();

	static char license_buf[64] = {};
	ImVec2 license_size(230, 38);
	ImVec2 license_pos(left_min.x + left_width * 0.5f - license_size.x * 0.5f, signin_now_pos.y + signin_now_size.y + 90);

	if (alpha_license > 0.0f)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha_license);
		ui::items::input_text("License", license_pos, license_size, license_buf, sizeof(license_buf), ImGuiInputTextFlags_None);
		ImGui::PopStyleVar();
	}

	ImGui::PushFont(font.montserrat_semibold[1]);
	float button_width = 230;
	float button_height = 38;
	float buttons_y = left_max.y - 180;
	float start_x = left_min.x + left_width * 0.5f - button_width * 0.5f;

	ImVec2 background_min(start_x - 5, buttons_y - 5);
	ImVec2 background_max(start_x + button_width + 5, buttons_y + button_height + 5);
	//drawlist->AddRect(background_min, background_max, IM_COL32(ui::colors::loader::outline.x * 255, ui::colors::loader::outline.y * 255, ui::colors::loader::outline.z * 255, (int)(alpha_buttons * 255)), 10.0f);

	ImVec2 btn_sign_min(start_x, buttons_y);
	ImVec2 btn_sign_max(btn_sign_min.x + button_width, btn_sign_min.y + button_height);

	bool hovered = ImGui::IsMouseHoveringRect(btn_sign_min, btn_sign_max) && alpha_buttons > 0.5f;

	drawlist->AddRectFilled(btn_sign_min, btn_sign_max, IM_COL32(ui::colors::loader::main.x * 255, ui::colors::loader::main.y * 255, ui::colors::loader::main.z * 255, (int)(alpha_buttons * 255)), 10.0f, ImDrawFlags_RoundCornersAll);
	drawlist->AddRect(btn_sign_min, btn_sign_max, IM_COL32(ui::colors::loader::outline_white.x * 255, ui::colors::loader::outline_white.y * 255, ui::colors::loader::outline_white.z * 255, (int)(alpha_buttons * 40)), 10.0f, ImDrawFlags_RoundCornersAll);

	ImVec2 btn_sign_text_pos(btn_sign_min.x + button_width * 0.5f - ImGui::CalcTextSize("Sign In").x * 0.5f, btn_sign_min.y + button_height * 0.5f - ImGui::CalcTextSize("Sign In").y * 0.5f);
	drawlist->AddText(btn_sign_text_pos, IM_COL32(255, 255, 255, (int)(alpha_buttons * 255)), "Sign In");

	if (hovered)
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

	if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ui::variables::logged_in) {
		if (strlen(license_buf) == 0) {
			ui::add_notification("C", "Please enter your license!", ImVec4(0.68f, 0.85f, 0.90f, 1.0f));
		}
		else if (!ui::variables::auth_in_progress) {
			ui::variables::auth_in_progress = true;
			ui::variables::auth_error_msg.clear();

			// Show loading spinner immediately — do NOT touch login_fade_timer here,
			// the login panel fades out on its own once logged_in becomes true
			ui::variables::loading = true;

			std::string key_copy(license_buf);

		std::thread([key_copy]() {
				try {
					std::string err;
					bool ok = AuthorizedLol::init(key_copy, err);

					if (ok) {
						ui::variables::logged_in = true;
					} else {
						ui::variables::auth_error_msg = err;
						ui::variables::auth_failed    = true;
						ui::variables::loading        = false;
					}
				} catch (const std::exception& e) {
					ui::variables::auth_error_msg = e.what();
					ui::variables::auth_failed    = true;
					ui::variables::loading        = false;
				} catch (...) {
					ui::variables::auth_error_msg = "Unknown exception in auth thread";
					ui::variables::auth_failed    = true;
					ui::variables::loading        = false;
				}
				ui::variables::auth_in_progress = false;
			}).detach();
		}
	}

	// Show auth error notification once back on the main thread (safe — auth_in_progress is false by now)
	if (ui::variables::auth_failed.exchange(false)) {
		ui::variables::login_fade_timer = 0.f;
		ui::add_notification("D", ui::variables::auth_error_msg, ImVec4(0.99f, 0.60f, 0.60f, 1.0f));
		ui::variables::auth_error_msg.clear();
	}

	// Show success notification once
	static bool login_notified = false;
	if (ui::variables::logged_in && !login_notified) {
		login_notified = true;
		ui::add_notification("A", "Authenticated successfully!", ImVec4(0.56f, 0.93f, 0.56f, 1.0f));
	}

	std::string support_text = "Contact with support";
	std::string before = "Contact with ";
	std::string word = "support";

	ImVec2 before_size = ImGui::CalcTextSize(before.c_str());
	ImVec2 word_size = ImGui::CalcTextSize(word.c_str());

	ImVec2 support_pos(left_min.x + left_width * 0.5f - (before_size.x + word_size.x) * 0.5f, background_max.y + 20);

	ImVec4 col_before = ImVec4(1.f, 1.f, 1.f, alpha_support);
	ImVec4 col_word = ui::colors::loader::main;
	col_word.w *= alpha_support;

	ImU32 u32_before = ImColor(col_before);
	ImU32 u32_word = ImColor(col_word);

	ImVec2 shadow_offset(1.5f, 1.5f);
	ImU32 shadow_col = IM_COL32(ui::colors::loader::main.x * 255, ui::colors::loader::main.y * 255, ui::colors::loader::main.z * 255, (int)(alpha_support * 50));

	drawlist->AddShadowRect(ImVec2(support_pos.x + before_size.x, support_pos.y), ImVec2(support_pos.x + before_size.x + word_size.x, support_pos.y + word_size.y), shadow_col, 80.0f, shadow_offset, ImDrawFlags_RoundCornersAll);

	drawlist->AddText(support_pos, u32_before, before.c_str());
	drawlist->AddText(ImVec2(support_pos.x + before_size.x, support_pos.y), u32_word, word.c_str());

	ImGui::PopFont();

	if (alpha_spoofer > 0.0f)
	{
		ui::images_alpha_transition();

		ImVec2 rust_min(window::size_max.x / 2, 0);
		ImVec2 rust_max(rust_min.x + 340, rust_min.y + 496);

		drawlist->AddImageRounded(images::rust, rust_min, rust_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, ui::alpha::rust_alpha * alpha_spoofer), window::rounding, ImDrawFlags_RoundCornersRight);
		drawlist->AddImageRounded(images::valorant, rust_min, rust_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, ui::alpha::valorant_alpha * alpha_spoofer), window::rounding, ImDrawFlags_RoundCornersRight);
		drawlist->AddImageRounded(images::pubg, rust_min, rust_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, ui::alpha::pubg_alpha * alpha_spoofer), window::rounding, ImDrawFlags_RoundCornersRight);

		static float eac_y = 0.0f;
		static float vanguard_y = 0.0f;
		static float be_y = 0.0f;
		float offset_speed = 10.f * io.DeltaTime;

		if (ui::variables::selected_tab == 0)
		{
			eac_y = ImLerp(eac_y, 0.0f, offset_speed);
			vanguard_y = ImLerp(vanguard_y, -40.0f, offset_speed);
			be_y = ImLerp(be_y, -40.0f, offset_speed);
		}
		else if (ui::variables::selected_tab == 1)
		{
			eac_y = ImLerp(eac_y, 40.0f, offset_speed);
			vanguard_y = ImLerp(vanguard_y, 0.0f, offset_speed);
			be_y = ImLerp(be_y, -40.0f, offset_speed);
		}
		else if (ui::variables::selected_tab == 2)
		{
			eac_y = ImLerp(eac_y, 40.0f, offset_speed);
			vanguard_y = ImLerp(vanguard_y, -40.0f, offset_speed);
			be_y = ImLerp(be_y, 0.0f, offset_speed);
		}

		float rust_center_x = (rust_min.x + rust_max.x) * 0.5f;
		float rust_center_y = (rust_min.y + rust_max.y) * 0.5f;

		float eac_width = 89.0f;
		float eac_height = 81.0f;
		ImVec2 eac_min(rust_center_x - eac_width * 0.5f, rust_center_y - eac_height * 0.5f + eac_y);
		ImVec2 eac_max(rust_center_x + eac_width * 0.5f, rust_center_y + eac_height * 0.5f + eac_y);
		drawlist->AddImageRounded(images::eac, eac_min, eac_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, ui::alpha::eac_alpha * alpha_spoofer), 6.0f, ImDrawFlags_RoundCornersAll);

		float vang_width = 59.0f;
		float vang_height = 78.0f;
		ImVec2 vang_min(rust_center_x - vang_width * 0.5f, rust_center_y - vang_height * 0.5f + vanguard_y);
		ImVec2 vang_max(rust_center_x + vang_width * 0.5f, rust_center_y + vang_height * 0.5f + vanguard_y);
		drawlist->AddImageRounded(images::vanguard, vang_min, vang_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, ui::alpha::vanguard_alpha * alpha_spoofer), 6.0f, ImDrawFlags_RoundCornersAll);

		float be_width = 68.0f;
		float be_height = 79.0f;
		ImVec2 be_min(rust_center_x - be_width * 0.5f, rust_center_y - be_height * 0.5f + be_y);
		ImVec2 be_max(rust_center_x + be_width * 0.5f, rust_center_y + be_height * 0.5f + be_y);
		drawlist->AddImageRounded(images::be, be_min, be_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, ui::alpha::be_alpha * alpha_spoofer), 6.0f, ImDrawFlags_RoundCornersAll);

		static std::string full_text;
		static std::vector<float> char_alpha;
		static bool fading_out = false;
		static int last_tab = -1;
		static float letter_timer = 0.0f;

		float text_speed = 20.0f;
		float fade_speed = 8.0f;

		if (ui::variables::selected_tab != last_tab && !fading_out)
			fading_out = true;

		if (fading_out)
		{
			bool all_hidden = true;
			for (size_t i = 0; i < char_alpha.size(); i++)
			{
				char_alpha[i] = ImLerp(char_alpha[i], 0.0f, fade_speed * io.DeltaTime);
				if (char_alpha[i] > 0.01f) all_hidden = false;
			}

			if (all_hidden)
			{
				last_tab = ui::variables::selected_tab;
				switch (last_tab)
				{
				case 0: full_text = "Support Easy Anti Cheat"; break;
				case 1: full_text = "Support Vanguard Anticheat"; break;
				case 2: full_text = "Support BattleEye Anticheat"; break;
				default: full_text = ""; break;
				}

				char_alpha.clear();
				char_alpha.resize(full_text.size(), 0.0f);
				fading_out = false;
				letter_timer = 0.0f;
			}
		}
		else
		{
			if (!full_text.empty())
			{
				letter_timer += io.DeltaTime * text_speed;
				int letters_to_show = (int)letter_timer;
				letters_to_show = std::clamp(letters_to_show, 0, (int)full_text.size());

				for (int i = 0; i < letters_to_show; i++)
					char_alpha[i] = ImLerp(char_alpha[i], 1.0f, fade_speed * io.DeltaTime);
			}
		}

		ImGui::PushFont(font.montserrat_semibold[1]);
		ImVec2 text_size = ImGui::CalcTextSize(full_text.c_str());

		float base_y = 0.0f;
		if (last_tab == 0)       base_y = eac_max.y;
		else if (last_tab == 1)  base_y = vang_max.y;
		else if (last_tab == 2)  base_y = be_max.y;

		ImVec2 text_pos(rust_center_x - text_size.x * 0.5f, base_y + 8.0f);

		float x = text_pos.x;
		for (size_t i = 0; i < full_text.size(); i++)
		{
			char c = full_text[i];
			if (c == ' ')
			{
				x += ImGui::CalcTextSize(" ").x;
				continue;
			}

			float alpha = char_alpha[i] * alpha_spoofer;
			ImVec2 char_size = ImGui::CalcTextSize(std::string(1, c).c_str());

			drawlist->AddText(ImVec2(x, text_pos.y), IM_COL32(255, 255, 255, (int)(alpha * 255)), std::string(1, c).c_str());
			x += char_size.x;
		}
		ImGui::PopFont();

		const float slider_height = 8.0f;
		const float slider_rounding = slider_height * 0.5f;
		float padding_x = 40.0f;

		ImVec2 slider_pos(pos.x + padding_x, window::size_max.y - 40.0f);
		ImVec2 slider_size(size.x - 2 * padding_x, slider_height);

		int count = int(carousel_images.size());
		if (count > 0)
		{
			float spacing = 10.0f;
			float base_width = 8.0f;
			float expanded_width = 24.0f;
			float animation_speed = 6.0f * io.DeltaTime;

			static std::vector<float> widths;
			if ((int)widths.size() != count)
				widths = std::vector<float>(count, base_width);

			float total_width = 0.0f;
			for (int i = 0; i < count; i++)
			{
				float target_width = (i == ui::variables::selected_tab) ? expanded_width : base_width;
				widths[i] = ImLerp(widths[i], target_width, animation_speed);
				total_width += widths[i];
				if (i < count - 1)
					total_width += spacing;
			}

			float start_x = rust_center_x - total_width * 0.5f;
			float y = slider_pos.y;
			static float timer = 0.0f;

			float x = start_x;
			for (int i = 0; i < count; i++)
			{
				ImVec2 rect_start(x, y);
				ImVec2 rect_end(x + widths[i], y + slider_height);

				ImU32 color = IM_COL32(255, 255, 255, (i == ui::variables::selected_tab) ? 220 : 100);
				drawlist->AddRectFilled(rect_start, rect_end, color, slider_rounding);

				ImVec2 mouse = io.MousePos;
				bool hovering = mouse.x >= rect_start.x && mouse.x <= rect_end.x && mouse.y >= rect_start.y && mouse.y <= rect_end.y;

				if (hovering)
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
					if (ImGui::IsMouseClicked(0))
					{
						ui::variables::selected_tab = i;
						timer = 0.0f;
					}
				}

				x += widths[i] + spacing;
			}

			timer += io.DeltaTime;
			if (timer > 3.0f)
			{
				ui::variables::selected_tab = (ui::variables::selected_tab + 1) % count;
				timer = 0.0f;
			}
		}
	}
}

void ui::render_outline(ImDrawList* drawlist)
{
	ImVec2 pos(0, 0);
	ImVec2 size(pos.x + window::size_max.x, pos.y + window::size_max.y);

	drawlist->AddRect(
		pos,
		size,
		IM_COL32(
			255,
			255,
			255,
			30
		),
		window::rounding,
		ImDrawFlags_RoundCornersAll);
}

void ui::render_loading(ImDrawList* drawlist)
{
	if (ui::alpha::loading <= 0.0f)
		return;

	float radius = 40.0f;
	float thickness = 6.0f;

	// --- Fullscreen opaque background (covers login panel entirely) ---
	ImVec2 win_min(0.f, 0.f);
	ImVec2 win_max(window::size_max.x, window::size_max.y);

	ImVec4 bg = ui::colors::loader::background;
	drawlist->AddRectFilled(win_min, win_max,
		IM_COL32((int)(bg.x * 255), (int)(bg.y * 255), (int)(bg.z * 255),
		         (int)(ui::alpha::loading * 255)),
		window::rounding, ImDrawFlags_RoundCornersAll);

	// Window outline on top of the background
	drawlist->AddRect(win_min, win_max, IM_COL32(255, 255, 255, (int)(ui::alpha::loading * 30)),
		window::rounding, ImDrawFlags_RoundCornersAll);

	// Spinner always centered in the window
	ImVec2 center(window::size_max.x * 0.5f, window::size_max.y * 0.5f - 20.f);

	const int segments = 120;
	const int steps = 8;

	static float current_angle = 0.0f;
	float speed = 2.0f;

	float delta_time = ImGui::GetIO().DeltaTime;

	current_angle += speed * delta_time;
	if (current_angle > 2.0f * IM_PI)
		current_angle -= 2.0f * IM_PI;

	float snake_length = IM_PI * 1.2f;

	bool spinner_faded = true;

	for (int i = 0; i < segments; i++)
	{
		float angle_i = (2.0f * IM_PI) * ((float)i / segments);
		float diff = angle_i - current_angle;
		if (diff < 0.0f) diff += 2.0f * IM_PI;

		float alpha = 0.0f;
		if (diff < snake_length)
			alpha = ui::alpha::loading * (1.0f - diff / snake_length);

		if (alpha >= 0.01f)
			spinner_faded = false;

		if (alpha <= 0.0f)
			continue;

		ImU32 color = ImGui::GetColorU32(ImVec4(
			ui::colors::loader::main.x,
			ui::colors::loader::main.y,
			ui::colors::loader::main.z,
			alpha));

		float angle_start = current_angle + angle_i;
		float angle_end = angle_start + (2.0f * IM_PI) / segments;

		for (int s = 0; s < steps; s++)
		{
			float segment_t0 = angle_start + (angle_end - angle_start) * (s / (float)steps);
			float segment_t1 = angle_start + (angle_end - angle_start) * ((s + 1) / (float)steps);

			ImVec2 p1 = ImVec2(center.x + cosf(segment_t0) * radius, center.y + sinf(segment_t0) * radius);
			ImVec2 p2 = ImVec2(center.x + cosf(segment_t1) * radius, center.y + sinf(segment_t1) * radius);

			drawlist->AddLine(p1, p2, color, thickness);
		}
	}

	ui::variables::spinner_alpha_zero = spinner_faded;

	static const char* loading_texts[] = {
		"Initializing authorized.lol...",
		"Loading modules...",
		"Finalizing setup..."
	};
	static const int texts_count = sizeof(loading_texts) / sizeof(loading_texts[0]);

	static float text_timer = 0.0f;
	static int text_index = 0;
	static bool was_loading = false;
	float display_time = 1.2f;
	float fade_time = 0.3f;

	// Reset cycling state each time a new load begins
	if (ui::variables::loading && !was_loading) {
		text_timer = 0.0f;
		text_index = 0;
	}
	was_loading = (bool)ui::variables::loading;

	// Only advance text cycling (and eventually stop loading) once auth has succeeded
	if (ui::variables::logged_in)
	{
		text_timer += delta_time;

		if (text_timer > display_time)
		{
			text_timer = 0.0f;
			text_index++;
			if (text_index >= texts_count)
			{
				text_index = 0;
				ui::variables::loading = false;
				ui::variables::timer_started = false;
				return;
			}
		}
	}
	else
	{
		// Auth still in progress — hold on first text, reset cycling
		text_index = 0;
		text_timer = 0.0f;
	}

	if (text_index < texts_count && loading_texts[text_index] != nullptr)
	{
		float alpha_text = 1.0f;
		if (text_timer < fade_time)
			alpha_text = text_timer / fade_time;
		else if (text_timer > display_time - fade_time)
			alpha_text = (display_time - text_timer) / fade_time;

		alpha_text *= ui::alpha::loading;

		ImGui::PushFont(font.montserrat_semibold[1]);

		const char* current_text = loading_texts[text_index];

		ImVec2 text_pos = ImVec2(center.x, center.y + radius + 26);
		ImVec2 text_size = ImGui::CalcTextSize(current_text ? current_text : "");
		ImVec2 text_draw_pos = ImVec2(text_pos.x - text_size.x * 0.5f, text_pos.y);

		ImGui::GetForegroundDrawList()->AddText(
			text_draw_pos,
			ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, alpha_text)),
			current_text ? current_text : ""
		);

		ImGui::PopFont();
	}

}

struct Tab {
	const char* name;
	const char* icon;
};

struct tab_state {
	float anim_t = 0.0f;
};

// render_spoof_loading kept as stub (called from main.cpp, just no-ops now)
void ui::render_spoof_loading(ImDrawList* drawlist, ImVec2 window_size, float progress, const std::vector<std::string>& options)
{
	(void)drawlist; (void)window_size; (void)progress; (void)options;
	ui::variables::spoof_loading = false;
}

// -------------------------------------------------------
//  Helper: draw a simple info row  label | value
//  panel_right_x = absolute X of the panel's right edge
// -------------------------------------------------------
static void draw_info_row(ImDrawList* dl, ImVec2& cursor, float panel_right_x,
	const char* label, const std::string& value,
	ImU32 label_col, ImU32 value_col, ImFont* fnt)
{
	ImGui::PushFont(fnt);
	ImVec2 lsz = ImGui::CalcTextSize(label);
	ImVec2 vsz = ImGui::CalcTextSize(value.c_str());
	dl->AddText(cursor, label_col, label);
	// right-align value with 14px padding from the panel's right edge
	dl->AddText(ImVec2(panel_right_x - vsz.x - 14.f, cursor.y), value_col, value.c_str());
	ImGui::PopFont();
	cursor.y += lsz.y + 10.f;
}

// -------------------------------------------------------
//  Helper: draw a panel with header
// -------------------------------------------------------
static void draw_panel(ImDrawList* dl, ImVec2 min, ImVec2 max, const char* title, ImFont* title_font)
{
	float r = 10.f;
	dl->AddRectFilled(min, max, ImGui::GetColorU32(ui::colors::loader::background), r);
	dl->AddRectFilled(min, ImVec2(max.x, min.y + 36.f),
		ImGui::GetColorU32(ui::colors::loader::child), r, ImDrawFlags_RoundCornersTop);
	dl->AddRect(min, max,
		ImGui::GetColorU32(ImVec4(ui::colors::loader::outline_white.x,
			ui::colors::loader::outline_white.y,
			ui::colors::loader::outline_white.z, 0.08f)), r);
	ImGui::PushFont(title_font);
	ImVec2 ts = ImGui::CalcTextSize(title);
	dl->AddText(ImVec2(min.x + 14.f, min.y + (36.f - ts.y) * 0.5f),
		IM_COL32(255, 255, 255, 255), title);
	ImGui::PopFont();
}

void ui::render_main(ImDrawList* drawlist)
{
	ImVec2 pos(0, 0);
	ImVec2 size = ImVec2(window::size_max.x, window::size_max.y);

	if (window::size_max.x < 664 || window::size_max.y < 480) return;

	ImGuiIO& io = ImGui::GetIO();

	// ---- window title ----
	{
		ImGui::PushFont(font.montserrat_semibold[3]);
		const char* title = ImGui::GetCurrentWindow()->Name;
		ImVec2 ts = ImGui::CalcTextSize(title);
		drawlist->AddText(
			ImVec2(pos.x + (size.x - ts.x) * 0.5f, pos.y + 18.f),
			ImGui::GetColorU32(ui::colors::loader::main), title);
		ImGui::PopFont();
	}

	// ---- tab bar ----
	static int selected_tab = 0;

	static const Tab tabs[] = {
		{ "Home",    "A" },
		{ "Session", "B" },
		{ "About",   "C" }
	};
	constexpr int tab_count = 3;

	float tab_height  = 30.f;
	float tab_spacing = 4.f;
	float total_width = size.x * 0.8f;
	float tab_width   = (total_width - tab_spacing * (tab_count - 1)) / tab_count;
	float base_x      = pos.x + (size.x - total_width) * 0.5f;
	float y_base      = pos.y + size.y - tab_height - 20.f;

	static float anim_y     = 0.f;
	static float anim_alpha = 0.f;
	static bool  first      = true;
	if (first) { anim_y = y_base + 40.f; anim_alpha = 0.f; first = false; }
	anim_y     = ImLerp(anim_y,     y_base, 0.12f);
	anim_alpha = ImLerp(anim_alpha, 1.0f,   0.08f);
	float y = anim_y;

	// tab background
	float px = 12.f, py = 10.f;
	drawlist->AddRectFilled(
		ImVec2(base_x - px, y - py), ImVec2(base_x + total_width + px, y + tab_height + py),
		ImGui::GetColorU32(ImVec4(0.067f, 0.071f, 0.078f, anim_alpha)), 6.f);
	drawlist->AddRect(
		ImVec2(base_x - px, y - py), ImVec2(base_x + total_width + px, y + tab_height + py),
		ImGui::GetColorU32(ImVec4(0.082f, 0.086f, 0.094f, anim_alpha)), 6.f);

	// animated selected highlight
	static float anim_x = 0.f;
	static bool  first_x = true;
	if (first_x) { anim_x = base_x; first_x = false; }
	anim_x = ImLerp(anim_x, base_x + selected_tab * (tab_width + tab_spacing), 0.15f);
	ImVec4 sel_col = ui::colors::loader::main; sel_col.w *= anim_alpha;
	drawlist->AddRectFilled(ImVec2(anim_x - 6.f, y - 6.f), ImVec2(anim_x + tab_width + 6.f, y + tab_height + 6.f),
		ImGui::GetColorU32(sel_col), 6.f);
	ImVec4 sel_out = ui::colors::loader::outline_white; sel_out.w *= anim_alpha;
	drawlist->AddRect(ImVec2(anim_x - 6.f, y - 6.f), ImVec2(anim_x + tab_width + 6.f, y + tab_height + 6.f),
		ImGui::GetColorU32(sel_out), 5.f);

	// tab labels
	float tx = base_x;
	for (int i = 0; i < tab_count; i++)
	{
		ImVec2 tmin(tx, y), tmax(tx + tab_width, y + tab_height);
		bool hov = io.MousePos.x >= tmin.x && io.MousePos.x <= tmax.x &&
		           io.MousePos.y >= tmin.y && io.MousePos.y <= tmax.y;
		if (hov) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		if (hov && ImGui::IsMouseClicked(0)) selected_tab = i;

		ImGui::PushFont(font.tab_icon);
		ImVec2 isz = ImGui::CalcTextSize(tabs[i].icon);
		ImGui::PopFont();
		ImGui::PushFont(font.montserrat_semibold[1]);
		ImVec2 nsz = ImGui::CalcTextSize(tabs[i].name);
		ImGui::PopFont();

		float cw = isz.x + 4.f + nsz.x;
		float sx = tmin.x + (tab_width - cw) * 0.5f;
		ImVec4 tc = (i == selected_tab)
			? ImVec4(1, 1, 1, anim_alpha)
			: ImVec4(0.45f, 0.45f, 0.45f, anim_alpha);

		ImGui::PushFont(font.tab_icon);
		drawlist->AddText(ImVec2(sx, tmin.y + (tab_height - isz.y) * 0.5f), ImGui::GetColorU32(tc), tabs[i].icon);
		ImGui::PopFont();
		ImGui::PushFont(font.montserrat_semibold[1]);
		drawlist->AddText(ImVec2(sx + isz.x + 4.f, tmin.y + (tab_height - nsz.y) * 0.5f), ImGui::GetColorU32(tc), tabs[i].name);
		ImGui::PopFont();

		tx += tab_width + tab_spacing;
	}

	// ---- content area ----
	float content_top    = pos.y + 52.f;
	float content_bottom = y - py - 10.f;
	float content_h      = content_bottom - content_top;
	float pad            = 20.f;
	float half_w         = (size.x - pad * 3.f) * 0.5f;

	static float ca_y     = 0.f;
	static float ca_alpha = 0.f;
	static int   last_tab = -1;
	if (last_tab != selected_tab) { ca_y = content_top + 30.f; ca_alpha = 0.f; last_tab = selected_tab; }
	ca_y     = ImLerp(ca_y,     content_top, 0.12f);
	ca_alpha = ImLerp(ca_alpha, 1.0f,        0.08f);

	ImU32 col_main  = ImGui::GetColorU32(ImVec4(ui::colors::loader::main.x, ui::colors::loader::main.y, ui::colors::loader::main.z, ca_alpha));
	ImU32 col_white = ImGui::GetColorU32(ImVec4(1, 1, 1, ca_alpha));
	ImU32 col_dim   = ImGui::GetColorU32(ImVec4(0.6f, 0.6f, 0.6f, ca_alpha));
	ImU32 col_green = ImGui::GetColorU32(ImVec4(0.4f, 1.f, 0.5f, ca_alpha));
	ImU32 col_red   = ImGui::GetColorU32(ImVec4(1.f, 0.4f, 0.4f, ca_alpha));

	// ================================================================
	//  TAB 0 — HOME  (license + user info)
	// ================================================================
	if (selected_tab == 0)
	{
		// --- Left panel: License Info ---
		ImVec2 lmin(pad, ca_y);
		ImVec2 lmax(pad + half_w, ca_y + content_h * 0.48f);
		draw_panel(drawlist, lmin, lmax, "License", font.montserrat_semibold[2]);

		ImVec2 cur(lmin.x + 14.f, lmin.y + 44.f);

		std::string lic = AuthorizedLol::license_key_str.empty() ? "N/A" : AuthorizedLol::license_key_str;
		std::string exp = AuthorizedLol::expires_at.empty()      ? "N/A" : AuthorizedLol::expires_at.substr(0, 10);
		std::string pln = AuthorizedLol::product_name.empty()    ? "N/A" : AuthorizedLol::product_name;
		std::string hwid_short = AuthorizedLol::hwid_str.size() > 16
			? AuthorizedLol::hwid_str.substr(0, 16) + "..." : AuthorizedLol::hwid_str;

		draw_info_row(drawlist, cur, lmax.x, "Key",     lic,        col_main,  col_white, font.montserrat_semibold[1]);
		draw_info_row(drawlist, cur, lmax.x, "Expires", exp,        col_dim,   col_white, font.montserrat_semibold[1]);
		draw_info_row(drawlist, cur, lmax.x, "Plan",    pln,        col_dim,   col_green, font.montserrat_semibold[1]);
		draw_info_row(drawlist, cur, lmax.x, "HWID",    hwid_short, col_dim,   col_dim,   font.montserrat_semibold[1]);

		// --- Right panel: User Info ---
		ImVec2 rmin(pad * 2.f + half_w, ca_y);
		ImVec2 rmax(rmin.x + half_w, lmax.y);
		draw_panel(drawlist, rmin, rmax, "User", font.montserrat_semibold[2]);

		ImVec2 cur2(rmin.x + 14.f, rmin.y + 44.f);

		char winuser[256]; DWORD wlen = 256;
		GetUserNameA(winuser, &wlen);

		std::string uname = AuthorizedLol::username.empty() ? "N/A" : AuthorizedLol::username;
		std::string email = AuthorizedLol::email.empty()    ? "N/A" : AuthorizedLol::email;

		draw_info_row(drawlist, cur2, rmax.x, "Username", uname,                col_dim, col_white, font.montserrat_semibold[1]);
		draw_info_row(drawlist, cur2, rmax.x, "Email",    email,                col_dim, col_white, font.montserrat_semibold[1]);
		draw_info_row(drawlist, cur2, rmax.x, "PC User",  std::string(winuser), col_dim, col_dim,   font.montserrat_semibold[1]);

		// --- Bottom panel: Status ---
		float status_h = content_h * 0.28f;
		ImVec2 smin(pad, lmax.y + 12.f);
		ImVec2 smax(smin.x + size.x - pad * 2.f, smin.y + status_h);
		draw_panel(drawlist, smin, smax, "Status", font.montserrat_semibold[2]);

		ImVec2 sc(smin.x + 14.f, smin.y + 44.f);

		bool session_ok = !AuthorizedLol::session_token.empty();
		draw_info_row(drawlist, sc, smax.x, "Session",    session_ok ? "Active" : "None",                                    col_dim, session_ok ? col_green : col_red, font.montserrat_semibold[1]);
		draw_info_row(drawlist, sc, smax.x, "Product ID", AuthorizedLol::product_id.empty() ? "N/A" : AuthorizedLol::product_id, col_dim, col_dim, font.montserrat_semibold[1]);
	}

	// ================================================================
	//  TAB 1 — SESSION  (validate / heartbeat / logout)
	// ================================================================
	if (selected_tab == 1)
	{
		float btn_h   = 46.f;
		float btn_w   = size.x - pad * 2.f;
		float btn_gap = 12.f;

		// Panel
		float panel_h = btn_h * 3.f + btn_gap * 4.f + 36.f + 16.f;
		ImVec2 pmin(pad, ca_y);
		ImVec2 pmax(pmin.x + btn_w, pmin.y + panel_h);
		draw_panel(drawlist, pmin, pmax, "Session Management", font.montserrat_semibold[2]);

		float bx = pmin.x + (btn_w - (btn_w - 32.f)) * 0.5f;
		float bw = btn_w - 32.f;
		float by = pmin.y + 36.f + btn_gap;

		auto draw_action_btn = [&](const char* label, ImVec2 bmin, ImVec2 bmax, bool danger) -> bool
		{
			bool hov = io.MousePos.x >= bmin.x && io.MousePos.x <= bmax.x &&
			           io.MousePos.y >= bmin.y && io.MousePos.y <= bmax.y;
			if (hov) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

			ImVec4 bg = danger
				? ImVec4(0.6f, 0.1f, 0.1f, ca_alpha)
				: ImVec4(ui::colors::loader::main.x, ui::colors::loader::main.y, ui::colors::loader::main.z, ca_alpha);
			if (hov) { bg.x = ImMin(bg.x * 1.2f, 1.f); bg.y = ImMin(bg.y * 1.2f, 1.f); bg.z = ImMin(bg.z * 1.2f, 1.f); }

			drawlist->AddRectFilled(bmin, bmax, ImGui::GetColorU32(bg), 8.f);
			drawlist->AddRect(bmin, bmax,
				ImGui::GetColorU32(ImVec4(1, 1, 1, 0.08f * ca_alpha)), 8.f);

			ImGui::PushFont(font.montserrat_semibold[1]);
			ImVec2 ts = ImGui::CalcTextSize(label);
			drawlist->AddText(
				ImVec2(bmin.x + (bmax.x - bmin.x - ts.x) * 0.5f,
				       bmin.y + (bmax.y - bmin.y - ts.y) * 0.5f),
				IM_COL32(255, 255, 255, (int)(ca_alpha * 255)), label);
			ImGui::PopFont();

			return hov && ImGui::IsMouseClicked(0);
		};

		// Validate button
		static std::string validate_result;
		static float       validate_flash = 0.f;
		ImVec2 v_min(bx, by), v_max(bx + bw, by + btn_h);
		if (draw_action_btn("Validate Session", v_min, v_max, false))
		{
			validate_result = AuthorizedLol::validate() ? "Session is valid" : "Session invalid or expired";
			validate_flash  = 3.f;
		}
		if (validate_flash > 0.f)
		{
			validate_flash -= io.DeltaTime;
			ImGui::PushFont(font.montserrat_semibold[1]);
			ImVec2 rs = ImGui::CalcTextSize(validate_result.c_str());
			drawlist->AddText(
				ImVec2(bx + (bw - rs.x) * 0.5f, v_max.y + 6.f),
				ImGui::GetColorU32(ImVec4(0.4f, 1.f, 0.5f, ImMin(validate_flash, 1.f) * ca_alpha)),
				validate_result.c_str());
			ImGui::PopFont();
		}
		by += btn_h + btn_gap + (validate_flash > 0.f ? 22.f : 0.f);

		// Heartbeat button
		static float hb_flash = 0.f;
		ImVec2 h_min(bx, by), h_max(bx + bw, by + btn_h);
		if (draw_action_btn("Send Heartbeat", h_min, h_max, false))
		{
			AuthorizedLol::heartbeat();
			hb_flash = 2.f;
		}
		if (hb_flash > 0.f)
		{
			hb_flash -= io.DeltaTime;
			ImGui::PushFont(font.montserrat_semibold[1]);
			const char* hbt = "Heartbeat sent";
			ImVec2 hs = ImGui::CalcTextSize(hbt);
			drawlist->AddText(
				ImVec2(bx + (bw - hs.x) * 0.5f, h_max.y + 6.f),
				ImGui::GetColorU32(ImVec4(0.4f, 1.f, 0.5f, ImMin(hb_flash, 1.f) * ca_alpha)), hbt);
			ImGui::PopFont();
		}
		by += btn_h + btn_gap + (hb_flash > 0.f ? 22.f : 0.f);

		// Logout button
		ImVec2 lo_min(bx, by), lo_max(bx + bw, by + btn_h);
		if (draw_action_btn("Logout", lo_min, lo_max, true))
		{
			AuthorizedLol::logout();
			ui::variables::logged_in        = false;
			ui::variables::login_fade_timer = 0.f;
			ui::variables::loading          = false;
			ui::variables::spinner_alpha_zero = false;
		}
	}

	// ================================================================
	//  TAB 2 — ABOUT  (product / api info)
	// ================================================================
	if (selected_tab == 2)
	{
		float panel_h = content_h * 0.72f;
		ImVec2 pmin(pad, ca_y);
		ImVec2 pmax(pmin.x + size.x - pad * 2.f, pmin.y + panel_h);
		draw_panel(drawlist, pmin, pmax, "About authorized.lol", font.montserrat_semibold[2]);

		ImVec2 cur(pmin.x + 14.f, pmin.y + 44.f);

		draw_info_row(drawlist, cur, pmax.x, "Service",    "authorized.lol",                                                          col_dim, col_main,  font.montserrat_semibold[1]);
		draw_info_row(drawlist, cur, pmax.x, "API",        "https://authorized.lol/api/v1",                                           col_dim, col_dim,   font.montserrat_semibold[1]);
		draw_info_row(drawlist, cur, pmax.x, "Product",    AuthorizedLol::product_name.empty() ? "N/A" : AuthorizedLol::product_name, col_dim, col_white, font.montserrat_semibold[1]);
		draw_info_row(drawlist, cur, pmax.x, "Product ID", AuthorizedLol::product_id.empty()   ? "N/A" : AuthorizedLol::product_id,   col_dim, col_dim,   font.montserrat_semibold[1]);

		cur.y += 10.f;

		// divider
		drawlist->AddLine(ImVec2(pmin.x + 14.f, cur.y), ImVec2(pmax.x - 14.f, cur.y),
			IM_COL32(255, 255, 255, (int)(20 * ca_alpha)));
		cur.y += 14.f;

		ImGui::PushFont(font.montserrat_semibold[1]);
		const char* desc =
			"This loader uses the authorized.lol authentication API.\n"
			"License keys are validated against your application,\n"
			"sessions are tracked server-side, and HWID binding\n"
			"ensures keys can only be used on authorized devices.";

		// word-wrap manually line by line
		std::istringstream ss(desc);
		std::string line;
		while (std::getline(ss, line))
		{
			drawlist->AddText(cur, col_dim, line.c_str());
			cur.y += ImGui::CalcTextSize(line.c_str()).y + 6.f;
		}
		ImGui::PopFont();
	}
}




bool ui::items::input_text(const char* label, ImVec2 pos, ImVec2 Size, char buf[], size_t buf_size, ImGuiInputTextFlags flag)
{
	const float Speed = 5.f;
	const float Rounding = 8.f;

	auto* window = ImGui::GetCurrentWindow();
	auto& style = ImGui::GetStyle();
	auto& io = ImGui::GetIO();

	float time = io.DeltaTime * Speed;
	ImGui::PushFont(font.montserrat_semibold[1]);

	ImVec4 prevFrameBg = style.Colors[ImGuiCol_FrameBg];
	ImVec4 prevTextDisabled = style.Colors[ImGuiCol_TextDisabled];
	ImVec4 prevTextSelectedBg = style.Colors[ImGuiCol_TextSelectedBg];
	ImVec2 prevFramePadding = style.FramePadding;

	style.Colors[ImGuiCol_FrameBg] = colors::input_text::frame_bg;
	style.Colors[ImGuiCol_TextDisabled] = colors::input_text::text_disabled;
	style.Colors[ImGuiCol_TextSelectedBg] = colors::input_text::text_selected_bg;
	style.FramePadding = ImVec2(8.f, 4.f);

	std::string lbl = "###";
	lbl += label;

	ImGui::PushID(label);

	ImGui::SetCursorPos(pos);

	ImGuiID id = window->GetID(label);
	static std::map<ImGuiID, input_state> anim;
	auto& i1 = anim[id];

	ImVec2 MIN = ImGui::GetCursorScreenPos();
	ImVec2 MAX = ImVec2(MIN.x + Size.x, MIN.y + Size.y);

	ImVec4 darker_bg = ImVec4(
		i1.background_color.x * 0.8f,
		i1.background_color.y * 0.8f,
		i1.background_color.z * 0.8f,
		i1.background_color.w * 1.0f
	);

	// Draw drop shadow
	window->DrawList->AddShadowRect(
		MIN, MAX,
		IM_COL32(0, 0, 0, 90),
		4.0f,
		ImVec2(2, 2),
		Rounding,
		ImDrawFlags_RoundCornersAll
	);

	window->DrawList->AddRectFilled(
		MIN, MAX,
		ImGui::GetColorU32(i1.background_color),
		Rounding
	);

	//window->DrawList->AddRect(MIN, MAX, ImGui::GetColorU32(i1.border_color), Rounding, ImDrawFlags_RoundCornersAll, 1.0f);

	ImVec2 labelPos = ImVec2(MIN.x + 4.f, MIN.y - ImGui::GetFontSize() - 8.f);
	window->DrawList->AddText(labelPos, ImGui::GetColorU32(i1.text_color), label);

	ImGui::PushStyleColor(ImGuiCol_Text, i1.text_color);

	float desiredHeight = Size.y;
	float fontSize = ImGui::GetFontSize();
	float framePaddingY = (desiredHeight - fontSize) * 0.5f;
	framePaddingY = framePaddingY < 0 ? 0 : framePaddingY;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, framePaddingY));
	ImGui::SetNextItemWidth(Size.x);
	bool result = ImGui::InputTextWithHint(lbl.c_str(), "", buf, (int)buf_size, flag);
	ImGui::PopStyleVar();

	ImVec4 Background_Target = ImGui::IsItemActive() ? colors::input_text::background_active : ImGui::IsItemHovered() ? colors::input_text::background_hovered : colors::input_text::background;
	ImVec4 Border_Target = ImGui::IsItemActive() ? colors::input_text::border_active : ImGui::IsItemHovered() || strlen(buf) != 0 ? colors::input_text::border : colors::input_text::border;
	ImVec4 Text_Color_Target = ImGui::IsItemActive() ? colors::input_text::text_active : ImGui::IsItemHovered() || strlen(buf) != 0 ? colors::input_text::text_hovered : colors::input_text::text;

	i1.background_color = ImVec4(ImLerp(i1.background_color, Background_Target, time));
	i1.border_color = ImVec4(ImLerp(i1.border_color, Border_Target, time));
	i1.text_color = ImVec4(ImLerp(i1.text_color, Text_Color_Target, time));

	ImGui::PopStyleColor();
	ImGui::PopID();

	style.Colors[ImGuiCol_FrameBg] = prevFrameBg;
	style.Colors[ImGuiCol_TextDisabled] = prevTextDisabled;
	style.Colors[ImGuiCol_TextSelectedBg] = prevTextSelectedBg;
	style.FramePadding = prevFramePadding;
	ImGui::PopFont();

	return result;
}

struct check_state {
	ImVec4 box_color, check_color, text_color;
	float check_alpha = 0.f;
	float check_progress = 0.f;
	float check_offset_y = -1.0f;
};

bool ui::items::checkbox(const char* label, ImVec2 pos, ImVec2 size, bool* active, int offset_x)
{
	auto* window = ImGui::GetCurrentWindow();
	auto& io = ImGui::GetIO();
	if (!window || !label || !active) return false;

	ImGui::PushFont(font.montserrat_semibold[3]);
	ImGui::PushID(label);

	auto id = window->GetID(label);
	static std::map<ImGuiID, check_state> anim;
	auto& state = anim[id];

	// Texto
	ImVec2 text_size = ImGui::CalcTextSize(label);
	ImVec2 text_pos = {
		pos.x - 10.0f + offset_x,
		pos.y + (size.y - text_size.y) / 2.0f - 2
	};

	// Posición checkbox
	const float box_size = 18.0f;
	ImVec2 box_pos = { pos.x + 56.0f + size.x - box_size, pos.y + (size.y - box_size) / 2.0f - 1.0f };

	// Invisible button
	ImGui::SetCursorPos(box_pos);
	bool pressed = ImGui::InvisibleButton(label, ImVec2(box_size, box_size));
	if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	if (pressed) *active = !*active;

	// Animación
	float delta = io.DeltaTime;
	float speed = 10.f;

	// Fondo del checkbox
	ImVec4 target_box_color = pressed ? ui::colors::loader::main : ui::colors::checkbox::bg;

	// Checkmark siempre blanco
	ImVec4 target_circle_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	// Lerping de animación
	state.box_color = ImLerp(state.box_color, target_box_color, delta * speed);
	state.check_color = ImLerp(state.check_color, target_circle_color, delta * speed);
	state.check_alpha = ImLerp(state.check_alpha, *active ? 1.0f : 0.0f, delta * speed);
	state.check_progress = ImLerp(state.check_progress, *active ? 1.0f : 0.0f, delta * speed);

	// Fondo animado
	static std::map<ImGuiID, ImVec4> bg_anim;
	ImVec4& state_bg = bg_anim[id];
	ImVec4 target_bg = *active ? ui::colors::loader::main : ui::colors::checkbox::bg;
	state_bg = ImLerp(state_bg, target_bg, io.DeltaTime * 10.0f);
	window->DrawList->AddRectFilled(box_pos, box_pos + ImVec2(box_size + 1, box_size), ImGui::GetColorU32(state_bg), 4);

	// Dibujar borde
	window->DrawList->AddRect(box_pos, box_pos + ImVec2(box_size + 1, box_size), ImGui::GetColorU32(ui::colors::checkbox::border), 4);

	if (*active) // dibuja solo si está activo
	{

		float check_size = (10.5 > 0.0f ? 10.5 : box_size - 4.0f); // usar variable si se pasa
		ImVec2 check_pos = box_pos + ImVec2((box_size - check_size) / 2 + 1, (box_size - check_size) / 2); // centrar checkmark

		ImU32 check_col = IM_COL32(255, 255, 255, 255);

		ImGui::RenderCheckMark(
			ImGui::GetWindowDrawList(),
			check_pos,
			check_col,
			check_size
		);
	}

	// Dibujar texto
	ImU32 text_col = ImGui::GetColorU32(ui::colors::checkbox::text);
	window->DrawList->AddText(text_pos, text_col, label);

	ImGui::PopFont();
	ImGui::PopID();
	return pressed;
}


struct button_state {
	ImVec4 background_color = ImVec4(0, 0, 0, 0);
	ImVec4 border_color = ImVec4(0, 0, 0, 0);
	ImVec4 text_color = ImVec4(0, 0, 0, 0);
	float hover_expand = 0.0f;
};

bool ui::items::button(const char* label, ImVec2 pos, ImVec2 size)
{
	const float Speed = 5.f;
	const float Rounding = 8.f;

	ImGui::PushFont(font.montserrat_semibold[2]);

	auto* window = ImGui::GetCurrentWindow();
	auto& io = ImGui::GetIO();
	float delta_time = io.DeltaTime;

	ImGui::PushID(label);
	ImGui::SetCursorPos(pos);

	ImVec2 MIN = ImGui::GetCursorScreenPos();
	ImVec2 MAX = ImVec2(MIN.x + size.x, MIN.y + size.y);

	ImGuiID id = window->GetID(label);
	static std::map<ImGuiID, button_state> anim;

	auto& i1 = anim[id];

	ImRect total_rect(MIN, MAX);
	ImGui::InvisibleButton(label, size);

	bool is_hovered = ImGui::IsItemHovered();
	bool is_active = ImGui::IsItemActive();

	if (is_hovered)
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

	ImVec4 Background_Target = is_hovered ? ui::colors::input_text::background_hovered : ui::colors::input_text::background;
	ImVec4 Border_Target = ui::colors::input_text::border;
	ImVec4 Text_Color_Target = is_hovered ? ui::colors::input_text::text_hovered : ui::colors::input_text::text;

	float lerp_time = delta_time * Speed;
	i1.background_color = ImVec4(ImLerp(i1.background_color, Background_Target, lerp_time));
	i1.border_color = ImVec4(ImLerp(i1.border_color, Border_Target, lerp_time));
	i1.text_color = ImVec4(ImLerp(i1.text_color, Text_Color_Target, lerp_time));

	window->DrawList->AddRectFilled(
		ImVec2(MIN.x + 1, MIN.y + 1),
		ImVec2(MAX.x - 1, MAX.y - 1),
		ImGui::GetColorU32(i1.background_color),
		Rounding
	);

	int max_expand = static_cast<int>(size.x * 0.5f);
	float hover_lerp_time = delta_time * 8;
	float target_expand = is_hovered ? max_expand : 0.0f;

	i1.hover_expand = ImLerp(i1.hover_expand, target_expand, hover_lerp_time);

	if (i1.hover_expand < 4.0f)
		i1.hover_expand = 0.0f;

	int hover_expand_int = static_cast<int>(roundf(i1.hover_expand));
	int half_expand = hover_expand_int / 2;

	ImVec2 center = ImVec2((MIN.x + MAX.x) / 2, (MIN.y + MAX.y) / 2);

	if (hover_expand_int > 0 && label == "Login") {
		window->DrawList->AddRectFilled(
			ImVec2(center.x - half_expand, MIN.y + 1),
			ImVec2(center.x + half_expand, MAX.y - 1),
			ImGui::GetColorU32(ImVec4(
				ui::colors::loader::main.x,
				ui::colors::loader::main.y,
				ui::colors::loader::main.z,
				0.6f)),
			8
		);
	}

	window->DrawList->AddRect(
		ImVec2(MIN.x + 1, MIN.y + 1),
		ImVec2(MAX.x - 1, MAX.y - 1),
		ImGui::GetColorU32(i1.border_color),
		Rounding - 2,
		0,
		1.0f
	);

	ImVec2 text_size = ImGui::CalcTextSize(label);
	ImVec2 text_pos = ImVec2(
		MIN.x + (size.x - text_size.x) * 0.5f,
		MIN.y + (size.y - text_size.y) * 0.5f - 1
	);

	window->DrawList->AddText(text_pos, ImGui::GetColorU32(i1.text_color), label);

	ImGui::PopFont();

	ImGui::PopID();
	return ImGui::IsItemClicked();
}

