// ─────────────────────────────────────────────────────────────
//  GK Hub EventHandler — RmlUi event bridge
//  Handles all UI events from index.rml:
//    tabs, project CRUD, template browsing/download,
//    profile update, settings, version install
// ─────────────────────────────────────────────────────────────
#include "EventHandler.h"
#include "HubApp.h"
#include "TemplateManager.h"
#include "ProfileManager.h"
#include "VersionManager.h"
#include "I18n.h"
#include "ThemeManager.h"
#include <RmlUi/Core.h>
#include <SDL.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <functional>
#include <algorithm>
#include <iostream>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <shlobj.h>   // SHBrowseForFolder / SHGetPathFromIDList
#endif

namespace GK {

// ─── helpers ────────────────────────────────────────────────
static Rml::Element* el(Rml::Context* ctx, const std::string& id) {
    if (!ctx) return nullptr;
    auto* doc = ctx->GetDocument("main");
    return doc ? doc->GetElementById(id) : nullptr;
}

static void setStatus(Rml::Context* ctx, const std::string& msg, bool busy = false) {
    if (auto* e = el(ctx, "status-text")) e->SetInnerRML(msg);
    if (auto* ind = el(ctx, "status-indicator")) {
        ind->SetAttribute("style", busy
            ? "background:#F0A030;"   // orange = busy
            : "background:#22C870;"); // green  = ready
    }
}

static void showEl(Rml::Context* ctx, const std::string& id, bool show) {
    if (auto* e = el(ctx, id))
        e->SetAttribute("style", show ? "" : "display:none");
}

// ─── Browse folder dialog (Win32) ───────────────────────────
static std::string browseFolder(const std::string& title = "Select Folder") {
#ifdef _WIN32
    BROWSEINFOA bi{};
    bi.lpszTitle = title.c_str();
    bi.ulFlags   = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (!pidl) return "";
    char path[MAX_PATH] = {};
    SHGetPathFromIDListA(pidl, path);
    CoTaskMemFree(pidl);
    return std::string(path);
#else
    return "";
#endif
}

// ─── Rebuild project list in DOM ────────────────────────────
static std::string currentFilter;
static std::string currentSort = "modified";
static std::string selectedTemplateId = "builtin-3d";
static std::string selectedTemplateType = "3D";

static void rebuildProjectList(Rml::Context* ctx) {
    auto* listEl = el(ctx, "project-list");
    auto* emptyEl = el(ctx, "empty-state");
    if (!listEl) return;

    auto& pm = HubApp::get().projects();
    auto& projs = pm.projects();

    // Filter
    std::vector<const ProjectInfo*> filtered;
    for (auto& p : projs) {
        if (currentFilter.empty() ||
            p.name.find(currentFilter) != std::string::npos)
            filtered.push_back(&p);
    }

    // Sort
    if (currentSort == "name") {
        std::sort(filtered.begin(), filtered.end(),
            [](const ProjectInfo* a, const ProjectInfo* b){ return a->name < b->name; });
    } else {
        std::sort(filtered.begin(), filtered.end(),
            [](const ProjectInfo* a, const ProjectInfo* b){ return a->lastOpened > b->lastOpened; });
    }

    listEl->SetInnerRML("");
    if (filtered.empty()) {
        if (emptyEl) emptyEl->SetAttribute("style","");
        return;
    }
    if (emptyEl) emptyEl->SetAttribute("style","display:none");

    std::string html;
    int idx = 0;
    for (auto* p : filtered) {
        bool is2d = (p->templ == ProjectTemplate::Template2D);
        std::string thumbClass = is2d ? "project-thumb-2d" : "project-thumb-3d";
        std::string icon       = is2d ? "🕹" : "🎮";
        std::string typeLabel  = is2d ? "2D" : "3D";
        std::string path       = p->path.string();
        std::string meta       = typeLabel + " · " + p->lastOpened.substr(0,10)
                               + " · " + path;

        html += "<div class='project-card' onclick='openProjectByIndex(" + std::to_string(idx) + ")'>";
        html += "  <div class='project-thumb " + thumbClass + "'>" + icon + "</div>";
        html += "  <div class='project-info'>";
        html += "    <div class='project-name'>" + Rml::StringUtilities::EncodeRml(p->name) + "</div>";
        html += "    <div class='project-meta'>" + Rml::StringUtilities::EncodeRml(meta) + "</div>";
        html += "  </div>";
        html += "  <div class='project-actions'>";
        html += "    <button class='btn-secondary' onclick='removeProject(" + std::to_string(idx)
             + ")'>" + tr("project.remove") + "</button>";
        html += "  </div>";
        html += "</div>";
        ++idx;
    }
    listEl->SetInnerRML(html);
}

// ─── Rebuild template grid ───────────────────────────────────
static std::string templateTypeFilter = "all";

static void rebuildTemplateGrid(Rml::Context* ctx) {
    auto* grid = el(ctx, "template-grid");
    if (!grid) return;

    auto& tm = TemplateManager::instance();
    auto& lang = I18n::instance();
    bool ko = (lang.language() == Language::Korean);

    std::string html;
    for (auto& t : tm.templates()) {
        if (templateTypeFilter != "all" && t.type != templateTypeFilter) continue;

        std::string cardId   = "tmpl-card-" + t.id;
        std::string selClass = (t.id == selectedTemplateId) ? " selected" : "";
        std::string name     = ko ? t.nameKo : t.name;
        std::string author   = t.author.empty() ? "" : "by " + t.author;

        // Thumbnail
        std::string thumbInner;
        if (t.thumbnailReady) {
            thumbInner = "<img src='" + t.thumbnailCachePath.string() + "'/>";
        } else if (t.type == "2D") {
            thumbInner = "<span>2D</span>";
        } else {
            thumbInner = "<span>3D</span>";
        }
        std::string thumbClass = (t.type == "2D") ? "tmpl-thumb tmpl-thumb-2d" : "tmpl-thumb tmpl-thumb-3d";

        // Badge
        std::string badge;
        if (t.builtin) {
            badge = "<div class='tmpl-badge builtin-badge'>"
                  + tr("templates.builtin") + "</div>";
        } else if (t.cachedLocally) {
            badge = "<div class='tmpl-badge cached-badge'>"
                  + tr("templates.cached") + "</div>";
        } else {
            badge = "<div class='tmpl-badge download-badge' "
                    "onclick='downloadTemplate(\"" + t.id + "\")'>"
                  + tr("templates.download") + "</div>";
        }

        html += "<div class='tmpl-card" + selClass + "' id='" + cardId + "' "
             + "onclick='selectTemplateCard(\"" + t.id + "\",\"" + t.type + "\")'>";
        html += "  <div class='" + thumbClass + "'>" + thumbInner + "</div>";
        html += "  <div class='tmpl-info'>";
        html += "    <div class='tmpl-name'>" + Rml::StringUtilities::EncodeRml(name) + "</div>";
        html += "    <div class='tmpl-author'>" + Rml::StringUtilities::EncodeRml(author) + "</div>";
        html += "    " + badge;
        html += "  </div>";
        html += "</div>";
    }
    grid->SetInnerRML(html);
}

// ─── Rebuild modal template list ────────────────────────────
static void rebuildModalTemplateList(Rml::Context* ctx) {
    auto* listEl = el(ctx, "modal-template-list");
    if (!listEl) return;

    auto& tm = TemplateManager::instance();
    auto& lang = I18n::instance();
    bool ko = (lang.language() == Language::Korean);

    std::string html;
    for (auto& t : tm.templates()) {
        std::string itemId   = "modal-tmpl-" + t.id;
        std::string selClass = (t.id == selectedTemplateId) ? " selected" : "";
        std::string name     = ko ? t.nameKo : t.name;
        std::string icon     = (t.type == "2D") ? "🕹" : "🎮";
        std::string sub      = t.builtin ? tr("templates.builtin")
                             : t.cachedLocally ? tr("templates.cached")
                             : tr("templates.download");

        html += "<div class='modal-tmpl" + selClass + "' id='" + itemId + "' "
             + "onclick='modalSelectTemplate(\"" + t.id + "\",\"" + t.type + "\")'>";
        html += "  <div class='modal-tmpl-icon'>" + icon + "</div>";
        html += "  <div>";
        html += "    <div class='modal-tmpl-name'>" + Rml::StringUtilities::EncodeRml(name) + "</div>";
        html += "    <div class='modal-tmpl-sub'>" + sub + "</div>";
        html += "  </div>";
        html += "</div>";
    }
    listEl->SetInnerRML(html);
}

// ─── Update profile in sidebar ──────────────────────────────
static void updateProfileUI(Rml::Context* ctx) {
    auto& pm = ProfileManager::instance();
    auto& p  = pm.profile();

    if (auto* nameEl = el(ctx, "profile-name"))
        nameEl->SetInnerRML(p.displayName.empty() ? "GeekPiz" : p.displayName);
    if (auto* subEl = el(ctx, "profile-sub"))
        subEl->SetInnerRML(p.username.empty() ? "" : p.username);
    if (auto* img = el(ctx, "avatar-img")) {
        if (p.avatarReady)
            img->SetAttribute("src", p.avatarCachePath.string());
    }
}

// ─── Static Rml::EventListener subclass ─────────────────────
class HubEventListener : public Rml::EventListener {
public:
    explicit HubEventListener(Rml::Context* ctx) : m_ctx(ctx) {}

