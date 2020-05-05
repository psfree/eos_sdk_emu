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

#include "settings.h"
//#include "epic_client.h"

template<typename T>
T get_setting(nlohmann::json& settings, std::string const& key, bool& save, T default_val)
{
    T val;
    try
    {
        val = settings[key].get <T>();
    }
    catch (...)
    {
        val = default_val;
        settings[key] = default_val;
        save = true;
    }
    return val;
}

Settings::Settings()
{
    load_settings();
}

Settings::~Settings()
{

}

void Settings::load_settings()
{
    LOG(Log::LogLevel::DEBUG, "");
    GLOBAL_LOCK();

    nlohmann::json settings;
    std::string config_path = std::move(get_executable_path() + settings_file_name);

    Log::set_loglevel(Log::LogLevel::INFO);
    LOG(Log::LogLevel::INFO, "Configuration Path: %s", config_path.c_str());

    if (!load_json(config_path, settings))
    {
        LOG(Log::LogLevel::WARN, "Error while loading settings, building a default one");
    }

    bool save_settings = false;

    userid._id     = get_setting(settings, "epicid"        , save_settings, std::string("0123456789ABCDEF0123456789ABCDEF"));
    username       = get_setting(settings, "username"      , save_settings, std::string(u8"DefaultName"));
    language       = get_setting(settings, "language"      , save_settings, std::string("english"));
    languages      = get_setting(settings, "languages"     , save_settings, std::string("english"));
    gamename       = get_setting(settings, "gamename"      , save_settings, std::string("Unreal"));
    unlock_dlcs    = get_setting(settings, "unlock_dlcs"   , save_settings, bool(true));
    enable_overlay = get_setting(settings, "enable_overlay", save_settings, bool(true));

#ifdef _DEBUG
    switchstr(get_setting(settings, "debug_level", save_settings, std::string("OFF")))
    {
        casestr("TRACE"): Log::set_loglevel(Log::LogLevel::TRACE); break;
        casestr("DEBUG"): Log::set_loglevel(Log::LogLevel::DEBUG); break;
        casestr("INFO") : Log::set_loglevel(Log::LogLevel::INFO) ; break;
        casestr("WARN") : Log::set_loglevel(Log::LogLevel::WARN) ; break;
        casestr("ERR")  : Log::set_loglevel(Log::LogLevel::ERR)  ; break;
        casestr("FATAL"): Log::set_loglevel(Log::LogLevel::FATAL); break;
        casestr("OFF")  :
        default         : Log::set_loglevel(Log::LogLevel::OFF);
    }
#endif


    try
    {// Emulator Savepath
        savepath = settings["savepath"].get<std::string>();

        bool clean_savepath = true;
        while (clean_savepath)
        {
            clean_savepath = false;
            std::string tmp = trim(savepath);
            if (tmp != savepath)
            {
                clean_savepath = true;
                save_settings = true;
            }
            if (!savepath.empty())
            {
                while (*savepath.rbegin() == '/' || *savepath.rbegin() == '\\')
                {
                    // Remove trailing '/' or '\'
                    savepath.pop_back();
                    clean_savepath = true;
                    save_settings = true;
                }
            }
        }
        settings["savepath"] = savepath;
        if (savepath == "appdata")
        {
            savepath = std::move(get_userdata_path());
        }
        else
        {
            if (savepath.empty())
                savepath = ".";
            savepath = std::move(canonical_path(savepath));
        }
    }
    catch (...)
    {
        save_settings = true;
        savepath = std::move(get_userdata_path());
        settings["savepath"] = "appdata";
    }

    savepath += PATH_SEPARATOR;
    savepath += emu_savepath;
    savepath += PATH_SEPARATOR;
    savepath += userid._id;
    savepath += PATH_SEPARATOR;
    savepath += gamename;

    create_folder(savepath);

    if(save_settings)
        save_json(config_path, settings);
}
