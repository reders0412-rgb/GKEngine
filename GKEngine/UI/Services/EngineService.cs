using System;
using System.Collections.Generic;
using System.Diagnostics;
using Avalonia.Threading;
using GKEngine.UI.ViewModels;

namespace GKEngine.UI.Services;

/// <summary>
/// C# 쪽 엔진 서비스 — NativeEngine(P/Invoke) 위에서 동작.
/// ViewModel은 NativeEngine을 직접 쓰지 않고 이걸 씀.
/// </summary>
public class EngineService
{
    public static readonly EngineService Instance = new();

    // 콜백을 GC로부터 보호하기 위해 필드에 유지
    private NativeEngine.LogCallback?          _logCb;
    private NativeEngine.FrameCallback?        _frameCb;
    private NativeEngine.EntitySelectCallback? _selectCb;

    private DispatcherTimer? _renderTimer;
    private readonly Stopwatch _sw = Stopwatch.StartNew();
    private double _lastMs;

    public event Action<int, string, string>? OnLog;
    public event Action<int>?                 OnEntitySelected;
    public event Action?                      OnFrameTick;

    public bool IsInitialized { get; private set; }

    // ─── 초기화 ──────────────────────────────────────────────
    public bool Init(IntPtr hwnd, int w, int h, string projectPath)
    {
        // 콜백 등록 (GC 보호)
        _logCb    = (level, msg, src) =>
            Dispatcher.UIThread.Post(() => OnLog?.Invoke(level, msg, src));

        _frameCb  = (dt) =>
            Dispatcher.UIThread.Post(() => OnFrameTick?.Invoke());

        _selectCb = (id) =>
            Dispatcher.UIThread.Post(() => OnEntitySelected?.Invoke(id));

        NativeEngine.GKEngine_SetLogCallback(_logCb);
        NativeEngine.GKEngine_SetFrameCallback(_frameCb);
        NativeEngine.GKEngine_SetEntitySelectCallback(_selectCb);

        int r = NativeEngine.GKEngine_Init(hwnd, w, h, projectPath);
        IsInitialized = (r == 0);

        if (IsInitialized)
            StartRenderLoop();

        return IsInitialized;
    }

    public void Shutdown()
    {
        _renderTimer?.Stop();
        if (IsInitialized)
            NativeEngine.GKEngine_Shutdown();
        IsInitialized = false;
    }

    public void Resize(int w, int h)
    {
        if (IsInitialized)
            NativeEngine.GKEngine_Resize(w, h);
    }

    // ─── 렌더 루프 (~60fps, Dispatcher Timer) ────────────────
    private void StartRenderLoop()
    {
        _lastMs = _sw.Elapsed.TotalMilliseconds;
        _renderTimer = new DispatcherTimer(
            TimeSpan.FromMilliseconds(16),
            DispatcherPriority.Render,
            OnRenderTick);
        _renderTimer.Start();
    }

    private void OnRenderTick(object? sender, EventArgs e)
    {
        double now = _sw.Elapsed.TotalMilliseconds;
        double dt  = (now - _lastMs) / 1000.0;
        _lastMs    = now;

        NativeEngine.GKEngine_Tick(dt);
        NativeEngine.GKEngine_RenderScene();
        // Game view는 Game 탭이 활성화됐을 때만
        // NativeEngine.GKEngine_RenderGame();
    }

    // ─── Play / Pause / Stop ─────────────────────────────────
    public void Play()  => NativeEngine.GKEngine_Play();
    public void Pause() => NativeEngine.GKEngine_Pause();
    public void Stop()  => NativeEngine.GKEngine_Stop();
    public bool IsPlaying => IsInitialized && NativeEngine.GKEngine_IsPlaying() != 0;

    // ─── Scene 조작 ──────────────────────────────────────────
    public List<SceneObjectItem> GetSceneObjects()
    {
        var result = new List<SceneObjectItem>();
        if (!IsInitialized) return result;

        var scene = NativeEngine.GKScene_GetActive();
        if (scene == IntPtr.Zero) return result;

        int count = NativeEngine.GKScene_GetEntityCount(scene);
        for (int i = 0; i < count; i++)
        {
            var e    = NativeEngine.GKScene_GetEntityByIndex(scene, i);
            var name = NativeEngine.GKEntity_GetName(e);
            var icon = NativeEngine.GKCamera_HasComponent(e) != 0 ? "📷"
                     : NativeEngine.GKLight_HasComponent(e)  != 0 ? "💡"
                     : "⬛";
            result.Add(new SceneObjectItem {
                Id       = NativeEngine.GKEntity_GetId(e),
                Name     = name,
                Icon     = icon,
                NativeHandle = e,
            });
        }
        return result;
    }

    public SceneObjectItem? CreateEntity(string name)
    {
        if (!IsInitialized) return null;
        var scene = NativeEngine.GKScene_GetActive();
        var e     = NativeEngine.GKScene_CreateEntity(scene, name);
        if (e == IntPtr.Zero) return null;
        return new SceneObjectItem {
            Id           = NativeEngine.GKEntity_GetId(e),
            Name         = NativeEngine.GKEntity_GetName(e),
            Icon         = "⬛",
            NativeHandle = e,
        };
    }

    public void DestroyEntity(IntPtr handle)
    {
        if (!IsInitialized || handle == IntPtr.Zero) return;
        var scene = NativeEngine.GKScene_GetActive();
        NativeEngine.GKScene_DestroyEntity(scene, handle);
    }

    // ─── Transform ───────────────────────────────────────────
    public (float x, float y, float z) GetPosition(IntPtr e)
    {
        NativeEngine.GKTransform_GetPosition(e, out float x, out float y, out float z);
        return (x, y, z);
    }
    public void SetPosition(IntPtr e, float x, float y, float z)
        => NativeEngine.GKTransform_SetPosition(e, x, y, z);

    public (float x, float y, float z) GetRotation(IntPtr e)
    {
        NativeEngine.GKTransform_GetRotation(e, out float x, out float y, out float z);
        return (x, y, z);
    }
    public void SetRotation(IntPtr e, float x, float y, float z)
        => NativeEngine.GKTransform_SetRotation(e, x, y, z);

    public (float x, float y, float z) GetScale(IntPtr e)
    {
        NativeEngine.GKTransform_GetScale(e, out float x, out float y, out float z);
        return (x, y, z);
    }
    public void SetScale(IntPtr e, float x, float y, float z)
        => NativeEngine.GKTransform_SetScale(e, x, y, z);

    // ─── Input 전달 ──────────────────────────────────────────
    public void MouseMove  (int x, int y)                  => NativeEngine.GKInput_MouseMove(x, y);
    public void MouseButton(int b, bool p, int x, int y)   => NativeEngine.GKInput_MouseButton(b, p?1:0, x, y);
    public void MouseScroll(float d)                        => NativeEngine.GKInput_MouseScroll(d);
    public void SetGizmoMode(int m)                         => NativeEngine.GKGizmo_SetMode(m);

    // ─── 씬 저장 ─────────────────────────────────────────────
    public void SaveScene(string path)
    {
        if (!IsInitialized) return;
        var scene = NativeEngine.GKScene_GetActive();
        NativeEngine.GKScene_Save(scene, path);
    }
}
