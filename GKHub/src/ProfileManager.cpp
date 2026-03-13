#include "ProfileManager.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <fstream>
#include <thread>
#include <iostream>

namespace GK {

static size_t curlWriteStr(void* d, size_t s, size_t n, std::string* o) {
    o->append((char*)d, s*n); return s*n;
}
static size_t curlWriteFile(void* d, size_t s, size_t n, FILE* f) {
    return fwrite(d, s, n, f);
}

ProfileManager& ProfileManager::instance() {
    static ProfileManager inst; return inst;
}

// ─── loadFromSettings ─────────────────────────────────────────
void ProfileManager::loadFromSettings(
    const std::string& username,
    const std::string& displayName,
    const std::string& avatarUrl)
{
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_profile.username    = username;
        m_profile.displayName = displayName.empty() ? username : displayName;
        m_profile.avatarUrl   = avatarUrl;
    }

    if (!avatarUrl.empty()) {
        downloadAvatarAsync(avatarUrl);
    } else if (!username.empty()) {
        // username 만 있으면 GitHub API 로 자동 조회
        fetchFromGitHub(username);
    }
}

// ─── fetchFromGitHub ─────────────────────────────────────────
void ProfileManager::fetchFromGitHub(const std::string& username) {
    if (username.empty()) return;
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_profile.username = username;
    }

    std::thread([this, username]() {
        // GitHub API: GET /users/<username>
        std::string apiUrl = "https://api.github.com/users/" + username;
        std::string body;

        CURL* c = curl_easy_init();
        curl_easy_setopt(c, CURLOPT_URL,           apiUrl.c_str());
        curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, curlWriteStr);
        curl_easy_setopt(c, CURLOPT_WRITEDATA,     &body);
        curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION,1L);
        curl_easy_setopt(c, CURLOPT_TIMEOUT,       15L);
        curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER,1L);
        // GitHub API requires User-Agent
        struct curl_slist* hdrs = nullptr;
        hdrs = curl_slist_append(hdrs, "User-Agent: GKHub/1.0");
        curl_easy_setopt(c, CURLOPT_HTTPHEADER, hdrs);
        CURLcode res = curl_easy_perform(c);
        curl_slist_free_all(hdrs);
        curl_easy_cleanup(c);

        if (res != CURLE_OK) {
            std::cerr << "[ProfileManager] GitHub API failed: " << curl_easy_strerror(res) << "\n";
            return;
        }

        try {
            auto j = nlohmann::json::parse(body);
            std::string avatarUrl   = j.value("avatar_url", "");
            std::string displayName = j.value("name",       username);
            {
                std::lock_guard<std::mutex> lk(m_mutex);
                m_profile.avatarUrl   = avatarUrl;
                m_profile.displayName = displayName.empty() ? username : displayName;
            }
            if (!avatarUrl.empty()) downloadAvatarAsync(avatarUrl);
        } catch (...) {}
    }).detach();
}

// ─── downloadAvatarAsync ─────────────────────────────────────
void ProfileManager::downloadAvatarAsync(const std::string& url) {
    std::string username;
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        username = m_profile.username;
        m_profile.avatarReady = false;
    }

    std::thread([this, url, username]() {
        auto cacheDir = Paths::avatarCacheDir();
        std::filesystem::create_directories(cacheDir);
        auto dest = cacheDir / (username + ".png");

        // 이미 캐시 있으면 즉시 사용
        if (std::filesystem::exists(dest)) {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_profile.avatarCachePath = dest;
            m_profile.avatarReady     = true;
            for (auto& [id, cb] : m_cbs) cb();
            return;
        }

        FILE* f = fopen(dest.string().c_str(), "wb");
        if (!f) return;

        CURL* c = curl_easy_init();
        curl_easy_setopt(c, CURLOPT_URL,           url.c_str());
        curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, curlWriteFile);
        curl_easy_setopt(c, CURLOPT_WRITEDATA,     f);
        curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION,1L);
        curl_easy_setopt(c, CURLOPT_TIMEOUT,       20L);
        curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER,1L);
        CURLcode res = curl_easy_perform(c);
        curl_easy_cleanup(c);
        fclose(f);

        if (res != CURLE_OK) {
            std::filesystem::remove(dest);
            return;
        }

        std::lock_guard<std::mutex> lk(m_mutex);
        m_profile.avatarCachePath = dest;
        m_profile.avatarReady     = true;
        for (auto& [id, cb] : m_cbs) cb();
    }).detach();
}

void ProfileManager::setDisplayName(const std::string& name) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_profile.displayName = name;
}
void ProfileManager::setAvatarUrl(const std::string& url) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_profile.avatarUrl = url;
    if (!url.empty()) downloadAvatarAsync(url);
}

int ProfileManager::onAvatarReady(AvatarCallback cb) {
    int id = m_nextId++; m_cbs[id] = std::move(cb); return id;
}
void ProfileManager::removeCallback(int id) { m_cbs.erase(id); }

} // namespace GK
