using System;
using System.Collections.ObjectModel;
using System.Windows.Input;
using GKEngine.UI.Models;
using GKEngine.UI.Services;
using GKEngine.UI.Interop;
using ReactiveUI;

namespace GKEngine.UI.ViewModels;

// ─── SceneObjectItem (C++ Entity 핸들 포함) ──────────────────
public class SceneObjectItem : ReactiveObject
{
    private string _name = "";
    private string _icon = "⬛";
    private bool   _isSelected;
    private bool   _isVisible = true;
    private bool   _isLocked;

    public int     Id           { get; set; }
    public IntPtr  NativeHandle { get; set; } // C++ Entity* (불투명 핸들)
    public string  Name         { get => _name;       set => this.RaiseAndSetIfChanged(ref _name,       value); }
    public string  Icon         { get => _icon;       set => this.RaiseAndSetIfChanged(ref _icon,       value); }
    public bool    IsSelected   { get => _isSelected; set => this.RaiseAndSetIfChanged(ref _isSelected, value); }
    public bool    IsVisible    { get => _isVisible;  set => this.RaiseAndSetIfChanged(ref _isVisible,  value); }
    public bool    IsLocked     { get => _isLocked;   set => this.RaiseAndSetIfChanged(ref _isLocked,   value); }
}

// ─── Console ─────────────────────────────────────────────────
public enum LogLevel { Info, Warning, Error }
public class ConsoleEntry : ReactiveObject
{
    public LogLevel Level   { get; init; }
    public string   Message { get; init; } = "";
    public string   Source  { get; init; } = "";
    public string   Time    { get; init; } = "";
    public string   Icon    => Level switch { LogLevel.Warning => "⚠", LogLevel.Error => "✖", _ => "ℹ" };
    public string   Color   => Level switch { LogLevel.Warning => "#F0A030", LogLevel.Error => "#E05252", _ => "#8080A0" };
}

// ─── Asset ───────────────────────────────────────────────────
public class AssetItem : ReactiveObject
{
    private bool _isSelected;
    public string Name       { get; init; } = "";
    public string FullPath   { get; init; } = "";
    public string Icon       { get; init; } = "📄";
    public bool   IsFolder   { get; init; }
    public bool   IsSelected { get => _isSelected; set => this.RaiseAndSetIfChanged(ref _isSelected, value); }
}

// ═══════════════════════════════════════════════════════════════
public class EditorViewModel : ReactiveObject
{
    private readonly I18nService    _i18n = I18nService.Instance;
    private readonly EngineService  _eng  = EngineService.Instance;

    // ── i18n labels (축약) ───────────────────────────────────
    private string _hierarchyLabel = I18nService.Instance.Hierarchy;
    private string _inspectorLabel = I18nService.Instance.Inspector;
    private string _projectLabel   = I18nService.Instance.Project;
    private string _consoleLabel   = I18nService.Instance.Console;
    private string _sceneLabel     = I18nService.Instance.Scene;
    private string _gameLabel      = I18nService.Instance.Game;
    private string _playLabel      = I18nService.Instance.Play;
    private string _pauseLabel     = I18nService.Instance.Pause;
    private string _stepLabel      = I18nService.Instance.Step;
    private string _addCompLabel   = I18nService.Instance.AddComponent;
    private string _transformLabel = I18nService.Instance.Transform;
    private string _positionLabel  = I18nService.Instance.Position;
    private string _rotationLabel  = I18nService.Instance.Rotation;
    private string _scaleLabel     = I18nService.Instance.Scale;
    private string _cameraLabel    = I18nService.Instance.Camera;

