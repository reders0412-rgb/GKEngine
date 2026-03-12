#include "../include/GKEngineAPI.h"
#include "Engine.h"
#include "Scene/Scene.h"
#include "ECS/Entity.h"

// ──────────────────────────────────────────────────────────────
//  모든 extern "C" 함수는 GK::Engine 싱글턴에 위임.
//  C# 쪽에서 P/Invoke로 직접 호출.
// ──────────────────────────────────────────────────────────────

#define ENG GK::Engine::get()

// ═══════════════════════════════════════════════════════════════
//  ENGINE LIFECYCLE
// ═══════════════════════════════════════════════════════════════

GKAPI GK_RESULT GKCALL GKEngine_Init(
    void* hwnd, int w, int h, const char* projectPath)
{
    return ENG.init(hwnd, w, h, projectPath);
}

GKAPI void GKCALL GKEngine_Shutdown()   { ENG.shutdown(); }

GKAPI GK_RESULT GKCALL GKEngine_Resize(int w, int h) {
    ENG.resize(w, h);
    return GK_OK;
}

GKAPI void GKCALL GKEngine_SetLogCallback(GKLogCallback cb)            { ENG.logCb    = cb; }
GKAPI void GKCALL GKEngine_SetFrameCallback(GKFrameCallback cb)        { ENG.frameCb  = cb; }
GKAPI void GKCALL GKEngine_SetEntitySelectCallback(GKEntitySelectCallback cb) { ENG.selectCb = cb; }

// ═══════════════════════════════════════════════════════════════
//  RENDER LOOP
// ═══════════════════════════════════════════════════════════════

GKAPI void GKCALL GKEngine_Tick(double dt)    { ENG.tick(dt); if (ENG.frameCb) ENG.frameCb(dt); }
GKAPI void GKCALL GKEngine_RenderScene()      { ENG.renderScene(); }
GKAPI void GKCALL GKEngine_RenderGame()       { ENG.renderGame();  }

GKAPI void GKCALL GKEngine_Play()             { ENG.play();  }
GKAPI void GKCALL GKEngine_Pause()            { ENG.pause(); }
GKAPI void GKCALL GKEngine_Stop()             { ENG.stop();  }
GKAPI int  GKCALL GKEngine_IsPlaying()        { return ENG.isPlaying() ? 1 : 0; }

// ═══════════════════════════════════════════════════════════════
//  SCENE
// ═══════════════════════════════════════════════════════════════

GKAPI GK_RESULT GKCALL GKScene_Load(const char* path, GKScene* out) {
    auto scene = std::make_unique<GK::Scene>();
    GK_RESULT r = scene->load(path);
    if (r == GK_OK) *out = scene.release();
    return r;
}

GKAPI GK_RESULT GKCALL GKScene_Save(GKScene s, const char* path) {
    return static_cast<GK::Scene*>(s)->save(path);
}

GKAPI void GKCALL GKScene_Unload(GKScene s) {
    delete static_cast<GK::Scene*>(s);
}

GKAPI GKScene GKCALL GKScene_GetActive() {
    return ENG.activeScene();
}

GKAPI int GKCALL GKScene_GetEntityCount(GKScene s) {
    return static_cast<GK::Scene*>(s)->entityCount();
}

GKAPI GKEntity GKCALL GKScene_GetEntityByIndex(GKScene s, int i) {
    return static_cast<GK::Scene*>(s)->entityAt(i);
}

GKAPI GKEntity GKCALL GKScene_CreateEntity(GKScene s, const char* name) {
    return static_cast<GK::Scene*>(s)->createEntity(name);
}

GKAPI void GKCALL GKScene_DestroyEntity(GKScene s, GKEntity e) {
    static_cast<GK::Scene*>(s)->destroyEntity(static_cast<GK::Entity*>(e));
}

// ═══════════════════════════════════════════════════════════════
//  ENTITY
// ═══════════════════════════════════════════════════════════════

#define ENT static_cast<GK::Entity*>(e)

GKAPI int         GKCALL GKEntity_GetId   (GKEntity e) { return ENT->id(); }
GKAPI const char* GKCALL GKEntity_GetName (GKEntity e) { return ENT->name().c_str(); }
GKAPI void        GKCALL GKEntity_SetName (GKEntity e, const char* n) { ENT->setName(n); }
GKAPI int         GKCALL GKEntity_IsActive(GKEntity e) { return ENT->isActive() ? 1 : 0; }
GKAPI void        GKCALL GKEntity_SetActive(GKEntity e, int a) { ENT->setActive(a != 0); }

