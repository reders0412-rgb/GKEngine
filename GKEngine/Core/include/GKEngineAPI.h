#pragma once

// ═══════════════════════════════════════════════════════════════
//  GK Engine Core — Public C API
//  C# (P/Invoke) ↔ C++ 브리지
//
//  규칙:
//   - 모든 함수는 extern "C" + __cdecl (P/Invoke 호환)
//   - 문자열은 const char* (UTF-8)
//   - 핸들은 불투명 void* (C# 쪽에서는 IntPtr)
//   - 에러는 GK_RESULT 반환값으로 처리
// ═══════════════════════════════════════════════════════════════

#ifdef _WIN32
#  ifdef GKCORE_EXPORTS
#    define GKAPI __declspec(dllexport)
#  else
#    define GKAPI __declspec(dllimport)
#  endif
#else
#  define GKAPI __attribute__((visibility("default")))
#endif

#define GKCALL __cdecl

#ifdef __cplusplus
extern "C" {
#endif

// ─── Result codes ────────────────────────────────────────────
typedef enum GK_RESULT {
    GK_OK            =  0,
    GK_ERR_GENERIC   = -1,
    GK_ERR_INIT      = -2,
    GK_ERR_NOT_FOUND = -3,
    GK_ERR_NULL      = -4,
} GK_RESULT;

// ─── Opaque handles ──────────────────────────────────────────
typedef void* GKScene;
typedef void* GKEntity;
typedef void* GKRenderer;

// ─── Callbacks (C# 델리게이트 → C++ 함수 포인터) ─────────────
typedef void(GKCALL* GKLogCallback  )(int level, const char* msg, const char* source);
typedef void(GKCALL* GKFrameCallback)(double deltaTime);
typedef void(GKCALL* GKEntitySelectCallback)(int entityId);

// ═══════════════════════════════════════════════════════════════
//  ENGINE LIFECYCLE
// ═══════════════════════════════════════════════════════════════

// 엔진 초기화 — C#이 hwnd(HWND)를 넘겨서 뷰포트 HWND child 설정
GKAPI GK_RESULT GKCALL GKEngine_Init(
    void*       nativeWindowHandle,   // HWND (Scene viewport)
    int         viewportWidth,
    int         viewportHeight,
    const char* projectPath           // 프로젝트 루트 경로
);

GKAPI void      GKCALL GKEngine_Shutdown();
GKAPI GK_RESULT GKCALL GKEngine_Resize(int w, int h);

// ─── 콜백 등록 ───────────────────────────────────────────────
GKAPI void GKCALL GKEngine_SetLogCallback(GKLogCallback cb);
GKAPI void GKCALL GKEngine_SetFrameCallback(GKFrameCallback cb);
GKAPI void GKCALL GKEngine_SetEntitySelectCallback(GKEntitySelectCallback cb);

// ═══════════════════════════════════════════════════════════════
//  RENDER LOOP
// ═══════════════════════════════════════════════════════════════

// C#의 DispatcherTimer 또는 별도 스레드에서 매 프레임 호출
GKAPI void GKCALL GKEngine_Tick(double deltaTime);
GKAPI void GKCALL GKEngine_RenderScene();
GKAPI void GKCALL GKEngine_RenderGame();

// Play / Pause / Stop
GKAPI void GKCALL GKEngine_Play();
GKAPI void GKCALL GKEngine_Pause();
GKAPI void GKCALL GKEngine_Stop();
GKAPI int  GKCALL GKEngine_IsPlaying();

// ═══════════════════════════════════════════════════════════════
//  SCENE
// ═══════════════════════════════════════════════════════════════

GKAPI GK_RESULT GKCALL GKScene_Load(const char* scenePath, GKScene* outScene);
GKAPI GK_RESULT GKCALL GKScene_Save(GKScene scene, const char* scenePath);
GKAPI void      GKCALL GKScene_Unload(GKScene scene);
GKAPI GKScene   GKCALL GKScene_GetActive();

// ─── Entity 목록 (Hierarchy 패널용) ──────────────────────────
GKAPI int       GKCALL GKScene_GetEntityCount(GKScene scene);
GKAPI GKEntity  GKCALL GKScene_GetEntityByIndex(GKScene scene, int index);
GKAPI GKEntity  GKCALL GKScene_CreateEntity(GKScene scene, const char* name);
GKAPI void      GKCALL GKScene_DestroyEntity(GKScene scene, GKEntity entity);

// ═══════════════════════════════════════════════════════════════
//  ENTITY / COMPONENTS
// ═══════════════════════════════════════════════════════════════

GKAPI int         GKCALL GKEntity_GetId  (GKEntity e);
GKAPI const char* GKCALL GKEntity_GetName(GKEntity e);
GKAPI void        GKCALL GKEntity_SetName(GKEntity e, const char* name);
GKAPI int         GKCALL GKEntity_IsActive(GKEntity e);
GKAPI void        GKCALL GKEntity_SetActive(GKEntity e, int active);

// ─── Transform ───────────────────────────────────────────────
GKAPI void GKCALL GKTransform_GetPosition(GKEntity e, float* x, float* y, float* z);
GKAPI void GKCALL GKTransform_SetPosition(GKEntity e, float  x, float  y, float  z);
GKAPI void GKCALL GKTransform_GetRotation(GKEntity e, float* x, float* y, float* z);
GKAPI void GKCALL GKTransform_SetRotation(GKEntity e, float  x, float  y, float  z);
GKAPI void GKCALL GKTransform_GetScale   (GKEntity e, float* x, float* y, float* z);
GKAPI void GKCALL GKTransform_SetScale   (GKEntity e, float  x, float  y, float  z);

// ─── Camera ──────────────────────────────────────────────────
GKAPI int   GKCALL GKCamera_HasComponent(GKEntity e);
GKAPI float GKCALL GKCamera_GetFov      (GKEntity e);
GKAPI void  GKCALL GKCamera_SetFov      (GKEntity e, float fov);
GKAPI float GKCALL GKCamera_GetNear     (GKEntity e);
GKAPI void  GKCALL GKCamera_SetNear     (GKEntity e, float near);
GKAPI float GKCALL GKCamera_GetFar      (GKEntity e);
GKAPI void  GKCALL GKCamera_SetFar      (GKEntity e, float far);

// ─── Light ───────────────────────────────────────────────────
GKAPI int   GKCALL GKLight_HasComponent  (GKEntity e);
GKAPI int   GKCALL GKLight_GetType       (GKEntity e); // 0=Dir,1=Point,2=Spot
GKAPI void  GKCALL GKLight_SetType       (GKEntity e, int type);
GKAPI float GKCALL GKLight_GetIntensity  (GKEntity e);
GKAPI void  GKCALL GKLight_SetIntensity  (GKEntity e, float intensity);
GKAPI void  GKCALL GKLight_GetColor      (GKEntity e, float* r, float* g, float* b);
GKAPI void  GKCALL GKLight_SetColor      (GKEntity e, float  r, float  g, float  b);

// ─── MeshRenderer ────────────────────────────────────────────
GKAPI int         GKCALL GKMesh_HasComponent(GKEntity e);
GKAPI const char* GKCALL GKMesh_GetMeshName (GKEntity e);
GKAPI const char* GKCALL GKMesh_GetMaterial (GKEntity e);
GKAPI void        GKCALL GKMesh_SetMaterial (GKEntity e, const char* material);

// ═══════════════════════════════════════════════════════════════
//  INPUT (Scene viewport으로 전달)
// ═══════════════════════════════════════════════════════════════

GKAPI void GKCALL GKInput_MouseMove   (int x, int y);
GKAPI void GKCALL GKInput_MouseButton (int button, int pressed, int x, int y);
GKAPI void GKCALL GKInput_MouseScroll (float delta);
GKAPI void GKCALL GKInput_Key         (int keyCode, int pressed);

// ─── 기즈모 모드 ─────────────────────────────────────────────
GKAPI void GKCALL GKGizmo_SetMode(int mode); // 0=Hand,1=Move,2=Rotate,3=Scale,4=Rect

// ═══════════════════════════════════════════════════════════════
//  ASSET IMPORT
// ═══════════════════════════════════════════════════════════════

GKAPI GK_RESULT GKCALL GKAsset_Import(const char* srcPath, const char* assetDir);
GKAPI GK_RESULT GKCALL GKAsset_Delete(const char* assetPath);

// ═══════════════════════════════════════════════════════════════
//  UTILS
// ═══════════════════════════════════════════════════════════════

GKAPI const char* GKCALL GKEngine_GetVersion();
GKAPI const char* GKCALL GKEngine_GetLastError();

#ifdef __cplusplus
} // extern "C"
#endif
