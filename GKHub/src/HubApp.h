#pragma once
#include "Common.h"
#include "ProjectManager.h"
#include "SettingsManager.h"
#include "ThemeManager.h"
#include "I18n.h"
#include <memory>
#include <atomic>

// Forward declare RmlUi types
namespace Rml { class Context; }
struct SDL_Window;

namespace GK {

class HubApp {
public:
    static HubApp& get();

    int  run(int argc, char** argv);
    void quit() { m_running = false; }

    ProjectManager&  projects()  { return *m_projects; }
    SettingsManager& settings()  { return *m_settings; }

private:
    HubApp() = default;
    bool initSDL();
    bool initRml();
    void loadDocuments();
    void mainLoop();
    void shutdown();

    SDL_Window*    m_window   = nullptr;
    void*          m_glCtx   = nullptr;
    Rml::Context*  m_ctx     = nullptr;

    std::unique_ptr<ProjectManager>  m_projects;
    std::unique_ptr<SettingsManager> m_settings;

    std::atomic<bool> m_running{false};
};

} // namespace GK
