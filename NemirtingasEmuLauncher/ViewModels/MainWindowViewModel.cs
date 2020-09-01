using Avalonia.Controls;
using Avalonia.Input;

using MessageBox.Avalonia;
using MessageBox.Avalonia.Enums;

using DynamicData.Kernel;

using System;
using System.IO;
using System.Collections.ObjectModel;
using System.Threading.Tasks;
using OSUtility;

namespace NemirtingasEmuLauncher.ViewModels
{

    public class MainWindowViewModel : ViewModelBase
    {
        private Windows.MainWindow _parent;
        public ObservableCollectionAndItems<GameConfig> AppList { get; set; } = new ObservableCollectionAndItems<GameConfig>();

        public MainWindowViewModel(Windows.MainWindow parent)
        {
            _parent = parent;
            EpicEmulator.LoadSave(AppList);

            string[] args = Environment.GetCommandLineArgs();
            bool started_shortcut = false;
            if (args.Length >= 3)
            {
                for (int i = 1; i < args.Length; ++i)
                {
                    if (args[i] == "--game-guid" && ++i < args.Length)
                    {
                        try
                        {
                            Guid guid = Guid.Parse(args[i]);
                            GameConfig app = AppList.AsList().Find(p => p.GameGuid == guid);
                            ApiResult res = EpicEmulator.StartGame(app);
                            if (!res.Success)
                            {
                                MessageBoxManager.GetMessageBoxStandardWindow(
                                        "Error",
                                        "Failed to start shortcut: " + res.Message,
                                        ButtonEnum.Ok,
                                        Icon.Error).ShowDialog(_parent);
                            }

                            started_shortcut = true;
                        }
                        catch (Exception)
                        { }
                        break;
                    }
                }
            }

            if (started_shortcut)
            {
                _parent.IsVisible = false;
                _parent.Close();
            }
        }

        //////////////////////////////////////////////////
        /// UI events/properties
        public void OnMenuExit()
        {
            _parent.Close();
        }

        public void OnMenuOpenAbout()
        {
            Windows.About aboutBox = new Windows.About();
            aboutBox.WindowStartupLocation = WindowStartupLocation.CenterOwner;
            aboutBox.ShowDialog(_parent);
        }

        public async void OnMenuAddGame()
        {
            GameConfig app = new GameConfig();
            if (await openGameProperties(app, false) == DialogResult.DialogOk)
            {
                addGame(app);
            }
        }

        public async void OnMenuOpenSettings()
        {
            GameConfig app = new GameConfig();
            if (await openGameProperties(app, true) == DialogResult.DialogOk)
            {
                EpicEmulator.Save(AppList);
            }
        }

        public void OnGameClick(GameConfig app)
        {
            startGame(app);
        }

        public async void OnDropFiles(object sender, DragEventArgs e)
        {
            foreach (string file in e.Data.GetFileNames())
            {
                GameConfig app = new GameConfig();
                app.FullPath = file;
                app.AppName = Path.GetFileNameWithoutExtension(file);
                app.StartFolder = Path.GetDirectoryName(file);

                if (await openGameProperties(app, false) == DialogResult.DialogOk)
                {
                    addGame(app);
                }
            }
        }

        //////////////////////////////////////////////////
        /// Context Menu events/properties
        public void OnMenuGameStart(GameConfig app)
        {
            startGame(app);
        }

        public void OnMenuGameOpenEmuFolder(GameConfig app)
        {
            ApiResult res = EpicEmulator.ShowGameEmuFolder(app);
            if (!res.Success)
            {
                MessageBoxManager.GetMessageBoxStandardWindow(
                        "Error",
                        res.Message,
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
            }
        }

        public void OnMenuGameCreateShortcut(GameConfig app)
        {
            ApiResult res = EpicEmulator.CreateShortcut(app);
            if (!res.Success)
            {
                MessageBoxManager.GetMessageBoxStandardWindow(
                        "Error",
                        res.Message,
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
            }
        }

        public void OnMenuGameGenInfos(GameConfig app)
        {
            genGameInfos(app, true);
            EpicEmulator.Save(AppList);
        }

        public void OnMenuGameGenAchievements(GameConfig app)
        {
            //ApiResult res = EpicEmulator.GenerateGameAchievements(app);
            //if(!res.Success)
            //{ 
            //    MessageBoxManager.GetMessageBoxStandardWindow(
            //            "Error",
            //            res.Message,
            //            ButtonEnum.Ok,
            //            Icon.Error).ShowDialog(_parent);
            //}
        }

        public void OnMenuGameGenDlcs(GameConfig app)
        {
            //ApiResult res = EpicEmulator.GenerateGameDLCs(app);
            //if (res.Success)
            //{
            //    EpicEmulator.Save(AppList);
            //}
            //else
            //{
            //    MessageBoxManager.GetMessageBoxStandardWindow(
            //            "Error",
            //            res.Message,
            //            ButtonEnum.Ok,
            //            Icon.Error).ShowDialog(_parent);
            //}
        }

        public void OnMenuGameGenItems(GameConfig app)
        {
            //ApiResult res = EpicEmulator.GenerateGameItems(app);
            //if (!res.Success)
            //{
            //    MessageBoxManager.GetMessageBoxStandardWindow(
            //            "Error",
            //            res.Message,
            //            ButtonEnum.Ok,
            //            Icon.Error).ShowDialog(_parent);
            //}
        }

        public void OnMenuGameRemove(GameConfig app)
        {
            AppList.Remove(app);
            EpicEmulator.Save(AppList);
        }

        public async void OnMenuGameProperties(GameConfig app)
        {
            string oldAppId = app.AppId;
            if (await openGameProperties(app, false) == DialogResult.DialogOk)
            {
                if(oldAppId != app.AppId)
                {
                    genGameInfos(app);
                }
                EpicEmulator.Save(AppList);
            }
        }

        //////////////////////////////////////////////////
        /// Class functions
        private void startGame(GameConfig app)
        {
            ApiResult res = EpicEmulator.StartGame(app);
            if(!res.Success)
            {
                MessageBoxManager.GetMessageBoxStandardWindow(
                    "Error",
                    res.Message,
                    ButtonEnum.Ok,
                    Icon.Error).ShowDialog(_parent);
            }
        }

        private void addGame(GameConfig app)
        {
            genGameInfos(app);
            AppList.Add(app);
            EpicEmulator.Save(AppList);
        }

        private void genGameInfos(GameConfig app, bool clear_cache = false)
        {
            ApiResult res = EpicEmulator.GenerateGameInfos(app, clear_cache);
            if(!res.Success)
            {
                MessageBoxManager.GetMessageBoxStandardWindow(
                        "Error",
                        "Failed to retrieve game Infos: " + res.Message,
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
            }
        }

        private async Task<DialogResult> openGameProperties(GameConfig app, bool is_default_app)
        {
            Windows.Settings settingWindow = new Windows.Settings();
            settingWindow.SetGameConfig(app, is_default_app);

            settingWindow.WindowStartupLocation = Avalonia.Controls.WindowStartupLocation.CenterOwner;
            return await settingWindow.ShowDialog<DialogResult>(_parent);
        }
    }
}
