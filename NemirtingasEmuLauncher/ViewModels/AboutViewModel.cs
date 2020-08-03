using OSUtility;
using System.Diagnostics;
using System.IO;
using MessageBox.Avalonia;
using MessageBox.Avalonia.Enums;
using NemirtingasEmuLauncher.Windows;

namespace NemirtingasEmuLauncher.ViewModels
{
    public class AboutViewModel : ViewModelBase
    {
        private About _parent;
        public AboutViewModel(Windows.About parent)
        {
            _parent = parent;
            EosApi = "Folder to epic emu\'s 32bits " + OSFuncs.GetEpicAPIName(false);
            EosApi64 = "Folder to epic emu\'s 64bits " + OSFuncs.GetEpicAPIName(true);
        }

        public void OpenGitPage()
        {
            string url = "https://gitlab.com/Nemirtingas/nemirtingas_steam_emu";
            Process.Start(new ProcessStartInfo(url) { CreateNoWindow = true, UseShellExecute = true });
        }

        public bool Is32BitsVisible => OSDetector.IsWindows();
        public bool Is64BitsVisible => true;

        private string eosApi;

        public string EosApi
        {
            get { return eosApi; }
            set { RaiseAndSetIfChanged(ref eosApi, value); }
        }

        private string eosApi64;

        public string EosApi64
        {
            get { return eosApi64; }
            set { RaiseAndSetIfChanged(ref eosApi64, value ); }
        }

        public void EosApi_Click()
        {
            OpenEmuFolder(false);
        }

        public void EosApi64_Click()
        {
            OpenEmuFolder(true);
        }

        private void OpenEmuFolder(bool x64)
        {
            string emu_folder = OSFuncs.GetEmuApiFolder(x64);

            if (!Directory.Exists(emu_folder))
            {
                Directory.CreateDirectory(emu_folder);
            }

            try
            {
                if (OSDetector.IsWindows())
                {
                    Process.Start("explorer.exe", emu_folder);
                }
                else if (OSDetector.IsLinux())
                {
                    Process.Start("nautilus", emu_folder);
                }
                else if (OSDetector.IsMacOS())
                {
                    Process.Start("open", emu_folder);
                }
            }
            catch
            {
                MessageBoxManager.GetMessageBoxStandardWindow(
                    "Game emu folder",
                    "Folder: " + emu_folder,
                    ButtonEnum.Ok,
                    Icon.Info).ShowDialog(_parent);
            }
        }

        public void Close_Click()
        {
            _parent.Close();
        }

    }
}
