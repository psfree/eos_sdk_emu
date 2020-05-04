/*
 * Copyright (C) 2019 Nemirtingas
 * This file is part of the Nemirtingas's Epic Emulator
 *
 * The Nemirtingas's Epic Emulator is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * The Nemirtingas's Epic Emulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Nemirtingas's Epic Emulator; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __INCLUDED_SETTINGS_H__
#define __INCLUDED_SETTINGS_H__

#include "common_includes.h"

class Settings
{
    Settings();
    Settings(Settings const&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(Settings const&) = delete;
    Settings& operator=(Settings&&) = delete;
public:
    static Settings& Inst()
    {
        static Settings inst;
        return inst;
    }

private:
    static constexpr const char* settings_file_name = "NemirtingasEpicEmu.json";

    std::string config_path;

public:
    EOS_EpicAccountIdDetails userid;
    std::string username;
    std::string languages;
    std::string language;
    std::string savepath;
    std::string gamename;
    bool unlock_dlcs;
    bool enable_overlay;

    ~Settings();

    void load_settings();
};

#endif