    public string HierarchyLabel    { get => _hierarchyLabel;  set => this.RaiseAndSetIfChanged(ref _hierarchyLabel,  value); }
    public string InspectorLabel    { get => _inspectorLabel;  set => this.RaiseAndSetIfChanged(ref _inspectorLabel,  value); }
    public string ProjectLabel      { get => _projectLabel;    set => this.RaiseAndSetIfChanged(ref _projectLabel,    value); }
    public string ConsoleLabel      { get => _consoleLabel;    set => this.RaiseAndSetIfChanged(ref _consoleLabel,    value); }
    public string SceneLabel        { get => _sceneLabel;      set => this.RaiseAndSetIfChanged(ref _sceneLabel,      value); }
    public string GameLabel         { get => _gameLabel;       set => this.RaiseAndSetIfChanged(ref _gameLabel,       value); }
    public string PlayLabel         { get => _playLabel;       set => this.RaiseAndSetIfChanged(ref _playLabel,       value); }
    public string PauseLabel        { get => _pauseLabel;      set => this.RaiseAndSetIfChanged(ref _pauseLabel,      value); }
    public string StepLabel         { get => _stepLabel;       set => this.RaiseAndSetIfChanged(ref _stepLabel,       value); }
    public string AddComponentLabel { get => _addCompLabel;    set => this.RaiseAndSetIfChanged(ref _addCompLabel,    value); }
    public string TransformLabel    { get => _transformLabel;  set => this.RaiseAndSetIfChanged(ref _transformLabel,  value); }
    public string PositionLabel     { get => _positionLabel;   set => this.RaiseAndSetIfChanged(ref _positionLabel,   value); }
    public string RotationLabel     { get => _rotationLabel;   set => this.RaiseAndSetIfChanged(ref _rotationLabel,   value); }
    public string ScaleLabel        { get => _scaleLabel;      set => this.RaiseAndSetIfChanged(ref _scaleLabel,      value); }
    public string CameraLabel       { get => _cameraLabel;     set => this.RaiseAndSetIfChanged(ref _cameraLabel,     value); }

    // ── 프로젝트 경로 ────────────────────────────────────────
    public string ProjectPath { get; set; } = "";

    // ── Scene 오브젝트 (C++ 엔티티 미러) ─────────────────────
    public ObservableCollection<SceneObjectItem> SceneObjects { get; } = new();

    private SceneObjectItem? _selected;
    public SceneObjectItem? SelectedObject
    {
        get => _selected;
        set {
            if (_selected != null) _selected.IsSelected = false;
            this.RaiseAndSetIfChanged(ref _selected, value);
            if (_selected != null) _selected.IsSelected = true;
            LoadInspector(_selected);
        }
    }

    // ── Inspector ────────────────────────────────────────────
    public ObservableCollection<ComponentModel> Components { get; } = new();

    private void LoadInspector(SceneObjectItem? item)
    {
        Components.Clear();
        if (item == null || item.NativeHandle == IntPtr.Zero) return;

        var e = item.NativeHandle;

        // Transform (항상 있음)
        var (px, py, pz) = _eng.GetPosition(e);
        var (rx, ry, rz) = _eng.GetRotation(e);
        var (sx, sy, sz) = _eng.GetScale(e);
        Components.Add(new TransformComponent {
            Name = _i18n.Transform,
            PosX = px, PosY = py, PosZ = pz,
            RotX = rx, RotY = ry, RotZ = rz,
            ScaleX = sx, ScaleY = sy, ScaleZ = sz,
        });

        // Camera
        if (NativeEngine.GKCamera_HasComponent(e) != 0)
            Components.Add(new CameraComponent {
                Name = _i18n.Camera,
                FOV  = NativeEngine.GKCamera_GetFov(e),
                Near = NativeEngine.GKCamera_GetNear(e),
                Far  = NativeEngine.GKCamera_GetFar(e),
            });

        // Light
        if (NativeEngine.GKLight_HasComponent(e) != 0)
            Components.Add(new LightComponent {
                Name      = "Light",
                LightType = NativeEngine.GKLight_GetType(e) == 0 ? "Directional" : "Point",
                Intensity = NativeEngine.GKLight_GetIntensity(e),
            });

        // MeshRenderer
        if (NativeEngine.GKMesh_HasComponent(e) != 0)
            Components.Add(new MeshRendererComponent {
                Name     = "Mesh Renderer",
                Mesh     = NativeEngine.GKMesh_GetMeshName(e),
                Material = NativeEngine.GKMesh_GetMaterial(e),
            });
    }

