#include "Engine.h"
#include "Scene/Scene.h"
#include <filesystem>

namespace GK {

Engine& Engine::get() {
    static Engine inst;
    return inst;
}

GK_RESULT Engine::init(void* hwnd, int w, int h, const char* projectPath) {
    m_hwnd        = hwnd;
    m_w = w; m_h  = h;
    m_projectPath = projectPath ? projectPath : "";

    m_input    = std::make_unique<Input>();
    m_renderer = std::make_unique<Renderer>();
    m_scene    = std::make_unique<Scene>();

    if (!m_renderer->init(hwnd, w, h)) {
        m_lastError = "OpenGL init failed";
        return GK_ERR_INIT;
    }

    // 프로젝트의 기본 씬 로드
    auto scenePath = std::filesystem::path(m_projectPath) / "Assets" / "Scenes" / "game.sce";
    m_scene->load(scenePath.string().c_str());

    log(0, "GK Engine Core 1.0 initialized.", "Engine");
    return GK_OK;
}

void Engine::shutdown() {
    if (m_renderer) m_renderer->shutdown();
    m_scene.reset();
    m_renderer.reset();
    m_input.reset();
}

void Engine::resize(int w, int h) {
    m_w = w; m_h = h;
    if (m_renderer) m_renderer->resize(w, h);
}

void Engine::tick(double dt) {
    if (!m_playing || m_paused) return;
    // GeekBehaviour 스크립트 Update() 호출 예정
    (void)dt;
}

void Engine::renderScene() {
    if (m_renderer && m_scene)
        m_renderer->renderScene(m_scene.get());
}

void Engine::renderGame() {
    if (m_renderer && m_scene)
        m_renderer->renderGame(m_scene.get());
}

void Engine::play()  { m_playing = true;  m_paused = false; log(0, "Play",  "Engine"); }
void Engine::pause() { m_paused  = !m_paused;               log(0, "Pause", "Engine"); }
void Engine::stop()  { m_playing = false; m_paused = false;  log(0, "Stop",  "Engine"); }

} // namespace GK
