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

#include "eossdk_userinfo.h"

/**
 * The UserInfo Interface is used to receive user information for Epic account IDs from the backend services and to retrieve that information once it is cached.
 * All UserInfo Interface calls take a handle of type EOS_HUserInfo as the first parameter.
 * This handle can be retrieved from a EOS_HPlatform handle by using the EOS_Platform_GetUserInfoInterface function.
 *
 * NOTE: At this time, this feature is only available for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetUserInfoInterface
 */

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfo(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoCallback CompletionDelegate)
{
    auto pInst = reinterpret_cast<sdk::EOSSDK_UserInfo*>(Handle);
    pInst->QueryUserInfo(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfoByDisplayName(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoByDisplayNameOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoByDisplayNameCallback CompletionDelegate)
{
    auto pInst = reinterpret_cast<sdk::EOSSDK_UserInfo*>(Handle);
    pInst->QueryUserInfoByDisplayName(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyUserInfo(EOS_HUserInfo Handle, const EOS_UserInfo_CopyUserInfoOptions* Options, EOS_UserInfo** OutUserInfo)
{
    auto pInst = reinterpret_cast<sdk::EOSSDK_UserInfo*>(Handle);
    return pInst->CopyUserInfo(Options, OutUserInfo);
}

/**
 * Release the memory associated with an EOS_UserInfo structure. This must be called on data retrieved from EOS_UserInfo_CopyUserInfo.
 *
 * @param UserInfo - The user info structure to release
 *
 * @see EOS_UserInfo
 * @see EOS_UserInfo_CopyUserInfo
 */
EOS_DECLARE_FUNC(void) EOS_UserInfo_Release(EOS_UserInfo* UserInfo)
{
    LOG(Log::LogLevel::TRACE, "");

    if (UserInfo != nullptr)
    {
        delete UserInfo->UserId;
        delete UserInfo;
    }
}