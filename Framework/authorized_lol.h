#pragma once
#include <string>
#include <functional>

// -------------------------------------------------------
//  authorized.lol  –  C++ integration header
//  Uses libcurl (already linked in the project) + nlohmann/json
// -------------------------------------------------------

#define CURL_STATICLIB
#include <curl/curl.h>
#include "json.hpp"

namespace AuthorizedLol
{
    // ---- configuration ----
    inline const std::string BASE_URL  = "https://authorized.lol/api/v1";
    inline std::string       API_KEY   = "";   // set before calling init()

    // ---- session state ----
    inline std::string session_token   = "";
    inline std::string username        = "";
    inline std::string email           = "";
    inline std::string license_key_str = "";
    inline std::string expires_at      = "";
    inline std::string product_name    = "";
    inline std::string product_id      = "";
    inline std::string hwid_str        = "";

    // ---- helpers ----
    namespace detail
    {
        static size_t write_cb(char* ptr, size_t size, size_t nmemb, std::string* out)
        {
            out->append(ptr, size * nmemb);
            return size * nmemb;
        }

        // Generate a simple HWID from the machine name + volume serial
        inline std::string get_hwid()
        {
            char buf[MAX_COMPUTERNAME_LENGTH + 1] = {};
            DWORD len = sizeof(buf);
            GetComputerNameA(buf, &len);

            DWORD serial = 0;
            GetVolumeInformationA("C:\\", nullptr, 0, &serial, nullptr, nullptr, nullptr, 0);

            char hwid[64];
            snprintf(hwid, sizeof(hwid), "%s-%08X", buf, serial);
            return std::string(hwid);
        }

        // Perform a POST request, return parsed JSON (or empty object on error)
        inline nlohmann::json post(const std::string& endpoint, const nlohmann::json& body)
        {
            CURL* curl = curl_easy_init();
            if (!curl) return {};

            std::string response;
            std::string url     = BASE_URL + endpoint;
            std::string payload;
            
            try { payload = body.dump(); } catch (...) { curl_easy_cleanup(curl); return {}; }

            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "Accept: application/json");

            curl_easy_setopt(curl, CURLOPT_URL,             url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS,      payload.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,   (long)payload.size());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER,      headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,   write_cb);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA,       &response);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,  0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,  0L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT,         15L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,  10L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,  1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT,       "authorized-lol-client/1.0");

            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK)
            {
                // Return a JSON error so the caller can surface the curl error
                return nlohmann::json{
                    { "success", false },
                    { "message", std::string("curl error: ") + curl_easy_strerror(res) }
                };
            }

            try   { return nlohmann::json::parse(response); }
            catch (...) { return { { "success", false }, { "message", "Invalid server response" } }; }
        }

        // Perform a GET request with Bearer auth header
        inline nlohmann::json get_bearer(const std::string& endpoint)
        {
            CURL* curl = curl_easy_init();
            if (!curl) return {};

            std::string response;
            std::string url = BASE_URL + endpoint;
            std::string auth_header = "Authorization: Bearer " + API_KEY;

            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, auth_header.c_str());

            curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  write_cb);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &response);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT,        15L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            curl_easy_perform(curl);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            try   { return nlohmann::json::parse(response); }
            catch (...) { return {}; }
        }
    }

    // -------------------------------------------------------
    //  init  –  validate license key and open a session
    //  Returns true on success; error_msg is set on failure.
    // -------------------------------------------------------
    inline bool init(const std::string& license_key, std::string& error_msg)
    {
        hwid_str = detail::get_hwid();

        nlohmann::json body = {
            { "api_key",     API_KEY     },
            { "license_key", license_key },
            { "hwid",        hwid_str    }
        };

        auto res = detail::post("/init", body);

        if (res.value("success", false))
        {
            auto safe_str = [&](const std::string& key, const std::string& fallback = "") -> std::string {
                if (res.contains(key) && res[key].is_string()) return res[key].get<std::string>();
                return fallback;
            };

            session_token   = safe_str("session_token");
            username        = safe_str("username");
            email           = safe_str("email");
            license_key_str = safe_str("license_key", license_key);
            expires_at      = safe_str("license_expires_at");
            product_name    = safe_str("product_name");
            product_id      = safe_str("product_id");

            // Also check nested product object for missing fields
            if (res.contains("product") && res["product"].is_object()) {
                auto& p = res["product"];
                if (product_name.empty() && p.contains("product_name") && p["product_name"].is_string())
                    product_name = p["product_name"].get<std::string>();
                if (product_id.empty() && p.contains("id") && p["id"].is_string())
                    product_id = p["id"].get<std::string>();
            }

            return true;
        }

        // Safe error message extraction
        if (res.contains("message") && res["message"].is_string())
            error_msg = res["message"].get<std::string>();
        else
            error_msg = "Authentication failed";
        return false;
    }

    // -------------------------------------------------------
    //  validate  –  check if the current session is still valid
    // -------------------------------------------------------
    inline bool validate()
    {
        if (session_token.empty()) return false;

        auto res = detail::post("/validate", {
            { "api_key",       API_KEY       },
            { "session_token", session_token }
        });

        return res.value("success", false);
    }

    // -------------------------------------------------------
    //  heartbeat  –  keep session alive (call every ~5 min)
    // -------------------------------------------------------
    inline void heartbeat()
    {
        if (session_token.empty()) return;

        detail::post("/heartbeat", {
            { "api_key",       API_KEY       },
            { "session_token", session_token }
        });
    }

    // -------------------------------------------------------
    //  logout  –  terminate the session
    // -------------------------------------------------------
    inline void logout()
    {
        if (session_token.empty()) return;

        detail::post("/logout", {
            { "api_key",       API_KEY       },
            { "session_token", session_token }
        });

        session_token.clear();
    }

} // namespace AuthorizedLol
