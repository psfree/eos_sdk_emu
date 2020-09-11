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

#pragma once

#include <common_includes.h>
#include <Log.h>

class FileManager
{
    std::string _root_directory;

    static inline FileManager& Inst();
    FileManager();
    FileManager(FileManager const&) = delete;
    FileManager(FileManager&&) = delete;
    FileManager& operator=(FileManager const&) = delete;
    FileManager& operator=(FileManager&&) = delete;

public:
#ifdef __WINDOWS__
    constexpr static char separator = '\\';
#else
    constexpr static char separator = '/';
#endif

    ~FileManager();

    static std::string clean_path(std::string const& path);
    static std::string filename(std::string const& path);
    static std::string dirname(std::string const& path);
    static bool is_absolute(std::string const& path);

    static std::string join(std::string const& path) { return path; }
    template<typename ...Args>
    static std::string join(std::string const& path, Args&& ...args)
    {
        std::string res(join(args...));
        if (*path.rbegin() == separator)
        {
            if (!res.empty() && *res.begin() == separator)
                return path + res.substr(1);

            return path + res;
        }

        if (!res.empty() && *res.begin() == separator)
            return path + res;

        return path + separator + res;
    }

    static std::string canonical_path(std::string const& path);

    static bool is_dir(std::string const& path);
    static bool is_file(std::string const& path);
    static bool exists(std::string const& path);
    static size_t file_size(std::string const& path);
    static time_t file_atime(std::string const& path);
    static time_t file_mtime(std::string const& path);
    static time_t file_ctime(std::string const& path);

    static bool create_directory(std::string const& directory, bool recursive = true);
    static bool delete_file(std::string const& path);
    static std::vector<std::string> list_files(std::string const& path, bool recursive = false);

    // std::ios::in is always appended to open_mode
    static std::ifstream open_read(std::string const& path, std::ios::openmode open_mode);
    // std::ios::out is always appended to open_mode
    static std::ofstream open_write(std::string const& path, std::ios::openmode open_mode);
    // std::ios::in | std::ios::out are always appended to open_mode
    static std::fstream  open_readwrite(std::string const& path, std::ios::openmode open_mode);

    static bool set_root_dir(std::string const& directory);
    static std::string const& root_dir();

    template<template<typename, typename, typename...> class ObjectType,
        template<typename, typename...> class ArrayType,
        class StringType, class BooleanType, class NumberIntegerType,
        class NumberUnsignedType, class NumberFloatType,
        template<typename> class AllocatorType,
        template<typename, typename = void> class JSONSerializer>
    static bool load_json(std::string const& _path, nlohmann::basic_json<ObjectType, ArrayType, StringType, BooleanType, NumberIntegerType, NumberUnsignedType, NumberFloatType, AllocatorType, JSONSerializer>& json)
    {
        std::string path(canonical_path(_path));
        std::ifstream file(path);

        APP_LOG(Log::LogLevel::INFO, "Loading %s", path.c_str());
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
                APP_LOG(Log::LogLevel::ERR, "Error while parsing JSON %s: %s", path.c_str(), e.what());
            }
        }
        else
        {
            APP_LOG(Log::LogLevel::WARN, "File not found: %s", path.c_str());
        }
        return false;
    }

    template<template<typename, typename, typename...> class ObjectType,
        template<typename, typename...> class ArrayType,
        class StringType, class BooleanType, class NumberIntegerType,
        class NumberUnsignedType, class NumberFloatType,
        template<typename> class AllocatorType,
        template<typename, typename = void> class JSONSerializer>
    static bool save_json(std::string const& _path, nlohmann::basic_json<ObjectType, ArrayType, StringType, BooleanType, NumberIntegerType, NumberUnsignedType, NumberFloatType, AllocatorType, JSONSerializer> const& json)
    {
        std::string path(canonical_path(_path));
        create_directory(dirname(_path));
        std::ofstream file(path, std::ios::trunc | std::ios::out);

        APP_LOG(Log::LogLevel::INFO, "Saving %s", path.c_str());
        if (!file)
        {
            APP_LOG(Log::LogLevel::ERR, "Failed to save: %s", path.c_str());
            return false;
        }
        file << std::setw(2) << json;
        return true;
    }
};