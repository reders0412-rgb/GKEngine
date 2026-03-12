using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;

namespace GKEngine.UI.Views;

public partial class ComponentBlockView : UserControl
{
    public static readonly StyledProperty<string>  IconProperty =
        AvaloniaProperty.Register<ComponentBlockView, string>(nameof(Icon), "⬛");

    public static readonly StyledProperty<string>  TitleProperty =
        AvaloniaProperty.Register<ComponentBlockView, string>(nameof(Title), "Component");

    public static readonly StyledProperty<bool>    IsExpandedProperty =
        AvaloniaProperty.Register<ComponentBlockView, bool>(nameof(IsExpanded), true);

    public static readonly StyledProperty<Control?> ComponentContentProperty =
        AvaloniaProperty.Register<ComponentBlockView, Control?>(nameof(ComponentContent));

    public string   Icon             { get => GetValue(IconProperty);             set => SetValue(IconProperty,             value); }
    public string   Title            { get => GetValue(TitleProperty);            set => SetValue(TitleProperty,            value); }
    public bool     IsExpanded       { get => GetValue(IsExpandedProperty);       set => SetValue(IsExpandedProperty,       value); }
    public Control? ComponentContent { get => GetValue(ComponentContentProperty); set => SetValue(ComponentContentProperty, value); }

    public ComponentBlockView()
    {
        InitializeComponent();
        this.GetObservable(IconProperty).Subscribe(v  => ComponentIconText.Text  = v);
        this.GetObservable(TitleProperty).Subscribe(v => ComponentTitleText.Text = v);
        this.GetObservable(ComponentContentProperty).Subscribe(v => ContentSlot.Content = v);
        this.GetObservable(IsExpandedProperty).Subscribe(UpdateExpanded);
    }

    private void Header_PointerPressed(object? sender, PointerPressedEventArgs e)
    {
        IsExpanded = !IsExpanded;
    }

    private void UpdateExpanded(bool expanded)
    {
        ContentArea.IsVisible = expanded;
        ExpandArrow.Text      = expanded ? "▼" : "▶";
    }
}
