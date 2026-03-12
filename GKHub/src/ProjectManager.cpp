#include "ProjectManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace GK {

static std::string now_iso() {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

ProjectManager::ProjectManager() {}

// ─── createProject ────────────────────────────────────────────
ProjectManager::CreateResult ProjectManager::createProject(
    const std::string& name,
    const std::filesystem::path& parentDir,
    ProjectTemplate templ,
    const std::string& engineVersion)
{
    CreateResult res;
    if (name.empty()) { res.error = "Project name is empty."; return res; }

    auto root = parentDir / name;
    if (std::filesystem::exists(root)) {
        res.error = "Folder already exists: " + root.string(); return res;
    }

    try {
        createFolderStructure(root, templ);

        ProjectInfo info;
        info.name          = name;
        info.path          = root;
        info.engineVersion = engineVersion;
        info.templ         = templ;
        info.lastOpened    = now_iso();

        writeProjectMeta(root, info);
        writeGameScene(root / "Assets" / "Scenes", templ);

        m_projects.push_back(info);
        save();

        res.ok = true;
        res.projectRoot = root;
    } catch(const std::exception& e) {
        res.error = std::string("Exception: ") + e.what();
    }
    return res;
}

void ProjectManager::createFolderStructure(
    const std::filesystem::path& root, ProjectTemplate t) const
{
    std::vector<std::filesystem::path> dirs = {
        root/"Assets", root/"Assets"/"Scenes", root/"Assets"/"Scripts",
        root/"Assets"/"Textures", root/"Assets"/"Models",
        root/"Assets"/"Audio", root/"Assets"/"Materials",
        root/"Assets"/"Prefabs", root/"Assets"/"Shaders",
        root/"ProjectSettings", root/"Logs", root/"Temp",
    };
    if (t == ProjectTemplate::Template2D) {
        dirs.push_back(root/"Assets"/"Sprites");
        dirs.push_back(root/"Assets"/"Tilemaps");
    }
    for (auto& d : dirs) std::filesystem::create_directories(d);

    std::ofstream(root/".gitignore") << "Temp/\nLogs/\n*.gktemp\n";

    nlohmann::json tags;
    tags["tags"]   = {"Untagged","MainCamera","Player"};
    tags["layers"] = {"Default","TransparentFX","UI","PostProcessing"};
    std::ofstream(root/"ProjectSettings"/"tags.json") << tags.dump(2);

    nlohmann::json phys;
    phys["gravity"] = {0.0,-9.81,0.0}; phys["iterations"] = 10;
    std::ofstream(root/"ProjectSettings"/"physics.json") << phys.dump(2);
}

void ProjectManager::writeProjectMeta(
    const std::filesystem::path& root, const ProjectInfo& info) const
{
    nlohmann::json j;
    j["name"]          = info.name;
    j["engineVersion"] = info.engineVersion;
    j["template"]      = info.templ == ProjectTemplate::Template2D ? "2D" : "3D";
    j["created"]       = info.lastOpened;
    j["gkVersion"]     = GK_VERSION;
    std::ofstream(root/"project.gkproj") << j.dump(2);
}

void ProjectManager::writeGameScene(
    const std::filesystem::path& scenesDir, ProjectTemplate t) const
{
    nlohmann::json s;
    s["version"] = GK_VERSION;
    s["name"]    = "SampleScene";
    s["type"]    = t == ProjectTemplate::Template2D ? "2D" : "3D";
    s["entities"] = nlohmann::json::array();

    if (t == ProjectTemplate::Template3D) {
        s["entities"].push_back({
            {"id",1},{"name","Main Camera"},{"tag","MainCamera"},
            {"components", nlohmann::json::array({
                {{"type","Transform"},{"position",{0,1,-10}},{"rotation",{0,0,0}},{"scale",{1,1,1}}},
                {{"type","Camera"},{"fov",60},{"near",0.1},{"far",1000},{"clearColor","#1A1A2A"}}
            })}
        });
        s["entities"].push_back({
            {"id",2},{"name","Directional Light"},
            {"components", nlohmann::json::array({
                {{"type","Transform"},{"position",{0,10,0}},{"rotation",{50,-30,0}},{"scale",{1,1,1}}},
                {{"type","Light"},{"lightType","Directional"},{"color","#FFFFFF"},{"intensity",1.0},{"castShadows",true}}
            })}
        });
        s["entities"].push_back({
            {"id",3},{"name","Cube"},
            {"components", nlohmann::json::array({
                {{"type","Transform"},{"position",{0,0.5,0}},{"rotation",{0,0,0}},{"scale",{1,1,1}}},
                {{"type","MeshRenderer"},{"mesh","BuiltIn/Cube"},{"material","Default"}}
            })}
        });
        s["entities"].push_back({
            {"id",4},{"name","Plane"},
            {"components", nlohmann::json::array({
                {{"type","Transform"},{"position",{0,0,0}},{"rotation",{0,0,0}},{"scale",{10,1,10}}},
                {{"type","MeshRenderer"},{"mesh","BuiltIn/Plane"},{"material","Default"}}
            })}
        });
    } else {
        s["entities"].push_back({
            {"id",1},{"name","Main Camera"},{"tag","MainCamera"},
            {"components", nlohmann::json::array({
                {{"type","Transform"},{"position",{0,0,-10}},{"rotation",{0,0,0}},{"scale",{1,1,1}}},
                {{"type","Camera2D"},{"size",5.0},{"clearColor","#1A1A2A"}}
            })}
        });
    }

    std::ofstream(scenesDir/"game.sce") << s.dump(2);
}

bool ProjectManager::openProject(
    const ProjectInfo& proj, const std::filesystem::path& engineExe)
{
    if (!std::filesystem::exists(engineExe)) return false;
    for (auto& p : m_projects)
        if (p.name == proj.name) { p.lastOpened = now_iso(); break; }
    save();
#ifdef _WIN32
    std::string cmd = "\"" + engineExe.string() + "\" \"" + proj.path.string() + "\"";
    STARTUPINFOA si{}; PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    return CreateProcessA(nullptr, cmd.data(), nullptr, nullptr,
                          FALSE, 0, nullptr, nullptr, &si, &pi) != 0;
#else
    return std::system(("\""+engineExe.string()+"\" \""+proj.path.string()+"\" &").c_str()) == 0;
#endif
}

void ProjectManager::removeProject(const std::string& name) {
    m_projects.erase(
        std::remove_if(m_projects.begin(), m_projects.end(),
            [&](const ProjectInfo& p){ return p.name == name; }),
        m_projects.end());
    save();
}

void ProjectManager::load() {
    auto file = Paths::projectsFile();
    if (!std::filesystem::exists(file)) return;
    std::ifstream in(file);
    try {
        auto root = nlohmann::json::parse(in);
        m_projects.clear();
        for (auto& item : root["projects"]) {
            ProjectInfo p;
            p.name          = item.value("name","");
            p.path          = item.value("path","");
            p.engineVersion = item.value("engineVersion","");
            p.lastOpened    = item.value("lastOpened","");
            p.templ         = item.value("template","3D")=="2D"
                                ? ProjectTemplate::Template2D
                                : ProjectTemplate::Template3D;
            if (!p.name.empty()) m_projects.push_back(p);
        }
    } catch(...) {}
}

void ProjectManager::save() {
    nlohmann::json root; root["projects"] = nlohmann::json::array();
    for (auto& p : m_projects) {
        root["projects"].push_back({
            {"name",p.name}, {"path",p.path.string()},
            {"engineVersion",p.engineVersion}, {"lastOpened",p.lastOpened},
            {"template",p.templ==ProjectTemplate::Template2D?"2D":"3D"}
        });
    }
    std::ofstream(Paths::projectsFile()) << root.dump(2);
}

std::string ProjectManager::nowIso() { return now_iso(); }

// ─── Paths impl ───────────────────────────────────────────────
std::filesystem::path Paths::exeDir()             { return std::filesystem::current_path(); }
std::filesystem::path Paths::editorsDir()         { return exeDir() / GK_EDITORS_SUBDIR; }
std::filesystem::path Paths::projectsFile()       { return exeDir() / "projects.json"; }
std::filesystem::path Paths::settingsFile()       { return exeDir() / "settings.json"; }
std::filesystem::path Paths::engineSettingsFile() { return exeDir() / "engine_settings.json"; }

} // namespace GK
