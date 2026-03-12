#pragma once
#include <string>
#include <functional>
#include <unordered_map>

namespace GK {

enum class Theme { System, Dark, Light };

class ThemeManager {
public:
    static ThemeManager& instance();

    void  setTheme(Theme t);
    Theme theme() const { return m_theme; }
    Theme effectiveTheme() const;

    std::string themeCode() const;
    void setThemeFromCode(const std::string& code);
    std::string activeThemeRcssPath() const;

    int  onThemeChanged(std::function<void(Theme)> cb);
    void removeCallback(int id);

private:
    ThemeManager() = default;
    Theme detectSystemTheme() const;

    Theme m_theme = Theme::Dark;
    std::unordered_map<int, std::function<void(Theme)>> m_callbacks;
    int m_nextId = 0;
};

} // namespace GK