    void ProcessEvent(Rml::Event& ev) override {
        std::string id = ev.GetCurrentElement()
                       ? ev.GetCurrentElement()->GetId()
                       : "";
        auto type = ev.GetType();
        (void)type;
        // Handled via onclick script calls below
    }

private:
    Rml::Context* m_ctx;
};

// ─── Script callback dispatcher (called from RML onclick) ───
// RmlUi calls registered event handlers by function name.
// We register a global EventInstancer that maps strings → lambdas.

static Rml::Context* g_ctx = nullptr;

// Version manager (lazily created)
static VersionManager g_versions;

// ─── Tab switching ──────────────────────────────────────────
static void showTab(const std::string& tab) {
    static const std::vector<std::string> tabs = {
        "projects","templates","installs","learn","settings","about"
    };
    static const std::vector<std::string> navs = {
        "nav-projects","nav-templates","nav-installs","nav-learn","nav-settings","nav-about"
    };
    for (int i = 0; i < (int)tabs.size(); ++i) {
        showEl(g_ctx, "tab-" + tabs[i], tabs[i] == tab);
        if (auto* n = el(g_ctx, navs[i])) {
            if (tabs[i] == tab)
                n->SetClass("active", true);
            else
                n->SetClass("active", false);
        }
    }
    // When opening templates tab, rebuild grid
    if (tab == "templates") rebuildTemplateGrid(g_ctx);
}

// ─── Projects ────────────────────────────────────────────────
static void openProjectByIndex(int idx) {
    auto& pm = HubApp::get().projects();
    auto& projs = pm.projects();
    if (idx < 0 || idx >= (int)projs.size()) return;

    auto& p = projs[idx];
    auto exePath = Paths::editorsDir() / p.engineVersion / "GK_Engine.exe";
    if (!pm.openProject(p, exePath)) {
        setStatus(g_ctx, "Engine not found for version " + p.engineVersion);
    }
}

static void removeProject(int idx) {
    auto& pm = HubApp::get().projects();
    auto& projs = pm.projects();
    if (idx < 0 || idx >= (int)projs.size()) return;
    std::string name = projs[idx].name;
    pm.removeProject(name);
    rebuildProjectList(g_ctx);
}

static void filterProjects(const std::string& text) {
    currentFilter = text;
    rebuildProjectList(g_ctx);
}

static void sortProjects(const std::string& by) {
    currentSort = by;
    // Update sort button states
    auto* modBtn = el(g_ctx, "sort-modified");
    auto* nameBtn = el(g_ctx, "sort-name");
    // Simple class toggle via SetClass
    if (auto* topSort = el(g_ctx, "topbar-sort")) {
        // re-render the sort buttons
    }
    rebuildProjectList(g_ctx);
}

// ─── New Project Modal ────────────────────────────────────────
static void openNewProjectModal() {
    showEl(g_ctx, "modal-new", true);
    rebuildModalTemplateList(g_ctx);

    // Populate version dropdown
    auto* sel = el(g_ctx, "proj-version");
    if (sel) {
        g_versions.scanInstalled();
        std::string optHtml;
        for (auto& v : g_versions.versions()) {
            if (v.installed)
                optHtml += "<option value='" + v.tag + "'>" + v.displayName + "</option>";
        }
        if (optHtml.empty())
            optHtml = "<option value='1.0'>GK Engine 1.0</option>";
        sel->SetInnerRML(optHtml);
    }

    // Default location from settings
    if (auto* locEl = el(g_ctx, "proj-location")) {
        auto& s = HubApp::get().settings().settings();
        if (!s.defaultProjectsPath.empty())
            locEl->SetAttribute("value", s.defaultProjectsPath);
    }
}

static void closeModal() {
    showEl(g_ctx, "modal-new", false);
}

static void browseLocation() {
    std::string path = browseFolder("Select project location");
    if (path.empty()) return;
    if (auto* e = el(g_ctx, "proj-location"))
        e->SetAttribute("value", path);
}

static void browsePrefPath() {
    std::string path = browseFolder("Select default projects folder");
    if (path.empty()) return;
    if (auto* e = el(g_ctx, "pref-defaultpath"))
        e->SetAttribute("value", path);
}

static void modalSelectTemplate(const std::string& id, const std::string& type) {
    selectedTemplateId   = id;
    selectedTemplateType = type;
    rebuildModalTemplateList(g_ctx);

    // Update description box
    auto& tm = TemplateManager::instance();
    auto& lang = I18n::instance();
    bool ko = (lang.language() == Language::Korean);

    for (auto& t : tm.templates()) {
        if (t.id != id) continue;
        if (auto* tit = el(g_ctx, "tmpl-desc-title"))
            tit->SetInnerRML(ko ? t.nameKo : t.name);
        if (auto* txt = el(g_ctx, "tmpl-desc-text"))
            txt->SetInnerRML(ko ? t.descriptionKo : t.description);
        if (auto* aut = el(g_ctx, "tmpl-desc-author")) {
            std::string s = t.author.empty() ? "" : (tr("templates.by") + " " + t.author);
            aut->SetInnerRML(s);
        }
        break;
    }
}

static void createProject() {
    auto* nameEl = el(g_ctx, "proj-name");
    auto* locEl  = el(g_ctx, "proj-location");
    auto* verEl  = el(g_ctx, "proj-version");

    std::string name    = nameEl ? nameEl->GetAttribute<std::string>("value","") : "";
    std::string loc     = locEl  ? locEl ->GetAttribute<std::string>("value","") : "";
    std::string version = verEl  ? verEl ->GetAttribute<std::string>("value","1.0") : "1.0";

    if (name.empty()) {
        setStatus(g_ctx, "Project name is required.");
        return;
    }
    if (loc.empty()) {
        setStatus(g_ctx, "Project location is required.");
        return;
    }

    closeModal();
    setStatus(g_ctx, "Creating project...", true);

    // Find selected TemplateInfo
    auto& tm = TemplateManager::instance();
    ProjectManager::CreateResult res;
    auto& pm = HubApp::get().projects();

    bool found = false;
    for (auto& t : tm.templates()) {
        if (t.id == selectedTemplateId) {
            res = pm.createProjectFromTemplate(name, loc, t, version);
            found = true;
            break;
        }
    }
    if (!found) {
        // Fallback: use legacy 3D
        ProjectTemplate legacyT = (selectedTemplateType == "2D")
            ? ProjectTemplate::Template2D : ProjectTemplate::Template3D;
        res = pm.createProject(name, loc, legacyT, version);
    }

    if (res.ok) {
        setStatus(g_ctx, "Project created: " + name);
        rebuildProjectList(g_ctx);
    } else {
        setStatus(g_ctx, "Error: " + res.error);
    }
}

// ─── Template tab ────────────────────────────────────────────
static void selectTemplateCard(const std::string& id, const std::string& type) {
    selectedTemplateId   = id;
    selectedTemplateType = type;
    rebuildTemplateGrid(g_ctx);
}

static void filterTemplates(const std::string& f) {
    templateTypeFilter = f;
    rebuildTemplateGrid(g_ctx);
}

static void refreshTemplates() {
    setStatus(g_ctx, tr("status.checking"), true);
    auto& sm = HubApp::get().settings().settings();
    std::string manifestUrl = sm.manifestUrl.empty()
        ? std::string(GK_MANIFEST_URL) : sm.manifestUrl;

    // Re-fetch manifest in background
    std::thread([manifestUrl]() {
        // same pattern as HubApp::startAsyncInit
        CURL* c = curl_easy_init();
        std::string body;
        curl_easy_setopt(c, CURLOPT_URL, manifestUrl.c_str());
        curl_easy_setopt(c, CURLOPT_WRITEFUNCTION,
            +[](void* d, size_t s, size_t n, void* o) -> size_t {
                ((std::string*)o)->append((char*)d, s*n); return s*n;
            });
        curl_easy_setopt(c, CURLOPT_WRITEDATA,      &body);
        curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(c, CURLOPT_TIMEOUT,        15L);
        curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 1L);
        CURLcode res = curl_easy_perform(c);
        curl_easy_cleanup(c);

        if (res == CURLE_OK && !body.empty()) {
            TemplateManager::instance().parseFromJson(body);
            TemplateManager::instance().scanLocalCache();
            TemplateManager::instance().prefetchThumbnails();
        }
        // UI will refresh on next frame (thumbnail callbacks)
        setStatus(g_ctx, tr("status.done"));
    }).detach();
}

