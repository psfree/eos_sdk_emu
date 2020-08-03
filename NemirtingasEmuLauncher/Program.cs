using System;
using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Logging.Serilog;
using OSUtility;

namespace NemirtingasEmuLauncher
{
    class Program
    {
        // Initialization code. Don't use any Avalonia, third-party APIs or any
        // SynchronizationContext-reliant code before AppMain is called: things aren't initialized
        // yet and stuff might break.
        [STAThread]
        public static void Main(string[] args) => BuildAvaloniaApp()
            .StartWithClassicDesktopLifetime(args, Avalonia.Controls.ShutdownMode.OnMainWindowClose);

        // Avalonia configuration, don't remove; also used by visual designer.
        public static AppBuilder BuildAvaloniaApp()
        {
            AppBuilder ab = AppBuilder.Configure<App>();
            ab.UsePlatformDetect();
#if DEBUG
            if (OSDetector.IsMacOS())
            {
                ab.With(new AvaloniaNativePlatformOptions { UseDeferredRendering = true, UseGpu = false });
            }
#endif
            ab.LogToDebug();

            return ab;
        }
    }
}
