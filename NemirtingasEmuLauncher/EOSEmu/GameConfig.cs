/* Copyright (C) 2019-2020 Nemirtingas
   This file is part of the NemirtingasEmuLauncher Launcher

   The NemirtingasEmuLauncher Launcher is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The NemirtingasEmuLauncher Launcher is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the NemirtingasEmuLauncher; if not, see
   <http://www.gnu.org/licenses/>.
 */
using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Collections.ObjectModel;
using Avalonia;
using System.Xml.Serialization;

namespace NemirtingasEmuLauncher
{
    /// <summary>
    /// Contains the configuration for the game
    /// </summary>
    public class GameConfig : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void RaiseAndSetIfChanged<TProp>(ref TProp property, TProp value, [CallerMemberName] string prop_name = "")
        {
            if (property == null || !property.Equals(value))
            {
                property = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(prop_name));
            }
        }

        private void RaiseChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(AppName)));
        }

        public string AppImage
        {
            get
            {
                string image_path = Path.Combine(EpicEmulator.LauncherAppsCacheFolder, AppId, "background.jpg");
                return File.Exists(image_path) ? image_path : string.Empty;
            }
        }

        private const double _maxWidth = 460;
        private const double _maxHeight = 215;
        private static readonly Size _defaultSize = new Size(_maxWidth, _maxHeight);

        private Size _appImageSize = _defaultSize;
        public double AppImageWidth
        {
            get => _appImageSize.Width;
        }

        public double AppImageHeight
        {
            get => _appImageSize.Height;
        }

        [XmlIgnore]
        public Size AppImageSize
        {
            get => _appImageSize;
            set
            {
                if (value.Width == 0 || value.Height == 0)
                {
                    value = _defaultSize;
                }
                if (value.Width > value.Height)
                {
                    double ratio = value.Height / value.Width;
                    if (value.Height * ratio > _maxHeight)
                    {
                        value = new Size(_maxHeight / ratio, _maxHeight);
                    }
                    else
                    {
                        value = new Size(_maxWidth, _maxWidth * ratio);
                    }
                }
                else if(value.Height > _maxHeight)
                {
                    double ratio = value.Width / value.Height;
                    value = new Size(_maxHeight * ratio, _maxHeight);
                }
                RaiseAndSetIfChanged(ref _appImageSize, value);
            }
        }

        public EmuConfig EmuConfig { get; set; }
        public string StartFolder { get; set; }
        private string _appName = string.Empty;
        public string AppName
        {
            get => _appName;
            set => RaiseAndSetIfChanged(ref _appName, value);
        }

        private string _appId = string.Empty;
        public string AppId
        {
            get => _appId;
            set => RaiseAndSetIfChanged(ref _appId, value);
        }

        public string DefaultParameters
        {
            get
            {
                string username = (string.IsNullOrEmpty(EmuConfig.UserName) ? EmuConfig.DefaultEmuConfig.UserName : EmuConfig.UserName);
                string param = "-AUTH_LOGIN=unused " +
                "-AUTH_PASSWORD=cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd " +
                "-AUTH_TYPE=exchangecode " +
                "-epicapp=" + AppId + " " +
                "-epicenv=Prod " +
                "-EpicPortal " +
                "-epicusername=\"" + username + "\" " +
                "-epicuserid=";

                string id;
                if (EmuConfig.EosId == null || string.IsNullOrWhiteSpace(EmuConfig.EosId.Id))
                {
                    if(EmuConfig.DefaultEmuConfig.EosId == null || string.IsNullOrWhiteSpace(EmuConfig.DefaultEmuConfig.EosId.Id))
                    {
                        id = EOSProductId.GenerateIdFromName(username);
                    }
                    else
                    {
                        id = EmuConfig.DefaultEmuConfig.EosId.Id;
                    }
                }
                else
                {
                    id = EmuConfig.EosId.Id;
                }

                param += id;

                return param;
            }
        }

        private string _Parameters;
        public string Parameters
        {
            get => _Parameters;
            set => RaiseAndSetIfChanged(ref _Parameters, value);
        }

        public bool UseX64 { get; set; }
        public List<string> CustomBroadcasts { get; private set; }
        public List<EnvVar> EnvVars { get; private set; }
        public string SavePath { get; set; }
        public ObservableCollection<string> Languages { get; private set; }
        public bool DisableOnlineNetworking { get; set; }
        public Guid GameGuid { get; set; }
        

        // Absolute game path
        private string _fullPath;

        public string FullPath
        {
            get => _fullPath;
            set
            {
                try
                {
                    _fullPath = Path.GetFullPath(value);
                }
                catch (Exception)
                {
                    _fullPath = string.Empty;
                }   
            }
        }

        private string _fullApiPath;
        public string FullApiPath
        {
            get => _fullApiPath;
            set
            {
                try
                {
                    _fullApiPath = Path.GetFullPath(value);
                }
                catch (Exception)
                {
                    _fullApiPath = string.Empty;
                }
            }  
        }

        public GameConfig()
        {
            EmuConfig = new EmuConfig();
            EmuConfig.UnlockEntitlements = null;
            EmuConfig.EnableOverlay = null;

            StartFolder      = string.Empty;
            AppName          = string.Empty;
            Parameters       = string.Empty;
            UseX64           = false;
            FullPath         = string.Empty;
            FullApiPath      = string.Empty;
            CustomBroadcasts = new List<string>();
            EnvVars          = new List<EnvVar>();
            SavePath         = string.Empty;
            Languages        = new ObservableCollection<string>();
            Languages.Add("english");
            Languages.CollectionChanged += RaiseChanged;
            DisableOnlineNetworking = false;
            GameGuid         = Guid.NewGuid();
        }

        ~GameConfig()
        {
            Languages.CollectionChanged -= RaiseChanged;
        }
        public string GetGameEmuFolder()
        {
            return Path.Combine(EpicEmulator.LauncherFolder, "games");
        }
    }
}
