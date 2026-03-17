<div align="center">

<img src="https://files.catbox.moe/zsyt8f.png" width="64" height="64" alt="authorized.lol logo" />

# authorized.lol — C++ Loader Example

**A fully functional ImGui/DirectX 11 loader template integrated with the [authorized.lol](https://authorized.lol) licensing API.**

[![Discord](https://img.shields.io/discord/672CnjSEB5?label=Discord&logo=discord&logoColor=white&color=5865F2)](https://discord.gg/672CnjSEB5)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](#license)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](#requirements)
[![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)](#requirements)

</div>

---

## Overview

This repository is an open-source example demonstrating how to build a professional software loader using the **authorized.lol** authentication API. It handles license validation, HWID binding, session management, and provides a polished ImGui-based UI — all ready to drop your own product into.

> Use this as a starting point for your own loader. Swap in your API key, customize the UI, and ship.

---

## Features

- **License authentication** via the authorized.lol REST API
- **HWID binding** — keys are locked to the machine on first use
- **Session management** — validate, heartbeat, and logout support
- **Fullscreen loading animation** with staged status text during auth
- **Smooth UI transitions** — fade-in login, animated tab bar, slide-in content
- **Notification system** — success, error, and info toasts
- **DirectX 11 + ImGui** rendering with FreeType font rasterization
- **Static libcurl** — no external DLL dependencies
- **Single header integration** — drop `authorized_lol.h` into any project

---

## Preview

| Login Screen | Main Dashboard |
|---|---|
| License key input with animated background | Home, Session, and About tabs |

---

## Requirements

| Dependency | Version |
|---|---|
| Windows SDK | 10.0+ |
| Visual Studio | 2019 or 2022 |
| DirectX 11 | Included in Windows SDK |
| libcurl (static) | 8.x (`libcurl_a.lib`) |
| FreeType | 2.x (`freetype64.lib`) |
| nlohmann/json | 3.x (header-only, included) |
| ImGui | 1.90.x (included) |

---

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/your-org/authorized-lol-example.git
cd authorized-lol-example
```

### 2. Set your API key

Open `Framework/src/Sources/main.cpp` and replace the placeholder:

```cpp
static constexpr const char* AUTHORIZED_LOL_API_KEY = "your_api_key_here";
```

You can find your API key in the [authorized.lol dashboard](https://authorized.lol/dashboard).

### 3. Build

Open `authorized_lol.sln` in Visual Studio, set the configuration to **Release x64**, and build.

---

## Project Structure

```
authorized.lol Example/
├── Framework/
│   ├── authorized_lol.h        # API client (init, validate, heartbeat, logout)
│   ├── gui.h                   # UI state, colors, alpha, font declarations
│   ├── gui.cpp                 # All rendering logic
│   ├── ext/
│   │   ├── ImGui/              # ImGui + FreeType backend
│   │   ├── freetype/           # FreeType headers + libs
│   │   └── SDK/                # DirectX headers
│   └── src/Sources/
│       └── main.cpp            # WinMain, render loop, D3D11 setup
└── authorized_lol.sln
```

---

## API Integration

The `authorized_lol.h` header wraps the full authorized.lol v1 API:

```cpp
// Initialize a session with a license key
bool AuthorizedLol::init(const std::string& license_key, std::string& error_msg);

// Check if the current session is still valid
bool AuthorizedLol::validate();

// Send a keepalive heartbeat (call every ~5 minutes)
void AuthorizedLol::heartbeat();

// Terminate the session server-side
void AuthorizedLol::logout();
```

After a successful `init()`, session data is available globally:

```cpp
AuthorizedLol::username        // authenticated username
AuthorizedLol::email           // account email
AuthorizedLol::expires_at      // license expiry date
AuthorizedLol::product_name    // product name from dashboard
AuthorizedLol::hwid_str        // bound HWID
AuthorizedLol::session_token   // active session token
```

---

## Configuration

| Field | Location | Description |
|---|---|---|
| `AUTHORIZED_LOL_API_KEY` | `main.cpp` | Your application API key |
| `BASE_URL` | `authorized_lol.h` | API base URL (default: `https://authorized.lol/api/v1`) |
| Window title | `main.cpp` ImGui window name | Shown in the loader title bar |
| UI colors | `gui.h` → `ui::colors::loader` | Accent, background, outline colors |

---

## Links

- Website: [https://authorized.lol](https://authorized.lol)
- Dashboard: [https://authorized.lol/dashboard](https://authorized.lol/dashboard)
- API Docs: [https://authorized.lol/docs](https://authorized.lol/docs)
- Discord: [https://discord.gg/672CnjSEB5](https://discord.gg/672CnjSEB5)

---

## License

This example is released under the [MIT License](LICENSE). You are free to use, modify, and distribute it in your own projects.

---

<div align="center">

Built with the [authorized.lol](https://authorized.lol) API &nbsp;·&nbsp; [Join our Discord](https://discord.gg/672CnjSEB5)

</div>
