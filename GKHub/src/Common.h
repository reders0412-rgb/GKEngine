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

// ─── Runtime paths ────────────────────────────────────────────
struct Paths {
    static std::filesystem::path exeDir();
    static std::filesystem::path editorsDir();
    static std::filesystem::path projectsFile();
    static std::filesystem::path settingsFile();
    static std::filesystem::path engineSettingsFile();
};

using ProgressCallback = std::function<void(float pct, const std::string& msg)>;

} // namespace GK
