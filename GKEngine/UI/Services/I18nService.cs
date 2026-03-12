using System;
using System.Collections.Generic;

namespace GKEngine.UI.Services;

public class I18nService
{
    public static readonly I18nService Instance = new();

    public event Action? LanguageChanged;

    private Dictionary<string, string> _strings = new();

    public string Hierarchy    => Get("engine.hierarchy",    "Hierarchy");
    public string Inspector    => Get("engine.inspector",    "Inspector");
    public string Project      => Get("engine.project",      "Project");
    public string Console      => Get("engine.console",      "Console");
    public string Scene        => Get("engine.scene",        "Scene");
    public string Game         => Get("engine.game",         "Game");
    public string Play         => Get("engine.play",         "▶ Play");
    public string Pause        => Get("engine.pause",        "⏸ Pause");
    public string Step         => Get("engine.step",         "⏭ Step");
    public string AddComponent => Get("engine.addComponent", "+ Add Component");
    public string Transform    => Get("engine.transform",    "Transform");
    public string Position     => Get("engine.position",     "Position");
    public string Rotation     => Get("engine.rotation",     "Rotation");
    public string Scale        => Get("engine.scale",        "Scale");
    public string Camera       => Get("engine.camera",       "Camera");

    public void SetLanguage(string langCode)
    {
        // engine_settings.json 의 strings 섹션에서 읽어옴
        // (Hub가 기록한 번역 문자열)
        if (File.Exists(Path.Combine(AppContext.BaseDirectory, "engine_settings.json")))
        {
            try {
                var j = Newtonsoft.Json.Linq.JObject.Parse(
                    System.IO.File.ReadAllText(
                        System.IO.Path.Combine(AppContext.BaseDirectory, "engine_settings.json")));
                var strings = j["strings"];
                if (strings != null)
                {
                    _strings.Clear();
                    foreach (var prop in strings.Children<Newtonsoft.Json.Linq.JProperty>())
                        _strings[prop.Name] = prop.Value.ToString();
                }
            } catch { }
        }
        LanguageChanged?.Invoke();
    }

    private string Get(string key, string fallback) =>
        _strings.TryGetValue(key, out var v) ? v : fallback;
}

// using 단축 (SetLanguage 내부 참조용)
file static class File
{
    public static bool Exists(string path) => System.IO.File.Exists(path);
}
