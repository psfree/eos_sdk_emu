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

LOCAL_API void random_string(std::string const& charset, char* buff, size_t length);
LOCAL_API std::string generate_account_id();
LOCAL_API std::string generate_account_id_from_name(std::string const& username);
LOCAL_API std::string generate_epic_id_user();
LOCAL_API std::string generate_epic_id_user_from_name(std::string const& username);

LOCAL_API void fatal_throw(const char* msg);

LOCAL_API std::string get_callback_name(int iCallback);
LOCAL_API const char* search_attr_to_string(EOS_EOnlineComparisonOp comp);

template<typename T>
constexpr inline void set_nullptr(T& v) { if (v != nullptr) *v = nullptr; }