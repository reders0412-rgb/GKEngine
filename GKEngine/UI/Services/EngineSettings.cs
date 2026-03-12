using System;
using System.IO;
using Newtonsoft.Json.Linq;

namespace GKEngine.UI.Services;

public class EngineSettingsData
{
    public string Theme    { get; set; } = "dark";
    public string Language { get; set; } = "en";
}

public static class EngineSettings
{
    public static EngineSettingsData Current { get; private set; } = new();

    private static FileSystemWatcher? _watcher;
    private static readonly string _path =
        Path.Combine(AppContext.BaseDirectory, "engine_settings.json");

    public static void Load()
    {
        if (!File.Exists(_path)) return;
        try {
            var j = JObject.Parse(File.ReadAllText(_path));
            Current.Theme    = j.Value<string>("theme")    ?? "dark";
            Current.Language = j.Value<string>("language") ?? "en";
        } catch { }
    }

    public static void StartWatcher(Action onChanged)
    {
        var dir = Path.GetDirectoryName(_path)!;
        _watcher = new FileSystemWatcher(dir, Path.GetFileName(_path))
        {
            NotifyFilter = NotifyFilters.LastWrite,
            EnableRaisingEvents = true,
        };
        _watcher.Changed += (_, _) => {
            System.Threading.Thread.Sleep(50); // 파일 쓰기 완료 대기
            Load();
            Avalonia.Threading.Dispatcher.UIThread.Post(onChanged);
        };
    }
}