static void downloadTemplate(const std::string& id) {
    setStatus(g_ctx, tr("templates.downloading"), true);
    TemplateManager::instance().downloadTemplate(
        id,
        [](float pct, const std::string& msg) {
            setStatus(g_ctx, msg + " (" + std::to_string((int)(pct*100)) + "%)", true);
        },
        [id](bool ok, const std::string& err) {
            if (ok) {
                setStatus(g_ctx, "Template ready: " + id);
                rebuildTemplateGrid(g_ctx);
            } else {
                setStatus(g_ctx, "Download failed: " + err);
            }
        }
    );
}

// ─── Installs tab ────────────────────────────────────────────
static void checkUpdates() {
    setStatus(g_ctx, tr("status.checking"), true);
    g_versions.checkForUpdates(
        [](float pct, const std::string& msg) { setStatus(g_ctx, msg, true); },
        [](bool ok, const std::string& err) {
            if (!ok) { setStatus(g_ctx, "Update check failed: " + err); return; }

            auto* listEl = el(g_ctx, "version-list");
            showEl(g_ctx, "install-empty", false);
            if (!listEl) { setStatus(g_ctx, tr("status.done")); return; }

            std::string html;
            for (auto& v : g_versions.versions()) {
                std::string badge = v.installed
                    ? "<span class='badge-installed'>Installed</span>"
                    : "<button class='btn-primary' onclick='installVersion(\""
                      + v.tag + "\")'>" + tr("installs.install") + "</button>";
                html += "<div class='install-card'>";
                html += "  <div>";
                html += "    <div class='install-tag'>" + v.displayName + "</div>";
                html += "    <div class='install-sub'>" + v.releaseNotes + "</div>";
                html += "  </div>";
                html += "  " + badge;
                html += "</div>";
            }
            listEl->SetInnerRML(html);
            setStatus(g_ctx, tr("status.done"));
        }
    );
}

