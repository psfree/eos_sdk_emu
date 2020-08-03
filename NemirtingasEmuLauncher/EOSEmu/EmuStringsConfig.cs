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

namespace NemirtingasEmuLauncher
{
    /// <summary>
    /// Contains the configuration stored in appsettings.json
    /// </summary>
    public class EmuStringsConfig
    {
        /// <summary>
        /// Collection of ApiInformation with steam api versiopn and hash
        /// </summary>
        public ApiInformation[] SteamApiHashMap { get; set; }
        /// <summary>
        /// Array of supported languages
        /// </summary>
        public string[] Languages { get; set; }
    }

    /// <summary>
    /// Provide Hashes informations for specific steam_api library
    /// </summary>
    public class ApiInformation
    {
        /// <summary>
        /// The name of the steam_api library
        /// </summary>
        public string Key { get; set; }
        /// <summary>
        /// Array of hashes based on version of the steam_api library
        /// </summary>
        public HashInformation[] Hashes { get; set; }
    }
    /// <summary>
    /// Provide Hash for a specifi version of steam_api library (Not to be used alone)
    /// </summary>
    public class HashInformation
    {
        /// <summary>
        /// The version of the steam api
        /// </summary>
        public string Version { get; set; }
        /// <summary>
        /// The hash corresponding to the version
        /// </summary>
        public string Hash { get; set; }
    }
}
