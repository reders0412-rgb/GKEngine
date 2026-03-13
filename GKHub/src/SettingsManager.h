#pragma once
#include "I18n.h"
#include "ThemeManager.h"
#include <string>
#include <filesystem>

namespace GK {

struct Settings {
    Language    language           = Language::English;
    Theme       theme              = Theme::Dark;
    bool        syncThemeToEngine  = true;
    bool        syncLangToEngine   = true;
    bool        autoCheckUpdates   = true;
    std::string manifestUrl        = GK_MANIFEST_URL;
    // Profile
    std::string username;
    std::string displayName;
    std::string avatarUrl;
    std::string defaultProjectsPath;
};

class SettingsManager {
public:
    SettingsManager();
    void load();
    void save();
    void apply();
    void writeEngineSettings();

    Settings&       settings()       { return m_s; }
    const Settings& settings() const { return m_s; }

private:
    Settings m_s;
    std::filesystem::path settingsPath() const;
    std::filesystem::path engineSettingsPath() const;
};

} // namespace GK
