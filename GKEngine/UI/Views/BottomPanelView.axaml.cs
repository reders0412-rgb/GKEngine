using Avalonia.Controls;
using Avalonia.Input;
using GKEngine.UI.ViewModels;

namespace GKEngine.UI.Views;

public partial class BottomPanelView : UserControl
{
    public BottomPanelView() => InitializeComponent();

    // ── 파일 드래그 앤 드롭 (Explorer → Assets 패널) ──────────
    private void OnAssetDragOver(object? sender, DragEventArgs e)
    {
        e.DragEffects = e.Data.Contains(DataFormats.Files)
            ? DragDropEffects.Copy
            : DragDropEffects.None;
    }

    private void OnAssetDrop(object? sender, DragEventArgs e)
    {
        if (!e.Data.Contains(DataFormats.Files)) return;

        var files = e.Data.GetFiles();
        if (files == null) return;

        var paths = new System.Collections.Generic.List<string>();
        foreach (var f in files)
        {
            var path = f.TryGetLocalPath();
            if (path != null) paths.Add(path);
        }

        (DataContext as EditorViewModel)?.OnFilesDropped(paths.ToArray());
    }
}
