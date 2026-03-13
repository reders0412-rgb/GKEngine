#include "I18n.h"
#include <iostream>

namespace GK {

I18n& I18n::instance() { static I18n inst; return inst; }
I18n::I18n() { loadEnglish(); }

void I18n::setLanguage(Language lang) {
    m_lang = lang;
    if (lang == Language::Korean) loadKorean();
    else                          loadEnglish();
    for (auto& [id, cb] : m_callbacks) cb(lang);
}

const std::string& I18n::tr(const std::string& key) const {
    auto it = m_strings.find(key);
    if (it != m_strings.end()) return it->second;
    static std::string miss;
    miss = "[" + key + "]";
    return miss;
}

int I18n::onLanguageChanged(std::function<void(Language)> cb) {
    int id = m_nextId++; m_callbacks[id] = std::move(cb); return id;
}
void I18n::removeCallback(int id) { m_callbacks.erase(id); }
std::string I18n::languageCode() const { return m_lang == Language::Korean ? "ko" : "en"; }
void I18n::setLanguageFromCode(const std::string& c) {
    setLanguage(c == "ko" ? Language::Korean : Language::English);
}

// ─── English ─────────────────────────────────────────────────
void I18n::loadEnglish() {
    m_strings = {
        {"nav.projects","Projects"},{"nav.installs","Installs"},
        {"nav.learn","Learn"},{"nav.settings","Settings"},{"nav.about","About"},
        {"topbar.newProject","New Project"},{"topbar.addExisting","Add Existing"},
        {"topbar.search","Search projects..."},
        {"status.ready","Ready."},{"status.checking","Checking for updates..."},
        {"status.downloading","Downloading..."},{"status.extracting","Extracting..."},
        {"status.done","Done!"},
        {"project.open","Open"},{"project.remove","Remove"},
        {"project.empty","No projects yet."},
        {"project.emptySub","Click + New Project to get started."},
        {"modal.newProject","New Project"},{"modal.cancel","Cancel"},
        {"modal.create","Create Project"},{"modal.template","Template"},
        {"modal.template3D","3D"},{"modal.template2D","2D"},
        {"modal.name","Project Name"},{"modal.location","Location"},
        {"modal.browse","Browse"},{"modal.engineVersion","Engine Version"},
        {"installs.title","GK Engine Versions"},{"installs.check","Check for Updates"},
        {"installs.install","Install"},{"installs.installed","Installed"},
        {"installs.empty","Click Check for Updates to load available versions."},
        {"learn.title","GK Engine Documentation"},
        {"learn.openDocs","Open Docs"},{"learn.gettingStarted","Getting Started"},
        {"learn.scripting","Scripting Guide"},{"learn.rendering","Rendering"},
        {"settings.appearance","Appearance"},{"settings.theme","Theme"},
        {"settings.themeSub","Choose color scheme for Hub and Engine"},
        {"settings.themeSystem","System"},{"settings.themeDark","Dark"},
        {"settings.themeLight","Light"},
        {"settings.language","Language"},{"settings.uiLanguage","Interface Language"},
        {"settings.uiLanguageSub","Applies to Hub and Engine editor UI"},
        {"settings.langNote","ⓘ Language change takes effect immediately."},
        {"settings.engineSync","Engine Sync"},
        {"settings.syncTheme","Sync Theme to Engine"},
        {"settings.syncLang","Sync Language to Engine"},
        {"settings.updates","Updates"},{"settings.autoCheck","Auto-check for Updates"},
        {"settings.manifestUrl","Manifest URL"},
        {"settings.save","Save Settings"},{"settings.saved","✓ Saved"},
        {"about.version","Version " GK_VERSION},
        {"about.tagline","Lightweight. Powerful. Yours."},
        {"about.builtBy","Built by"},
        {"about.openSource","Open Source Libraries"},
        {"about.openSourceSub","GK Suite is built on the shoulders of these excellent projects."},
        {"about.copyright","© 2024 GeekPiz. All rights reserved."},
        {"engine.hierarchy","Hierarchy"},{"engine.inspector","Inspector"},
        {"engine.project","Project"},{"engine.console","Console"},
        {"engine.scene","Scene"},{"engine.game","Game"},
        {"engine.play","Play"},{"engine.pause","Pause"},{"engine.step","Step"},
        {"engine.addComponent","+ Add Component"},{"engine.transform","Transform"},
        {"engine.position","Position"},{"engine.rotation","Rotation"},
        {"engine.scale","Scale"},{"engine.camera","Camera"},
        // Templates
        {"nav.templates","Templates"},
        {"templates.title","Project Templates"},
        {"templates.refresh","Refresh"},
        {"templates.download","Download"},
        {"templates.downloading","Downloading..."},
        {"templates.cached","Cached"},
        {"templates.builtin","Built-in"},
        {"templates.by","by"},
        {"templates.selectHint","Select a template to use for your new project."},
        {"modal.useTemplate","Use Template"},
        // Profile / Settings
        {"settings.profile","Profile"},
        {"settings.username","GitHub Username"},
        {"settings.usernameSub","Used to fetch your avatar automatically"},
        {"settings.displayName","Display Name"},
        {"settings.defaultPath","Default Projects Path"},
        {"settings.defaultPathSub","Where new projects are created by default"},
    };
}

// ─── Korean ──────────────────────────────────────────────────
void I18n::loadKorean() {
    m_strings = {
        {"nav.projects","프로젝트"},{"nav.installs","설치"},
        {"nav.learn","학습"},{"nav.settings","설정"},{"nav.about","정보"},
        {"topbar.newProject","새 프로젝트"},{"topbar.addExisting","기존 추가"},
        {"topbar.search","프로젝트 검색..."},
        {"status.ready","준비됨."},{"status.checking","업데이트 확인 중..."},
        {"status.downloading","다운로드 중..."},{"status.extracting","압축 해제 중..."},
        {"status.done","완료!"},
        {"project.open","열기"},{"project.remove","제거"},
        {"project.empty","아직 프로젝트가 없습니다."},
        {"project.emptySub","+ 새 프로젝트를 눌러 시작하세요."},
        {"modal.newProject","새 프로젝트"},{"modal.cancel","취소"},
        {"modal.create","프로젝트 생성"},{"modal.template","템플릿"},
        {"modal.template3D","3D"},{"modal.template2D","2D"},
        {"modal.name","프로젝트 이름"},{"modal.location","저장 위치"},
        {"modal.browse","찾아보기"},{"modal.engineVersion","엔진 버전"},
        {"installs.title","GK 엔진 버전"},{"installs.check","업데이트 확인"},
        {"installs.install","설치"},{"installs.installed","설치됨"},
        {"installs.empty","업데이트 확인을 눌러 버전을 불러오세요."},
        {"learn.title","GK 엔진 문서"},
        {"learn.openDocs","문서 열기"},{"learn.gettingStarted","시작하기"},
        {"learn.scripting","스크립팅 가이드"},{"learn.rendering","렌더링"},
        {"settings.appearance","화면"},{"settings.theme","테마"},
        {"settings.themeSub","허브와 엔진의 색상 테마 선택"},
        {"settings.themeSystem","시스템"},{"settings.themeDark","다크"},
        {"settings.themeLight","라이트"},
        {"settings.language","언어"},{"settings.uiLanguage","인터페이스 언어"},
        {"settings.uiLanguageSub","허브 및 엔진 편집기 UI에 적용"},
        {"settings.langNote","ⓘ 언어 변경은 즉시 적용됩니다."},
        {"settings.engineSync","엔진 동기화"},
        {"settings.syncTheme","엔진에 테마 동기화"},
        {"settings.syncLang","엔진에 언어 동기화"},
        {"settings.updates","업데이트"},{"settings.autoCheck","업데이트 자동 확인"},
        {"settings.manifestUrl","매니페스트 URL"},
        {"settings.save","설정 저장"},{"settings.saved","✓ 저장됨"},
        {"about.version","버전 " GK_VERSION},
        {"about.tagline","가볍고. 강력하고. 당신의 것."},
        {"about.builtBy","제작자"},
        {"about.openSource","오픈소스 라이브러리"},
        {"about.openSourceSub","GK Suite는 이 훌륭한 프로젝트들 위에 세워졌습니다."},
        {"about.copyright","© 2024 GeekPiz. All rights reserved."},
        {"engine.hierarchy","하이어라키"},{"engine.inspector","인스펙터"},
        {"engine.project","프로젝트"},{"engine.console","콘솔"},
        {"engine.scene","씬"},{"engine.game","게임"},
        {"engine.play","플레이"},{"engine.pause","일시정지"},{"engine.step","스텝"},
        {"engine.addComponent","+ 컴포넌트 추가"},{"engine.transform","트랜스폼"},
        {"engine.position","위치"},{"engine.rotation","회전"},
        {"engine.scale","크기"},{"engine.camera","카메라"},
        // Templates
        {"nav.templates","템플릿"},
        {"templates.title","프로젝트 템플릿"},
        {"templates.refresh","새로고침"},
        {"templates.download","다운로드"},
        {"templates.downloading","다운로드 중..."},
        {"templates.cached","캐시됨"},
        {"templates.builtin","내장"},
        {"templates.by","제작:"},
        {"templates.selectHint","새 프로젝트에 사용할 템플릿을 선택하세요."},
        {"modal.useTemplate","템플릿 사용"},
        // Profile / Settings
        {"settings.profile","프로필"},
        {"settings.username","GitHub 사용자 이름"},
        {"settings.usernameSub","아바타를 자동으로 가져오는 데 사용됩니다"},
        {"settings.displayName","표시 이름"},
        {"settings.defaultPath","기본 프로젝트 경로"},
        {"settings.defaultPathSub","새 프로젝트가 기본으로 생성되는 위치"},
    };
}

} // namespace GK
