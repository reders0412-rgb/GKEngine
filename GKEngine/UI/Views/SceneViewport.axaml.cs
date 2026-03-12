using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using GKEngine.UI.Services;
using GKEngine.UI.ViewModels;

namespace GKEngine.UI.Views;

/// <summary>
/// Scene 뷰포트 — NativeControlHost가 C++ OpenGL HWND를 child로 품음.
/// 마우스/키 이벤트를 P/Invoke로 C++ Core에 전달.
/// </summary>
public partial class SceneViewport : UserControl
{
    private bool _initialized;

    public SceneViewport()
    {
        InitializeComponent();

        // NativeControlHost가 HWND를 얻으면 엔진 초기화
        GlSceneHost.HandleCreated += OnHandleCreated;
        GlSceneHost.HandleDestroyed += OnHandleDestroyed;

        // 마우스 이벤트
        AddHandler(PointerMovedEvent,  OnPointerMoved,   RoutingStrategies.Tunnel);
        AddHandler(PointerPressedEvent, OnPointerPressed, RoutingStrategies.Tunnel);
        AddHandler(PointerReleasedEvent,OnPointerReleased,RoutingStrategies.Tunnel);
        AddHandler(PointerWheelChangedEvent, OnWheel,    RoutingStrategies.Tunnel);
    }

    // ── HWND 생성됐을 때 OpenGL init ─────────────────────────
    private void OnHandleCreated(object? sender, EventArgs e)
    {
        if (_initialized) return;
        _initialized = true;

        var hwnd    = GlSceneHost.Handle?.Handle ?? IntPtr.Zero;
        var bounds  = GlSceneHost.Bounds;
        int w = Math.Max(1, (int)bounds.Width);
        int h = Math.Max(1, (int)bounds.Height);

        var vm          = DataContext as EditorViewModel;
        var projectPath = vm?.ProjectPath ?? "";

        bool ok = EngineService.Instance.Init(hwnd, w, h, projectPath);
        if (ok && vm != null)
        {
            // C++ 씬에서 엔티티 목록 읽어 Hierarchy ViewModel에 반영
            vm.SyncFromCore();

            // C++ → C# 로그 콜백
            EngineService.Instance.OnLog += (level, msg, src) =>
                vm.AddConsoleEntry(level, msg, src);

            // C++ → C# 엔티티 선택 콜백
            EngineService.Instance.OnEntitySelected += (id) =>
                vm.SelectEntityById(id);
        }
    }

    private void OnHandleDestroyed(object? sender, EventArgs e)
    {
        EngineService.Instance.Shutdown();
        _initialized = false;
    }

    // ── 뷰포트 크기 변경 → Core resize ─────────────────────
    protected override void OnSizeChanged(SizeChangedEventArgs e)
    {
        base.OnSizeChanged(e);
        if (_initialized)
            EngineService.Instance.Resize((int)e.NewSize.Width, (int)e.NewSize.Height);
    }

    // ── 마우스 → P/Invoke → C++ Input ───────────────────────
    private void OnPointerMoved(object? sender, PointerEventArgs e)
    {
        var pt = e.GetPosition(GlSceneHost);
        EngineService.Instance.MouseMove((int)pt.X, (int)pt.Y);
    }

    private void OnPointerPressed(object? sender, PointerPressedEventArgs e)
    {
        var pt   = e.GetPosition(GlSceneHost);
        var prop = e.GetCurrentPoint(this).Properties;
        int btn  = prop.IsLeftButtonPressed  ? 0
                 : prop.IsRightButtonPressed ? 1
                 : 2;
        EngineService.Instance.MouseButton(btn, true, (int)pt.X, (int)pt.Y);
    }

    private void OnPointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        var pt   = e.GetPosition(GlSceneHost);
        int btn  = e.InitialPressMouseButton == MouseButton.Left  ? 0
                 : e.InitialPressMouseButton == MouseButton.Right ? 1
                 : 2;
        EngineService.Instance.MouseButton(btn, false, (int)pt.X, (int)pt.Y);
    }

    private void OnWheel(object? sender, PointerWheelEventArgs e)
    {
        EngineService.Instance.MouseScroll((float)e.Delta.Y);
    }

    // ── 기즈모 버튼 클릭 ─────────────────────────────────────
    public void OnGizmoModeChanged(int mode)
    {
        EngineService.Instance.SetGizmoMode(mode);
    }
}
