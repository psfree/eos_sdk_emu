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

#include "eossdk_presence.h"

namespace sdk
{

/**
 * Modifies a user's online status to be the new state.
 *
 * @param Options Object containing properties related to setting a user's Status
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 */
EOS_EResult EOSSDK_PresenceModification::SetStatus(const EOS_PresenceModification_SetStatusOptions* Options)
{
    // TODO: Check the return codes from the real sdk
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options != nullptr)
    {
        infos.set_status((int32_t)Options->Status);
        return EOS_EResult::EOS_Success;
    }
    
    return EOS_EResult::EOS_InvalidParameters;
}

/**
 * Modifies a user's Rich Presence string to a new state. This is the exact value other users will see
 * when they query the local user's presence.
 *
 * @param Options Object containing properties related to setting a user's RichText string
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_RICH_TEXT_MAX_VALUE_LENGTH
 */
EOS_EResult EOSSDK_PresenceModification::SetRawRichText(const EOS_PresenceModification_SetRawRichTextOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options != nullptr && Options->RichText != nullptr)
    {
        infos.set_richtext(Options->RichText);
        return EOS_EResult::EOS_Success;
    }

    return EOS_EResult::EOS_InvalidParameters;
}

/**
 * Modifies one or more rows of user-defined presence data for a local user. At least one InfoData object
 * must be specified.
 *
 * @param Options Object containing an array of new presence data.
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_DATA_MAX_KEYS
 * @see EOS_PRESENCE_DATA_MAX_KEY_LENGTH
 * @see EOS_PRESENCE_DATA_MAX_VALUE_LENGTH
 */
EOS_EResult EOSSDK_PresenceModification::SetData(const EOS_PresenceModification_SetDataOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options != nullptr)
    {
        for (int i = 0; i < Options->RecordsCount; ++i)
        {
            if (strlen(Options->Records[i].Key) > EOS_PRESENCE_DATA_MAX_KEY_LENGTH)
                return EOS_EResult::EOS_Presence_DataKeyLengthInvalid;

            if (strlen(Options->Records[i].Value) > EOS_PRESENCE_DATA_MAX_VALUE_LENGTH)
                return EOS_EResult::EOS_Presence_DataValueLengthInvalid;
        }

        auto &records = *infos.mutable_records();
        for (int i = 0; i < Options->RecordsCount; ++i)
        {
            records[Options->Records[i].Key] = Options->Records[i].Value;
        }
        return EOS_EResult::EOS_Success;
    }

    return EOS_EResult::EOS_InvalidParameters;
}

/**
 * Removes one or more rows of user-defined presence data for a local user. At least one DeleteDataInfo object
 * must be specified.
 *
 * @param Options Object containing an array of new presence data.
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_DATA_MAX_KEYS
 * @see EOS_PRESENCE_DATA_MAX_KEY_LENGTH
 * @see EOS_PRESENCE_DATA_MAX_VALUE_LENGTH
 */
EOS_EResult EOSSDK_PresenceModification::DeleteData(const EOS_PresenceModification_DeleteDataOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options != nullptr)
    {
        for (int i = 0; i < Options->RecordsCount; ++i)
        {
            if (strlen(Options->Records[i].Key) > EOS_PRESENCE_DATA_MAX_KEY_LENGTH)
                return EOS_EResult::EOS_Presence_DataKeyLengthInvalid;
        }

        auto& records = *infos.mutable_records();
        for (int i = 0; i < Options->RecordsCount; ++i)
        {
            auto it = records.find(Options->Records[i].Key);
            if (it == records.end())
                records.erase(it);
        }
        return EOS_EResult::EOS_Success;
    }

    return EOS_EResult::EOS_InvalidParameters;
}

/**
 * Sets your new join info custom game-data string. This is a helper function for reading the presence data related to how a user can be joined.
 * Its meaning is entirely application dependent.
 *
 * @param Options Object containing a join info string and associated user data
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_JOIN_INFO_STRING_LENGTH
 */
EOS_EResult EOSSDK_PresenceModification::SetJoinInfo(const EOS_PresenceModification_SetJoinInfoOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options != nullptr && Options->JoinInfo != nullptr)
    {
        if (strlen(Options->JoinInfo) > EOS_PRESENCEMODIFICATION_JOININFO_MAX_LENGTH)
            return EOS_EResult::EOS_UnexpectedError;
        
        infos.set_joininfo(Options->JoinInfo);
        return EOS_EResult::EOS_Success;
    }

    return EOS_EResult::EOS_InvalidParameters;
}

}