using System;
using System.Collections.Generic;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using GKEngine.UI.Services;
using GKEngine.UI.ViewModels;

namespace GKEngine.UI.Views;

public partial class EditorWindow : Window
{
    // Track detached panel windows so they can be re-docked
    private readonly Dictionary<string, Window> _detachedPanels = new();

    public EditorWindow()
    {
        InitializeComponent();
        DataContext = new EditorViewModel();

        I18nService.Instance.LanguageChanged += () =>
            (DataContext as EditorViewModel)?.RefreshI18n();

        // Load project from command-line arg
        var args = Environment.GetCommandLineArgs();
        if (args.Length > 1 && System.IO.Directory.Exists(args[1]))
        {
            Title = $"GK Engine 1.0 — {System.IO.Path.GetFileName(args[1])}";
            (DataContext as EditorViewModel)?.LoadAssets(args[1]);
        }
    }

    // ── Panel detach / re-dock ───────────────────────────────
    public void DetachPanel(string panelId, Control content, string title)
    {
        if (_detachedPanels.ContainsKey(panelId)) return;

        var win = new Window
        {
            Title   = $"GK Engine — {title}",
            Width   = 420,
            Height  = 500,
            Content = content,
            Background = Background,
        };
        win.Closed += (_, _) => _detachedPanels.Remove(panelId);
        _detachedPanels[panelId] = win;
        win.Show();
    }

    public bool IsPanelDetached(string panelId) => _detachedPanels.ContainsKey(panelId);
}
