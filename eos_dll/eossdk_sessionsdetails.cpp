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
    LOG(Log::LogLevel::TRACE, "");

    if (Options == nullptr || OutSessionInfo == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    EOS_SessionDetails_Info *details = new EOS_SessionDetails_Info;
    EOS_SessionDetails_Settings *settings = new EOS_SessionDetails_Settings;

    details->ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;
    details->NumOpenPublicConnections = infos.maxplayers() - infos.players_size();
    {
        std::string const& hostaddr = infos.host_address();
        if (!hostaddr.empty())
        {
            char* str = new char[hostaddr.length() + 1];
            strncpy(str, hostaddr.c_str(), hostaddr.length() + 1);
            details->HostAddress = str;
        }
        else
            details->HostAddress = nullptr;
    }
    {
        std::string const& sessionid = infos.sessionid();
        if (!sessionid.empty())
        {
            char* str = new char[sessionid.length() + 1];
            strncpy(str, sessionid.c_str(), sessionid.length() + 1);
            details->SessionId = str;
        }
        else
            details->SessionId = nullptr;
    }
    
    settings->ApiVersion = EOS_SESSIONDETAILS_SETTINGS_API_LATEST;
    settings->bAllowJoinInProgress = infos.join_in_progress_allowed();
    settings->bInvitesAllowed = infos.invites_allowed();
    settings->PermissionLevel = static_cast<EOS_EOnlineSessionPermissionLevel>(infos.permission_level());
    settings->NumPublicConnections = infos.maxplayers();
    {
        std::string const& bucketid = infos.bucketid();
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
        settings->BucketId = str;
    }

    details->Settings = settings;
    *OutSessionInfo = details;
    
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
    LOG(Log::LogLevel::TRACE, "");
    if (Options == nullptr)
        return 0;

    return infos.attributes_size();
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
    LOG(Log::LogLevel::TRACE, "");
    if (Options == nullptr || Options->AttrIndex >= infos.attributes_size() || OutSessionAttribute == nullptr)
        return EOS_EResult::EOS_InvalidParameters;
    
    EOS_SessionDetails_Attribute* attr = new EOS_SessionDetails_Attribute;
    EOS_Sessions_AttributeData* data = new EOS_Sessions_AttributeData;

    attr->ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST;

    auto it = infos.attributes().begin();
    std::advance(it, Options->AttrIndex);

    attr->AdvertisementType = static_cast<EOS_ESessionAttributeAdvertisementType>(it->second.advertisement_type());
    
    data->ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
    {
        std::string const& key = it->first;
        char* str = new char[key.length() + 1];
        strncpy(str, key.c_str(), key.length() + 1);
        data->Key = str;
    }

    switch (it->second.value().value_case())
    {
        case Session_Attr_Value::ValueCase::kB: 
        {
            data->ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
            data->Value.AsBool = it->second.value().b();
        }
        break;

        case Session_Attr_Value::ValueCase::kD:
        {
            data->ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
            data->Value.AsDouble = it->second.value().d();
        }
        break;

        case Session_Attr_Value::ValueCase::kI:
        {
            data->ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
            data->Value.AsInt64 = it->second.value().i();
        }
        break;

        case Session_Attr_Value::ValueCase::kS:
        {
            data->ValueType = EOS_ESessionAttributeType::EOS_AT_STRING;
            std::string const& value = it->second.value().s();
            char* str = new char[value.length() + 1];
            strncpy(str, value.c_str(), value.length() + 1);
            data->Value.AsUtf8 = str;
        }
    }
    
    
    attr->Data = data;
    *OutSessionAttribute = attr;
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
    LOG(Log::LogLevel::TRACE, "");
    if (Options == nullptr || Options->AttrKey == nullptr || OutSessionAttribute == nullptr)
        return EOS_EResult::EOS_InvalidParameters;
    
    EOS_SessionDetails_Attribute* attr = new EOS_SessionDetails_Attribute;
    EOS_Sessions_AttributeData* data = new EOS_Sessions_AttributeData;

    attr->ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST;

    auto it = infos.attributes().find(Options->AttrKey);
    if(it == infos.attributes().end())
        return EOS_EResult::EOS_NotFound;

    attr->AdvertisementType = static_cast<EOS_ESessionAttributeAdvertisementType>(it->second.advertisement_type());

    data->ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
    {
        std::string const& key = it->first;
        char* str = new char[key.length() + 1];
        strncpy(str, key.c_str(), key.length() + 1);
        data->Key = str;
    }

    switch (it->second.value().value_case())
    {
        case Session_Attr_Value::ValueCase::kB:
        {
            data->ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
            data->Value.AsBool = it->second.value().b();
        }
        break;

        case Session_Attr_Value::ValueCase::kD:
        {
            data->ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
            data->Value.AsDouble = it->second.value().d();
        }
        break;

        case Session_Attr_Value::ValueCase::kI:
        {
            data->ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
            data->Value.AsInt64 = it->second.value().i();
        }
        break;

        case Session_Attr_Value::ValueCase::kS:
        {
            data->ValueType = EOS_ESessionAttributeType::EOS_AT_STRING;
            std::string const& value = it->second.value().s();
            char* str = new char[value.length() + 1];
            strncpy(str, value.c_str(), value.length() + 1);
            data->Value.AsUtf8 = str;
        }
    }


    attr->Data = data;
    *OutSessionAttribute = attr;
    return EOS_EResult::EOS_Success;
}

}