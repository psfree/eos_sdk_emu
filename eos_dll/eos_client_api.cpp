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

#include "eos_client_api.h"
#include "eossdk_platform.h"
#include "eossdk_auth.h"
#include "settings.h"

EOSSDK_Client::EOSSDK_Client():
    _sdk_initialized(false)
{}

EOSSDK_Client::~EOSSDK_Client()
{
    for (auto& id : _epicuserids)
        delete id.second;

    for (auto& id : _productuserids)
        delete id.second;
}

EOSSDK_Client& EOSSDK_Client::Inst()
{
    static EOSSDK_Client inst;
    return inst;
}

EOS_EpicAccountId EOSSDK_Client::get_epicuserid(std::string const& userid)
{
    EOS_EpicAccountId res;
    if (userid.empty())
        return GetInvalidEpicUserId();

    auto it = _epicuserids.find(userid);
    if (it == _epicuserids.end())
    {
        EOS_EpicAccountId& accountid = _epicuserids[userid];
        accountid = new EOS_EpicAccountIdDetails;
        accountid->from_string(userid);
        res = accountid;
    }
    else
        res = it->second;

    return res;
}

EOS_ProductUserId EOSSDK_Client::get_productuserid(std::string const& userid)
{
    EOS_ProductUserId res;
    if (userid.empty())
        return GetInvalidProductUserId();

    auto it = _productuserids.find(userid);
    if (it == _productuserids.end())
    {
        EOS_ProductUserId& accountid = _productuserids[userid];
        accountid = new EOS_ProductUserIdDetails;
        accountid->from_string(userid);
        res = accountid;
    }
    else
        res = it->second;

    return res;
}

/**
 * Initialize the Epic Online Services SDK.
 *
 * Before calling any other function in the SDK, clients must call this function.
 *
 * This function must only be called one time and must have a corresponding EOS_Shutdown call.
 *
 * @param Options - The initialization options to use for the SDK.
 * @return An EOS_EResult is returned to indicate success or an error.
 *
 * EOS_Success is returned if the SDK successfully initializes.
 * EOS_AlreadyConfigured is returned if the function has already been called.
 * EOS_InvalidParameters is returned if the provided options are invalid.
 */
