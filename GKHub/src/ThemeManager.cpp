#include "ThemeManager.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace GK {

ThemeManager& ThemeManager::instance() { static ThemeManager inst; return inst; }

void ThemeManager::setTheme(Theme t) {
    m_theme = t;
    for (auto& [id, cb] : m_callbacks) cb(t);
}

Theme ThemeManager::effectiveTheme() const {
    if (m_theme != Theme::System) return m_theme;
    return detectSystemTheme();
}

Theme ThemeManager::detectSystemTheme() const {
#ifdef _WIN32
    HKEY key; DWORD val = 1, sz = sizeof(DWORD);
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        RegQueryValueExA(key,"AppsUseLightTheme",nullptr,nullptr,(LPBYTE)&val,&sz);
        RegCloseKey(key);
    }
    return (val == 0) ? Theme::Dark : Theme::Light;
#else
    return Theme::Dark;
#endif
}

std::string ThemeManager::themeCode() const {
    switch (m_theme) {
        case Theme::System: return "system";
        case Theme::Light:  return "light";
        default:            return "dark";
    }
}

void ThemeManager::setThemeFromCode(const std::string& c) {
    if      (c=="light")  setTheme(Theme::Light);
    else if (c=="system") setTheme(Theme::System);
    else                  setTheme(Theme::Dark);
}

std::string ThemeManager::activeThemeRcssPath() const {
    return effectiveTheme() == Theme::Light
        ? "assets/ui/theme-light.rcss"
        : "assets/ui/theme-dark.rcss";
}

int  ThemeManager::onThemeChanged(std::function<void(Theme)> cb) {
    int id = m_nextId++; m_callbacks[id] = std::move(cb); return id;
}
void ThemeManager::removeCallback(int id) { m_callbacks.erase(id); }

} // namespace GK