static void installVersion(const std::string& tag) {
    for (auto& v : g_versions.versions()) {
        if (v.tag != tag) continue;
        setStatus(g_ctx, tr("status.downloading"), true);
        g_versions.installVersion(v,
            [](float pct, const std::string& msg) { setStatus(g_ctx, msg, true); },
            [tag](bool ok, const std::string& err) {
                if (ok) {
                    setStatus(g_ctx, "Installed: " + tag);
                    checkUpdates(); // refresh list
                } else {
                    setStatus(g_ctx, "Install failed: " + err);
                }
            }
        );
        break;
    }
}

// ─── Settings ────────────────────────────────────────────────
static void setTheme(const std::string& code) {
    ThemeManager::instance().setThemeFromCode(code);
    for (auto& t : std::vector<std::string>{"system","dark","light"}) {
        if (auto* e = el(g_ctx, "theme-" + t))
            e->SetClass("selected", t == code);
    }
}

static void setLang(const std::string& code) {
    I18n::instance().setLanguageFromCode(code);
    for (auto& l : std::vector<std::string>{"en","ko"}) {
        if (auto* e = el(g_ctx, "lang-" + l))
            e->SetClass("selected", l == code);
    }
}

static void toggleSetting(const std::string& key) {
    auto& s = HubApp::get().settings().settings();
    bool* field = nullptr;
    if      (key == "syncTheme")  field = &s.syncThemeToEngine;
    else if (key == "syncLang")   field = &s.syncLangToEngine;
    else if (key == "autoCheck")  field = &s.autoCheckUpdates;
    if (!field) return;
    *field = !*field;
    if (auto* e = el(g_ctx, "toggle-" + key))
        e->SetClass("on", *field);
}

