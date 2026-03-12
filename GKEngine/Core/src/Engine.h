#pragma once
#include "../include/GKEngineAPI.h"
#include "Scene/Scene.h"
#include "Renderer/Renderer.h"
#include "Input/Input.h"
#include <memory>
#include <string>
#include <atomic>

namespace GK {

class Engine {
public:
    static Engine& get();

    GK_RESULT init(void* hwnd, int w, int h, const char* projectPath);
    void shutdown();
    void resize(int w, int h);

    void tick(double dt);
    void renderScene();
    void renderGame();

    void play();
    void pause();
    void stop();
    bool isPlaying() const { return m_playing; }

    // ─── 콜백 ───────────────────────────────────────────────
    GKLogCallback           logCb   = nullptr;
    GKFrameCallback         frameCb = nullptr;
    GKEntitySelectCallback  selectCb = nullptr;

    void log(int level, const char* msg, const char* src) const {
        if (logCb) logCb(level, msg, src);
    }

    Scene*    activeScene()  { return m_scene.get(); }
    Renderer* renderer()     { return m_renderer.get(); }
    Input*    input()        { return m_input.get(); }

    const std::string& projectPath() const { return m_projectPath; }
    const std::string& lastError()   const { return m_lastError; }
    void setError(const std::string& e)    { m_lastError = e; }

private:
    Engine() = default;

    std::unique_ptr<Scene>    m_scene;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Input>    m_input;

    std::string m_projectPath;
    std::string m_lastError;

    std::atomic<bool> m_playing  { false };
    std::atomic<bool> m_paused   { false };

    void* m_hwnd = nullptr;
    int   m_w = 0, m_h = 0;
};

} // namespace GK
