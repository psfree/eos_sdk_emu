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
#include "eos_client_api.h"

namespace sdk
{

EOSSDK_LobbyDetails::EOSSDK_LobbyDetails()
{}

EOSSDK_LobbyDetails::~EOSSDK_LobbyDetails()
{}

/**
 * A "read only" representation of an existing lobby that games interact with externally.
 * Both the lobby and lobby search interfaces interface use this common class for lobby management and search results
 */

/**
  * Get the product user id of the current owner for a given lobby
  *
  * @param Options Structure containing the input parameters
  *
  * @return the product user id for the lobby owner or null if the input parameters are invalid
  */
EOS_ProductUserId EOSSDK_LobbyDetails::GetLobbyOwner(const EOS_LobbyDetails_GetLobbyOwnerOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
        return GetInvalidProductUserId();

    return GetProductUserId(_infos.owner_id());
}

/**
 * EOS_LobbyDetails_CopyInfo is used to immediately retrieve a copy of lobby information from a given source such as a existing lobby or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsInfo, must be passed to EOS_LobbyDetails_Info_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutLobbyDetailsInfo Out parameter used to receive the EOS_LobbyDetails_Info structure.
 *
 * @return EOS_Success if the information is available and passed out in OutLobbyDetailsInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_LobbyDetails_Info
 * @see EOS_LobbyDetails_CopyInfoOptions
 * @see EOS_LobbyDetails_Info_Release
 */
EOS_EResult EOSSDK_LobbyDetails::CopyInfo(const EOS_LobbyDetails_CopyInfoOptions* Options, EOS_LobbyDetails_Info** OutLobbyDetailsInfo)
{
    TRACE_FUNC();

    if (Options == nullptr)
    {
        *OutLobbyDetailsInfo = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    EOS_LobbyDetails_Info* pLobbyDetailsInfos = new EOS_LobbyDetails_Info;

    pLobbyDetailsInfos->ApiVersion = EOS_LOBBYDETAILS_INFO_API_LATEST;
    {
        size_t len = _infos.lobby_id().length() + 1;
        char* str = new char[len];
        strncpy(str, _infos.lobby_id().c_str(), len);
        pLobbyDetailsInfos->LobbyId = str;
    }
    
    pLobbyDetailsInfos->LobbyOwnerUserId = GetProductUserId(_infos.owner_id());
    pLobbyDetailsInfos->PermissionLevel = (EOS_ELobbyPermissionLevel)_infos.permission_level();
    pLobbyDetailsInfos->AvailableSlots = _infos.max_lobby_member() - _infos.members_size();
    pLobbyDetailsInfos->MaxMembers = _infos.max_lobby_member();
    pLobbyDetailsInfos->bAllowInvites = EOS_TRUE;

    *OutLobbyDetailsInfo = pLobbyDetailsInfos;
    return EOS_EResult::EOS_Success;
}

/**
 * Get the number of attributes associated with this lobby
 *
 * @param Options the Options associated with retrieving the attribute count
 *
 * @return number of attributes on the lobby or 0 if there is an error
 */
uint32_t EOSSDK_LobbyDetails::GetAttributeCount(const EOS_LobbyDetails_GetAttributeCountOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
        return 0;

    return _infos.attributes_size();
}

/**
 * EOS_LobbyDetails_CopyAttributeByIndex is used to immediately retrieve a copy of a lobby attribute from a given source such as a existing lobby or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutAttribute Out parameter used to receive the EOS_Lobby_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_Lobby_Attribute
 * @see EOS_LobbyDetails_CopyAttributeByIndexOptions
 * @see EOS_Lobby_Attribute_Release
 */
EOS_EResult EOSSDK_LobbyDetails::CopyAttributeByIndex(const EOS_LobbyDetails_CopyAttributeByIndexOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->AttrIndex >= _infos.attributes_size() || OutAttribute == nullptr)
    {
        set_nullptr(OutAttribute);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto attr_it = _infos.attributes().begin();
    std::advance(attr_it, Options->AttrIndex);

    EOS_Lobby_Attribute* pAttribute = new EOS_Lobby_Attribute;
    EOS_Lobby_AttributeData* pData = new EOS_Lobby_AttributeData;
    
    {
        size_t len = attr_it->first.length() + 1;
        char* str = new char[len];
        strncpy(str, attr_it->first.c_str(), len);
        pData->Key = str;
    }
    
    switch (attr_it->second.value().value_case())
    {
        case Session_Attr_Value::ValueCase::kB: 
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
            pData->Value.AsBool = attr_it->second.value().b();
        }
        break;

        case Session_Attr_Value::ValueCase::kD:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
            pData->Value.AsDouble = attr_it->second.value().d();
        }
        break;

        case Session_Attr_Value::ValueCase::kI:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
            pData->Value.AsInt64 = attr_it->second.value().i();
        }
        break;

