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

class EOSSDK_Client
{
public:
    /**
     * Function prototype type definition for functions that allocate memory.
     *
     * Functions passed to EOS_Initialize to serve as memory allocators should return a pointer to the allocated memory.
     *
     * The returned pointer should have at least SizeInBytes available capacity and the memory address should be a multiple of Alignment.
     * The SDK will always call the provided function with an Alignment that is a power of 2.
     * Allocation failures should return a null pointer.
     */
    EOS_AllocateMemoryFunc _allocate_memory_func;

    /**
     * Function prototype type definition for functions that reallocate memory.
     *
     * Functions passed to EOS_Initialize to serve as memory reallocators should return a pointer to the reallocated memory.
     * The returned pointer should have at least SizeInBytes available capacity and the memory address should be a multiple of alignment.
     * The SDK will always call the provided function with an Alignment that is a power of 2.
     * Reallocation failures should return a null pointer.
     */
    EOS_ReallocateMemoryFunc _reallocate_memory_func;

    /**
     * Function prototype type definition for functions that release memory.
     *
     * When the SDK is done with memory that has been allocated by a custom allocator passed to EOS_Initialize, it will call the corresponding memory release function.
     */
    EOS_ReleaseMemoryFunc _release_memory_func;

    bool _sdk_initialized;

    std::map<std::string, EOS_EpicAccountId> _epicuserids;
    std::map<std::string, EOS_ProductUserId> _productuserids;

    int32_t api_version;
    //std::string product_name;
    std::string _product_version;

    EOSSDK_Client();
    ~EOSSDK_Client();

    static EOSSDK_Client& Inst();

    EOS_EpicAccountId get_epicuserid(std::string const& userid);
    EOS_ProductUserId get_productuserid(std::string const& userid);
};

inline EOS_EpicAccountId GetInvalidEpicUserId()
{
    return EOSSDK_Client::Inst().get_epicuserid(sdk::NULL_USER_ID);
}

inline EOS_EpicAccountId GetEpicUserId(std::string const& userid)
{
    return EOSSDK_Client::Inst().get_epicuserid(userid);
}

inline EOS_ProductUserId GetInvalidProductUserId()
{
    return EOSSDK_Client::Inst().get_productuserid(sdk::NULL_USER_ID);
}

inline EOS_ProductUserId GetProductUserId(std::string const& userid)
{
    return EOSSDK_Client::Inst().get_productuserid(userid);
}