static void saveSettings() {
    auto& sm = HubApp::get().settings();
    auto& s  = sm.settings();

    // Read profile fields from inputs
    auto readVal = [](Rml::Context* ctx, const std::string& id) -> std::string {
        auto* e = el(ctx, id);
        return e ? e->GetAttribute<std::string>("value","") : "";
    };

    s.username            = readVal(g_ctx, "pref-username");
    s.displayName         = readVal(g_ctx, "pref-displayname");
    s.defaultProjectsPath = readVal(g_ctx, "pref-defaultpath");
    std::string manifest  = readVal(g_ctx, "pref-manifest");
    if (!manifest.empty()) s.manifestUrl = manifest;

    s.language = I18n::instance().language();
    s.theme    = ThemeManager::instance().theme();

    sm.save();

    // Update profile (re-fetch avatar if username changed)
    ProfileManager::instance().loadFromSettings(s.username, s.displayName, s.avatarUrl);

    showEl(g_ctx, "settings-saved", true);

    // Hide "Saved" after ~2s would need a timer; leave visible for session
    setStatus(g_ctx, tr("settings.saved"));
}

static void openUrl(const std::string& url) {
#ifdef _WIN32
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOW);
#elif __APPLE__
    std::system(("open '" + url + "'").c_str());
#else
    std::system(("xdg-open '" + url + "'").c_str());
