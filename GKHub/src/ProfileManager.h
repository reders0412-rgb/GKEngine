#pragma once
#include "Common.h"
#include <functional>
#include <mutex>

namespace GK {

// ─────────────────────────────────────────────────────────────
//  ProfileManager
//
//  설정에서 username + avatarUrl 읽어서 아바타 즉시 다운로드.
//  GitHub API 에서 username 으로 avatar_url 자동 조회 가능.
// ─────────────────────────────────────────────────────────────
class ProfileManager {
public:
    static ProfileManager& instance();

    // 설정에서 프로필 로드 후 아바타 즉시 비동기 다운로드
    void loadFromSettings(const std::string& username,
                          const std::string& displayName,
                          const std::string& avatarUrl);

    // GitHub username 으로 아바타 URL 자동 조회 + 다운로드
    void fetchFromGitHub(const std::string& username);

    // 프로필 저장
    void setDisplayName(const std::string& name);
    void setAvatarUrl(const std::string& url);

    const UserProfile& profile() const { return m_profile; }

    using AvatarCallback = std::function<void()>;
    int  onAvatarReady(AvatarCallback cb);
    void removeCallback(int id);

private:
    ProfileManager() = default;
    void downloadAvatarAsync(const std::string& url);

    UserProfile m_profile;
    std::mutex  m_mutex;
    std::unordered_map<int, AvatarCallback> m_cbs;
    int m_nextId = 0;
};

} // namespace GK
