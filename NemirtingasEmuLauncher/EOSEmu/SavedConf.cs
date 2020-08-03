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
using System.Collections.Generic;

namespace NemirtingasEmuLauncher
{
    public class SavedConf
    {
        public EmuConfig config = new EmuConfig();
        public List<GameConfig> apps = new List<GameConfig>();
    }
}
