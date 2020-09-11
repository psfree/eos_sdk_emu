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
        val = settings[key].get<T>();
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

static bool load_json(std::string const& file_path, nlohmann::json& json)
{
    std::ifstream file(file_path);
    if (file)
    {
        file.seekg(0, std::ios::end);
        size_t size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);

        std::string buffer(size, '\0');

        file.read(&buffer[0], size);
        file.close();

        try
        {
            json = std::move(nlohmann::json::parse(buffer));

            return true;
        }
        catch (std::exception& e)
        {
            APP_LOG(Log::LogLevel::ERR, "Error while parsing JSON %s: %s", file_path.c_str(), e.what());
        }
    }
    else
    {
        APP_LOG(Log::LogLevel::WARN, "File not found: %s", file_path.c_str());
    }
    return false;
}

static bool save_json(std::string const& file_path, nlohmann::json const& json)
{
    std::ofstream file(file_path, std::ios::trunc | std::ios::out);
    if (!file)
    {
        APP_LOG(Log::LogLevel::ERR, "Failed to save: %s", file_path.c_str());
        return false;
    }
    file << std::setw(2) << json;
    return true;
}

Settings& Settings::Inst()
{
    static Settings inst;
    return inst;
}

void Settings::load_settings()
{
    bool default_config = false;

    GLOBAL_LOCK();

    nlohmann::json settings;
    config_path = std::move(FileManager::dirname(get_executable_path()) + settings_file_name);

    if (!load_json(config_path, settings))
    {
        default_config = true;
    }

#ifndef DISABLE_LOG
    Log::LogLevel llvl;
    switchstr(get_setting(settings, "log_level", std::string("OFF")))
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
    APP_LOG(Log::LogLevel::INFO, "Setting log level to: %s", Log::loglevel_to_str(llvl));
    Log::set_loglevel(llvl);
#endif

    APP_LOG(Log::LogLevel::INFO, "Configuration Path: %s", config_path.c_str());
    if (default_config)
    {
        APP_LOG(Log::LogLevel::WARN, "Error while loading settings, building a default one");
    }

    APP_LOG(Log::LogLevel::INFO, "Emulator version %s", _EMU_VERSION_);

    username = get_setting(settings, "username", std::string(u8"DefaultName"));
    if (username.empty() || !utf8::is_valid(username.begin(), username.end()))
    {
        APP_LOG(Log::LogLevel::WARN, "Invalid username, resetting to default name.");
        username = u8"DefaultName";
    }

    settings["username"] = username;

    userid = GetEpicUserId(get_setting(settings, "epicid", std::string("")));
    if (!userid->IsValid())
    {
        if (username == "DefaultName")
        {
            APP_LOG(Log::LogLevel::INFO, "Username == DefaultName, generating random epic id");
            userid = GetEpicUserId(generate_epic_id_user());
        }
        else
        {
            APP_LOG(Log::LogLevel::INFO, "Username != DefaultName, generating random epic id based on your username");
            userid = GetEpicUserId(generate_epic_id_user_from_name(username));
        }
    }

    language                  = get_setting(settings, "language", std::string("en"));
    gamename                  = get_setting(settings, "gamename", std::string("DefaultGameName"));
    appid                     = get_setting(settings, "appid", std::string("InvalidAppid"));
    unlock_dlcs               = get_setting(settings, "unlock_dlcs", bool(true));
    enable_overlay            = get_setting(settings, "enable_overlay", bool(true));
    disable_online_networking = get_setting(settings, "disable_online_networking", bool(false));
    savepath                  = get_setting(settings, "savepath", std::string("appdata"));

    std::string productuserid = get_setting(settings, "productuserid", generate_account_id_from_name(appid + userid->to_string()));
    this->productuserid = GetProductUserId(productuserid);

    std::string settings_dir;
    if (savepath == "appdata")
    {
        settings_dir = get_userdata_path();
    }
    else if (FileManager::is_absolute(savepath))
    {
        settings_dir = savepath;
    }
    else
    {
        settings_dir = FileManager::join(FileManager::dirname(get_executable_path()), savepath);
    }

    FileManager::set_root_dir(settings_dir);
    settings_dir = FileManager::root_dir();
    settings_dir = FileManager::join(settings_dir, emu_savepath, userid->to_string());

    //load_avatar(settings_dir);

    FileManager::set_root_dir(FileManager::join(settings_dir, appid));
    
    save_settings();
}

void Settings::save_settings()
{
    nlohmann::json settings;
    APP_LOG(Log::LogLevel::INFO, "Saving emu settings: %s", config_path.c_str());

    settings["appid"]                     = appid;
    settings["username"]                  = username;
    settings["epicid"]                    = userid->to_string();
    settings["productuserid"]             = productuserid->to_string();
    settings["language"]                  = language;
    settings["gamename"]                  = gamename;
    settings["unlock_dlcs"]               = unlock_dlcs;
    settings["enable_overlay"]            = enable_overlay;
    settings["disable_online_networking"] = disable_online_networking;
#ifndef DISABLE_LOG
    settings["log_level"]                 = Log::loglevel_to_str();
#endif
    settings["savepath"]                  = savepath;

    save_json(config_path, settings);
}
