using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace NemirtingasEmuLauncher.Windows
{
    public class About : Window
    {
        public About()
        {
            this.InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
        }
        private ViewModels.AboutViewModel _model ;
        private void InitializeComponent()
        {
            AvaloniaXamlLoader.Load(this);
            _model = new ViewModels.AboutViewModel(this);
            this.DataContext = _model;
        }
    }
}
