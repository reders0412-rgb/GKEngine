#pragma once
#include "Common.h"
#include <vector>
#include <functional>

namespace GK {

class VersionManager {
public:
    // ★ URL은 GKHub/CMakeLists.txt의 GK_MANIFEST_URL 에서 주입됨
    void checkForUpdates(ProgressCallback onProgress,
                         std::function<void(bool ok, const std::string& err)> onDone);

    void installVersion(const EngineVersion& ver,
                        ProgressCallback onProgress,
                        std::function<void(bool ok, const std::string& err)> onDone);

    void scanInstalled();

    const std::vector<EngineVersion>& versions() const { return m_versions; }
    bool isInstalled(const std::string& tag) const;
    std::filesystem::path engineExePath(const std::string& tag) const;

private:
    std::vector<EngineVersion> m_versions;

    bool fetchManifest(std::string& outJson, std::string& outErr);
    bool parseManifest(const std::string& json, std::string& outErr);
    bool downloadAndExtract(const EngineVersion& ver,
                            ProgressCallback onProgress, std::string& outErr);
};

} // namespace GK
