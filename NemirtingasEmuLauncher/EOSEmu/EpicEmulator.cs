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
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using System.Diagnostics;

using OSUtility;
using Newtonsoft.Json.Linq;
using System.Net;
using SkiaSharp;
using Avalonia.Media.Imaging;

namespace NemirtingasEmuLauncher
{
    static class EpicEmulator
    {
        private static string _launcherFolder;
        public static string LauncherFolder => _launcherFolder ??= Path.GetDirectoryName(Environment.GetCommandLineArgs()[0]);

        private static string _launcherAppsCacheFolder;
        public static string LauncherAppsCacheFolder => _launcherAppsCacheFolder ??= Path.Combine(LauncherFolder, "apps_cache_infos");

        private static string _gameEmuFolder;
        public static string GameEmuFolder => _gameEmuFolder ??= Path.Combine(LauncherFolder, "games");

        private static Dictionary<Process, GameConfig> emuGamesProcess = new Dictionary<Process, GameConfig>();

        public static readonly string EmuJsonName = "NemirtingasEpicEmu.json";

        public static EmuStringsConfig EmuConfigs { get; private set; } = new EmuStringsConfig();

        static EpicEmulator()
        {
            EmuConfigs.Languages = new string[]{ "english", "french" };
        }

        private static ApiResult getGameSavePath(GameConfig app)
        {
            EOSProductId eos_id = app.EmuConfig.EosId;
            if (eos_id == null)
            {
                eos_id = EmuConfig.DefaultEmuConfig.EosId;
                if(eos_id == null)
                {
                    return new ApiResult { Success = false, Message = "Invalid EosId: not a 128 bits hex string" };
                }
            }

            string save_path;
            if (string.IsNullOrWhiteSpace(app.SavePath))
            {
                save_path = GameEmuFolder;
            }
            else if (app.SavePath == "appdata")
            {
                save_path = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData));
            }
            else
            {
                save_path = Path.Combine(app.StartFolder, app.SavePath);
            }

