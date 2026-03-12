#include "SettingsManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace GK {

SettingsManager::SettingsManager() = default;

void SettingsManager::load() {
    auto p = settingsPath();
    if (!std::filesystem::exists(p)) return;
    std::ifstream in(p);
    try {
        auto j = nlohmann::json::parse(in);
        m_s.language          = j.value("language","en")=="ko" ? Language::Korean : Language::English;
        auto tc               = j.value("theme","dark");
        m_s.theme             = tc=="light" ? Theme::Light : tc=="system" ? Theme::System : Theme::Dark;
        m_s.syncThemeToEngine = j.value("syncTheme",true);
        m_s.syncLangToEngine  = j.value("syncLang",true);
        m_s.autoCheckUpdates  = j.value("autoCheck",true);
        m_s.manifestUrl       = j.value("manifestUrl", std::string(GK_MANIFEST_URL));
    } catch(const std::exception& e) {
        std::cerr << "[Settings] " << e.what() << "\n";
    }
}

void SettingsManager::save() {
    nlohmann::json j;
    j["language"]    = I18n::instance().languageCode();
    j["theme"]       = ThemeManager::instance().themeCode();
    j["syncTheme"]   = m_s.syncThemeToEngine;
    j["syncLang"]    = m_s.syncLangToEngine;
    j["autoCheck"]   = m_s.autoCheckUpdates;
    j["manifestUrl"] = m_s.manifestUrl;
    std::ofstream(settingsPath()) << j.dump(2);
    if (m_s.syncThemeToEngine || m_s.syncLangToEngine)
        writeEngineSettings();
}

void SettingsManager::apply() {
    I18n::instance().setLanguage(m_s.language);
    ThemeManager::instance().setTheme(m_s.theme);
}

void SettingsManager::writeEngineSettings() {
    nlohmann::json j;
    auto& i18n = I18n::instance();
    if (m_s.syncLangToEngine)  j["language"] = i18n.languageCode();
    if (m_s.syncThemeToEngine) j["theme"]    = ThemeManager::instance().themeCode();

    nlohmann::json strings;
    static const std::vector<std::string> keys = {
        "engine.hierarchy","engine.inspector","engine.project","engine.console",
        "engine.scene","engine.game","engine.play","engine.pause","engine.step",
        "engine.addComponent","engine.transform","engine.position",
        "engine.rotation","engine.scale","engine.camera"
    };
    for (auto& k : keys) strings[k] = i18n.tr(k);
    j["strings"] = strings;

    std::ofstream(engineSettingsPath()) << j.dump(2);
}

std::filesystem::path SettingsManager::settingsPath() const {
    return std::filesystem::current_path() / "settings.json";
}
std::filesystem::path SettingsManager::engineSettingsPath() const {
    return std::filesystem::current_path() / "engine_settings.json";
}

} // namespace GK