        case Session_Attr_Value::ValueCase::kS:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_STRING;
            std::string const& value = attr_it->second.value().s();
            char* str = new char[value.length() + 1];
            strncpy(str, value.c_str(), value.length() + 1);
            pData->Value.AsUtf8 = str;
        }
    }

    pAttribute->ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;
    pAttribute->Data = pData;
    pAttribute->Visibility = (EOS_ELobbyAttributeVisibility)attr_it->second.visibility_type();

    *OutAttribute = pAttribute;

    return EOS_EResult::EOS_Success;
}

/**
 * EOS_LobbyDetails_CopyAttributeByKey is used to immediately retrieve a copy of a lobby attribute from a given source such as a existing lobby or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutAttribute Out parameter used to receive the EOS_Lobby_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_Lobby_Attribute
 * @see EOS_LobbyDetails_CopyAttributeByKeyOptions
 * @see EOS_Lobby_Attribute_Release
 */
EOS_EResult EOSSDK_LobbyDetails::CopyAttributeByKey(const EOS_LobbyDetails_CopyAttributeByKeyOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->AttrKey == nullptr || OutAttribute == nullptr)
    {
        set_nullptr(OutAttribute);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto attr_it = _infos.attributes().find(Options->AttrKey);
    if (attr_it == _infos.attributes().end())
    {
        *OutAttribute = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    EOS_Lobby_Attribute* pAttribute = new EOS_Lobby_Attribute;
    EOS_Lobby_AttributeData* pData = new EOS_Lobby_AttributeData;

    {
        size_t len = attr_it->first.length() + 1;
        char* str = new char[len];
        strncpy(str, attr_it->first.c_str(), len);
        pData->Key = str;
    }
    
    switch (attr_it->second.value().value_case())
    {
        case Session_Attr_Value::ValueCase::kB: 
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
            pData->Value.AsBool = attr_it->second.value().b();
        }
        break;

        case Session_Attr_Value::ValueCase::kD:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
            pData->Value.AsDouble = attr_it->second.value().d();
        }
        break;

        case Session_Attr_Value::ValueCase::kI:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
            pData->Value.AsInt64 = attr_it->second.value().i();
        }
        break;

        case Session_Attr_Value::ValueCase::kS:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_STRING;
            std::string const& value = attr_it->second.value().s();
            char* str = new char[value.length() + 1];
            strncpy(str, value.c_str(), value.length() + 1);
            pData->Value.AsUtf8 = str;
        }
    }

    pAttribute->ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;
    pAttribute->Data = pData;
    pAttribute->Visibility = (EOS_ELobbyAttributeVisibility)attr_it->second.visibility_type();

    *OutAttribute = pAttribute;

    return EOS_EResult::EOS_Success;
}

/**
 * Get the number of members associated with this lobby
 *
 * @param Options the Options associated with retrieving the member count
 *
 * @return number of members in the existing lobby or 0 if there is an error
 */
uint32_t EOSSDK_LobbyDetails::GetMemberCount(const EOS_LobbyDetails_GetMemberCountOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
        return 0;

    return _infos.members_size();
}

/**
 * EOS_LobbyDetails_GetMemberByIndex is used to immediately retrieve individual members registered with a lobby.
 *
 * @param Options Structure containing the input parameters
 *
 * @return the product user id for the registered member at a given index or null if that index is invalid
 *
 * @see EOS_LobbyDetails_GetMemberCount
 * @see EOS_LobbyDetails_GetMemberByIndexOptions
 */
EOS_ProductUserId EOSSDK_LobbyDetails::GetMemberByIndex(const EOS_LobbyDetails_GetMemberByIndexOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->MemberIndex >= _infos.members_size())
        return GetInvalidProductUserId();

    auto member_it = _infos.members().begin();
    std::advance(member_it, Options->MemberIndex);

    return GetProductUserId(member_it->first);
}

/**
 * EOS_LobbyDetails_GetMemberAttributeCount is used to immediately retrieve the attribute count for members in a lobby.
 *
 * @param Options Structure containing the input parameters
 *
 * @return the number of attributes associated with a given lobby member or 0 if that member is invalid
 *
 * @see EOS_LobbyDetails_GetMemberCount
 * @see EOS_LobbyDetails_GetMemberAttributeCountOptions
 */
uint32_t EOSSDK_LobbyDetails::GetMemberAttributeCount(const EOS_LobbyDetails_GetMemberAttributeCountOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->TargetUserId == nullptr)
        return 0;

    auto members_it = _infos.members().find(Options->TargetUserId->to_string());
    if (members_it == _infos.members().end())
        return 0;

    return members_it->second.attributes_size();
}

