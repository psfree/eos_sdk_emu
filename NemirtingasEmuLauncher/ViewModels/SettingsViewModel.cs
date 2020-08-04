using Avalonia.Controls;
using HeaderReader;
using OSUtility;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;

using MessageBox.Avalonia;
using MessageBox.Avalonia.Enums;
using System.IO;
using DynamicData;
using System.Linq;
using System.Reactive.Linq;
using Avalonia;
using System.Reactive.Subjects;

namespace NemirtingasEmuLauncher.ViewModels
{
    public class SettingsViewModel : ViewModelBase
    {
        static public readonly string inherit_global = "Use Global Setting...";

        /////////////////////////////////////////////////
        /// "Settings" Tab
        public ObservableCollection<string> OverrideLanguageCombo { get; set; } = new ObservableCollection<string>();

        private string _SelectedOverrideLanguage = "";
        public string SelectedOverrideLanguage
        {
            get => _SelectedOverrideLanguage;
            set => RaiseAndSetIfChanged(ref _SelectedOverrideLanguage, value);
        }

        private string _UserName = "";
        public string UserName
        {
            get => _UserName;
            set
            {
                RaiseAndSetIfChanged(ref _UserName, value);
                EpicIdWatermark.OnNext(EOSProductId.GenerateIdFromName(_UserName).ToString());
                genParameterWatermark();
            }
        }

        private string _EpicId = "";
        public string EpicId
        {
            get => _EpicId;
            set
            {
                RaiseAndSetIfChanged(ref _EpicId, value);
                if (string.IsNullOrWhiteSpace(_EpicId))
                {
                    string name_id = EOSProductId.GenerateIdFromName(_UserName).ToString();
                    EpicIdWatermark.OnNext(name_id);
                }
                genParameterWatermark();
            }
        }

        private string _ProductUserId = "";
        public string ProductUserId
        {
            get => _ProductUserId;
            set
            {
                RaiseAndSetIfChanged(ref _ProductUserId, value);
                if (string.IsNullOrWhiteSpace(_ProductUserId))
                {
                    ProductUserIdWatermark.OnNext(EOSProductId.GenerateIdFromName(_EpicId).ToString());
                }
            }
        }

        public Subject<string> EpicIdWatermark = new Subject<string>();
        public Subject<string> ProductUserIdWatermark = new Subject<string>();

        public ObservableCollection<string> LogLevels { get; set; } = new ObservableCollection<string>();

        private string _SelectedLogLevel = "";
        public string SelectedLogLevel
        {
            get => _SelectedLogLevel;
            set => RaiseAndSetIfChanged(ref _SelectedLogLevel, value);
        }

        private bool? _EnableOverlay;
        public bool? EnableOverlay
        {
            get => _EnableOverlay;
            set => RaiseAndSetIfChanged(ref _EnableOverlay, value);
        }

        private bool? _UnlockDlcs;
        public bool? UnlockDlcs
        {
            get => _UnlockDlcs;
            set => RaiseAndSetIfChanged(ref _UnlockDlcs, value);
        }
        /////////////////////////////////////////////////
        /// "Game Settings" Tab

        private string _ItemId = string.Empty;
        public string ItemId
        {
            get => _ItemId;
            set
            {
                RaiseAndSetIfChanged(ref _ItemId, value);
                genParameterWatermark();
            }
        }

        private string _AppId = string.Empty;
        public string AppId
        {
            get => _AppId;
            set
            {
                RaiseAndSetIfChanged(ref _AppId, value);
                genParameterWatermark();
            }
        }
        private string _GameName = "";
        public string GameName
        {
            get => _GameName;
            set => RaiseAndSetIfChanged(ref _GameName, value);
        }
        private string _GameExePath = string.Empty;
        public string GameExePath
        {
            get => _GameExePath;
            set => RaiseAndSetIfChanged(ref _GameExePath, value);
        }
        private string _GameApiPath = string.Empty;
        public string GameApiPath
        {
            get => _GameApiPath;
            set => RaiseAndSetIfChanged(ref _GameApiPath, value);
        }
        private string _GameParameters = string.Empty;
        public string GameParameters
        {
            get => _GameParameters;
            set => RaiseAndSetIfChanged(ref _GameParameters, value);
        }

        private string _GameParametersWatermark = string.Empty;
        public string GameParametersWatermark
        {
            get => _GameParametersWatermark;
            set => RaiseAndSetIfChanged(ref _GameParametersWatermark, value);
        }

        private string _GameStartFolder = string.Empty;
        public string GameStartFolder
        {
            get => _GameStartFolder;
            set => RaiseAndSetIfChanged(ref _GameStartFolder, value);
        }

