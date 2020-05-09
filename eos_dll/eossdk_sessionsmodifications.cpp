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

#include "eossdk_sessions.h"
#include "eossdk_platform.h"
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

 /**
  * Set the bucket id associated with this session.
  * Values such as region, game mode, etc can be combined here depending on game need.
  * Setting this is strongly recommended to improve search performance.
  *
  * @param Options Options associated with the bucket id of the session
  *
  * @return EOS_Success if setting this parameter was successful
  *         EOS_InvalidParameters if the bucket id is invalid or null
  *         EOS_IncompatibleVersion if the API version passed in is incorrect
  */
EOS_EResult EOSSDK_SessionModification::SetBucketId(const EOS_SessionModification_SetBucketIdOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();
    
    if (Options == nullptr || Options->BucketId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _infos.set_bucketid(Options->BucketId);

    return EOS_EResult::EOS_Success;
}

/**
 * Set the host address associated with this session
 * Setting this is optional, if the value is not set the SDK will fill the value in from the service.
 * It is useful to set if other addressing mechanisms are desired or if LAN addresses are preferred during development
 *
 * NOTE: No validation of this value occurs to allow for flexibility in addressing methods
 *
 * @param Options Options associated with the host address of the session
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_InvalidParameters if the host id is an empty string
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_SessionModification::SetHostAddress(const EOS_SessionModification_SetHostAddressOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();
    
    if (Options == nullptr || Options->HostAddress == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _infos.set_host_address(Options->HostAddress);

    return EOS_EResult::EOS_Success;
}

/**
 * Set the session permissions associated with this session.
 * The permissions range from "public" to "invite only" and are described by EOS_EOnlineSessionPermissionLevel
 *
 * @param Options Options associated with the permission level of the session
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_SessionModification::SetPermissionLevel(const EOS_SessionModification_SetPermissionLevelOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();
    
    if (Options == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _infos.set_permission_level(static_cast<int32_t>(Options->PermissionLevel));

    return EOS_EResult::EOS_Success;
}

/**
 * Set whether or not join in progress is allowed
 * Once a session is started, it will no longer be visible to search queries unless this flag is set or the session returns to the pending or ended state
 *
 * @param Options Options associated with setting the join in progress state the session
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_SessionModification::SetJoinInProgressAllowed(const EOS_SessionModification_SetJoinInProgressAllowedOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();
    
    if (Options == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _infos.set_join_in_progress_allowed(Options->bAllowJoinInProgress);

    return EOS_EResult::EOS_Success;
}

/**
 * Set the maximum number of players allowed in this session.
 * When updating the session, it is not possible to reduce this number below the current number of existing players
 *
 * @param Options Options associated with max number of players in this session
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_SessionModification::SetMaxPlayers(const EOS_SessionModification_SetMaxPlayersOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();
    
    if (Options == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _infos.set_maxplayers(Options->MaxPlayers);

    return EOS_EResult::EOS_Success;
}

/**
 * Allows enabling or disabling invites for this session.
 * The session will also need to have `bPresenceEnabled` true.
 *
 * @param Options Options associated with invites allowed flag for this session.
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_SessionModification::SetInvitesAllowed(const EOS_SessionModification_SetInvitesAllowedOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();
    
    if (Options == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _infos.set_invites_allowed(Options->bInvitesAllowed);

    return EOS_EResult::EOS_Success;
}

/**
 * Associate an attribute with this session
 * An attribute is something that may or may not be advertised with the session.
 * If advertised, it can be queried for in a search, otherwise the data remains local to the client
 *
 * @param Options Options to set the attribute and its advertised state
 *
 * @return EOS_Success if setting this parameter was successful
 *		   EOS_InvalidParameters if the attribution is missing information or otherwise invalid
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_SessionModification::AddAttribute(const EOS_SessionModification_AddAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();
    
    if (Options == nullptr || Options->SessionAttribute->Key == nullptr ||
        (Options->SessionAttribute->ValueType == EOS_ESessionAttributeType::EOS_AT_STRING && Options->SessionAttribute->Value.AsUtf8 == nullptr))
        return EOS_EResult::EOS_InvalidParameters;
    
    auto& attribute = (*_infos.mutable_attributes())[Options->SessionAttribute->Key];
    attribute.set_advertisement_type(static_cast<int32_t>(Options->AdvertisementType));
    switch (Options->SessionAttribute->ValueType)
    {
        case EOS_ESessionAttributeType::EOS_AT_BOOLEAN: attribute.mutable_value()->set_b(Options->SessionAttribute->Value.AsBool); break;
        case EOS_ESessionAttributeType::EOS_AT_DOUBLE : attribute.mutable_value()->set_d(Options->SessionAttribute->Value.AsDouble); break;
        case EOS_ESessionAttributeType::EOS_AT_INT64  : attribute.mutable_value()->set_i(Options->SessionAttribute->Value.AsInt64); break;
        case EOS_ESessionAttributeType::EOS_AT_STRING : attribute.mutable_value()->set_s(Options->SessionAttribute->Value.AsUtf8); break;
        default                                       : return EOS_EResult::EOS_InvalidParameters;
    }

    return EOS_EResult::EOS_Success;
}

/**
 * Remove an attribute from this session
 *
 * @param Options Specify the key of the attribute to remove
 *
 * @return EOS_Success if removing this parameter was successful
 *		   EOS_InvalidParameters if the key is null or empty
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_SessionModification::RemoveAttribute(const EOS_SessionModification_RemoveAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();
    
    if (Options == nullptr || Options->Key == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto &attributes = *_infos.mutable_attributes();
    auto it = attributes.find(Options->Key);
    if (it != attributes.end())
        attributes.erase(it);

    return EOS_EResult::EOS_Success;
}

}