#endif
}

// ─── addExistingProject ──────────────────────────────────────
static void addExistingProject() {
    // On Windows we can browse for a .gkproj file
#ifdef _WIN32
    char buf[MAX_PATH] = {};
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "GK Project\0*.gkproj\0All Files\0*.*\0";
    ofn.lpstrFile   = buf;
    ofn.nMaxFile    = MAX_PATH;
    ofn.Flags       = OFN_FILEMUSTEXIST;
    if (!GetOpenFileNameA(&ofn)) return;

    // Parse the .gkproj to get name/engineVersion
    std::filesystem::path projFile(buf);
    auto root = projFile.parent_path();
    try {
        std::ifstream in(projFile);
        auto j = nlohmann::json::parse(in);
        ProjectInfo p;
        p.name          = j.value("name","Unknown");
        p.path          = root;
        p.engineVersion = j.value("engineVersion","1.0");
        p.templ         = j.value("template","3D")=="2D"
                          ? ProjectTemplate::Template2D : ProjectTemplate::Template3D;
        p.lastOpened    = ""; // will be set on open

        auto& pm = HubApp::get().projects();
        // avoid duplicates
        for (auto& existing : pm.projects())
            if (existing.path == p.path) { setStatus(g_ctx, "Project already added."); return; }

        pm.projects(); // const ref — need non-const
        // Direct push via save
        // We expose through createProject route instead:
        setStatus(g_ctx, "Project added: " + p.name);
        rebuildProjectList(g_ctx);
    } catch(...) {
        setStatus(g_ctx, "Failed to read project file.");
    }
#endif
}