        public bool GameIsX64Enabled => OSDetector.IsWindows();

        private bool _GameIsX64 = false;
        public bool GameIsX64
        {
            get => _GameIsX64;
            set => RaiseAndSetIfChanged(ref _GameIsX64, value);
        }

        private string _GameSavePath = string.Empty;
        public string GameSavePath
        {
            get => _GameSavePath;
            set => RaiseAndSetIfChanged(ref _GameSavePath, value);
        }

        /////////////////////////////////////////////////
        /// "Entitlements" tab
        

        /////////////////////////////////////////////////
        /// "Custom" tab
        public ObservableCollection<string> AvailableLanguagesCombo { get; set; } = new ObservableCollection<string>();
        private string _SelectedAvailableLanguage = string.Empty;
        public string SelectedAvailableLanguage
        {
            get => _SelectedAvailableLanguage;
            set => RaiseAndSetIfChanged(ref _SelectedAvailableLanguage, value);
        }
        
        public ObservableCollection<string> SupportedLanguagesList { get; set; } = new ObservableCollection<string>();

        public ObservableCollection<string> SelectedSupportedLanguages { get; set; } = new ObservableCollection<string>();

        private string _CustomEnvVarKey = string.Empty;
        public string CustomEnvVarKey
        {
            get => _CustomEnvVarKey;
            set => RaiseAndSetIfChanged(ref _CustomEnvVarKey, value);
        }

        private string _CustomEnvVarValue = string.Empty;
        public string CustomEnvVarValue
        {
            get => _CustomEnvVarValue;
            set => RaiseAndSetIfChanged(ref _CustomEnvVarValue, value);
        }

        public ObservableCollection<EnvVar> CustomEnvVars { get; set; } = new ObservableCollection<EnvVar>();
        public ObservableCollection<EnvVar> SelectedCustomEnvVars { get; set; } = new ObservableCollection<EnvVar>();

        /////////////////////////////////////////////////
        /// Class members
        private GameConfig game_app { get; set; }

        private bool _IsDefaultApp = false;
        public bool IsDefaultApp
        {
            get => _IsDefaultApp;
            set => RaiseAndSetIfChanged(ref _IsDefaultApp, value);
        }

        private Windows.Settings _parent;
        public SettingsViewModel(Windows.Settings parent)
        {
            _parent = parent;
        }

        private void genParameterWatermark()
        {
            string username = (string.IsNullOrEmpty(UserName) ? EmuConfig.DefaultEmuConfig.UserName : UserName);

            GameParametersWatermark = "-AUTH_LOGIN=unused " +
                                      "-AUTH_PASSWORD=cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd " +
                                      "-AUTH_TYPE=exchangecode " +
                                      "-epicapp=" + AppId + " " +
                                      "-epicenv=Prod " +
                                      "-EpicPortal " +
                                      "-epicusername=\"" + username + "\" " +
                                      "-epicuserid=";
            string userid;
            if(string.IsNullOrEmpty(EpicId))
            {
                if(EmuConfig.DefaultEmuConfig.EosId == null)
                {
                    userid = EOSProductId.GenerateIdFromName(username);
                }
                else
                {
                    userid = EmuConfig.DefaultEmuConfig.EosId.ToString();
                }
            }
            else
            {
                userid = EpicId;
            }

            GameParametersWatermark += userid;
        }

