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

#include "eossdk_lobby.h"
#include "eossdk_platform.h"

namespace sdk
{

EOSSDK_LobbyModification::EOSSDK_LobbyModification():
    _lobby_modified(false),
    _member_modified(false)
{}

EOSSDK_LobbyModification::~EOSSDK_LobbyModification()
{}

/**
 * To modify lobbies or the lobby member data, you must call EOS_Lobby_UpdateLobbyModification to create a Lobby Modification handle. To modify that handle, call
 * EOS_LobbyModification_* methods. Once you are finished, call EOS_Lobby_UpdateLobby with your handle. You must then release your Lobby Modification
 * handle by calling EOS_LobbyModification_Release.
 */

 /**
  * Set the permissions associated with this lobby.
  * The permissions range from "public" to "invite only" and are described by EOS_ELobbyPermissionLevel
  *
  * @param Options Options associated with the permission level of the lobby
  *
  * @return EOS_Success if setting this parameter was successful
  *         EOS_IncompatibleVersion if the API version passed in is incorrect
  */
EOS_EResult EOSSDK_LobbyModification::SetPermissionLevel(const EOS_LobbyModification_SetPermissionLevelOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _infos.set_permission_level(get_enum_value(Options->PermissionLevel));

    _lobby_modified = true;
    return EOS_EResult::EOS_Success;
}

/**
 * Set the maximum number of members allowed in this lobby.
 * When updating the lobby, it is not possible to reduce this number below the current number of existing members
 *
 * @param Options Options associated with max number of members in this lobby
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_LobbyModification::SetMaxMembers(const EOS_LobbyModification_SetMaxMembersOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || Options->MaxMembers < _infos.members_size() || Options->MaxMembers > EOS_LOBBY_MAX_LOBBY_MEMBERS)
        return EOS_EResult::EOS_InvalidParameters;

    _infos.set_max_lobby_member(Options->MaxMembers);

    _lobby_modified = true;
    return EOS_EResult::EOS_Success;
}

/**
 * Associate an attribute with this lobby
 * An attribute is something may be public or private with the lobby.
 * If public, it can be queried for in a search, otherwise the data remains known only to lobby members
 *
 * @param Options Options to set the attribute and its visibility state
 *
 * @return EOS_Success if setting this parameter was successful
 *		   EOS_InvalidParameters if the attribute is missing information or otherwise invalid
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_LobbyModification::AddAttribute(const EOS_LobbyModification_AddAttributeOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);
    
    if (Options == nullptr || Options->Attribute == nullptr || Options->Attribute->Key == nullptr || 
        (Options->Attribute->ValueType == EOS_ELobbyAttributeType::EOS_AT_STRING && Options->Attribute->Value.AsUtf8 == nullptr))
        return EOS_EResult::EOS_InvalidParameters;

    // Don't allow more than EOS_SESSIONMODIFICATION_MAX_SESSION_ATTRIBUTES key length
    if (strlen(Options->Attribute->Key) > EOS_LOBBYMODIFICATION_MAX_ATTRIBUTE_LENGTH)
        return EOS_EResult::EOS_InvalidParameters;

    auto it = _infos.attributes().find(Options->Attribute->Key);
    // Don't allow more than EOS_LOBBYMODIFICATION_MAX_ATTRIBUTES attributes
    if (it != _infos.attributes().end() && _infos.attributes_size() >= EOS_LOBBYMODIFICATION_MAX_ATTRIBUTES)
        return EOS_EResult::EOS_InvalidParameters;

    auto& attribute = (*_infos.mutable_attributes())[Options->Attribute->Key];
    attribute.set_visibility_type(get_enum_value(Options->Visibility));
    
    switch (Options->Attribute->ValueType)
    {
        case EOS_ESessionAttributeType::EOS_AT_BOOLEAN: attribute.mutable_value()->set_b(Options->Attribute->Value.AsBool); break;
        case EOS_ESessionAttributeType::EOS_AT_DOUBLE : attribute.mutable_value()->set_d(Options->Attribute->Value.AsDouble); break;
        case EOS_ESessionAttributeType::EOS_AT_INT64  : attribute.mutable_value()->set_i(Options->Attribute->Value.AsInt64); break;
        case EOS_ESessionAttributeType::EOS_AT_STRING : attribute.mutable_value()->set_s(Options->Attribute->Value.AsUtf8); break;
        default                                       : return EOS_EResult::EOS_InvalidParameters;
    }

    _lobby_modified = true;
    return EOS_EResult::EOS_Success;
}

/**
 * Remove an attribute associated with the lobby
 *
 * @param Options Specify the key of the attribute to remove
 *
 * @return EOS_Success if removing this parameter was successful
 *		   EOS_InvalidParameters if the key is null or empty
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_LobbyModification::RemoveAttribute(const EOS_LobbyModification_RemoveAttributeOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || Options->Key == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto& attributes = (*_infos.mutable_attributes());
    auto it = attributes.find(Options->Key);
    if (it != attributes.end())
        attributes.erase(it);

    _lobby_modified = true;
    return EOS_EResult::EOS_Success;
}

/**
 * Associate an attribute with a member of the lobby
 * Lobby member data is always private to the lobby
 *
 * @param Options Options to set the attribute and its visibility state
 *
 * @return EOS_Success if setting this parameter was successful
 *		   EOS_InvalidParameters if the attribute is missing information or otherwise invalid
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_LobbyModification::AddMemberAttribute(const EOS_LobbyModification_AddMemberAttributeOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || Options->Attribute == nullptr || Options->Attribute->Key == nullptr || *Options->Attribute->Key == '\0' ||
        (Options->Attribute->ValueType == EOS_ELobbyAttributeType::EOS_AT_STRING && Options->Attribute->Value.AsUtf8 == nullptr))
        return EOS_EResult::EOS_InvalidParameters;

    // Don't allow more than EOS_SESSIONMODIFICATION_MAX_SESSION_ATTRIBUTES key length
    if (strlen(Options->Attribute->Key) > EOS_LOBBYMODIFICATION_MAX_ATTRIBUTE_LENGTH)
        return EOS_EResult::EOS_InvalidParameters;

    auto& member_attributes = (*_infos.mutable_members())[GetEOS_Connect().get_myself()->first->to_string()];

    auto it = member_attributes.attributes().find(Options->Attribute->Key);
    // Don't allow more than EOS_LOBBYMODIFICATION_MAX_ATTRIBUTES attributes
    if (it != member_attributes.attributes().end() && member_attributes.attributes_size() >= EOS_LOBBYMODIFICATION_MAX_ATTRIBUTES)
        return EOS_EResult::EOS_InvalidParameters;

    auto& attribute = (*member_attributes.mutable_attributes())[Options->Attribute->Key];
        
    attribute.set_visibility_type(get_enum_value(Options->Visibility));
    
    switch (Options->Attribute->ValueType)
    {
        case EOS_ESessionAttributeType::EOS_AT_BOOLEAN: attribute.mutable_value()->set_b(Options->Attribute->Value.AsBool); break;
        case EOS_ESessionAttributeType::EOS_AT_DOUBLE : attribute.mutable_value()->set_d(Options->Attribute->Value.AsDouble); break;
        case EOS_ESessionAttributeType::EOS_AT_INT64  : attribute.mutable_value()->set_i(Options->Attribute->Value.AsInt64); break;
        case EOS_ESessionAttributeType::EOS_AT_STRING : attribute.mutable_value()->set_s(Options->Attribute->Value.AsUtf8); break;
        default                                       : return EOS_EResult::EOS_InvalidParameters;
    }

    _member_modified = true;
    return EOS_EResult::EOS_Success;
}

/**
 * Remove an attribute associated with of member of the lobby
 *
 * @param Options Specify the key of the member attribute to remove
 *
 * @return EOS_Success if removing this parameter was successful
 *		   EOS_InvalidParameters if the key is null or empty
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_EResult EOSSDK_LobbyModification::RemoveMemberAttribute(const EOS_LobbyModification_RemoveMemberAttributeOptions* Options)
{
    TRACE_FUNC();
    std::lock_guard<std::mutex> lk(_local_mutex);

    if (Options == nullptr || Options->Key == nullptr || *Options->Key == '\0')
        return EOS_EResult::EOS_InvalidParameters;

    auto& member_attributes = (*_infos.mutable_members())[GetEOS_Connect().get_myself()->first->to_string()];
    auto& attributes = (*member_attributes.mutable_attributes());

    auto it = attributes.find(Options->Key);
    if (it != attributes.end())
        attributes.erase(it);

    _member_modified = true;
    return EOS_EResult::EOS_Success;
}

void EOSSDK_LobbyModification::Release()
{
    TRACE_FUNC();

    delete this;
}

}