using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

using NemirtingasEmuLauncher.ViewModels;

namespace NemirtingasEmuLauncher.Windows
{
    public class Settings : Window
    {
        private TabControl _settingsTab;
        public TabControl SettingsTab => _settingsTab ??= this.FindControl<TabControl>("SettingsTab");

        private TextBox _tbxUserName;
        public TextBox TbxUserName => _tbxUserName ??= this.FindControl<TextBox>("tbxUserName");

        private TextBox _tbxEpicId;
        public TextBox TbxEpicId => _tbxEpicId ??= this.FindControl<TextBox>("tbxEpicId");

        private TextBox _tbxGameSavePath;
        public TextBox TbxGameSavePath => _tbxGameSavePath ??= this.FindControl<TextBox>("tbxGameSavePath");

        private TextBox _tbxEnvVarKey;
        public TextBox TbxEnvVarKey => _tbxEnvVarKey ??= this.FindControl<TextBox>("tbxEnvVarKey");

        private TextBox _tbxEnvVarValue;
        public TextBox TbxEnvVarValue => _tbxEnvVarValue ??= this.FindControl<TextBox>("tbxEnvVarValue");

        public Settings()
        {
            this.InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
        }

        private void InitializeComponent()
        {
            AvaloniaXamlLoader.Load(this);
            this.DataContext = new ViewModels.SettingsViewModel(this);
        }

        private SettingsViewModel Model => (SettingsViewModel)this.DataContext;

        public void SetGameConfig(GameConfig config, bool IsDefault)
        {
            config.EmuConfig = IsDefault ? EmuConfig.DefaultEmuConfig : config.EmuConfig;
            Model.SetApp(config, IsDefault);
        }
    }
}
