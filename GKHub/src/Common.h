#pragma once
#include <string>
#include <vector>
#include <functional>
#include <filesystem>

namespace GK {

// ─── 버전 (build.yml env.GK_VERSION 에서 주입) ────────────────
#ifndef GK_VERSION
#  define GK_VERSION "1.0"
#endif

// ─── manifest URL ─────────────────────────────────────────────
// ★ 레포 이름 바꿀 경우 GKHub/CMakeLists.txt 의 GK_MANIFEST_URL 수정
#ifndef GK_MANIFEST_URL
#  define GK_MANIFEST_URL \
    "https://raw.githubusercontent.com/reders0412-rgb/GKEngine/main/versions.json"
#endif

#define GK_EDITORS_SUBDIR "Editors"

// ─── Engine version ───────────────────────────────────────────
struct EngineVersion {
    std::string tag;
    std::string displayName;
    std::string downloadUrl;
    std::string releaseNotes;
    bool        installed = false;
};

// ─── Project ──────────────────────────────────────────────────
enum class ProjectTemplate { Template2D, Template3D };

struct ProjectInfo {
    std::string           name;
    std::filesystem::path path;
    std::string           engineVersion;
    ProjectTemplate       templ      = ProjectTemplate::Template3D;
    std::string           lastOpened;
};

// ─── Template (from versions.json["templates"]) ───────────────
struct TemplateInfo {
    std::string id;           // "builtin-3d", "community-fps", ...
    std::string name;         // en display name
    std::string nameKo;       // ko display name
    std::string description;
    std::string descriptionKo;
    std::string author;
    std::string thumbnailUrl; // remote URL
    std::string downloadUrl;  // zip URL (empty = builtin)
    bool        builtin      = false;
    std::string type;         // "2D" | "3D" | "Custom"
    std::vector<std::string> tags;

    // set at runtime after thumbnail download
    std::filesystem::path thumbnailCachePath;
    bool thumbnailReady = false;

    // set after zip is cached in Library/
    bool cachedLocally = false;
    std::filesystem::path cacheDir; // Library/<id>/
};

// ─── User Profile ─────────────────────────────────────────────
struct UserProfile {
    std::string username;
    std::string displayName;
    std::string avatarUrl;
    std::filesystem::path avatarCachePath;
    bool        avatarReady = false;
};

// ─── Runtime paths ────────────────────────────────────────────
struct Paths {
    static std::filesystem::path exeDir();
    static std::filesystem::path editorsDir();
    static std::filesystem::path projectsFile();
    static std::filesystem::path settingsFile();
    static std::filesystem::path engineSettingsFile();

    // Template library cache: exe/Library/templates/<id>/
    static std::filesystem::path templateLibraryDir();
    static std::filesystem::path templateCacheDir(const std::string& id);
    // Avatar cache: exe/Library/avatars/<username>.png
    static std::filesystem::path avatarCacheDir();
};

using ProgressCallback = std::function<void(float pct, const std::string& msg)>;

} // namespace GK