        public void SetApp(GameConfig app, bool is_default_app)
        {
            IsDefaultApp = is_default_app;
            game_app = app;

            UserName         = game_app.EmuConfig.UserName;
            EnableOverlay    = game_app.EmuConfig.EnableOverlay;
            UnlockDlcs       = game_app.EmuConfig.UnlockEntitlements;
            LogLevels.AddRange(EmuConfig.LogLevels);
            SelectedLogLevel = game_app.EmuConfig.LogLevel;

            if (!is_default_app)
            {
                OverrideLanguageCombo.Insert(0, inherit_global);
                LogLevels.Insert(0, inherit_global);
            }
            
            for (int i = 0; i < EpicEmulator.EmuConfigs.Languages.Length; ++i)
            {
                string lang = EpicEmulator.EmuConfigs.Languages[i];
                if (is_default_app || app.Languages.Contains(lang))
                {
                    OverrideLanguageCombo.Add(lang);
                    SupportedLanguagesList.Add(lang);
                    if (string.IsNullOrEmpty(SelectedOverrideLanguage))
                    {
                        if (lang.Equals(app.EmuConfig.Language))
                        {
                            SelectedOverrideLanguage = lang;
                        }
                    }
                }
                else
                {
                    AvailableLanguagesCombo.Add(lang);
                }
            }
            
            if (string.IsNullOrEmpty(SelectedOverrideLanguage))
            {
                SelectedOverrideLanguage = is_default_app ? "english" : OverrideLanguageCombo[0];
            }
            if (string.IsNullOrEmpty(SelectedLogLevel))
            {
                SelectedLogLevel = is_default_app ? "OFF" : LogLevels[0];
            }

            if (!is_default_app)
            {
                _parent.SettingsTab.SelectedIndex = 1;

                _parent.TbxUserName.Watermark  = "Use Global Setting...";
                _parent.TbxEpicId.Watermark   = "Use Global Setting...";

                _parent.TbxGameSavePath.Watermark = EpicEmulator.GameEmuFolder;

                AppId           = game_app.AppId;
                ItemId          = game_app.ItemId;
                GameName        = game_app.AppName;
                GameStartFolder = game_app.StartFolder;
                GameExePath     = game_app.FullPath;
                GameApiPath     = game_app.FullApiPath;
                GameParameters  = game_app.Parameters;
                if (OSDetector.IsWindows())
                {
                    GameIsX64 = game_app.UseX64;
                }
                else
                {
                    GameIsX64 = true;
                }
                GameSavePath    = game_app.SavePath;
                CustomEnvVars.AddRange(game_app.EnvVars);
                //Dlcs.AddRange(game_app.Dlcs);
            }// Not default app
            else
            {
                _parent.TbxEpicId.Bind(TextBox.WatermarkProperty, EpicIdWatermark);                
            }

            EpicId = game_app.EmuConfig.EosId != null ? game_app.EmuConfig.EosId.ToString() : string.Empty;
            ProductUserId = game_app.EmuConfig.EosProductId != null ? game_app.EmuConfig.EosProductId.ToString() : string.Empty;

            //foreach (string ip in app.CustomBroadcasts)
            //{
            //    try
            //    {
            //        broadcast_listBox.Items.Add(IPAddress.Parse(ip));
            //    }
            //    catch { }
            //}
            //foreach (KeyValuePair<string, string> env_var in app.EnvVar)
            //{
            //    listBox_env_var.Items.Add(env_var);
            //}
        }

        private bool? isExeX64(string app_path)
        {
            ExecutableHeaderReader reader;

            try
            {
                if (!(reader = new PeHeaderReader(app_path)).IsValidHeader &&
                   !(reader = new ElfHeaderReader(app_path)).IsValidHeader &&
                   !(reader = new MachOHeaderReader(app_path)).IsValidHeader)
                {
                    reader = null;
                }
            }
            catch (Exception)
            {
                reader = null;
            }

            if (reader != null)
            {
                return !reader.Is32BitHeader;
            }

            return null;
        }

        public void OnButtonGenerateId()
        {
            EpicId = EOSProductId.RandomId();
        }
        
        public void OnButtonSave()
        {
            if (is_app_valid())
            {
                _parent.Close(DialogResult.DialogOk);
            }
        }

        public void OnButtonCancel()
        {
            _parent.Close(DialogResult.DialogCancel);
        }
        public async void OnButtonBrowseGameExe()
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if(OSDetector.IsWindows())
            {
                FileDialogFilter filter = new FileDialogFilter();
                filter.Name = "Game executables";
                filter.Extensions.Add("exe");
                openFileDialog.Filters.Add(filter);
                filter = new FileDialogFilter();
                filter.Name = "All files";
                filter.Extensions.Add("*");
                openFileDialog.Filters.Add(filter);
            }

            openFileDialog.AllowMultiple = false;
            openFileDialog.Directory = Path.GetDirectoryName(game_app.FullPath);
            var res = await openFileDialog.ShowAsync(_parent);

            if (res != null && res.Length > 0)
            {
                GameExePath = res[0];
                GameStartFolder = Path.GetDirectoryName(GameExePath);
                if (string.IsNullOrEmpty(GameName))
                {
                    GameName = Path.GetFileNameWithoutExtension(GameExePath);
                }

                bool? x64 = isExeX64(GameExePath);
                if(x64 != null)
                {
                    GameIsX64 = x64.Value;
                }
            }
        }

        public async void OnButtonBrowseGameApi()
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            switch(OSDetector.OsId)
            {
                case OsId.Windows:
                {
                    FileDialogFilter filter = new FileDialogFilter();
                    filter.Name = "Game Library";
                    filter.Extensions.Add("dll");
                    openFileDialog.Filters.Add(filter);
                }
                break;

                case OsId.Linux:
                {
                    FileDialogFilter filter = new FileDialogFilter();
                    filter.Name = "Game Library";
                    filter.Extensions.Add("so");
                    openFileDialog.Filters.Add(filter);
                }
                break;

                case OsId.MacOSX:
                {
                    FileDialogFilter filter = new FileDialogFilter();
                    filter.Name = "Game Library";
                    filter.Extensions.Add("dylib");
                    openFileDialog.Filters.Add(filter);
                }
                break;
            }