    // ── Console ──────────────────────────────────────────────
    public ObservableCollection<ConsoleEntry> ConsoleEntries  { get; } = new();
    public ObservableCollection<ConsoleEntry> FilteredConsole { get; } = new();

    private bool   _showInfo = true, _showWarn = true, _showErr = true;
    private string _conSearch = "";
    public bool ShowInfo    { get => _showInfo;   set { this.RaiseAndSetIfChanged(ref _showInfo,   value); RefreshConsole(); } }
    public bool ShowWarning { get => _showWarn;   set { this.RaiseAndSetIfChanged(ref _showWarn,   value); RefreshConsole(); } }
    public bool ShowError   { get => _showErr;    set { this.RaiseAndSetIfChanged(ref _showErr,    value); RefreshConsole(); } }
    public string ConsoleSearch { get => _conSearch; set { this.RaiseAndSetIfChanged(ref _conSearch, value); RefreshConsole(); } }

    public void AddConsoleEntry(int level, string msg, string src)
    {
        var lv = level switch { 1 => LogLevel.Warning, 2 => LogLevel.Error, _ => LogLevel.Info };
        ConsoleEntries.Add(new ConsoleEntry {
            Level = lv, Message = msg, Source = src,
            Time = DateTime.Now.ToString("HH:mm:ss")
        });
        RefreshConsole();
    }

    private void RefreshConsole()
    {
        FilteredConsole.Clear();
        foreach (var e in ConsoleEntries)
        {
            if (e.Level == LogLevel.Info    && !_showInfo)  continue;
            if (e.Level == LogLevel.Warning && !_showWarn)  continue;
            if (e.Level == LogLevel.Error   && !_showErr)   continue;
            if (!string.IsNullOrEmpty(_conSearch) &&
                !e.Message.Contains(_conSearch, StringComparison.OrdinalIgnoreCase)) continue;
            FilteredConsole.Add(e);
        }
    }

    // ── Assets ───────────────────────────────────────────────
    private string _curAssetPath = "";
    public ObservableCollection<AssetItem> AssetItems     { get; } = new();
    public ObservableCollection<AssetItem> AssetBreadcrumb{ get; } = new();

    public void LoadAssets(string projectRoot)
    {
        ProjectPath    = projectRoot;
        _curAssetPath  = System.IO.Path.Combine(projectRoot, "Assets");
        RefreshAssets();
    }

    private void RefreshAssets()
    {
        AssetItems.Clear();
        AssetBreadcrumb.Clear();
        if (!System.IO.Directory.Exists(_curAssetPath)) return;

        var rel   = System.IO.Path.GetRelativePath(ProjectPath, _curAssetPath);
        var parts = rel.Split(System.IO.Path.DirectorySeparatorChar);
        var cur   = ProjectPath;
        foreach (var p in parts) {
            cur = System.IO.Path.Combine(cur, p);
            AssetBreadcrumb.Add(new AssetItem { Name = p, FullPath = cur, IsFolder = true, Icon = "📁" });
        }
        foreach (var dir in System.IO.Directory.EnumerateDirectories(_curAssetPath))
            AssetItems.Add(new AssetItem { Name = System.IO.Path.GetFileName(dir), FullPath = dir, IsFolder = true, Icon = "📁" });
        foreach (var file in System.IO.Directory.EnumerateFiles(_curAssetPath))
        {
            var ext  = System.IO.Path.GetExtension(file).ToLower();
            var icon = ext switch {
                ".sce" or ".gkscene" => "🎬", ".cs" => "📜",
                ".png" or ".jpg"     => "🖼",  ".fbx" or ".glb" => "🗿",
                ".wav" or ".mp3"     => "🔊",  _ => "📄" };
            AssetItems.Add(new AssetItem { Name = System.IO.Path.GetFileName(file), FullPath = file, IsFolder = false, Icon = icon });
        }
    }