            string game_folder = Path.Combine(save_path, "NemirtingasEpicEmu", eos_id.ToString(), app.AppName);
            Directory.CreateDirectory(game_folder);
            return new ApiResult { Success = true, Message = game_folder };
        }

        public static void LoadSave(ICollection<GameConfig> app_list)
        {
            app_list.Clear();
            try
            {
                SavedConf save = new SavedConf();
                var xmlserializer = new XmlSerializer(save.GetType());

                using (FileStream file = File.Open(Path.Combine(LauncherFolder, "NemirtingasEmuLauncher.cfg"), FileMode.Open))
                {
                    save = (SavedConf)xmlserializer.Deserialize(file);

                    foreach (var app in save.apps)
                    {
                        if (app != null)
                        {
                            try
                            {
                                string app_cache_image = Path.Combine(EpicEmulator.LauncherAppsCacheFolder, app.AppId.ToString(), "background.jpg");
                                Bitmap img = new Bitmap(app_cache_image);
                                app.AppImageWidth = (int)img.Size.Width;
                                app.AppImageHeight = (int)img.Size.Height;
                            }
                            catch(Exception)
                            { }
                            app_list.Add(app);
                        }
                    }
                    EmuConfig.DefaultEmuConfig = save.config;
                }

                if(EmuConfig.DefaultEmuConfig.EosId == null)
                {
                    EmuConfig.DefaultEmuConfig.EosId = new EOSProductId();
                }
            }
            catch (Exception)
            {
            }
        }

        public static void Save(IEnumerable<GameConfig> app_list)
        {
            SavedConf save = new SavedConf();
            var xmlserializer = new XmlSerializer(save.GetType());
            var stringWriter = new StringWriter();

            XmlWriterSettings settings = new XmlWriterSettings();
            settings.Indent = true;
            settings.IndentChars = ("  ");
            settings.OmitXmlDeclaration = true;

            save.apps.AddRange(app_list);
            save.config = EmuConfig.DefaultEmuConfig;

            using (var writer = XmlWriter.Create(stringWriter, settings))
            {
                xmlserializer.Serialize(writer, save);
                using (FileStream file = File.Open(Path.Combine(LauncherFolder, "NemirtingasEmuLauncher.cfg"), FileMode.Create))
                {
                    byte[] datas = new UTF8Encoding(true).GetBytes(stringWriter.ToString());
                    file.Write(datas, 0, datas.Length);
                }
            }
        }

        private static string getAppUsername(GameConfig app)
        {
            if (string.IsNullOrWhiteSpace(app.EmuConfig.UserName))
            {
                if (string.IsNullOrWhiteSpace(EmuConfig.DefaultEmuConfig.UserName))
                {
                    return null;
                }
                return EmuConfig.DefaultEmuConfig.UserName;
            }
            else
            {
                return app.EmuConfig.UserName;
            }
        }

        private static EOSProductId getAppUserId(GameConfig app)
        {
            if (app.EmuConfig.EosId == null || string.IsNullOrWhiteSpace(app.EmuConfig.EosId.Id))
            {
                if (EmuConfig.DefaultEmuConfig.EosId == null || string.IsNullOrWhiteSpace(EmuConfig.DefaultEmuConfig.EosId.Id))
                {
                    string username = getAppUsername(app);
                    if (string.IsNullOrWhiteSpace(username))
                        return null;

                    return new EOSProductId(EOSProductId.GenerateIdFromName(username));
                }
                else
                {
                    return EmuConfig.DefaultEmuConfig.EosId;
                }
            }
            else
            {
                return app.EmuConfig.EosId;
            }
        }

        private static ApiResult BuildEmuJsonCfg(GameConfig game_app, out JObject emu_cfg)
        {
            string os_folder = OSFuncs.GetEmuApiFolder(game_app.UseX64);

            emu_cfg = new JObject();

            if (string.IsNullOrWhiteSpace(os_folder))
            {
                return new ApiResult { Success = false, Message = "Unable to find emulator folder" };
            }

            string emu_path = os_folder + OSFuncs.GetEpicAPIName(game_app.UseX64);
            if (!File.Exists(emu_path))
            {
                return new ApiResult { Success = false, Message = "Cannot find epic emulator: " + emu_path };
            }

            if (string.IsNullOrWhiteSpace(game_app.EmuConfig.Language))
            {
                if(string.IsNullOrWhiteSpace(EmuConfig.DefaultEmuConfig.Language))
                {
                    // This should never happen
                    return new ApiResult { Success = false, Message = "Invalid language: must not be empty" };
                }
                emu_cfg["language"] = EmuConfig.DefaultEmuConfig.Language;
            }
            else
            {
                emu_cfg["language"] = game_app.EmuConfig.Language;
            }

            if (string.IsNullOrWhiteSpace(game_app.EmuConfig.LogLevel))
            {
                if (string.IsNullOrWhiteSpace(EmuConfig.DefaultEmuConfig.LogLevel))
                {
                    // This should never happen
                    return new ApiResult { Success = false, Message = "Invalid LogLevel: must not be empty" };
                }
                emu_cfg["log_level"] = EmuConfig.DefaultEmuConfig.LogLevel;
            }
            else
            {
                emu_cfg["log_level"] = game_app.EmuConfig.LogLevel;
            }

            string username = getAppUsername(game_app);
            if(string.IsNullOrWhiteSpace(username))
            {
                return new ApiResult { Success = false, Message = "Invalid username: must not be empty" };
            }

            emu_cfg["username"] = username;

            EOSProductId epicid = getAppUserId(game_app);
            if(!epicid.IsValid())
            {
                return new ApiResult { Success = false, Message = "Invalid epicid: must be a 128bits hex string" };
            }

            emu_cfg["epicid"] = epicid.Id;

            if (string.IsNullOrWhiteSpace(game_app.AppName))
            {
                return new ApiResult { Success = false, Message = "Invalid game name: must not be empty" };
            }

            if(game_app.EmuConfig.EosProductId == null)
            {
                if(EmuConfig.DefaultEmuConfig.EosProductId == null)
                {
                    emu_cfg["productuserid"] = EOSProductId.GenerateIdFromName(game_app.AppName + epicid.Id);
                }
                else
                {
                    emu_cfg["productuserid"] = EmuConfig.DefaultEmuConfig.EosProductId.ToString();
                }
            }
            else
            {
                emu_cfg["productuserid"] = game_app.EmuConfig.EosProductId.ToString();
            }

            emu_cfg["appid"] = game_app.AppId;
            emu_cfg["gamename"] = game_app.AppName;

            emu_cfg["savepath"] = string.IsNullOrWhiteSpace(game_app.SavePath) ? GameEmuFolder + Path.DirectorySeparatorChar : game_app.SavePath;

            emu_cfg["languages"] = string.Join(",", game_app.Languages);
            emu_cfg["disable_online_networking"] = game_app.DisableOnlineNetworking;

            emu_cfg["enable_overlay"] = game_app.EmuConfig.EnableOverlay ?? EmuConfig.DefaultEmuConfig.EnableOverlay;
            emu_cfg["unlock_dlcs"] = game_app.EmuConfig.UnlockEntitlements ?? EmuConfig.DefaultEmuConfig.UnlockEntitlements;
            
            //_emu_cfg[""] = ;
            //_emu_cfg[""] = app.custom_broadcasts.Count;

            return new ApiResult { Success = true };
        }

        private static ApiResult BuildEmuEntitlements(GameConfig app, out JObject emu_dlcs)
        {
            emu_dlcs = new JObject();
            
            //foreach (var dlc in app.Dlcs)
            //{
            //    JObject emu_dlc = new JObject();
            //    emu_dlc.Add("dlc_name", dlc.Name);
            //    emu_dlc.Add("enabled", dlc.Enabled);
            //    //emu_dlc.Add("icon_gray", dlc.GrayIcon);
            //    //emu_dlc.Add("icon", dlc.Icon);
            //
            //    emu_dlcs.Add(dlc.DlcId.ToString(), emu_dlc);
            //}

            return new ApiResult { Success = true };
        }

        private static ApiResult SetupEpicEmu(GameConfig app)
        {
            // Copy original api and copy emu
            if(string.IsNullOrEmpty(app.AppId))
            {
                return new ApiResult { Success = false, Message = "Invalid AppId" };
            }

            if (string.IsNullOrEmpty(app.AppName))
            {
                return new ApiResult { Success = false, Message = "Invalid AppName" };
            }

            if (string.IsNullOrEmpty(app.FullPath) || !File.Exists(app.FullPath))
            {
                return new ApiResult { Success = false, Message = "Invalid App Exe Path" };
            }

            string emu_api_name = OSFuncs.GetEpicAPIName(app.UseX64);

            string api_name = Path.GetFileName(app.FullApiPath);
            if (string.IsNullOrEmpty(app.FullApiPath) || 
                api_name != emu_api_name ||
                !File.Exists(app.FullApiPath))
            {
                return new ApiResult { Success = false, Message = "Invalid Api Path, it should at least point to a library called " + api_name };
            }

            string api_bkp_folder = Path.Combine(OSFuncs.GetEmuApiFolder(app.UseX64), app.AppId);
            string api_bkp_path = Path.Combine(api_bkp_folder, emu_api_name);
            try
            {
                Directory.CreateDirectory(api_bkp_folder);
            }
            catch(Exception)
            {
                return new ApiResult { Success = false, Message = "Failed to create backup directory " + OSFuncs.GetEmuApiFolder(app.UseX64) };
            }

            if(!File.Exists(api_bkp_path))
            {
                try
                {
                    File.Copy(app.FullApiPath, api_bkp_path);
                }
                catch(Exception)
                {
                    return new ApiResult { Success = false, Message = "Failed to backup api " + app.FullApiPath };
                }
            }

            try
            {
                File.Copy(Path.Combine(OSFuncs.GetEmuApiFolder(app.UseX64), emu_api_name), app.FullApiPath, true);
            }
            catch(Exception)
            {
                return new ApiResult { Success = false, Message = "Failed to apply emu " + app.FullApiPath };
            }

            return new ApiResult { Success = true };
        }

        public static ApiResult ShowGameEmuFolder(GameConfig app)
        {
            ApiResult res = getGameSavePath(app);

            if(!res.Success)
            {
                return res;
            }

            try
            {
                if (OSDetector.IsWindows())
                    Process.Start("explorer.exe", res.Message);
                else if (OSDetector.IsLinux())
                    Process.Start("nautilus", res.Message);
                else if (OSDetector.IsMacOS())
                    Process.Start("open", res.Message);
            }
            catch
            {
                return new ApiResult { Success = false, Message = "Folder: " + res.Message };
            }

            return new ApiResult { Success = true };
        }

        public static ApiResult StartGame(GameConfig app)
        {
            JObject emu_cfg;
            ApiResult res = BuildEmuJsonCfg(app, out emu_cfg);
            if(!res.Success)
            {
                return res;
            }

            JObject emu_dlcs;
            res = BuildEmuEntitlements(app, out emu_dlcs);
            if (!res.Success)
            {
                return res;
            }

            res = SetupEpicEmu(app);
            if (!res.Success)
            {
                return res;
            }

            res = getGameSavePath(app);
            if(!res.Success)
            {
                return res;
            }

            //using (StreamWriter streamWriter = new StreamWriter(new FileStream(Path.Combine(res.Message, GameConfig.AppDlcFile), FileMode.Create), Encoding.UTF8))
            //{
            //    string buffer = Newtonsoft.Json.JsonConvert.SerializeObject(emu_dlcs, Newtonsoft.Json.Formatting.Indented);
            //    streamWriter.Write(buffer);
            //}

            string json_cfg = Path.Combine(Path.GetDirectoryName(app.FullPath), EmuJsonName);

            using (StreamWriter streamWriter = new StreamWriter(new FileStream(json_cfg, FileMode.Create), Encoding.UTF8))
            {
                string buffer = Newtonsoft.Json.JsonConvert.SerializeObject(emu_cfg, Newtonsoft.Json.Formatting.Indented);
                streamWriter.Write(buffer);
            }

            ProcessStartInfo psi = new ProcessStartInfo();

            psi.CreateNoWindow = false;
            psi.UseShellExecute = false;
            psi.FileName = app.FullPath;
            psi.WindowStyle = ProcessWindowStyle.Normal;
            psi.Arguments = string.IsNullOrWhiteSpace(app.Parameters) ? app.DefaultParameters : app.Parameters;
            psi.WorkingDirectory = app.StartFolder;

            foreach (EnvVar var in app.EnvVars)
            {
                psi.EnvironmentVariables.Add(var.Key, var.Value);
            }

            Process p = new Process();
            p.EnableRaisingEvents = true;
            p.StartInfo = psi;
            p.Exited += OnProcessExited;
            emuGamesProcess.Add(p, app);

            p.Start();

            return new ApiResult { Success = true };
        }

        private static void OnProcessExited(object sender, EventArgs e)
        {
            Process p = (Process)sender;
            GameConfig app = emuGamesProcess[p];
            emuGamesProcess.Remove(p);
            RestoreEosApi(app);
        }

        public static void RestoreEosApi(GameConfig app)
        {
            // TODO: Restore original api
            try
            {
                File.Copy(Path.Combine(OSFuncs.GetEmuApiFolder(app.UseX64), app.AppId, OSFuncs.GetEpicAPIName(app.UseX64)), app.FullApiPath, true);
            }
            catch (Exception)
            { }
        }

        public static ApiResult CreateShortcut(GameConfig app)
        {
            //if (!OSDetector.IsWindows())
            {
                return new ApiResult { Success = false, Message = "This feature is only available on Windows for the moment" };
            }

            //string launcherPath = Path.GetFullPath(Environment.GetCommandLineArgs()[0]);
            //string desktopDir = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory) + Path.DirectorySeparatorChar;
            //
            //
            //OSUtility.Shortcut.Create(desktopDir + app.AppName, launcherPath, "--game-guid " + app.GameGuid.ToString(),
            //    Path.GetDirectoryName(launcherPath), "Starts " + app.AppName + " with the steam emulator",
            //    string.Empty, app.FullPath);
            //
            //return new ApiResult { Success = true };
        }

        public static ApiResult GenerateGameInfos(GameConfig app, bool clear_cache = false)
        {
            JObject json_cache = null;

            string app_cache_directory = Path.Combine(EpicEmulator.LauncherAppsCacheFolder, app.AppId.ToString());
            string app_cache_file = Path.Combine(app_cache_directory, "infos.json");
            string app_cache_image = Path.Combine(app_cache_directory, "background.jpg");

            if (!clear_cache && File.Exists(app_cache_file))
            {// Try to read the cache file
                using (StreamReader streamReader = new StreamReader(File.OpenRead(app_cache_file)))
                {
                    try
                    {
                        json_cache = JObject.Parse(streamReader.ReadToEnd());
                    }
                    catch (Exception)
                    { }
                }
            }

            if (json_cache == null)
            {// Clear cache or can't find the cache file
                Directory.CreateDirectory(app_cache_directory);
                string url = "https://raw.githubusercontent.com/EpicData-info/items-tracker/master/database/items/" + app.ItemId + ".json";

                JObject json;
                HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
                request.MaximumAutomaticRedirections = 3;
                request.AllowAutoRedirect = true;

                try
                {
                    using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
                    {
                        if (response.StatusCode != HttpStatusCode.OK)
                        {
                            return new ApiResult { Success = false, Message = "Failed to connect to " + url };
                        }

                        Stream sresult = response.GetResponseStream();
                        using (StreamReader streamReader = new StreamReader(sresult))
                        {
                            json = JObject.Parse(streamReader.ReadToEnd());
                        }
                    }
                }
                catch(Exception)
                {
                    return new ApiResult { Success = false, Message = "Page not found " + url };
                }

                if (json.ContainsKey("keyImages"))
                {
                    string best_image = null;
                    int found_value = 0;
                    int img_width = 0;
                    int img_height = 0;
                    foreach (var image in json.Value<JArray>("keyImages"))
                    {
                        try
                        {
                            if (image.Value<string>("type") == "DieselGameBox")
                            {
                                img_width = image.Value<int>("width");
                                img_height = image.Value<int>("height");
                                best_image = image.Value<string>("url");
                                
                                break;
                            }
                            else if(image.Value<string>("type") == "DieselGameBoxTall" && found_value < 2)
                            {
                                img_width = image.Value<int>("width");
                                img_height = image.Value<int>("height");
                                best_image = image.Value<string>("url");
                                found_value = 2;
                            }
                            else if (image.Value<string>("type") == "Thumbnail" && found_value < 1)
                            {
                                img_width = image.Value<int>("width");
                                img_height = image.Value<int>("height");
                                best_image = image.Value<string>("url");
                                found_value = 1;
                            }
                        }
                        catch(Exception)
                        { }
                    }

                    if (best_image != null)
                    {
                        if (img_width > img_height)
                        {
                            double ratio = (double)img_width / (double)img_height;
                            img_height = 215;
                            img_width = (int)(img_height * ratio);
                        }
                        else
                        {
                            double ratio = (double)img_width / (double)img_height;
                            img_height = 280;
                            img_width = (int)(img_height * ratio);
                        }

                        request = (HttpWebRequest)WebRequest.Create(best_image);
                        request.Headers.Add("Accept-encoding:gzip, deflate, br");
                        request.AutomaticDecompression = DecompressionMethods.Deflate | DecompressionMethods.GZip;

                        using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
                        {
                            if (response.StatusCode == HttpStatusCode.OK)
                            {
                                SKBitmap img = SKBitmap.Decode(response.GetResponseStream());
                                if (!Directory.Exists(app_cache_directory))
                                {
                                    Directory.CreateDirectory(app_cache_directory);
                                }

                                SKBitmap resized_img = new SKBitmap(img_width, img_height);

                                img.ScalePixels(resized_img, SKFilterQuality.High);

                                // Qualite de 0 - 100
                                try
                                {
                                    using (SKWStream write_stream = new SKFileWStream(app_cache_image))
                                    {
                                        if (!SKPixmap.Encode(write_stream, resized_img, SKEncodedImageFormat.Jpeg, 90))
                                        {
                                            File.Delete(app_cache_image);
                                        }
                                    }

                                    app.AppImageHeight = img_height;
                                    app.AppImageWidth = img_width;
                                }
                                catch (Exception)
                                {
                                    try
                                    {
                                        File.Delete(app_cache_image);
                                    }
                                    catch(Exception)
                                    { }
                                }
                            }
                        }
                    }
                }

                json_cache = new JObject();
                json_cache["name"] = json.Value<string>("title");
                try
                {
                    json_cache["app_id"] = json["releaseInfo"][0].Value<string>("appId");
                }
                catch(Exception)
                {
                    json_cache["app_id"] = app.AppId;
                }

                using (StreamWriter streamWriter = new StreamWriter(new FileStream(app_cache_file, FileMode.Create), Encoding.UTF8))
                {
                    string buffer = Newtonsoft.Json.JsonConvert.SerializeObject(json_cache, Newtonsoft.Json.Formatting.Indented);
                    streamWriter.Write(buffer);
                }
            }

            app.AppName = string.Empty;
            app.AppName = json_cache.Value<string>("name");
            app.AppId = json_cache.Value<string>("app_id");

            return new ApiResult { Success = true };
        }
    }
}
