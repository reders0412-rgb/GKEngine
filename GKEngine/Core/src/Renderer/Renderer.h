#pragma once
#include <cstdint>

namespace GK {

class Scene;

class Renderer {
public:
    bool init(void* hwnd, int w, int h);
    void shutdown();
    void resize(int w, int h);

    // 에디터 Scene 뷰 렌더
    void renderScene(Scene* scene);
    // 게임 Game 뷰 렌더 (카메라 기준)
    void renderGame(Scene* scene);

    void setGizmoMode(int mode) { m_gizmoMode = mode; }
    int  gizmoMode() const      { return m_gizmoMode; }

    void setSelectedEntity(int id) { m_selectedId = id; }

private:
    void* m_hwnd     = nullptr;
    void* m_hdc      = nullptr;
    void* m_hglrc    = nullptr;
    int   m_w = 0,   m_h = 0;
    int   m_gizmoMode  = 1;    // 0=Hand,1=Move,2=Rotate,3=Scale,4=Rect
    int   m_selectedId = -1;

    uint32_t m_gridVAO  = 0;
    uint32_t m_gridVBO  = 0;
    uint32_t m_cubeVAO  = 0;
    uint32_t m_planeVAO = 0;
    uint32_t m_shaderProg = 0;
    uint32_t m_gridShader = 0;
    uint32_t m_gizmoShader = 0;

    bool initGL();
    bool compileShaders();
    void buildBuiltinMeshes();
    void drawGrid();
    void drawEntity(class Entity* e);
    void drawGizmo(class Entity* e);
    void drawSelectionOutline(class Entity* e);
};

} // namespace GK
