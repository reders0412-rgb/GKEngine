using Avalonia;
using Avalonia.Controls;

namespace GKEngine.UI.Views;

public partial class DetachablePanelView : UserControl
{
    public static readonly StyledProperty<string>  TitleProperty =
        AvaloniaProperty.Register<DetachablePanelView, string>(nameof(Title), "Panel");

    public static readonly StyledProperty<Control?> PanelBodyProperty =
        AvaloniaProperty.Register<DetachablePanelView, Control?>(nameof(PanelBody));

    public string  Title     { get => GetValue(TitleProperty);     set => SetValue(TitleProperty, value); }
    public Control? PanelBody { get => GetValue(PanelBodyProperty); set => SetValue(PanelBodyProperty, value); }

    private Window? _detachedWindow;

    public DetachablePanelView()
    {
        InitializeComponent();

        this.GetObservable(TitleProperty).Subscribe(t => PanelTitleText.Text = t);
        this.GetObservable(PanelBodyProperty).Subscribe(c => PanelContent.Content = c);

        DetachButton.Click += (_, _) => Detach();
        RedockButton.Click  += (_, _) => Redock();
    }

    private void Detach()
    {
        if (_detachedWindow != null) return;
        var body = PanelBody;
        if (body == null) return;

        // Remove from current parent
        PanelContent.Content = null;
        DetachButton.IsVisible = false;
        RedockButton.IsVisible = true;

        _detachedWindow = new Window
        {
            Title   = $"GK Engine — {Title}",
            Width   = 420,
            Height  = 520,
            Content = body,
            Background = Background,
        };
        _detachedWindow.Closed += (_, _) => Redock();
        _detachedWindow.Show();
    }

    private void Redock()
    {
        if (_detachedWindow == null) return;
        var body = _detachedWindow.Content as Control;
        _detachedWindow.Content = null;

        try { _detachedWindow.Close(); } catch { }
        _detachedWindow = null;

        PanelContent.Content   = body ?? PanelBody;
        DetachButton.IsVisible = true;
        RedockButton.IsVisible = false;
    }
}
