using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;

using System.ComponentModel;

namespace NemirtingasEmuLauncher
{
    [DefaultValue(DialogUnknown)]
    public enum DialogResult
    {
        DialogUnknown = 0,
        DialogOk = 1,
        DialogCancel = 2,
    }

    public class App : Application
    {
        public override void Initialize()
        {
            AvaloniaXamlLoader.Load(this);
        }

        public override void OnFrameworkInitializationCompleted()
        {
            if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
            {
                desktop.MainWindow = new Windows.MainWindow();
            }

            base.OnFrameworkInitializationCompleted();
        }
    }
}
