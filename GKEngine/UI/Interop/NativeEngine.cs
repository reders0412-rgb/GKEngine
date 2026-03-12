using System;
using System.Runtime.InteropServices;

namespace GKEngine.UI.Interop;

// ═══════════════════════════════════════════════════════════════
//  NativeEngine — GK_Engine_Core.dll P/Invoke 선언
//
//  GKEngineAPI.h의 모든 extern "C" 함수와 1:1 대응.
//  C# 쪽에서 IntPtr = C++ void* (GKScene, GKEntity 등)
// ═══════════════════════════════════════════════════════════════
public static class NativeEngine
{
    // DLL 이름 — 런타임에 같은 폴더에 있어야 함
    private const string DLL = "GK_Engine_Core.dll";

    // ─── 콜백 델리게이트 ─────────────────────────────────────
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void LogCallback(int level,
        [MarshalAs(UnmanagedType.LPStr)] string msg,
        [MarshalAs(UnmanagedType.LPStr)] string source);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void FrameCallback(double deltaTime);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void EntitySelectCallback(int entityId);

    // ═══════════════════════════════════════════════════════════
    //  ENGINE LIFECYCLE
    // ═══════════════════════════════════════════════════════════
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKEngine_Init(
        IntPtr nativeWindowHandle,
        int viewportWidth,
        int viewportHeight,
        [MarshalAs(UnmanagedType.LPStr)] string projectPath);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEngine_Shutdown();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKEngine_Resize(int w, int h);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEngine_SetLogCallback(LogCallback cb);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEngine_SetFrameCallback(FrameCallback cb);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEngine_SetEntitySelectCallback(EntitySelectCallback cb);

    // ═══════════════════════════════════════════════════════════
    //  RENDER LOOP
    // ═══════════════════════════════════════════════════════════
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEngine_Tick(double deltaTime);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEngine_RenderScene();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEngine_RenderGame();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEngine_Play();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEngine_Pause();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEngine_Stop();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKEngine_IsPlaying();

    // ═══════════════════════════════════════════════════════════
    //  SCENE
    // ═══════════════════════════════════════════════════════════
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKScene_Load(
        [MarshalAs(UnmanagedType.LPStr)] string path,
        out IntPtr outScene);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKScene_Save(IntPtr scene,
        [MarshalAs(UnmanagedType.LPStr)] string path);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKScene_Unload(IntPtr scene);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GKScene_GetActive();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKScene_GetEntityCount(IntPtr scene);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GKScene_GetEntityByIndex(IntPtr scene, int index);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GKScene_CreateEntity(IntPtr scene,
        [MarshalAs(UnmanagedType.LPStr)] string name);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKScene_DestroyEntity(IntPtr scene, IntPtr entity);

    // ═══════════════════════════════════════════════════════════
    //  ENTITY
    // ═══════════════════════════════════════════════════════════
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKEntity_GetId(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.LPStr)]
    public static extern string GKEntity_GetName(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEntity_SetName(IntPtr e,
        [MarshalAs(UnmanagedType.LPStr)] string name);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKEntity_IsActive(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKEntity_SetActive(IntPtr e, int active);

    // ─── Transform ───────────────────────────────────────────
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKTransform_GetPosition(IntPtr e,
        out float x, out float y, out float z);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKTransform_SetPosition(IntPtr e, float x, float y, float z);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKTransform_GetRotation(IntPtr e,
        out float x, out float y, out float z);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKTransform_SetRotation(IntPtr e, float x, float y, float z);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKTransform_GetScale(IntPtr e,
        out float x, out float y, out float z);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKTransform_SetScale(IntPtr e, float x, float y, float z);

    // ─── Camera ──────────────────────────────────────────────
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKCamera_HasComponent(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern float GKCamera_GetFov(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKCamera_SetFov(IntPtr e, float fov);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern float GKCamera_GetNear(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKCamera_SetNear(IntPtr e, float v);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern float GKCamera_GetFar(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKCamera_SetFar(IntPtr e, float v);

    // ─── Light ───────────────────────────────────────────────
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKLight_HasComponent(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKLight_GetType(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKLight_SetType(IntPtr e, int type);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern float GKLight_GetIntensity(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKLight_SetIntensity(IntPtr e, float v);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKLight_GetColor(IntPtr e,
        out float r, out float g, out float b);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKLight_SetColor(IntPtr e, float r, float g, float b);

    // ─── MeshRenderer ────────────────────────────────────────
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GKMesh_HasComponent(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.LPStr)]
    public static extern string GKMesh_GetMeshName(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.LPStr)]
    public static extern string GKMesh_GetMaterial(IntPtr e);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKMesh_SetMaterial(IntPtr e,
        [MarshalAs(UnmanagedType.LPStr)] string material);

    // ═══════════════════════════════════════════════════════════
    //  INPUT
    // ═══════════════════════════════════════════════════════════
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKInput_MouseMove(int x, int y);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKInput_MouseButton(int button, int pressed, int x, int y);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKInput_MouseScroll(float delta);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKInput_Key(int keyCode, int pressed);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void GKGizmo_SetMode(int mode);

    // ═══════════════════════════════════════════════════════════
    //  UTILS
    // ═══════════════════════════════════════════════════════════
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.LPStr)]
    public static extern string GKEngine_GetVersion();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.LPStr)]
    public static extern string GKEngine_GetLastError();
}
