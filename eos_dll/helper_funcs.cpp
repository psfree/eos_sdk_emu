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

#include "helper_funcs.h"

LOCAL_API std::random_device& get_rd()
{
    // Random device generator
    static std::random_device rd;
    return rd;
}

LOCAL_API std::mt19937_64& get_gen()
{
    static std::mt19937_64 gen(get_rd()());
    return gen;
}

LOCAL_API std::recursive_mutex& global_mutex()
{
    static std::recursive_mutex global_mutex;
    return global_mutex;
}

LOCAL_API static void randombytes(char* buf, size_t len)
{
    // uniform integer distribution
    std::uniform_int_distribution<int64_t> dis;
    std::mt19937_64& gen = get_gen();

    dis(gen); dis(gen); dis(gen);
    dis(gen); dis(gen); dis(gen);

    // Make sure we can hold our buffer size as int64_t
    size_t rand_buf_len = len / sizeof(int64_t) + (len % sizeof(int64_t) ? 1 : 0);
    int64_t* rand_buf = new int64_t[rand_buf_len];
    // Generate some (pseudo) random numbers
    for (size_t i = 0; i < rand_buf_len; ++i)
        rand_buf[i] = dis(gen);

    // Copy the random bytes to buffer
    memcpy(buf, rand_buf, len);

    // Don't forget to free it
    delete[]rand_buf;
}

LOCAL_API void fatal_throw(const char* msg)
{
    LOG(Log::LogLevel::FATAL, "%s", msg);

    throw std::exception();
}

LOCAL_API bool load_json(std::string const& file_path, nlohmann::json &json)
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
        catch (std::exception &e)
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

LOCAL_API bool save_json(std::string const& file_path, nlohmann::json const& json)
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

LOCAL_API std::string get_callback_name(int iCallback)
{
    switch (iCallback)
    {
#define I_CALLBACK(TYPE) case TYPE::k_iCallback: return #TYPE
        I_CALLBACK(EOS_Auth_LoginCallbackInfo);
        I_CALLBACK(EOS_Auth_LogoutCallbackInfo);
        I_CALLBACK(EOS_Auth_VerifyUserAuthCallbackInfo);
        I_CALLBACK(EOS_Auth_DeletePersistentAuthCallbackInfo);
        I_CALLBACK(EOS_Auth_LoginStatusChangedCallbackInfo);

#undef I_CALLBACK
    }

    return "";
}
