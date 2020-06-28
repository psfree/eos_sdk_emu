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
 * This class represents the details of a session, including its session properties and the attribution associated with it
 * Locally created or joined active sessions will contain this information as will search results.
 * A handle to a session is required to join a session via search or invite
 */

 /**
  * EOS_SessionDetails_CopyInfo is used to immediately retrieve a copy of session information from a given source such as a active session or a search result.
  * If the call returns an EOS_Success result, the out parameter, OutSessionInfo, must be passed to EOS_SessionDetails_Info_Release to release the memory associated with it.
  *
  * @param Options Structure containing the input parameters
  * @param OutSessionInfo Out parameter used to receive the EOS_SessionDetails_Info structure.
  *
  * @return EOS_Success if the information is available and passed out in OutSessionInfo
  *         EOS_InvalidParameters if you pass a null pointer for the out parameter
  *         EOS_IncompatibleVersion if the API version passed in is incorrect
  *
  * @see EOS_SessionDetails_Info
  * @see EOS_SessionDetails_CopyInfoOptions
  * @see EOS_SessionDetails_Info_Release
  */
EOS_EResult EOSSDK_SessionDetails::CopyInfo(const EOS_SessionDetails_CopyInfoOptions* Options, EOS_SessionDetails_Info** OutSessionInfo)
{
    TRACE_FUNC();
    
    if (Options == nullptr || OutSessionInfo == nullptr)
    {
        *OutSessionInfo = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    EOS_SessionDetails_Info *pDetails = new EOS_SessionDetails_Info;
    EOS_SessionDetails_Settings *pSettings = new EOS_SessionDetails_Settings;
    
    pSettings->ApiVersion = EOS_SESSIONDETAILS_SETTINGS_API_LATEST;
    pSettings->bAllowJoinInProgress = _infos.join_in_progress_allowed();
    pSettings->bInvitesAllowed = _infos.invites_allowed();
    pSettings->PermissionLevel = static_cast<EOS_EOnlineSessionPermissionLevel>(_infos.permission_level());
    pSettings->NumPublicConnections = _infos.maxplayers();
    {
        std::string const& bucketid = _infos.bucketid();
        char* str;
        if (!bucketid.empty())
        {
            str = new char[bucketid.length() + 1];
            strncpy(str, bucketid.c_str(), bucketid.length() + 1);
        }
        else
        {
            str = new char[1];
            *str = 0;
        }
        pSettings->BucketId = str;
    }

    pDetails->ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;
    {
        std::string const& sessionid = _infos.sessionid();
        char *str;
        if (!sessionid.empty())
        {
            str = new char[sessionid.length() + 1];
            strncpy(str, sessionid.c_str(), sessionid.length() + 1);
        }
        else
        {
            str = new char[1];
            *str = '\0';
        }
        pDetails->SessionId = str;
    }
    {
        std::string const& hostaddr = _infos.host_address();
        char* str;
        if (!hostaddr.empty())
        {
            str = new char[hostaddr.length() + 1];
            strncpy(str, hostaddr.c_str(), hostaddr.length() + 1);
        }
        else
        {
            str = new char[1];
            *str = '\0';
        }
        pDetails->HostAddress = str;
    }
    pDetails->NumOpenPublicConnections = _infos.maxplayers() - _infos.players_size();
    pDetails->Settings = pSettings;
    *OutSessionInfo = pDetails;
    
    return EOS_EResult::EOS_Success;
}

/**
 * Get the number of attributes associated with this session
 *
 * @param Options the Options associated with retrieving the attribute count
 *
 * @return number of attributes on the session or 0 if there is an error
 */
uint32_t EOSSDK_SessionDetails::GetSessionAttributeCount(const EOS_SessionDetails_GetSessionAttributeCountOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
        return 0;

    return _infos.attributes_size();
}

/**
 * EOS_SessionDetails_CopySessionAttributeByIndex is used to immediately retrieve a copy of session attribution from a given source such as a active session or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutSessionAttribute, must be passed to EOS_SessionDetails_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionAttribute Out parameter used to receive the EOS_SessionDetails_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutSessionAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_SessionDetails_Attribute
 * @see EOS_SessionDetails_CopySessionAttributeByIndexOptions
 * @see EOS_SessionDetails_Attribute_Release
 */
EOS_EResult EOSSDK_SessionDetails::CopySessionAttributeByIndex(const EOS_SessionDetails_CopySessionAttributeByIndexOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->AttrIndex >= static_cast<uint32_t>(_infos.attributes_size()) || OutSessionAttribute == nullptr)
    {
        *OutSessionAttribute = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }
    
    EOS_SessionDetails_Attribute* pAttr = new EOS_SessionDetails_Attribute;
    EOS_Sessions_AttributeData* pData = new EOS_Sessions_AttributeData;

    auto it = _infos.attributes().begin();
    std::advance(it, Options->AttrIndex);
    
    pData->ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
    {
        std::string const& key = it->first;
        char* str = new char[key.length() + 1];
        strncpy(str, key.c_str(), key.length() + 1);
        pData->Key = str;
    }

    switch (it->second.value().value_case())
    {
        case Session_Attr_Value::ValueCase::kB: 
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
            pData->Value.AsBool = it->second.value().b();
        }
        break;

        case Session_Attr_Value::ValueCase::kD:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
            pData->Value.AsDouble = it->second.value().d();
        }
        break;

        case Session_Attr_Value::ValueCase::kI:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
            pData->Value.AsInt64 = it->second.value().i();
        }
        break;

        case Session_Attr_Value::ValueCase::kS:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_STRING;
            std::string const& value = it->second.value().s();
            char* str = new char[value.length() + 1];
            strncpy(str, value.c_str(), value.length() + 1);
            pData->Value.AsUtf8 = str;
        }
    }
    
    
    pAttr->ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST;
    pAttr->Data = pData;
    pAttr->AdvertisementType = static_cast<EOS_ESessionAttributeAdvertisementType>(it->second.advertisement_type());
    *OutSessionAttribute = pAttr;
    return EOS_EResult::EOS_Success;
}