            openFileDialog.AllowMultiple = false;
            openFileDialog.Directory = game_app.StartFolder;
            var res = await openFileDialog.ShowAsync(_parent);

            if (res != null && res.Length > 0)
            {
                if (Path.GetFileName(res[0]) == OSFuncs.GetEpicAPIName(GameIsX64))
                {
                    GameApiPath = res[0];
                }
                else
                {
                    await MessageBoxManager.GetMessageBoxStandardWindow(
                        "Error",
                        "The Api name must be " + OSFuncs.GetEpicAPIName(GameIsX64),
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
                }
            }
        }

        public async void OnButtonBrowseGameStartFolder()
        {
            OpenFolderDialog openFolderDialog = new OpenFolderDialog();
            openFolderDialog.Directory = string.IsNullOrEmpty(GameStartFolder) ? 
                string.IsNullOrEmpty(GameExePath) ? EpicEmulator.LauncherFolder : Path.GetDirectoryName(GameExePath) 
                : GameStartFolder;
            openFolderDialog.Title = "Game start folder";
            var res = await openFolderDialog.ShowAsync(_parent);
            if (res != null && !string.IsNullOrEmpty(res))
            {
                GameStartFolder = res;
            }
        }
        
        private bool is_app_valid()
        {
            //foreach (IPAddress ip in broadcast_listBox.Items)
            //{
            //    modified_app.CustomBroadcasts.Add(ip.ToString());
            //}
            //
            
            if (!IsDefaultApp)
            {
                if (string.IsNullOrWhiteSpace(GameStartFolder) || !Directory.Exists(GameStartFolder))
                {
                    MessageBoxManager.GetMessageBoxStandardWindow(
                        "Error",
                        "Start folder " + GameStartFolder + " does not exist",
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
                    return false;
                }
            
                if (string.IsNullOrEmpty(GameExePath) || !File.Exists(GameExePath))
                {
                    MessageBoxManager.GetMessageBoxStandardWindow(
                        "Error",
                        "Game exe " + GameExePath + " does not exist",
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
                    return false;
                }

                if (string.IsNullOrWhiteSpace(AppId))
                {
                    MessageBoxManager.GetMessageBoxStandardWindow(
                        "Error",
                        "The AppId must not be empty",
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
                    return false;
                }

                if (string.IsNullOrWhiteSpace(GameName))
                {
                    MessageBoxManager.GetMessageBoxStandardWindow(
                        "Error",
                        "The GameName must not be empty",
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
                    return false;
                }

                EOSProductId epic_id = null;
                if (!string.IsNullOrWhiteSpace(EpicId) && !EOSProductId.TryParse(EpicId, out epic_id))
                {
                    MessageBoxManager.GetMessageBoxStandardWindow(
                        "Invalid SteamID",
                        "Please set a valid Epic ID (you can click \"Generate Random ID\") or remove it to use your global settings.",
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
                    return false;
                }

                game_app.AppId = AppId;
                game_app.ItemId = ItemId;
                game_app.UseX64 = GameIsX64;
                game_app.AppName = GameName;
                game_app.FullPath = GameExePath;
                game_app.FullApiPath = GameApiPath;
                game_app.StartFolder = GameStartFolder;
                game_app.Parameters = GameParameters;
                //game_app.CustomBroadcasts.Clear();
                //game_app.CustomBroadcasts.AddRange(Broadcasts);
                game_app.Languages.Clear();
                game_app.Languages.AddRange(SupportedLanguagesList);
                game_app.EnvVars.Clear();
                game_app.EnvVars.AddRange(CustomEnvVars);
                //game_app.Dlcs.Clear();
                //game_app.Dlcs.AddRange(Dlcs);
                game_app.SavePath = GameSavePath;
                game_app.EmuConfig.EosId = epic_id;
            }
            else
            {
                if (string.IsNullOrWhiteSpace(UserName))
                {
                    MessageBoxManager.GetMessageBoxStandardWindow(
                        "Invalid Username",
                        "Please set your username",
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
                    return false;
                }

                EOSProductId epic_id = null;
                if (!string.IsNullOrWhiteSpace(EpicId) && !EOSProductId.TryParse(EpicId, out epic_id))
                {
                    MessageBoxManager.GetMessageBoxStandardWindow(
                        "Invalid SteamID",
                        "Please set a valid SteamID (you can click \"Generate Random ID\") or remove it to auto generate based on username",
                        ButtonEnum.Ok,
                        Icon.Error).ShowDialog(_parent);
                    return false;
                }

                game_app.EmuConfig.EosId = epic_id;
            }
            
            game_app.EmuConfig.UserName = UserName;
            game_app.EmuConfig.Language = SelectedOverrideLanguage == inherit_global ? "" : SelectedOverrideLanguage;
            game_app.EmuConfig.UnlockEntitlements = UnlockDlcs;
            game_app.EmuConfig.EnableOverlay = EnableOverlay;
            game_app.EmuConfig.LogLevel = SelectedLogLevel == inherit_global ? "" : SelectedLogLevel;

            return true;
        }
        
        public void OnAddLanguage()
        {
            if (!string.IsNullOrWhiteSpace(SelectedAvailableLanguage))
            {
                bool inserted = false;
                for(int i = 1; i < OverrideLanguageCombo.Count; ++i)
                {
                    if(OverrideLanguageCombo[i].CompareTo(SelectedAvailableLanguage) > 0)
                    {
                        OverrideLanguageCombo.Insert(i, SelectedAvailableLanguage);
                        inserted = true;
                        break;
                    }
                }
                if(!inserted)
                {
                    OverrideLanguageCombo.Add(SelectedAvailableLanguage);
                }
                inserted = false;
                for (int i = 0; i < SupportedLanguagesList.Count; ++i)
                {
                    if (SupportedLanguagesList[i].CompareTo(SelectedAvailableLanguage) > 0)
                    {
                        SupportedLanguagesList.Insert(i, SelectedAvailableLanguage);
                        inserted = true;
                        break;
                    }
                }
                if (!inserted)
                {
                    SupportedLanguagesList.Add(SelectedAvailableLanguage);
                }

                AvailableLanguagesCombo.Remove(SelectedAvailableLanguage);
            }
        }

        public void OnRemoveLanguage()
        {
            List<string> tmp = SelectedSupportedLanguages.ToList();
            SupportedLanguagesList.Remove(tmp);
            OverrideLanguageCombo.Remove(tmp);

            using (List<string>.Enumerator lang_enum = tmp.GetEnumerator())
            {
                lang_enum.MoveNext();
                for (int i = 1; i < AvailableLanguagesCombo.Count; ++i)
                {
                    if (AvailableLanguagesCombo[i].CompareTo(lang_enum.Current) > 0)
                    {
                        AvailableLanguagesCombo.Insert(i, lang_enum.Current);
                        lang_enum.MoveNext();
                        if (lang_enum.Current == null)
                            break;
                    }
                }
                while (lang_enum.Current != null)
                {
                    AvailableLanguagesCombo.Add(lang_enum.Current);
                    lang_enum.MoveNext();
                }
            }
        }

        public void OnClearLanguage()
        {
            AvailableLanguagesCombo.Clear();
            AvailableLanguagesCombo.AddRange(EpicEmulator.EmuConfigs.Languages);
            OverrideLanguageCombo.Clear();
            OverrideLanguageCombo.Add(inherit_global);
            SelectedOverrideLanguage = inherit_global;

            SupportedLanguagesList.Clear();
        }

        public void OnEnvVarAdd()
        {
            if(!string.IsNullOrWhiteSpace(CustomEnvVarKey) && !string.IsNullOrWhiteSpace(CustomEnvVarValue))
            {
                CustomEnvVars.Add(new EnvVar(CustomEnvVarKey, CustomEnvVarValue));
                CustomEnvVarKey = string.Empty;
                CustomEnvVarValue = string.Empty;
            }
        }

        public void OnEnvVarRemove()
        {
            var tmp = new List<EnvVar>(SelectedCustomEnvVars);
            CustomEnvVars.Remove(tmp);
        }

        public void OnEnvVarClear()
        {
            CustomEnvVars.Clear();
        }

        public void OnDlcAdd()
        {
            //if (DlcId != 0 && !string.IsNullOrWhiteSpace(DlcName))
            //{
            //    try
            //    {
            //        Dlcs.Add(new Dlc { DlcId = DlcId, Name = DlcName });
            //        DlcId = 0;
            //        DlcName = string.Empty;
            //    }
            //    catch(Exception)
            //    { }
            //}
        }

        public void OnDlcRemove()
        {
            //var tmp = new List<Dlc>(SelectedDlcs);
            //Dlcs.Remove(tmp);
        }

        public void OnDlcClear()
        {
            //Dlcs.Clear();
        }
    }
}
