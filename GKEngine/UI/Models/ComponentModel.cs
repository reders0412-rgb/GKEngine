using ReactiveUI;

namespace GKEngine.UI.Models;

// ─── Base ────────────────────────────────────────────────────
public abstract class ComponentModel : ReactiveObject
{
    private bool _isExpanded = true;
    public string Name        { get; set; } = "";
    public string Icon        { get; set; } = "⬛";
    public bool   IsExpanded  { get => _isExpanded; set => this.RaiseAndSetIfChanged(ref _isExpanded, value); }
}

// ─── Transform ───────────────────────────────────────────────
public class TransformComponent : ComponentModel
{
    private double _px, _py, _pz;
    private double _rx, _ry, _rz;
    private double _sx = 1, _sy = 1, _sz = 1;

    public string Icon   => "⬛";
    public double PosX   { get => _px; set => this.RaiseAndSetIfChanged(ref _px, value); }
    public double PosY   { get => _py; set => this.RaiseAndSetIfChanged(ref _py, value); }
    public double PosZ   { get => _pz; set => this.RaiseAndSetIfChanged(ref _pz, value); }
    public double RotX   { get => _rx; set => this.RaiseAndSetIfChanged(ref _rx, value); }
    public double RotY   { get => _ry; set => this.RaiseAndSetIfChanged(ref _ry, value); }
    public double RotZ   { get => _rz; set => this.RaiseAndSetIfChanged(ref _rz, value); }
    public double ScaleX { get => _sx; set => this.RaiseAndSetIfChanged(ref _sx, value); }
    public double ScaleY { get => _sy; set => this.RaiseAndSetIfChanged(ref _sy, value); }
    public double ScaleZ { get => _sz; set => this.RaiseAndSetIfChanged(ref _sz, value); }
}

// ─── Camera ──────────────────────────────────────────────────
public class CameraComponent : ComponentModel
{
    private double _fov  = 60;
    private double _near = 0.1;
    private double _far  = 1000;
    private string _clearColor = "#1A1A2A";

    public double FOV        { get => _fov;        set => this.RaiseAndSetIfChanged(ref _fov,        value); }
    public double Near       { get => _near;       set => this.RaiseAndSetIfChanged(ref _near,       value); }
    public double Far        { get => _far;        set => this.RaiseAndSetIfChanged(ref _far,        value); }
    public string ClearColor { get => _clearColor; set => this.RaiseAndSetIfChanged(ref _clearColor, value); }
}

// ─── Light ───────────────────────────────────────────────────
public class LightComponent : ComponentModel
{
    private string _lightType  = "Directional";
    private string _color      = "#FFFFFF";
    private double _intensity  = 1.0;
    private bool   _castShadows = true;

    public string LightType   { get => _lightType;   set => this.RaiseAndSetIfChanged(ref _lightType,   value); }
    public string Color       { get => _color;       set => this.RaiseAndSetIfChanged(ref _color,       value); }
    public double Intensity   { get => _intensity;   set => this.RaiseAndSetIfChanged(ref _intensity,   value); }
    public bool   CastShadows { get => _castShadows; set => this.RaiseAndSetIfChanged(ref _castShadows, value); }
}

// ─── MeshRenderer ────────────────────────────────────────────
public class MeshRendererComponent : ComponentModel
{
    private string _mesh     = "BuiltIn/Cube";
    private string _material = "Default";
    private bool   _castShadows    = true;
    private bool   _receiveShadows = true;

    public string Mesh            { get => _mesh;           set => this.RaiseAndSetIfChanged(ref _mesh,           value); }
    public string Material        { get => _material;       set => this.RaiseAndSetIfChanged(ref _material,       value); }
    public bool   CastShadows     { get => _castShadows;    set => this.RaiseAndSetIfChanged(ref _castShadows,    value); }
    public bool   ReceiveShadows  { get => _receiveShadows; set => this.RaiseAndSetIfChanged(ref _receiveShadows, value); }
}

// ─── Script component placeholder ────────────────────────────
public class ScriptComponent : ComponentModel
{
    public string ScriptName { get; set; } = "MyScript";
}
