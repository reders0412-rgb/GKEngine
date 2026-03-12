#pragma once
#include "Common.h"
#include <vector>

namespace GK {

class ProjectManager {
public:
    ProjectManager();
    void load();
    void save();

    struct CreateResult {
        bool ok = false;
        std::string error;
        std::filesystem::path projectRoot;
    };

    CreateResult createProject(
        const std::string& name,
        const std::filesystem::path& parentDir,
        ProjectTemplate templ,
        const std::string& engineVersion);

    bool openProject(const ProjectInfo& proj, const std::filesystem::path& engineExe);
    void removeProject(const std::string& name);

    const std::vector<ProjectInfo>& projects() const { return m_projects; }

private:
    std::vector<ProjectInfo> m_projects;

    void createFolderStructure(const std::filesystem::path& root, ProjectTemplate t) const;
    void writeProjectMeta(const std::filesystem::path& root, const ProjectInfo& info) const;
    void writeGameScene(const std::filesystem::path& scenesDir, ProjectTemplate t) const;
    static std::string nowIso();
};

} // namespace GK