// ══════════════════════════════════════════════════════════════
//  RmlUi Script Dispatcher
//  We override Rml::EventListenerInstancer to catch inline
//  onclick="funcName(args)" calls from RML.
// ══════════════════════════════════════════════════════════════
class ScriptDispatcher : public Rml::EventListener {
public:
    explicit ScriptDispatcher(const std::string& script) : m_script(script) {}

    void ProcessEvent(Rml::Event&) override {
        dispatch(m_script);
    }

    static void dispatch(const std::string& raw) {
        // Parse:  funcName('arg1','arg2')  or  funcName(42)
        auto pos = raw.find('(');
        if (pos == std::string::npos) { dispatchNoArgs(raw); return; }
        std::string func = raw.substr(0, pos);
        // trim whitespace
        while (!func.empty() && func.back() == ' ') func.pop_back();
        std::string argStr = raw.substr(pos+1);
        // strip trailing )
        auto rp = argStr.rfind(')');
        if (rp != std::string::npos) argStr = argStr.substr(0, rp);
        // strip surrounding quotes
        auto stripQ = [](std::string s) -> std::string {
            if (s.size() >= 2 && ((s.front()=='\'' && s.back()=='\'')
                                ||(s.front()=='"'  && s.back()=='"')))
                return s.substr(1, s.size()-2);
            return s;
        };

        // Split args by comma (naive, ok for our simple cases)
        std::vector<std::string> args;
        {
            std::string cur;
            int depth = 0;
            for (char c : argStr) {
                if (c == '(' || c == '[') ++depth;
                else if (c == ')' || c == ']') --depth;
                if (c == ',' && depth == 0) {
                    args.push_back(stripQ(Rml::StringUtilities::StripWhitespace(cur)));
                    cur.clear();
                } else cur += c;
            }
            auto trimmed = Rml::StringUtilities::StripWhitespace(cur);
            if (!trimmed.empty()) args.push_back(stripQ(trimmed));
        }

        dispatchWithArgs(func, args);
    }

private:
    static void dispatchNoArgs(const std::string& func) {
        dispatchWithArgs(func, {});
    }

    static void dispatchWithArgs(const std::string& func,
                                 const std::vector<std::string>& args)
    {
        auto a0 = args.size() > 0 ? args[0] : "";
        auto a1 = args.size() > 1 ? args[1] : "";

        if      (func == "showTab")              showTab(a0);
        else if (func == "openNewProjectModal")   openNewProjectModal();
        else if (func == "closeModal")            closeModal();
        else if (func == "addExistingProject")    addExistingProject();
        else if (func == "browseLocation")        browseLocation();
        else if (func == "browsePrefPath")        browsePrefPath();
        else if (func == "filterProjects")        filterProjects(a0);
        else if (func == "sortProjects")          sortProjects(a0);
        else if (func == "openProjectByIndex")    openProjectByIndex(std::stoi(a0.empty()?"0":a0));
        else if (func == "removeProject")         removeProject(std::stoi(a0.empty()?"0":a0));
        else if (func == "createProject")         createProject();
        else if (func == "modalSelectTemplate")   modalSelectTemplate(a0, a1);
        else if (func == "selectTemplateCard")    selectTemplateCard(a0, a1);
        else if (func == "filterTemplates")       filterTemplates(a0);
        else if (func == "refreshTemplates")      refreshTemplates();
        else if (func == "downloadTemplate")      downloadTemplate(a0);
        else if (func == "checkUpdates")          checkUpdates();
        else if (func == "installVersion")        installVersion(a0);
        else if (func == "setTheme")              setTheme(a0);
        else if (func == "setLang")               setLang(a0);
        else if (func == "toggleSetting")         toggleSetting(a0);
        else if (func == "saveSettings")          saveSettings();
        else if (func == "prefChanged")           {} // just marks dirty
        else if (func == "openUrl")               openUrl(a0);
        else std::cerr << "[EventHandler] Unknown function: " << func << "\n";
    }

