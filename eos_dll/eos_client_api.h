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
    bool _sdk_initialized;

    std::map<std::string, EOS_EpicAccountId> _epicuserids;
    std::map<std::string, EOS_ProductUserId> _productuserids;

    int32_t api_version;
    std::string product_name;
    std::string product_version;

    EOSSDK_Client();
    ~EOSSDK_Client();

    static EOSSDK_Client& Inst();

    EOS_EpicAccountId get_epicuserid(std::string userid);
    EOS_ProductUserId get_productuserid(std::string userid);
};

inline EOS_EpicAccountId GetEpicUserId(std::string const& userid)
{
    return EOSSDK_Client::Inst().get_epicuserid(userid);
}

inline EOS_ProductUserId GetProductUserId(std::string const& userid)
{
    return EOSSDK_Client::Inst().get_productuserid(userid);
}