// I:\TetrisEffect\TetrisEffect\Binaries\Win64\TetrisEffect-Win64-Shipping.exe
static bool set_eos_compat(int32_t compat_version)
{
    int failed = false;
#if ! defined(__WINDOWS_32__)
    if (compat_version == 1)
    {
        LOG(Log::LogLevel::DEBUG, "Tryiing to replace EOS_Auth_CopyUserAuthToken(%p) with EOS_Auth_CopyUserAuthTokenOld(%p)", EOS_Auth_CopyUserAuthToken, EOS_Auth_CopyUserAuthTokenOld);
        if (mini_detour::replace_func((void*)EOS_Auth_CopyUserAuthToken, (void*)EOS_Auth_CopyUserAuthTokenOld) ||
            mini_detour::replace_func((void*)EOS_Auth_AddNotifyLoginStatusChanged, (void*)EOS_Auth_AddNotifyLoginStatusChangedOld))
        {
            failed = true;
        }
    }
    else
    {
        LOG(Log::LogLevel::DEBUG, "Tryiing to replace EOS_Auth_CopyUserAuthToken(%p) with EOS_Auth_CopyUserAuthTokenNew(%p)", EOS_Auth_CopyUserAuthToken, EOS_Auth_CopyUserAuthTokenNew);
        if (mini_detour::replace_func((void*)EOS_Auth_CopyUserAuthToken, (void*)EOS_Auth_CopyUserAuthTokenNew) ||
            mini_detour::replace_func((void*)EOS_Auth_AddNotifyLoginStatusChanged, (void*)EOS_Auth_AddNotifyLoginStatusChangedNew))
        {
            failed = true;
        }
    }
#endif
    return failed;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Initialize(const EOS_InitializeOptions* Options)
{
    GLOBAL_LOCK();

    Settings::Inst();
    TRACE_FUNC();

    auto &inst = EOSSDK_Client::Inst();

    if (inst._sdk_initialized)
        return EOS_EResult::EOS_AlreadyConfigured;

    if (Options == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (Settings::Inst().disable_online_networking)
    {
        disable_online_networking();
    }

    if (set_eos_compat(Options->ApiVersion))
    {
        LOG(Log::LogLevel::FATAL, "Couldn't replace our dummy EOS_Auth_CopyUserAuthToken, the function will not work and thus we terminate.");
        abort();
    }

    switch (Options->ApiVersion)
    {
        case EOS_INITIALIZE_API_003:
        {
            auto p = reinterpret_cast<const EOS_InitializeOptions003*>(Options);
            LOG(Log::LogLevel::DEBUG, "SystemInitializeOptions = %p", p->SystemInitializeOptions);
        }

        case EOS_INITIALIZE_API_002:
        {
            auto p = reinterpret_cast<const EOS_InitializeOptions002*>(Options);
            LOG(Log::LogLevel::DEBUG, "Reserved = %p", p->Reserved);
        }

        case EOS_INITIALIZE_API_001:
        {
            auto p = reinterpret_cast<const EOS_InitializeOptions001*>(Options);
            LOG(Log::LogLevel::DEBUG, "ApiVersion = %u", p->ApiVersion);
            LOG(Log::LogLevel::DEBUG, "AllocateMemoryFunction = %p", p->AllocateMemoryFunction);
            LOG(Log::LogLevel::DEBUG, "ReallocateMemoryFunction = %p", p->ReallocateMemoryFunction);
            LOG(Log::LogLevel::DEBUG, "ReleaseMemoryFunction = %p", p->ReleaseMemoryFunction);
            LOG(Log::LogLevel::DEBUG, "ProductName = %s", p->ProductName);
            LOG(Log::LogLevel::DEBUG, "ProductVersion = %s", p->ProductVersion);

            inst._allocate_memory_func = p->AllocateMemoryFunction;
            inst._reallocate_memory_func = p->ReallocateMemoryFunction;
            inst._release_memory_func = p->ReleaseMemoryFunction;

            inst.api_version = p->ApiVersion;
            if (p->ProductName != nullptr && *p->ProductName != 0)
            {
                if (Settings::Inst().gamename.empty() || Settings::Inst().gamename == "DefaultGameName")
                {// Set the gamename in the settings if a gamename was passed in the cmdline and its empty in the JSon
                    Settings::Inst().gamename = Options->ProductName;
                    Settings::Inst().save_settings();
                }
            }
            if (p->ProductVersion != nullptr)
                inst.product_version = Options->ProductVersion;
        }
    }

    inst._sdk_initialized = true;
    return EOS_EResult::EOS_Success;
}

/**
 * Tear down the Epic Online Services SDK.
 *
 * Once this function has been called, no more SDK calls are permitted; calling anything after EOS_Shutdown will result in undefined behavior.
 * @return An EOS_EResult is returned to indicate success or an error.
 * EOS_Success is returned if the SDK is successfully torn down.
 * EOS_NotConfigured is returned if a successful call to EOS_Initialize has not been made.
 * EOS_UnexpectedError is returned if EOS_Shutdown has already been called.
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Shutdown()
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (!EOSSDK_Client::Inst()._sdk_initialized)
        return EOS_EResult::EOS_NotConfigured;

    EOSSDK_Client::Inst()._sdk_initialized = false;
    return EOS_EResult::EOS_Success;
}

/**
 * Returns a string representation of an EOS_EResult.
 * The return value is never null.
 * The return value must not be freed.
 *
 * Example: EOS_EResult_ToString(EOS_Success) returns "EOS_Success"
 */
EOS_DECLARE_FUNC(const char*) EOS_EResult_ToString(EOS_EResult Result)
{
    TRACE_FUNC();

    switch (Result)
    {
#define CASE_RESULT(RES) case EOS_EResult::RES: return #RES
        CASE_RESULT(EOS_Success);
        CASE_RESULT(EOS_NoConnection);
        CASE_RESULT(EOS_InvalidCredentials);
        CASE_RESULT(EOS_InvalidUser);
        CASE_RESULT(EOS_InvalidAuth);
        CASE_RESULT(EOS_AccessDenied);
        CASE_RESULT(EOS_MissingPermissions);
        CASE_RESULT(EOS_Token_Not_Account);
        CASE_RESULT(EOS_TooManyRequests);
        CASE_RESULT(EOS_AlreadyPending);
        CASE_RESULT(EOS_InvalidParameters);
        CASE_RESULT(EOS_InvalidRequest);
        CASE_RESULT(EOS_UnrecognizedResponse);
        CASE_RESULT(EOS_IncompatibleVersion);
        CASE_RESULT(EOS_NotConfigured);
        CASE_RESULT(EOS_AlreadyConfigured);
        CASE_RESULT(EOS_NotImplemented);
        CASE_RESULT(EOS_Canceled);
        CASE_RESULT(EOS_NotFound);
        CASE_RESULT(EOS_OperationWillRetry);
        CASE_RESULT(EOS_NoChange);
        CASE_RESULT(EOS_VersionMismatch);
        CASE_RESULT(EOS_LimitExceeded);
        CASE_RESULT(EOS_Disabled);
        CASE_RESULT(EOS_DuplicateNotAllowed);
        CASE_RESULT(EOS_MissingParameters_DEPRECATED);

        CASE_RESULT(EOS_InvalidSandboxId);
        CASE_RESULT(EOS_TimedOut);
        CASE_RESULT(EOS_PartialResult);
        CASE_RESULT(EOS_Missing_Role);
        CASE_RESULT(EOS_Missing_Feature);
        CASE_RESULT(EOS_Invalid_Sandbox);
        CASE_RESULT(EOS_Invalid_Deployment);
        CASE_RESULT(EOS_Invalid_Product);
        CASE_RESULT(EOS_Invalid_ProductUserID);
        CASE_RESULT(EOS_ServiceFailure);
        CASE_RESULT(EOS_CacheDirectoryMissing);
        CASE_RESULT(EOS_CacheDirectoryInvalid);
        CASE_RESULT(EOS_InvalidState);

        CASE_RESULT(EOS_Auth_AccountLocked);
        CASE_RESULT(EOS_Auth_AccountLockedForUpdate);
        CASE_RESULT(EOS_Auth_InvalidRefreshToken);
        CASE_RESULT(EOS_Auth_InvalidToken);
        CASE_RESULT(EOS_Auth_AuthenticationFailure);
        CASE_RESULT(EOS_Auth_InvalidPlatformToken);
        CASE_RESULT(EOS_Auth_WrongAccount);
        CASE_RESULT(EOS_Auth_WrongClient);
        CASE_RESULT(EOS_Auth_FullAccountRequired);
        CASE_RESULT(EOS_Auth_HeadlessAccountRequired);
        CASE_RESULT(EOS_Auth_PasswordResetRequired);
        CASE_RESULT(EOS_Auth_PasswordCannotBeReused);
        CASE_RESULT(EOS_Auth_Expired);
        CASE_RESULT(EOS_Auth_ScopeConsentRequired);
        CASE_RESULT(EOS_Auth_ApplicationNotFound);
        CASE_RESULT(EOS_Auth_ScopeNotFound);
        CASE_RESULT(EOS_Auth_AccountFeatureRestricted);
        CASE_RESULT(EOS_Auth_PinGrantCode);
        CASE_RESULT(EOS_Auth_PinGrantExpired);
        CASE_RESULT(EOS_Auth_PinGrantPending);
        CASE_RESULT(EOS_Auth_ExternalAuthNotLinked);
        CASE_RESULT(EOS_Auth_ExternalAuthRevoked);
        CASE_RESULT(EOS_Auth_ExternalAuthInvalid);
        CASE_RESULT(EOS_Auth_ExternalAuthRestricted);
        CASE_RESULT(EOS_Auth_ExternalAuthCannotLogin);
        CASE_RESULT(EOS_Auth_ExternalAuthExpired);
        CASE_RESULT(EOS_Auth_ExternalAuthIsLastLoginType);
        CASE_RESULT(EOS_Auth_ExchangeCodeNotFound);
        CASE_RESULT(EOS_Auth_OriginatingExchangeCodeSessionExpired);
        CASE_RESULT(EOS_Auth_PersistentAuth_AccountNotActive);
        CASE_RESULT(EOS_Auth_MFARequired);
        CASE_RESULT(EOS_Auth_ParentalControls);
        CASE_RESULT(EOS_Auth_NoRealId);

        CASE_RESULT(EOS_Friends_InviteAwaitingAcceptance);
        CASE_RESULT(EOS_Friends_NoInvitation);
        CASE_RESULT(EOS_Friends_AlreadyFriends);
        CASE_RESULT(EOS_Friends_NotFriends);
        CASE_RESULT(EOS_Friends_TargetUserTooManyInvites);
        CASE_RESULT(EOS_Friends_LocalUserTooManyInvites);
        CASE_RESULT(EOS_Friends_TargetUserFriendLimitExceeded);
        CASE_RESULT(EOS_Friends_LocalUserFriendLimitExceeded);

        CASE_RESULT(EOS_Presence_DataInvalid);
        CASE_RESULT(EOS_Presence_DataLengthInvalid);
        CASE_RESULT(EOS_Presence_DataKeyInvalid);
        CASE_RESULT(EOS_Presence_DataKeyLengthInvalid);
        CASE_RESULT(EOS_Presence_DataValueInvalid);
        CASE_RESULT(EOS_Presence_DataValueLengthInvalid);
        CASE_RESULT(EOS_Presence_RichTextInvalid);
        CASE_RESULT(EOS_Presence_RichTextLengthInvalid);
        CASE_RESULT(EOS_Presence_StatusInvalid);

        CASE_RESULT(EOS_Ecom_EntitlementStale);
        CASE_RESULT(EOS_Ecom_CatalogOfferStale);
        CASE_RESULT(EOS_Ecom_CatalogItemStale);
        CASE_RESULT(EOS_Ecom_CatalogOfferPriceInvalid);
        CASE_RESULT(EOS_Ecom_CheckoutLoadError);

        CASE_RESULT(EOS_Sessions_SessionInProgress);
        CASE_RESULT(EOS_Sessions_TooManyPlayers);
        CASE_RESULT(EOS_Sessions_NoPermission);
        CASE_RESULT(EOS_Sessions_SessionAlreadyExists);
        CASE_RESULT(EOS_Sessions_InvalidLock);
        CASE_RESULT(EOS_Sessions_InvalidSession);
        CASE_RESULT(EOS_Sessions_SandboxNotAllowed);
        CASE_RESULT(EOS_Sessions_InviteFailed);
        CASE_RESULT(EOS_Sessions_InviteNotFound);
        CASE_RESULT(EOS_Sessions_UpsertNotAllowed);
        CASE_RESULT(EOS_Sessions_AggregationFailed);
        CASE_RESULT(EOS_Sessions_HostAtCapacity);
        CASE_RESULT(EOS_Sessions_SandboxAtCapacity);
        CASE_RESULT(EOS_Sessions_SessionNotAnonymous);
        CASE_RESULT(EOS_Sessions_OutOfSync);
        CASE_RESULT(EOS_Sessions_TooManyInvites);
        CASE_RESULT(EOS_Sessions_PresenceSessionExists);
        CASE_RESULT(EOS_Sessions_DeploymentAtCapacity);
        CASE_RESULT(EOS_Sessions_NotAllowed);

        CASE_RESULT(EOS_PlayerDataStorage_FilenameInvalid);
        CASE_RESULT(EOS_PlayerDataStorage_FilenameLengthInvalid);
        CASE_RESULT(EOS_PlayerDataStorage_FilenameInvalidChars);
        CASE_RESULT(EOS_PlayerDataStorage_FileSizeTooLarge);
        CASE_RESULT(EOS_PlayerDataStorage_FileSizeInvalid);
        CASE_RESULT(EOS_PlayerDataStorage_FileHandleInvalid);
        CASE_RESULT(EOS_PlayerDataStorage_DataInvalid);
        CASE_RESULT(EOS_PlayerDataStorage_DataLengthInvalid);
        CASE_RESULT(EOS_PlayerDataStorage_StartIndexInvalid);
        CASE_RESULT(EOS_PlayerDataStorage_RequestInProgress);
        CASE_RESULT(EOS_PlayerDataStorage_UserThrottled);
        CASE_RESULT(EOS_PlayerDataStorage_EncryptionKeyNotSet);
        CASE_RESULT(EOS_PlayerDataStorage_UserErrorFromDataCallback);
        CASE_RESULT(EOS_PlayerDataStorage_FileHeaderHasNewerVersion);
        CASE_RESULT(EOS_PlayerDataStorage_FileCorrupted);

        CASE_RESULT(EOS_Connect_ExternalTokenValidationFailed);
        CASE_RESULT(EOS_Connect_UserAlreadyExists);
        CASE_RESULT(EOS_Connect_AuthExpired);
        CASE_RESULT(EOS_Connect_InvalidToken);
        CASE_RESULT(EOS_Connect_UnsupportedTokenType);
        CASE_RESULT(EOS_Connect_LinkAccountFailed);
        CASE_RESULT(EOS_Connect_ExternalServiceUnavailable);
        CASE_RESULT(EOS_Connect_ExternalServiceConfigurationFailure);
        CASE_RESULT(EOS_Connect_LinkAccountFailedMissingNintendoIdAccount);

        CASE_RESULT(EOS_UI_SocialOverlayLoadError);

        CASE_RESULT(EOS_Lobby_NotOwner);
        CASE_RESULT(EOS_Lobby_InvalidLock);
        CASE_RESULT(EOS_Lobby_LobbyAlreadyExists);
        CASE_RESULT(EOS_Lobby_SessionInProgress);
        CASE_RESULT(EOS_Lobby_TooManyPlayers);
        CASE_RESULT(EOS_Lobby_NoPermission);
        CASE_RESULT(EOS_Lobby_InvalidSession);
        CASE_RESULT(EOS_Lobby_SandboxNotAllowed);
        CASE_RESULT(EOS_Lobby_InviteFailed);
        CASE_RESULT(EOS_Lobby_InviteNotFound);
        CASE_RESULT(EOS_Lobby_UpsertNotAllowed);
        CASE_RESULT(EOS_Lobby_AggregationFailed);
        CASE_RESULT(EOS_Lobby_HostAtCapacity);
        CASE_RESULT(EOS_Lobby_SandboxAtCapacity);
        CASE_RESULT(EOS_Lobby_TooManyInvites);
        CASE_RESULT(EOS_Lobby_DeploymentAtCapacity);
        CASE_RESULT(EOS_Lobby_NotAllowed);
        CASE_RESULT(EOS_Lobby_MemberUpdateOnly);

        default: return "EOS_UnexpectedError";
#undef CASE_RESULT
    }
}

/**
 * Returns whether a result is to be considered the final result, or false if the callback that returned this result
 * will be called again either after some time or from another action.
 *
 * @param Result The result to check against being a final result for an operation
 * @return True if this result means the operation is complete, false otherwise
 */
EOS_DECLARE_FUNC(EOS_Bool) EOS_EResult_IsOperationComplete(EOS_EResult Result)
{
    TRACE_FUNC();

    switch (Result)
    {
        case EOS_EResult::EOS_OperationWillRetry:
        case EOS_EResult::EOS_Auth_PinGrantCode:
        case EOS_EResult::EOS_Auth_MFARequired:
            return EOS_FALSE;
    }

    return EOS_TRUE;
}

/**
 * Encode a byte array into hex encoded string
 *
 * @return An EOS_EResult that indicates whether the byte array was converted and copied into the OutBuffer.
 *         EOS_Success if the encoding was successful and passed out in OutBuffer
 *         EOS_InvalidParameters if you pass a null pointer on invalid length for any of the parameters
 *         EOS_LimitExceeded - The OutBuffer is not large enough to receive the encoding. InOutBufferLength contains the required minimum length to perform the operation successfully.
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_ByteArray_ToString(const uint8_t* ByteArray, const uint32_t Length, char* OutBuffer, uint32_t* InOutBufferLength)
{
    TRACE_FUNC();

    return EOS_EResult::EOS_Success;
}

/**
 * Check whether or not the given account unique id is considered valid
 *
 * @param AccountId The account id to check for validity
 * @return EOS_TRUE if the EOS_EpicAccountId is valid, otherwise EOS_FALSE
 */
EOS_DECLARE_FUNC(EOS_Bool) EOS_AccountId_IsValid(EOS_AccountId AccountId)
{
    return EOS_EpicAccountId_IsValid(AccountId);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_EpicAccountId_IsValid(EOS_EpicAccountId AccountId)
{
    //TRACE_FUNC();
    if (AccountId == nullptr)
        return EOS_FALSE;

    return AccountId->IsValid();
}

/**
 * Retrieve a string-ified account ID from an EOS_EpicAccountId. This is useful for replication of Epic account IDs in multiplayer games.
 *
 * @param AccountId The account ID for which to retrieve the string-ified version.
 * @param OutBuffer The buffer into which the character data should be written
 * @param InOutBufferLength The size of the OutBuffer in characters.
 *                          The input buffer should include enough space to be null-terminated.
 *                          When the function returns, this parameter will be filled with the length of the string copied into OutBuffer.
 *
 * @return An EOS_EResult that indicates whether the account ID string was copied into the OutBuffer.
 *         EOS_Success - The OutBuffer was filled, and InOutBufferLength contains the number of characters copied into OutBuffer excluding the null terminator.
 *         EOS_InvalidParameters - Either OutBuffer or InOutBufferLength were passed as NULL parameters.
 *         EOS_InvalidUser - The AccountId is invalid and cannot be string-ified
 *         EOS_LimitExceeded - The OutBuffer is not large enough to receive the account ID string. InOutBufferLength contains the required minimum length to perform the operation successfully.
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_AccountId_ToString(EOS_AccountId AccountId, char* OutBuffer, int32_t* InOutBufferLength)
{
    return EOS_EpicAccountId_ToString(AccountId, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_EpicAccountId_ToString(EOS_EpicAccountId AccountId, char* OutBuffer, int32_t* InOutBufferLength)
{
    //TRACE_FUNC();
    if (AccountId == nullptr || !AccountId->IsValid())
        return EOS_EResult::EOS_InvalidUser;

    return AccountId->ToString(OutBuffer, InOutBufferLength);
}

/**
 * Retrieve an EOS_EpicAccountId from a raw account ID string. The input string must be null-terminated.
 *
 * @param AccountIdString The string-ified account ID for which to retrieve the EOS_EpicAccountId
 * @return The EOS_EpicAccountId that corresponds to the AccountIdString
 */
EOS_DECLARE_FUNC(EOS_AccountId) EOS_AccountId_FromString(const char* AccountIdString)
{
    return EOS_EpicAccountId_FromString(AccountIdString);
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_EpicAccountId_FromString(const char* AccountIdString)
{
    //TRACE_FUNC();
    if (AccountIdString == nullptr)
        return EOSSDK_Client::Inst().get_epicuserid(sdk::NULL_USER_ID);

    return EOSSDK_Client::Inst().get_epicuserid(AccountIdString);
}

/**
 * Check whether or not the given account unique id is considered valid
 *
 * @param AccountId The account id to check for validity
 * @return EOS_TRUE if the EOS_ProductUserId is valid, otherwise EOS_FALSE
 */
EOS_DECLARE_FUNC(EOS_Bool) EOS_ProductUserId_IsValid(EOS_ProductUserId AccountId)
{
    //TRACE_FUNC();
    if (AccountId == nullptr)
        return EOS_FALSE;

    return AccountId->IsValid();
}

/**
 * Retrieve a string-ified account ID from an EOS_ProductUserId. This is useful for replication of Product User IDs in multiplayer games.
 *
 * @param AccountId The account ID for which to retrieve the string-ified version.
 * @param OutBuffer The buffer into which the character data should be written
 * @param InOutBufferLength The size of the OutBuffer in characters.
 *                          The input buffer should include enough space to be null-terminated.
 *                          When the function returns, this parameter will be filled with the length of the string copied into OutBuffer.
 *
 * @return An EOS_EResult that indicates whether the account ID string was copied into the OutBuffer.
 *         EOS_Success - The OutBuffer was filled, and InOutBufferLength contains the number of characters copied into OutBuffer excluding the null terminator.
 *         EOS_InvalidParameters - Either OutBuffer or InOutBufferLength were passed as NULL parameters.
 *         EOS_InvalidUser - The AccountId is invalid and cannot be string-ified
 *         EOS_LimitExceeded - The OutBuffer is not large enough to receive the account ID string. InOutBufferLength contains the required minimum length to perform the operation successfully.
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_ProductUserId_ToString(EOS_ProductUserId AccountId, char* OutBuffer, int32_t* InOutBufferLength)
{
    //TRACE_FUNC();

    if (AccountId == nullptr || !AccountId->IsValid())
        return EOS_EResult::EOS_InvalidUser;

    return AccountId->ToString(OutBuffer, InOutBufferLength);
}

/**
 * Retrieve an EOS_EpicAccountId from a raw account ID string. The input string must be null-terminated.
 *
 * @param AccountIdString The string-ified account ID for which to retrieve the EOS_ProductUserId
 * @return The EOS_ProductUserId that corresponds to the AccountIdString
 */
EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_ProductUserId_FromString(const char* AccountIdString)
{
    //TRACE_FUNC();
    if (AccountIdString == nullptr)
        return EOSSDK_Client::Inst().get_productuserid(sdk::NULL_USER_ID);

    return EOSSDK_Client::Inst().get_productuserid(AccountIdString);
}

/**
 * Set the callback function to use for SDK log messages. Any previously set callback will no longer be called.
 *
 * @param Callback the function to call when the SDK logs messages
 * @return EOS_Success is returned if the callback will be used for future log messages.
 *         EOS_NotConfigured is returned if the SDK has not yet been initialized, or if it has been shut down
 *
 * @see EOS_Initialize
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Logging_SetCallback(EOS_LogMessageFunc Callback)
{
    TRACE_FUNC();

    return EOS_EResult::EOS_Success;
}

/**
 * Set the logging level for the specified logging category. By default all log categories will callback for Warnings, Errors, and Fatals.
 *
 * @param LogCategory the specific log category to configure. Use EOS_LC_ALL_CATEGORIES to configure all categories simultaneously to the same log level.
 * @param LogLevel the log level to use for the log category
 *
 * @return EOS_Success is returned if the log levels are now in use.
 *         EOS_NotConfigured is returned if the SDK has not yet been initialized, or if it has been shut down.
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Logging_SetLogLevel(EOS_ELogCategory LogCategory, EOS_ELogLevel LogLevel)
{
    TRACE_FUNC();

    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(const char*) EOS_GetVersion(void)
{
    std::map<std::string, const char*> versions{
        { "1.0.0", "1.0.0-5464091"  },
        { "1.1.0", "1.1.0-6537116"  },
        { "1.2.0", "1.2.0-9765216"  },
        { "1.3.0", "1.3.0-11034880" },
        { "1.3.1", "1.3.1-11123224" },
        { "1.5.0", "1.5.0-12496671" },
        { "1.6.0", "1.6.0-13289764" },
        { "1.6.1", "1.6.1-13568552" },
        { "1.6.2", "1.6.2-13619780" },
    };
    
    return versions[EOS_VERSION_STRING];
}