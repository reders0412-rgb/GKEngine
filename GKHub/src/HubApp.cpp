#include "HubApp.h"
#include "EventHandler.h"
#include "RmlBackend.h"
#include "SplashScreen.h"
#include <RmlUi/Core.h>
#include <SDL.h>
#include <iostream>

namespace GK {

HubApp& HubApp::get() { static HubApp inst; return inst; }

int HubApp::run(int argc, char** argv) {
    (void)argc; (void)argv;

    m_projects = std::make_unique<ProjectManager>();
    m_settings = std::make_unique<SettingsManager>();

    m_settings->load();
    m_settings->apply();
    m_projects->load();

    if (!initSDL())  return 1;
    if (!initRml())  return 1;

    loadDocuments();

    m_running = true;
    mainLoop();
    shutdown();
    return 0;
}

bool HubApp::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    m_window = SDL_CreateWindow("GK Hub " GK_VERSION,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1100, 680,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!m_window) return false;

    m_glCtx = SDL_GL_CreateContext(m_window);
    SDL_GL_SetSwapInterval(1);
    return true;
}

bool HubApp::initRml() {
    RmlBackend::init(m_window, m_glCtx);
    Rml::Initialise();
    m_ctx = Rml::CreateContext("main", Rml::Vector2i(1100, 680));
    if (!m_ctx) return false;

    // Load theme RCSS
    Rml::LoadFontFace("assets/fonts/NotoSans-Regular.ttf");
    Rml::LoadFontFace("assets/fonts/NotoSans-Bold.ttf");
    Rml::LoadFontFace("assets/fonts/NotoSansMono-Regular.ttf");
    return true;
}

void HubApp::loadDocuments() {
    // Load shared style + active theme
    m_ctx->LoadDocument("assets/ui/style.rcss");
    m_ctx->LoadDocument(ThemeManager::instance().activeThemeRcssPath());
    // Main UI
    auto* doc = m_ctx->LoadDocument("assets/ui/index.rml");
    if (doc) doc->Show();
}

void HubApp::mainLoop() {
    SDL_Event ev;
    while (m_running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) m_running = false;
            EventHandler::process(ev, m_ctx);
            RmlBackend::processEvent(ev);
        }
        m_ctx->Update();
        RmlBackend::beginFrame();
        m_ctx->Render();
        RmlBackend::endFrame(m_window);
    }
}

void HubApp::shutdown() {
    if (m_ctx) Rml::RemoveContext("main");
    Rml::Shutdown();
    RmlBackend::shutdown();
    if (m_glCtx) SDL_GL_DeleteContext(m_glCtx);
    if (m_window) SDL_DestroyWindow(m_window);
    SDL_Quit();
}

} // namespace GK