/**
 * EOS_LobbyDetails_CopyMemberAttributeByIndex is used to immediately retrieve a copy of a lobby member attribute from an existing lobby.
 * If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutAttribute Out parameter used to receive the EOS_Lobby_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_Lobby_Attribute
 * @see EOS_LobbyDetails_CopyMemberAttributeByIndexOptions
 * @see EOS_Lobby_Attribute_Release
 */
EOS_EResult EOSSDK_LobbyDetails::CopyMemberAttributeByIndex(const EOS_LobbyDetails_CopyMemberAttributeByIndexOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->TargetUserId == nullptr)
    {
        *OutAttribute = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto members_it = _infos.members().find(Options->TargetUserId->to_string());
    if (members_it == _infos.members().end())
    {
        *OutAttribute = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto attr_it = members_it->second.attributes().begin();
    std::advance(attr_it, Options->AttrIndex);

    EOS_Lobby_Attribute* pAttribute = new EOS_Lobby_Attribute;
    EOS_Lobby_AttributeData* pData = new EOS_Lobby_AttributeData;

    {
        size_t len = attr_it->first.length() + 1;
        char* str = new char[len];
        strncpy(str, attr_it->first.c_str(), len);
        pData->Key = str;
    }

    switch (attr_it->second.value().value_case())
    {
        case Session_Attr_Value::ValueCase::kB:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
            pData->Value.AsBool = attr_it->second.value().b();
        }
        break;

        case Session_Attr_Value::ValueCase::kD:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
            pData->Value.AsDouble = attr_it->second.value().d();
        }
        break;

        case Session_Attr_Value::ValueCase::kI:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
            pData->Value.AsInt64 = attr_it->second.value().i();
        }
        break;

        case Session_Attr_Value::ValueCase::kS:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_STRING;
            std::string const& value = attr_it->second.value().s();
            char* str = new char[value.length() + 1];
            strncpy(str, value.c_str(), value.length() + 1);
            pData->Value.AsUtf8 = str;
        }
    }

    pAttribute->ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;
    pAttribute->Data = pData;
    pAttribute->Visibility = (EOS_ELobbyAttributeVisibility)attr_it->second.visibility_type();

    *OutAttribute = pAttribute;

    return EOS_EResult::EOS_Success;
}

/**
 * EOS_LobbyDetails_CopyMemberAttributeByKey is used to immediately retrieve a copy of a lobby member attribute from an existing lobby.
 * If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutAttribute Out parameter used to receive the EOS_Lobby_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_Lobby_Attribute
 * @see EOS_LobbyDetails_CopyMemberAttributeByKeyOptions
 * @see EOS_Lobby_Attribute_Release
 */
EOS_EResult EOSSDK_LobbyDetails::CopyMemberAttributeByKey(const EOS_LobbyDetails_CopyMemberAttributeByKeyOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->AttrKey == nullptr || Options->TargetUserId == nullptr)
    {
        *OutAttribute = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto members_it = _infos.members().find(Options->TargetUserId->to_string());
    if (members_it == _infos.members().end())
    {
        *OutAttribute = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto attr_it = members_it->second.attributes().find(Options->AttrKey);
    if (attr_it == members_it->second.attributes().end())
    {
        *OutAttribute = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    EOS_Lobby_Attribute* pAttribute = new EOS_Lobby_Attribute;
    EOS_Lobby_AttributeData* pData = new EOS_Lobby_AttributeData;

    {
        size_t len = attr_it->first.length() + 1;
        char* str = new char[len];
        strncpy(str, attr_it->first.c_str(), len);
        pData->Key = str;
    }

    switch (attr_it->second.value().value_case())
    {
        case Session_Attr_Value::ValueCase::kB:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
            pData->Value.AsBool = attr_it->second.value().b();
        }
        break;

        case Session_Attr_Value::ValueCase::kD:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
            pData->Value.AsDouble = attr_it->second.value().d();
        }
        break;

        case Session_Attr_Value::ValueCase::kI:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
            pData->Value.AsInt64 = attr_it->second.value().i();
        }
        break;

        case Session_Attr_Value::ValueCase::kS:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_STRING;
            std::string const& value = attr_it->second.value().s();
            char* str = new char[value.length() + 1];
            strncpy(str, value.c_str(), value.length() + 1);
            pData->Value.AsUtf8 = str;
        }
    }

    pAttribute->ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;
    pAttribute->Data = pData;
    pAttribute->Visibility = (EOS_ELobbyAttributeVisibility)attr_it->second.visibility_type();

    *OutAttribute = pAttribute;

    return EOS_EResult::EOS_Success;
}

/**
 * Release the memory associated with a single lobby. This must be called on data retrieved from EOS_LobbySearch_CopySearchResultByIndex.
 *
 * @param LobbyHandle - The lobby handle to release
 *
 * @see EOS_LobbySearch_CopySearchResultByIndex
 */
void EOSSDK_LobbyDetails::Release()
{
    TRACE_FUNC();

    delete this;
}

}