// ─── Transform ───────────────────────────────────────────────
GKAPI void GKCALL GKTransform_GetPosition(GKEntity e, float* x, float* y, float* z) {
    auto p = ENT->transform().position;
    *x = p.x; *y = p.y; *z = p.z;
}
GKAPI void GKCALL GKTransform_SetPosition(GKEntity e, float x, float y, float z) {
    ENT->transform().position = {x, y, z};
}
GKAPI void GKCALL GKTransform_GetRotation(GKEntity e, float* x, float* y, float* z) {
    auto r = ENT->transform().rotation;
    *x = r.x; *y = r.y; *z = r.z;
}
GKAPI void GKCALL GKTransform_SetRotation(GKEntity e, float x, float y, float z) {
    ENT->transform().rotation = {x, y, z};
}
GKAPI void GKCALL GKTransform_GetScale(GKEntity e, float* x, float* y, float* z) {
    auto s = ENT->transform().scale;
    *x = s.x; *y = s.y; *z = s.z;
}
GKAPI void GKCALL GKTransform_SetScale(GKEntity e, float x, float y, float z) {
    ENT->transform().scale = {x, y, z};
}

// ─── Camera ──────────────────────────────────────────────────
GKAPI int   GKCALL GKCamera_HasComponent(GKEntity e) { return ENT->hasCamera() ? 1 : 0; }
GKAPI float GKCALL GKCamera_GetFov (GKEntity e) { return ENT->camera().fov; }
GKAPI void  GKCALL GKCamera_SetFov (GKEntity e, float v) { ENT->camera().fov = v; }
GKAPI float GKCALL GKCamera_GetNear(GKEntity e) { return ENT->camera().nearPlane; }
GKAPI void  GKCALL GKCamera_SetNear(GKEntity e, float v) { ENT->camera().nearPlane = v; }
GKAPI float GKCALL GKCamera_GetFar (GKEntity e) { return ENT->camera().farPlane; }
GKAPI void  GKCALL GKCamera_SetFar (GKEntity e, float v) { ENT->camera().farPlane = v; }

// ─── Light ───────────────────────────────────────────────────
GKAPI int   GKCALL GKLight_HasComponent(GKEntity e)       { return ENT->hasLight() ? 1 : 0; }
GKAPI int   GKCALL GKLight_GetType(GKEntity e)            { return (int)ENT->light().type; }
GKAPI void  GKCALL GKLight_SetType(GKEntity e, int t)     { ENT->light().type = (GK::LightType)t; }
GKAPI float GKCALL GKLight_GetIntensity(GKEntity e)       { return ENT->light().intensity; }
GKAPI void  GKCALL GKLight_SetIntensity(GKEntity e, float v) { ENT->light().intensity = v; }
GKAPI void  GKCALL GKLight_GetColor(GKEntity e, float* r, float* g, float* b) {
    auto& c = ENT->light().color;
    *r = c.r; *g = c.g; *b = c.b;
}
GKAPI void  GKCALL GKLight_SetColor(GKEntity e, float r, float g, float b) {
    ENT->light().color = {r, g, b};
}

// ─── MeshRenderer ────────────────────────────────────────────
GKAPI int         GKCALL GKMesh_HasComponent(GKEntity e)          { return ENT->hasMesh() ? 1 : 0; }
GKAPI const char* GKCALL GKMesh_GetMeshName (GKEntity e)          { return ENT->mesh().meshName.c_str(); }
GKAPI const char* GKCALL GKMesh_GetMaterial (GKEntity e)          { return ENT->mesh().material.c_str(); }
GKAPI void        GKCALL GKMesh_SetMaterial (GKEntity e, const char* m) { ENT->mesh().material = m; }

// ═══════════════════════════════════════════════════════════════
//  INPUT
// ═══════════════════════════════════════════════════════════════

GKAPI void GKCALL GKInput_MouseMove   (int x, int y)             { ENG.input()->mouseMove(x, y); }
GKAPI void GKCALL GKInput_MouseButton (int b, int p, int x, int y){ ENG.input()->mouseButton(b, p!=0, x, y); }
GKAPI void GKCALL GKInput_MouseScroll (float d)                   { ENG.input()->mouseScroll(d); }
GKAPI void GKCALL GKInput_Key         (int k, int p)             { ENG.input()->key(k, p!=0); }
GKAPI void GKCALL GKGizmo_SetMode     (int m)                    { ENG.renderer()->setGizmoMode(m); }

// ═══════════════════════════════════════════════════════════════
//  ASSET
// ═══════════════════════════════════════════════════════════════

GKAPI GK_RESULT GKCALL GKAsset_Import(const char* src, const char* dst) {
    return GK::AssetImporter::importFile(src, dst);
}
GKAPI GK_RESULT GKCALL GKAsset_Delete(const char* path) {
    std::filesystem::remove(path);
    return GK_OK;
}

// ═══════════════════════════════════════════════════════════════
//  UTILS
// ═══════════════════════════════════════════════════════════════

GKAPI const char* GKCALL GKEngine_GetVersion()   { return GK_ENGINE_VERSION; }
GKAPI const char* GKCALL GKEngine_GetLastError() { return ENG.lastError().c_str(); }