    public void OnFilesDropped(string[] paths)
    {
        foreach (var src in paths)
        {
            try {
                NativeEngine.GKAsset_Import(src, _curAssetPath);
                AddConsoleEntry(0, $"Imported: {System.IO.Path.GetFileName(src)}", "AssetImporter");
            } catch (Exception ex) {
                AddConsoleEntry(2, $"Import failed: {ex.Message}", "AssetImporter");
            }
        }
        RefreshAssets();
    }

    // ── Play state ───────────────────────────────────────────
    private bool _isPlaying;
    public bool IsPlaying { get => _isPlaying; set => this.RaiseAndSetIfChanged(ref _isPlaying, value); }

    // ── Commands ─────────────────────────────────────────────
    public ICommand PlayCommand         { get; }
    public ICommand PauseCommand        { get; }
    public ICommand StepCommand         { get; }
    public ICommand ClearConsoleCommand { get; }
    public ICommand AddObjectCommand    { get; }
    public ICommand DeleteObjectCommand { get; }
    public ICommand UndoCommand         { get; }
    public ICommand RedoCommand         { get; }
    public ICommand SaveSceneCommand    { get; }
    public ICommand NewSceneCommand     { get; }

    public EditorViewModel()
    {
        RefreshConsole();

        PlayCommand = ReactiveCommand.Create(() => {
            if (_eng.IsPlaying) { _eng.Stop();  IsPlaying = false; }
            else                { _eng.Play();  IsPlaying = true;  }
            AddConsoleEntry(0, IsPlaying ? "▶ Play" : "⏹ Stop", "Editor");
        });

        PauseCommand        = ReactiveCommand.Create(() => _eng.Pause());
        StepCommand         = ReactiveCommand.Create(() => { });
        ClearConsoleCommand = ReactiveCommand.Create(() => { ConsoleEntries.Clear(); FilteredConsole.Clear(); });
        UndoCommand         = ReactiveCommand.Create(() => { });
        RedoCommand         = ReactiveCommand.Create(() => { });

        SaveSceneCommand = ReactiveCommand.Create(() => {
            var path = System.IO.Path.Combine(ProjectPath, "Assets", "Scenes", "game.sce");
            _eng.SaveScene(path);
            AddConsoleEntry(0, "Scene saved.", "Editor");
        });

        NewSceneCommand = ReactiveCommand.Create(() => {
            SceneObjects.Clear(); Components.Clear();
        });

        AddObjectCommand = ReactiveCommand.Create(() => {
            var obj = _eng.CreateEntity("New Object");
            if (obj != null) { SceneObjects.Add(obj); SelectedObject = obj; }
        });

        DeleteObjectCommand = ReactiveCommand.Create(() => {
            if (SelectedObject == null) return;
            _eng.DestroyEntity(SelectedObject.NativeHandle);
            SceneObjects.Remove(SelectedObject);
        });
    }

    // ── Core 엔티티 동기화 ───────────────────────────────────
    public void SyncFromCore()
    {
        SceneObjects.Clear();
        foreach (var obj in _eng.GetSceneObjects())
            SceneObjects.Add(obj);
        if (SceneObjects.Count > 0)
            SelectedObject = SceneObjects[0];
    }

    public void SelectEntityById(int id)
    {
        foreach (var obj in SceneObjects)
            if (obj.Id == id) { SelectedObject = obj; return; }
    }

    public void RefreshI18n()
    {
        HierarchyLabel    = _i18n.Hierarchy;
        InspectorLabel    = _i18n.Inspector;
        ProjectLabel      = _i18n.Project;
        ConsoleLabel      = _i18n.Console;
        SceneLabel        = _i18n.Scene;
        GameLabel         = _i18n.Game;
        PlayLabel         = _i18n.Play;
        PauseLabel        = _i18n.Pause;
        StepLabel         = _i18n.Step;
        AddComponentLabel = _i18n.AddComponent;
        TransformLabel    = _i18n.Transform;
        PositionLabel     = _i18n.Position;
        RotationLabel     = _i18n.Rotation;
        ScaleLabel        = _i18n.Scale;
        CameraLabel       = _i18n.Camera;
    }
}
