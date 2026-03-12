using System;
using System.IO;
using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using GKEngine.UI.Services;
using GKEngine.UI.Views;

namespace GKEngine.UI;

public partial class App : Application
{
    public override void Initialize() => AvaloniaXamlLoader.Load(this);

    public override void OnFrameworkInitializationCompleted()
    {
        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            // engine_settings.json 읽어서 테마/언어 적용
            EngineSettings.Load();
            ApplyTheme(EngineSettings.Current.Theme);

            // Hub → Engine 실시간 동기화 감시
            EngineSettings.StartWatcher(() =>
            {
                ApplyTheme(EngineSettings.Current.Theme);
                I18nService.Instance.SetLanguage(EngineSettings.Current.Language);
            });

            desktop.MainWindow = new EditorWindow();
        }
        base.OnFrameworkInitializationCompleted();
    }

    public void ApplyTheme(string theme)
    {
        RequestedThemeVariant = theme == "light"
            ? ThemeVariant.Light
            : ThemeVariant.Dark;
    }
}
