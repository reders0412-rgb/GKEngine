#pragma once
#include <string>
#include <unordered_map>
#include <functional>

namespace GK {

enum class Language { English, Korean };

class I18n {
public:
    static I18n& instance();

    void setLanguage(Language lang);
    Language language() const { return m_lang; }
    const std::string& tr(const std::string& key) const;

    int  onLanguageChanged(std::function<void(Language)> cb);
    void removeCallback(int id);

    std::string languageCode() const;
    void setLanguageFromCode(const std::string& code);

private:
    I18n();
    void loadEnglish();
    void loadKorean();

    Language m_lang = Language::English;
    std::unordered_map<std::string, std::string> m_strings;
    std::unordered_map<int, std::function<void(Language)>> m_callbacks;
    int m_nextId = 0;
};

inline const std::string& tr(const std::string& key) {
    return I18n::instance().tr(key);
}

} // namespace GK