/**
 * EOS_SessionDetails_CopySessionAttributeByKey is used to immediately retrieve a copy of session attribution from a given source such as a active session or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutSessionAttribute, must be passed to EOS_SessionDetails_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionAttribute Out parameter used to receive the EOS_SessionDetails_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutSessionAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_SessionDetails_Attribute
 * @see EOS_SessionDetails_CopySessionAttributeByKeyOptions
 * @see EOS_SessionDetails_Attribute_Release
 */
EOS_EResult EOSSDK_SessionDetails::CopySessionAttributeByKey(const EOS_SessionDetails_CopySessionAttributeByKeyOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->AttrKey == nullptr || OutSessionAttribute == nullptr)
    {
        *OutSessionAttribute = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }
    
    EOS_SessionDetails_Attribute* pAttr = new EOS_SessionDetails_Attribute;
    EOS_Sessions_AttributeData* pData = new EOS_Sessions_AttributeData;

    auto it = _infos.attributes().find(Options->AttrKey);
    if (it == _infos.attributes().end())
    {
        *OutSessionAttribute = nullptr;
        return EOS_EResult::EOS_NotFound;
    }

    pData->ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
    {
        std::string const& key = it->first;
        char* str = new char[key.length() + 1];
        strncpy(str, key.c_str(), key.length() + 1);
        pData->Key = str;
    }

    switch (it->second.value().value_case())
    {
        case Session_Attr_Value::ValueCase::kB:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
            pData->Value.AsBool = it->second.value().b();
        }
        break;

        case Session_Attr_Value::ValueCase::kD:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
            pData->Value.AsDouble = it->second.value().d();
        }
        break;

        case Session_Attr_Value::ValueCase::kI:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
            pData->Value.AsInt64 = it->second.value().i();
        }
        break;

        case Session_Attr_Value::ValueCase::kS:
        {
            pData->ValueType = EOS_ESessionAttributeType::EOS_AT_STRING;
            std::string const& value = it->second.value().s();
            char* str = new char[value.length() + 1];
            strncpy(str, value.c_str(), value.length() + 1);
            pData->Value.AsUtf8 = str;
        }
    }


    pAttr->ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST;
    pAttr->Data = pData;
    pAttr->AdvertisementType = static_cast<EOS_ESessionAttributeAdvertisementType>(it->second.advertisement_type());

    *OutSessionAttribute = pAttr;
    return EOS_EResult::EOS_Success;
}

}