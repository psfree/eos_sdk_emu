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

#include "common_includes.h"

#if defined(__clang__) || defined(__GNUC__)
    #define CLANG_GCC_DONT_OPTIMIZE __attribute__((optnone))
#else
    #define CLANG_GCC_DONT_OPTIMIZE
#endif

using global_lock_t = std::lock_guard<std::recursive_mutex>;
LOCAL_API std::recursive_mutex& global_mutex();
#define GLOBAL_LOCK() global_lock_t __global_lock(global_mutex())

LOCAL_API std::random_device& get_rd();
LOCAL_API std::mt19937_64& get_gen();

LOCAL_API std::string generate_account_id();
LOCAL_API std::string generate_account_id_from_name(std::string const& username);
LOCAL_API std::string generate_epic_id_user();
LOCAL_API std::string generate_epic_id_user_from_name(std::string const& username);

LOCAL_API void fatal_throw(const char* msg);

LOCAL_API std::string get_callback_name(int iCallback);

template<typename T>
constexpr inline void set_nullptr(T& v) { if (v != nullptr) *v = nullptr; }

// Workaround to use fifo_map in json
// A workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
template<class K, class V, class dummy_compare, class A>
using my_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using fifo_json = nlohmann::basic_json<my_fifo_map>;

template<template<typename, typename, typename...> class ObjectType,
    template<typename, typename...> class ArrayType,
    class StringType, class BooleanType, class NumberIntegerType,
    class NumberUnsignedType, class NumberFloatType,
    template<typename> class AllocatorType,
    template<typename, typename = void> class JSONSerializer>
bool load_json(std::string const& file_path, nlohmann::basic_json<ObjectType, ArrayType, StringType, BooleanType, NumberIntegerType, NumberUnsignedType, NumberFloatType, AllocatorType, JSONSerializer>& json)
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
        catch (std::exception & e)
        {
            LOG(Log::LogLevel::ERR, "Error while parsing JSON %s: %s", file_path.c_str(), e.what());
        }
    }
    else
    {
        LOG(Log::LogLevel::WARN, "File not found: %s", file_path.c_str());
    }
    return false;
}

template<template<typename, typename, typename...> class ObjectType,
    template<typename, typename...> class ArrayType,
    class StringType, class BooleanType, class NumberIntegerType,
    class NumberUnsignedType, class NumberFloatType,
    template<typename> class AllocatorType,
    template<typename, typename = void> class JSONSerializer>
bool save_json(std::string const& file_path, nlohmann::basic_json<ObjectType, ArrayType, StringType, BooleanType, NumberIntegerType, NumberUnsignedType, NumberFloatType, AllocatorType, JSONSerializer> const& json)
{
    std::ofstream file(file_path, std::ios::trunc | std::ios::out);
    if (!file)
    {
        LOG(Log::LogLevel::ERR, "Failed to save: %s", file_path.c_str());
        return false;
    }
    file << std::setw(2) << json;
    return true;
}