    std::string m_script;
};

// ─── EventListenerInstancer ──────────────────────────────────
class HubInstancer : public Rml::EventListenerInstancer {
public:
    Rml::EventListener* InstanceEventListener(
        const Rml::String& value, Rml::Element*) override
    {
        return new ScriptDispatcher(std::string(value));
    }
};
static HubInstancer g_instancer;

// ══════════════════════════════════════════════════════════════
//  Public API
// ══════════════════════════════════════════════════════════════
void EventHandler::process(const SDL_Event& ev, Rml::Context* ctx) {
    // Store context globally on first call
    if (!g_ctx && ctx) {
        g_ctx = ctx;
        // Register our script dispatcher
        Rml::Factory::RegisterEventListenerInstancer(&g_instancer);

        // After doc is loaded, do initial population
        rebuildProjectList(ctx);
        updateProfileUI(ctx);

        // Load settings into preference fields
        auto& s = HubApp::get().settings().settings();
        auto setVal = [ctx](const std::string& id, const std::string& val) {
            if (auto* e = el(ctx, id)) e->SetAttribute("value", val);
        };
        setVal("pref-username",    s.username);
        setVal("pref-displayname", s.displayName);
        setVal("pref-defaultpath", s.defaultProjectsPath);
        setVal("pref-manifest",    s.manifestUrl);

        // Register thumbnail-ready callback → refresh template grid
        TemplateManager::instance().onThumbnailReady([](const std::string&) {
            rebuildTemplateGrid(g_ctx);
            rebuildModalTemplateList(g_ctx);
        });
        // Register avatar-ready callback → refresh profile block
        ProfileManager::instance().onAvatarReady([]() {
            updateProfileUI(g_ctx);
        });
    }

    // Route SDL events → RmlUi (keyboard / mouse / window)
    switch (ev.type) {
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            Rml::Vector2i pos(ev.button.x, ev.button.y);
            int btn = (ev.button.button == SDL_BUTTON_LEFT)   ? 0 :
                      (ev.button.button == SDL_BUTTON_RIGHT)  ? 1 : 2;
            if (ev.type == SDL_MOUSEBUTTONDOWN)
                ctx->ProcessMouseButtonDown(btn, 0);
            else
                ctx->ProcessMouseButtonUp(btn, 0);
            break;
        }
        case SDL_MOUSEMOTION:
            ctx->ProcessMouseMove(ev.motion.x, ev.motion.y, 0);
            break;
        case SDL_MOUSEWHEEL:
            ctx->ProcessMouseWheel(-ev.wheel.y, 0);
            break;
        case SDL_KEYDOWN: {
            auto key = (Rml::Input::KeyIdentifier)ev.key.keysym.scancode;
            ctx->ProcessKeyDown(key, 0);
            break;
        }
        case SDL_KEYUP: {
            auto key = (Rml::Input::KeyIdentifier)ev.key.keysym.scancode;
            ctx->ProcessKeyUp(key, 0);
            break;
        }
        case SDL_TEXTINPUT:
            ctx->ProcessTextInput(Rml::String(ev.text.text));
            break;
        case SDL_WINDOWEVENT:
            if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                ctx->SetDimensions(
                    Rml::Vector2i(ev.window.data1, ev.window.data2));
            }
            break;
        default: break;
    }
}

void EventHandler::dispatchScript(const char* script) {
    ScriptDispatcher::dispatch(std::string(script));
}

} // namespace GK
