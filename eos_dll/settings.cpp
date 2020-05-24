/*
 * Copyright (C) 2020 Nemirtingas
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
#include "eos_client_api.h"

template<typename T>
T get_setting(nlohmann::json& settings, std::string const& key, T default_val)
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

void Settings::build_save_path()
{
    try
    {// Emulator Savepath
        savepath = setting_savepath;

        bool clean_savepath = true;
        while (clean_savepath)
        {
            clean_savepath = false;
            std::string tmp = trim(savepath);
            if (tmp != savepath)
            {
                clean_savepath = true;
            }
            if (!savepath.empty())
            {
                while (*savepath.rbegin() == '/' || *savepath.rbegin() == '\\')
                {
                    // Remove trailing '/' or '\'
                    savepath.pop_back();
                    clean_savepath = true;
                }
            }
        }

        setting_savepath = savepath;
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
        savepath = std::move(get_userdata_path());
        setting_savepath = "appdata";
    }

    savepath += PATH_SEPARATOR;
    savepath += emu_savepath;
    savepath += PATH_SEPARATOR;
    savepath += userid->to_string();
    savepath += PATH_SEPARATOR;
    savepath += gamename;
}

void Settings::load_settings()
{
    LOG(Log::LogLevel::DEBUG, "");
    GLOBAL_LOCK();

    nlohmann::json settings;
    config_path = std::move(get_executable_path() + settings_file_name);

    Log::set_loglevel(Log::LogLevel::INFO);
    LOG(Log::LogLevel::INFO, "Configuration Path: %s", config_path.c_str());

    if (!load_json(config_path, settings))
    {
        LOG(Log::LogLevel::WARN, "Error while loading settings, building a default one");
    }

    username       = get_setting(settings, "username"      , std::string(u8"DefaultName"));
    if (username.empty() || !utf8::is_valid(username.begin(), username.end()))
    {
        LOG(Log::LogLevel::WARN, "Invalid username, resetting to default name.");
        username = u8"DefaultName";
    }

    settings["username"] = username;

    
    userid = GetEpicUserId(get_setting(settings, "epicid", std::string("")));
    while (!userid->IsValid())
        userid = GetEpicUserId(generate_epic_id_user_from_name(username));

    settings["epicid"] = userid->to_string();

    language       = get_setting(settings, "language"      , std::string("english"));
    languages      = get_setting(settings, "languages"     , std::string("english"));
    gamename       = get_setting(settings, "gamename"      , std::string("Unreal"));
    unlock_dlcs    = get_setting(settings, "unlock_dlcs"   , bool(true));
    enable_overlay = get_setting(settings, "enable_overlay", bool(true));

    Log::LogLevel llvl;
    switchstr(get_setting(settings, "debug_level", std::string("OFF")))
    {
        casestr("TRACE"): llvl = Log::LogLevel::TRACE; break;
        casestr("DEBUG"): llvl = Log::LogLevel::DEBUG; break;
        casestr("INFO") : llvl = Log::LogLevel::INFO ; break;
        casestr("WARN") : llvl = Log::LogLevel::WARN ; break;
        casestr("ERR")  : llvl = Log::LogLevel::ERR  ; break;
        casestr("FATAL"): llvl = Log::LogLevel::FATAL; break;
        casestr("OFF")  :
        default         : llvl = Log::LogLevel::OFF;
    }
    LOG(Log::LogLevel::INFO, "Setting log level to: %s", Log::loglevel_to_str(llvl));
    Log::set_loglevel(llvl);

    try
    {
        setting_savepath = settings["savepath"].get_ref<std::string&>();
    }
    catch (...)
    {
        setting_savepath = "appdata";
    }
    build_save_path();

    save_settings();
}

void Settings::save_settings()
{
    nlohmann::json settings;
    std::string config_path = std::move(get_executable_path() + settings_file_name);

    LOG(Log::LogLevel::INFO, "Saving emu settings: %s", config_path.c_str());

    build_save_path();

    settings["username"] = username;
    settings["epicid"] = userid->to_string();
    settings["language"] = language;
    settings["languages"] = languages;
    settings["gamename"] = gamename;
    settings["unlock_dlcs"] = unlock_dlcs;
    settings["enable_overlay"] = enable_overlay;
#ifdef LIBRARY_DEBUG
    settings["debug_level"] = Log::loglevel_to_str();
#endif
    settings["savepath"] = setting_savepath;

    create_folder(savepath);

    save_json(config_path, settings);
}
