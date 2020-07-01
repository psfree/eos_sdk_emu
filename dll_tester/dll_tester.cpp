// dll_tester.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <thread>
#include <iomanip>

#define EOS_BUILD_DLL 1

#include <eos_common.h>
#include <eos_sdk.h>
#include <eos_auth.h>
#include <eos_logging.h>
#include <eos_presence.h>

#include <Windows.h>

#pragma comment(lib, "user32")

#include "Log.h"

#include "mini_detour/mini_detour.h"
#include "utils.h"

static HMODULE original_dll = nullptr;
static const char* original_dll_name = "EOSSDK-Win64-Shipping.original.dll";
static std::string exe_path;

using callback_t = void(EOS_CALL *)(void*);

struct callback_wrapper
{
    void* ClientData;
    callback_t CbFunc;
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            original_dll = LoadLibrary(original_dll_name);

            Log::set_loglevel(Log::LogLevel::TRACE);

            LOG(Log::LogLevel::INFO, "Original dll load (%s): %p", original_dll_name, original_dll);
            LOG(Log::LogLevel::INFO, "command line: %s", GetCommandLine());
        }
        break;

        case DLL_THREAD_ATTACH:
        break;
        case DLL_THREAD_DETACH:
        break;
        case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

EOS_HPlatform hPlatform = nullptr;
EOS_HMetrics  hMetrics  = nullptr;
EOS_HUserInfo hUserInfo = nullptr;

#define ORIGINAL_FUNCTION(NAME) static decltype(NAME)* _##NAME = (decltype(_##NAME))GetProcAddress(original_dll, #NAME)

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenOld(EOS_HAuth Handle, EOS_AccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken);
EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenNew(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken);
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedOld(EOS_HAuth Handle, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification);
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedNew(EOS_HAuth Handle, const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification);

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
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Initialize);

    if (set_eos_compat(Options->ApiVersion))
    {
        LOG(Log::LogLevel::FATAL, "Couldn't replace our dummy EOS_Auth_CopyUserAuthToken, the function will not work and thus we terminate.");
        abort();
    }

    auto res = _EOS_Initialize(Options);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion              : " << Options->ApiVersion << std::endl;
    switch (Options->ApiVersion)
    {
        case EOS_INITIALIZE_API_003:
        {
            auto* v = reinterpret_cast<const EOS_InitializeOptions003*>(Options);
            sstr << "SystemInitializeOptions : " << Options->SystemInitializeOptions << std::endl;
        }

        case EOS_INITIALIZE_API_002:
        {
            auto* v = reinterpret_cast<const EOS_InitializeOptions002*>(Options);
            sstr << "Reserved                : " << Options->Reserved << std::endl;
        }

        case EOS_INITIALIZE_API_001:
        {
            auto* v = reinterpret_cast<const EOS_InitializeOptions001*>(Options);
            sstr << "AllocateMemoryFunction  : " << Options->AllocateMemoryFunction << std::endl;
            sstr << "ReallocateMemoryFunction: " << Options->ReallocateMemoryFunction << std::endl;
            sstr << "ReleaseMemoryFunction   : " << Options->ReleaseMemoryFunction << std::endl;
            sstr << "ProductName             : " << (Options->ProductName == nullptr ? "" : Options->ProductName) << std::endl;
            sstr << "ProductVersion          : " << (Options->ProductVersion == nullptr ? "" : Options->ProductVersion) << std::endl;
        }
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Shutdown()
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Shutdown);
    return _EOS_Shutdown();
}

EOS_DECLARE_FUNC(const char*) EOS_EResult_ToString(EOS_EResult Result)
{
    //LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_EResult_ToString);
    return _EOS_EResult_ToString(Result);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_EResult_IsOperationComplete(EOS_EResult Result)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_EResult_IsOperationComplete);
    return _EOS_EResult_IsOperationComplete(Result);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ByteArray_ToString(const uint8_t* ByteArray, const uint32_t Length, char* OutBuffer, uint32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ByteArray_ToString);
    return _EOS_ByteArray_ToString(ByteArray, Length, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_EpicAccountId_IsValid(EOS_EpicAccountId AccountId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_EpicAccountId_IsValid);
    auto res = _EOS_EpicAccountId_IsValid(AccountId);

    std::stringstream sstr;
    if (res == EOS_TRUE)
    {
        char buff[2048];
        int32_t size = 2048;
        EOS_EpicAccountId_ToString(AccountId, buff, &size);

        sstr << buff;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_EpicAccountId_ToString(EOS_EpicAccountId AccountId, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_EpicAccountId_ToString);
    auto res = _EOS_EpicAccountId_ToString(AccountId, OutBuffer, InOutBufferLength);
    return res;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_EpicAccountId_FromString(const char* AccountIdString)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_EpicAccountId_FromString);
    auto res = _EOS_EpicAccountId_FromString(AccountIdString);
    LOG(Log::LogLevel::DEBUG, "%p = %s", res, AccountIdString);
    return res;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_ProductUserId_IsValid(EOS_ProductUserId AccountId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ProductUserId_IsValid);
    auto res = _EOS_ProductUserId_IsValid(AccountId);

    std::stringstream sstr;
    if (res == EOS_TRUE)
    {
        char buff[2048];
        int32_t size = 2048;
        EOS_ProductUserId_ToString(AccountId, buff, &size);

        sstr << buff;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProductUserId_ToString(EOS_ProductUserId AccountId, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ProductUserId_ToString);
    auto res = _EOS_ProductUserId_ToString(AccountId, OutBuffer, InOutBufferLength);
    return res;
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_ProductUserId_FromString(const char* AccountIdString)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ProductUserId_FromString);
    auto res = _EOS_ProductUserId_FromString(AccountIdString);
    LOG(Log::LogLevel::DEBUG, "%p = %s", res, AccountIdString);
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Logging_SetCallback(EOS_LogMessageFunc Callback)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Logging_SetCallback);
    return _EOS_Logging_SetCallback(Callback);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Logging_SetLogLevel(EOS_ELogCategory LogCategory, EOS_ELogLevel LogLevel)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Logging_SetLogLevel);
    return _EOS_Logging_SetLogLevel(LogCategory, LogLevel);
}

////////////////////////////////////////////
// UserInfo
EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfo(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UserInfo_QueryUserInfo);
    return _EOS_UserInfo_QueryUserInfo(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfoByDisplayName(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoByDisplayNameOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoByDisplayNameCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UserInfo_QueryUserInfoByDisplayName);
    return _EOS_UserInfo_QueryUserInfoByDisplayName(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyUserInfo(EOS_HUserInfo Handle, const EOS_UserInfo_CopyUserInfoOptions* Options, EOS_UserInfo** OutUserInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UserInfo_CopyUserInfo);
    return _EOS_UserInfo_CopyUserInfo(Handle, Options, OutUserInfo);
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_Release(EOS_UserInfo* UserInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UserInfo_Release);
    return _EOS_UserInfo_Release(UserInfo);
}

////////////////////////////////////////////
// UI
EOS_DECLARE_FUNC(void) EOS_UI_ShowFriends(EOS_HUI Handle, const EOS_UI_ShowFriendsOptions* Options, void* ClientData, const EOS_UI_OnShowFriendsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_ShowFriends);
    return _EOS_UI_ShowFriends(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_UI_HideFriends(EOS_HUI Handle, const EOS_UI_HideFriendsOptions* Options, void* ClientData, const EOS_UI_OnHideFriendsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_HideFriends);
    return _EOS_UI_HideFriends(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_GetFriendsVisible(EOS_HUI Handle, const EOS_UI_GetFriendsVisibleOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_GetFriendsVisible);
    return _EOS_UI_GetFriendsVisible(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_SetToggleFriendsKeyOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_SetToggleFriendsKey);
    return _EOS_UI_SetToggleFriendsKey(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_UI_EKeyCombination) EOS_UI_GetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_GetToggleFriendsKeyOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_GetToggleFriendsKey);
    return _EOS_UI_GetToggleFriendsKey(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsValidKeyCombination(EOS_HUI Handle, EOS_UI_EKeyCombination KeyCombination)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_IsValidKeyCombination);
    return _EOS_UI_IsValidKeyCombination(Handle, KeyCombination);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetDisplayPreference(EOS_HUI Handle, const EOS_UI_SetDisplayPreferenceOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_SetDisplayPreference);
    return _EOS_UI_SetDisplayPreference(Handle, Options);
}


EOS_DECLARE_FUNC(EOS_UI_ENotificationLocation) EOS_UI_GetNotificationLocationPreference(EOS_HUI Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_GetNotificationLocationPreference);
    return _EOS_UI_GetNotificationLocationPreference(Handle);
}

////////////////////////////////////////////
// Stats
EOS_DECLARE_FUNC(void) EOS_Stats_IngestStat(EOS_HStats Handle, const EOS_Stats_IngestStatOptions* Options, void* ClientData, const EOS_Stats_OnIngestStatCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_IngestStat);
    return _EOS_Stats_IngestStat(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Stats_QueryStats(EOS_HStats Handle, const EOS_Stats_QueryStatsOptions* Options, void* ClientData, const EOS_Stats_OnQueryStatsCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_QueryStats);
    return _EOS_Stats_QueryStats(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Stats_GetStatsCount(EOS_HStats Handle, const EOS_Stats_GetStatCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_GetStatsCount);
    return _EOS_Stats_GetStatsCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByIndex(EOS_HStats Handle, const EOS_Stats_CopyStatByIndexOptions* Options, EOS_Stats_Stat** OutStat)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_CopyStatByIndex);
    return _EOS_Stats_CopyStatByIndex(Handle, Options, OutStat);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByName(EOS_HStats Handle, const EOS_Stats_CopyStatByNameOptions* Options, EOS_Stats_Stat** OutStat)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_CopyStatByName);
    return _EOS_Stats_CopyStatByName(Handle, Options, OutStat);
}

EOS_DECLARE_FUNC(void) EOS_Stats_Stat_Release(EOS_Stats_Stat* Stat)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_Stat_Release);
    return _EOS_Stats_Stat_Release(Stat);
}

////////////////////////////////////////////
// Sessions
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CreateSessionModification(EOS_HSessions Handle, const EOS_Sessions_CreateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CreateSessionModification);
    return _EOS_Sessions_CreateSessionModification(Handle, Options, OutSessionModificationHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_UpdateSessionModification(EOS_HSessions Handle, const EOS_Sessions_UpdateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_UpdateSessionModification);
    return _EOS_Sessions_UpdateSessionModification(Handle, Options, OutSessionModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_UpdateSession(EOS_HSessions Handle, const EOS_Sessions_UpdateSessionOptions* Options, void* ClientData, const EOS_Sessions_OnUpdateSessionCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_UpdateSession);

    callback_wrapper* wrapper = new callback_wrapper;
    wrapper->ClientData = ClientData;
    wrapper->CbFunc = (callback_t)CompletionDelegate;
    auto f = [](const EOS_Sessions_UpdateSessionCallbackInfo *cbinfo) {
        callback_wrapper* wrapper = (callback_wrapper*)cbinfo->ClientData;

        LOG(Log::LogLevel::DEBUG, "%s: '%s' - '%s'", EOS_EResult_ToString(cbinfo->ResultCode), cbinfo->SessionId, cbinfo->SessionName);

        const_cast<EOS_Sessions_UpdateSessionCallbackInfo*>(cbinfo)->ClientData = wrapper->ClientData;
        wrapper->CbFunc((void*)cbinfo);
        delete wrapper;
    };

    return _EOS_Sessions_UpdateSession(Handle, Options, wrapper, f);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_DestroySession(EOS_HSessions Handle, const EOS_Sessions_DestroySessionOptions* Options, void* ClientData, const EOS_Sessions_OnDestroySessionCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_DestroySession);
    return _EOS_Sessions_DestroySession(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_JoinSession(EOS_HSessions Handle, const EOS_Sessions_JoinSessionOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_JoinSession);
    return _EOS_Sessions_JoinSession(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_StartSession(EOS_HSessions Handle, const EOS_Sessions_StartSessionOptions* Options, void* ClientData, const EOS_Sessions_OnStartSessionCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_StartSession);
    return _EOS_Sessions_StartSession(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_EndSession(EOS_HSessions Handle, const EOS_Sessions_EndSessionOptions* Options, void* ClientData, const EOS_Sessions_OnEndSessionCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_EndSession);
    return _EOS_Sessions_EndSession(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RegisterPlayers(EOS_HSessions Handle, const EOS_Sessions_RegisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnRegisterPlayersCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_RegisterPlayers);
    return _EOS_Sessions_RegisterPlayers(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_UnregisterPlayers(EOS_HSessions Handle, const EOS_Sessions_UnregisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnUnregisterPlayersCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_UnregisterPlayers);
    return _EOS_Sessions_UnregisterPlayers(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_SendInvite(EOS_HSessions Handle, const EOS_Sessions_SendInviteOptions* Options, void* ClientData, const EOS_Sessions_OnSendInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_SendInvite);
    return _EOS_Sessions_SendInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RejectInvite(EOS_HSessions Handle, const EOS_Sessions_RejectInviteOptions* Options, void* ClientData, const EOS_Sessions_OnRejectInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_RejectInvite);
    return _EOS_Sessions_RejectInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_QueryInvites(EOS_HSessions Handle, const EOS_Sessions_QueryInvitesOptions* Options, void* ClientData, const EOS_Sessions_OnQueryInvitesCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_QueryInvites);
    return _EOS_Sessions_QueryInvites(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Sessions_GetInviteCount(EOS_HSessions Handle, const EOS_Sessions_GetInviteCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_GetInviteCount);
    return _EOS_Sessions_GetInviteCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_GetInviteIdByIndex(EOS_HSessions Handle, const EOS_Sessions_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_GetInviteIdByIndex);
    return _EOS_Sessions_GetInviteIdByIndex(Handle, Options, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CreateSessionSearch(EOS_HSessions Handle, const EOS_Sessions_CreateSessionSearchOptions* Options, EOS_HSessionSearch* OutSessionSearchHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CreateSessionSearch);
    return _EOS_Sessions_CreateSessionSearch(Handle, Options, OutSessionSearchHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopyActiveSessionHandle(EOS_HSessions Handle, const EOS_Sessions_CopyActiveSessionHandleOptions* Options, EOS_HActiveSession* OutSessionHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CopyActiveSessionHandle);
    return _EOS_Sessions_CopyActiveSessionHandle(Handle, Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteReceived(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteReceivedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteReceivedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_AddNotifySessionInviteReceived);
    return _EOS_Sessions_AddNotifySessionInviteReceived(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteReceived(EOS_HSessions Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_RemoveNotifySessionInviteReceived);
    return _EOS_Sessions_RemoveNotifySessionInviteReceived(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteAccepted(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteAcceptedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_AddNotifySessionInviteAccepted);
    return _EOS_Sessions_AddNotifySessionInviteAccepted(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteAccepted(EOS_HSessions Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_RemoveNotifySessionInviteAccepted);
    return _EOS_Sessions_RemoveNotifySessionInviteAccepted(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleByInviteId(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleByInviteIdOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CopySessionHandleByInviteId);
    return _EOS_Sessions_CopySessionHandleByInviteId(Handle, Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleForPresence(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleForPresenceOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CopySessionHandleForPresence);
    auto res = _EOS_Sessions_CopySessionHandleForPresence(Handle, Options, OutSessionHandle);
    LOG(Log::LogLevel::DEBUG, "'%s': %p", EOS_EResult_ToString(res), OutSessionHandle);

    //EOS_SessionDetails_CopyInfoOptions options;
    //EOS_SessionDetails_Info* OutSessionInfo = new EOS_SessionDetails_Info;
    //EOS_SessionDetails_CopyInfo(*OutSessionHandle, &options, &OutSessionInfo);
    //EOS_SessionDetails_Info_Release(OutSessionInfo);
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_IsUserInSession(EOS_HSessions Handle, const EOS_Sessions_IsUserInSessionOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_IsUserInSession);
    return _EOS_Sessions_IsUserInSession(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_DumpSessionState(EOS_HSessions Handle, const EOS_Sessions_DumpSessionStateOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_DumpSessionState);
    return _EOS_Sessions_DumpSessionState(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetBucketId(EOS_HSessionModification Handle, const EOS_SessionModification_SetBucketIdOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetBucketId);
    return _EOS_SessionModification_SetBucketId(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetHostAddress(EOS_HSessionModification Handle, const EOS_SessionModification_SetHostAddressOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetHostAddress);
    return _EOS_SessionModification_SetHostAddress(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetPermissionLevel(EOS_HSessionModification Handle, const EOS_SessionModification_SetPermissionLevelOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetPermissionLevel);
    return _EOS_SessionModification_SetPermissionLevel(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetJoinInProgressAllowed(EOS_HSessionModification Handle, const EOS_SessionModification_SetJoinInProgressAllowedOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetJoinInProgressAllowed);
    return _EOS_SessionModification_SetJoinInProgressAllowed(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetMaxPlayers(EOS_HSessionModification Handle, const EOS_SessionModification_SetMaxPlayersOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetMaxPlayers);
    return _EOS_SessionModification_SetMaxPlayers(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetInvitesAllowed(EOS_HSessionModification Handle, const EOS_SessionModification_SetInvitesAllowedOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetInvitesAllowed);
    return _EOS_SessionModification_SetInvitesAllowed(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_AddAttribute(EOS_HSessionModification Handle, const EOS_SessionModification_AddAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_AddAttribute);
    auto res = _EOS_SessionModification_AddAttribute(Handle, Options);
    switch (Options->SessionAttribute->ValueType)
    {
        case EOS_ESessionAttributeType::EOS_AT_BOOLEAN: LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%d'", EOS_EResult_ToString(res)  , Options->SessionAttribute->Key, Options->SessionAttribute->Value.AsBool); break;
        case EOS_ESessionAttributeType::EOS_AT_DOUBLE : LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%f'", EOS_EResult_ToString(res)  , Options->SessionAttribute->Key, Options->SessionAttribute->Value.AsDouble); break;
        case EOS_ESessionAttributeType::EOS_AT_INT64  : LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%lli'", EOS_EResult_ToString(res), Options->SessionAttribute->Key, Options->SessionAttribute->Value.AsInt64); break;
        case EOS_ESessionAttributeType::EOS_AT_STRING : LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%s'", EOS_EResult_ToString(res)  , Options->SessionAttribute->Key, Options->SessionAttribute->Value.AsUtf8); break;
    }
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_RemoveAttribute(EOS_HSessionModification Handle, const EOS_SessionModification_RemoveAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_RemoveAttribute);
    return _EOS_SessionModification_RemoveAttribute(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ActiveSession_CopyInfo(EOS_HActiveSession Handle, const EOS_ActiveSession_CopyInfoOptions* Options, EOS_ActiveSession_Info** OutActiveSessionInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ActiveSession_CopyInfo);
    return _EOS_ActiveSession_CopyInfo(Handle, Options, OutActiveSessionInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_ActiveSession_GetRegisteredPlayerCount(EOS_HActiveSession Handle, const EOS_ActiveSession_GetRegisteredPlayerCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ActiveSession_GetRegisteredPlayerCount);
    return _EOS_ActiveSession_GetRegisteredPlayerCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_ActiveSession_GetRegisteredPlayerByIndex(EOS_HActiveSession Handle, const EOS_ActiveSession_GetRegisteredPlayerByIndexOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ActiveSession_GetRegisteredPlayerByIndex);
    return _EOS_ActiveSession_GetRegisteredPlayerByIndex(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopyInfo(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopyInfoOptions* Options, EOS_SessionDetails_Info** OutSessionInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_CopyInfo);
    return _EOS_SessionDetails_CopyInfo(Handle, Options, OutSessionInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_SessionDetails_GetSessionAttributeCount(EOS_HSessionDetails Handle, const EOS_SessionDetails_GetSessionAttributeCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_GetSessionAttributeCount);
    return _EOS_SessionDetails_GetSessionAttributeCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopySessionAttributeByIndex(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopySessionAttributeByIndexOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_CopySessionAttributeByIndex);
    return _EOS_SessionDetails_CopySessionAttributeByIndex(Handle, Options, OutSessionAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopySessionAttributeByKey(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopySessionAttributeByKeyOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_CopySessionAttributeByKey);
    return _EOS_SessionDetails_CopySessionAttributeByKey(Handle, Options, OutSessionAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetSessionId(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetSessionIdOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_SetSessionId);
    return _EOS_SessionSearch_SetSessionId(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetTargetUserId(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetTargetUserIdOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_SetTargetUserId);
    return _EOS_SessionSearch_SetTargetUserId(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetParameter(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetParameterOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_SetParameter);
    auto res = _EOS_SessionSearch_SetParameter(Handle, Options);
    switch (Options->Parameter->ValueType)
    {
        case EOS_ESessionAttributeType::EOS_AT_BOOLEAN: LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%d'"  , EOS_EResult_ToString(res), Options->Parameter->Key, Options->Parameter->Value.AsBool); break;
        case EOS_ESessionAttributeType::EOS_AT_DOUBLE : LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%f'"  , EOS_EResult_ToString(res), Options->Parameter->Key, Options->Parameter->Value.AsDouble); break;
        case EOS_ESessionAttributeType::EOS_AT_INT64  : LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%lli'", EOS_EResult_ToString(res), Options->Parameter->Key, Options->Parameter->Value.AsInt64); break;
        case EOS_ESessionAttributeType::EOS_AT_STRING : LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%s'"  , EOS_EResult_ToString(res), Options->Parameter->Key, Options->Parameter->Value.AsUtf8); break;
    }
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_RemoveParameter(EOS_HSessionSearch Handle, const EOS_SessionSearch_RemoveParameterOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_RemoveParameter);
    return _EOS_SessionSearch_RemoveParameter(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetMaxResults(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetMaxResultsOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_SetMaxResults);
    return _EOS_SessionSearch_SetMaxResults(Handle, Options);
}

EOS_DECLARE_FUNC(void) EOS_SessionSearch_Find(EOS_HSessionSearch Handle, const EOS_SessionSearch_FindOptions* Options, void* ClientData, const EOS_SessionSearch_OnFindCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_Find);

    callback_wrapper* wrapper = new callback_wrapper;
    wrapper->ClientData = ClientData;
    wrapper->CbFunc = (callback_t)CompletionDelegate;
    auto f = [](const EOS_SessionSearch_FindCallbackInfo* cbinfo) {
        callback_wrapper* wrapper = (callback_wrapper*)cbinfo->ClientData;

        LOG(Log::LogLevel::DEBUG, "%s", EOS_EResult_ToString(cbinfo->ResultCode));

        const_cast<EOS_SessionSearch_FindCallbackInfo*>(cbinfo)->ClientData = wrapper->ClientData;
        wrapper->CbFunc((void*)cbinfo);
        delete wrapper;
    };

    return _EOS_SessionSearch_Find(Handle, Options, wrapper, f);
}

EOS_DECLARE_FUNC(uint32_t) EOS_SessionSearch_GetSearchResultCount(EOS_HSessionSearch Handle, const EOS_SessionSearch_GetSearchResultCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_GetSearchResultCount);
    auto res = _EOS_SessionSearch_GetSearchResultCount(Handle, Options);
    LOG(Log::LogLevel::DEBUG, "%d", res);
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_CopySearchResultByIndex(EOS_HSessionSearch Handle, const EOS_SessionSearch_CopySearchResultByIndexOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_CopySearchResultByIndex);
    return _EOS_SessionSearch_CopySearchResultByIndex(Handle, Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(void) EOS_SessionModification_Release(EOS_HSessionModification SessionModificationHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_Release);
    return _EOS_SessionModification_Release(SessionModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_ActiveSession_Release(EOS_HActiveSession ActiveSessionHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ActiveSession_Release);
    return _EOS_ActiveSession_Release(ActiveSessionHandle);
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Release(EOS_HSessionDetails SessionHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_Release);
    return _EOS_SessionDetails_Release(SessionHandle);
}

EOS_DECLARE_FUNC(void) EOS_SessionSearch_Release(EOS_HSessionSearch SessionSearchHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_Release);
    return _EOS_SessionSearch_Release(SessionSearchHandle);
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Attribute_Release(EOS_SessionDetails_Attribute* SessionAttribute)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_Attribute_Release);
    return _EOS_SessionDetails_Attribute_Release(SessionAttribute);
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Info_Release(EOS_SessionDetails_Info* SessionInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_Info_Release);
    return _EOS_SessionDetails_Info_Release(SessionInfo);
}

EOS_DECLARE_FUNC(void) EOS_ActiveSession_Info_Release(EOS_ActiveSession_Info* ActiveSessionInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ActiveSession_Info_Release);
    return _EOS_ActiveSession_Info_Release(ActiveSessionInfo);
}

////////////////////////////////////////////
// Presence
EOS_DECLARE_FUNC(void) EOS_Presence_QueryPresence(EOS_HPresence Handle, const EOS_Presence_QueryPresenceOptions* Options, void* ClientData, const EOS_Presence_OnQueryPresenceCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_QueryPresence);
    return _EOS_Presence_QueryPresence(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_Presence_HasPresence(EOS_HPresence Handle, const EOS_Presence_HasPresenceOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_HasPresence);
    auto res = _EOS_Presence_HasPresence(Handle, Options);



    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CopyPresence(EOS_HPresence Handle, const EOS_Presence_CopyPresenceOptions* Options, EOS_Presence_Info** OutPresence)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_CopyPresence);
    auto res = _EOS_Presence_CopyPresence(Handle, Options, OutPresence);

    char local_id[2048];
    int32_t size = 2048;
    EOS_EpicAccountId_ToString(Options->LocalUserId, local_id, &size);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion : " << Options->ApiVersion << std::endl;
    sstr << "LocalUserId: " << local_id << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion    : " << (*OutPresence)->ApiVersion << std::endl;
        switch ((*OutPresence)->ApiVersion)
        {
            case EOS_PRESENCE_COPYPRESENCE_API_002:
            {
                auto v = reinterpret_cast<EOS_Presence_Info002*>(*OutPresence);
                sstr << "  ProductName   : " << v->ProductName << std::endl;
            }

            case EOS_PRESENCE_COPYPRESENCE_API_001:
            {
                auto v = reinterpret_cast<EOS_Presence_Info001*>(*OutPresence);
                char target_id[2048];
                size = 2048;
                EOS_EpicAccountId_ToString(v->UserId, target_id, &size);

                sstr << "  Status        : " << v->Status << std::endl;
                sstr << "  UserId        : " << target_id << std::endl;
                sstr << "  ProductId     : " << v->ProductId << std::endl;
                sstr << "  ProductVersion: " << v->ProductVersion << std::endl;
                sstr << "  Platform      : " << v->Platform << std::endl;
                sstr << "  RichText      : " << v->RichText << std::endl;
            }
        }

        for (int i = 0; i < (*OutPresence)->RecordsCount; ++i)
        {
            switch ((*OutPresence)->Records[i].ApiVersion)
            {
                case EOS_PRESENCE_DATARECORD_API_001:
                {
                    sstr << "    Records[" << i << "]: " << (*OutPresence)->Records[i].Key << " = " << (*OutPresence)->Records[i].Value << std::endl;
                }
                break;
            }
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());
    
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CreatePresenceModification(EOS_HPresence Handle, const EOS_Presence_CreatePresenceModificationOptions* Options, EOS_HPresenceModification* OutPresenceModificationHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_CreatePresenceModification);
    return _EOS_Presence_CreatePresenceModification(Handle, Options, OutPresenceModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_Presence_SetPresence(EOS_HPresence Handle, const EOS_Presence_SetPresenceOptions* Options, void* ClientData, const EOS_Presence_SetPresenceCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_SetPresence);
    return _EOS_Presence_SetPresence(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyOnPresenceChanged(EOS_HPresence Handle, const EOS_Presence_AddNotifyOnPresenceChangedOptions* Options, void* ClientData, const EOS_Presence_OnPresenceChangedCallback NotificationHandler)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_AddNotifyOnPresenceChanged);
    return _EOS_Presence_AddNotifyOnPresenceChanged(Handle, Options, ClientData, NotificationHandler);
}

EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyOnPresenceChanged(EOS_HPresence Handle, EOS_NotificationId NotificationId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_RemoveNotifyOnPresenceChanged);
    return _EOS_Presence_RemoveNotifyOnPresenceChanged(Handle, NotificationId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyJoinGameAccepted(EOS_HPresence Handle, const EOS_Presence_AddNotifyJoinGameAcceptedOptions* Options, void* ClientData, const EOS_Presence_OnJoinGameAcceptedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_AddNotifyJoinGameAccepted);
    return _EOS_Presence_AddNotifyJoinGameAccepted(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyJoinGameAccepted(EOS_HPresence Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_RemoveNotifyJoinGameAccepted);
    return _EOS_Presence_RemoveNotifyJoinGameAccepted(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_GetJoinInfo(EOS_HPresence Handle, const EOS_Presence_GetJoinInfoOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_GetJoinInfo);
    return _EOS_Presence_GetJoinInfo(Handle, Options, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetStatus(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetStatusOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_SetStatus);
    auto res = _EOS_PresenceModification_SetStatus(Handle, Options);
    switch (Options->Status)
    {
        case EOS_Presence_EStatus::EOS_PS_Online      : LOG(Log::LogLevel::DEBUG, "%s: %s", EOS_EResult_ToString(res), "EOS_PS_Online"); break;
        case EOS_Presence_EStatus::EOS_PS_Offline     : LOG(Log::LogLevel::DEBUG, "%s: %s", EOS_EResult_ToString(res), "EOS_PS_Offline"); break;
        case EOS_Presence_EStatus::EOS_PS_Away        : LOG(Log::LogLevel::DEBUG, "%s: %s", EOS_EResult_ToString(res), "EOS_PS_Away"); break;
        case EOS_Presence_EStatus::EOS_PS_DoNotDisturb: LOG(Log::LogLevel::DEBUG, "%s: %s", EOS_EResult_ToString(res), "EOS_PS_DoNotDisturb"); break;
        case EOS_Presence_EStatus::EOS_PS_ExtendedAway: LOG(Log::LogLevel::DEBUG, "%s: %s", EOS_EResult_ToString(res), "EOS_PS_ExtendedAway"); break;
    }
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetRawRichText(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetRawRichTextOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_SetRawRichText);
    auto res = _EOS_PresenceModification_SetRawRichText(Handle, Options);
    LOG(Log::LogLevel::DEBUG, "%s: %s", EOS_EResult_ToString(res), Options->RichText);
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetData(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetDataOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_SetData);
    auto res = _EOS_PresenceModification_SetData(Handle, Options);
    for (int i = 0; i < Options->RecordsCount; ++i)
    {
        LOG(Log::LogLevel::DEBUG, "%s: '%s'='%s'", EOS_EResult_ToString(res), Options->Records[i].Key, Options->Records[i].Value);
    }
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_DeleteData(EOS_HPresenceModification Handle, const EOS_PresenceModification_DeleteDataOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_DeleteData);
    return _EOS_PresenceModification_DeleteData(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetJoinInfo(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetJoinInfoOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_SetJoinInfo);
    return _EOS_PresenceModification_SetJoinInfo(Handle, Options);
}

EOS_DECLARE_FUNC(void) EOS_Presence_Info_Release(EOS_Presence_Info* PresenceInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_Info_Release);
    return _EOS_Presence_Info_Release(PresenceInfo);
}

EOS_DECLARE_FUNC(void) EOS_PresenceModification_Release(EOS_HPresenceModification PresenceModificationHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_Release);
    return _EOS_PresenceModification_Release(PresenceModificationHandle);
}

////////////////////////////////////////////
// PlayerDataStorage
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_QueryFileOptions* QueryFileOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_QueryFile);
    return _EOS_PlayerDataStorage_QueryFile(Handle, QueryFileOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFileList(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_QueryFileListOptions* QueryFileListOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileListCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_QueryFileList);
    return _EOS_PlayerDataStorage_QueryFileList(Handle, QueryFileListOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataByFilename(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_CopyFileMetadataByFilenameOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata** OutMetadata)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_CopyFileMetadataByFilename);
    return _EOS_PlayerDataStorage_CopyFileMetadataByFilename(Handle, CopyFileMetadataOptions, OutMetadata);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_GetFileMetadataCount(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_GetFileMetadataCountOptions* GetFileMetadataCountOptions, int32_t* OutFileMetadataCount)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_GetFileMetadataCount);
    return _EOS_PlayerDataStorage_GetFileMetadataCount(Handle, GetFileMetadataCountOptions, OutFileMetadataCount);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataAtIndex(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_CopyFileMetadataAtIndexOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata** OutMetadata)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_CopyFileMetadataAtIndex);
    return _EOS_PlayerDataStorage_CopyFileMetadataAtIndex(Handle, CopyFileMetadataOptions, OutMetadata);
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DuplicateFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DuplicateFileOptions* DuplicateOptions, void* ClientData, const EOS_PlayerDataStorage_OnDuplicateFileCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_DuplicateFile);
    return _EOS_PlayerDataStorage_DuplicateFile(Handle, DuplicateOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DeleteFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DeleteFileOptions* DeleteOptions, void* ClientData, const EOS_PlayerDataStorage_OnDeleteFileCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_DeleteFile);
    return _EOS_PlayerDataStorage_DeleteFile(Handle, DeleteOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_HPlayerDataStorageFileTransferRequest) EOS_PlayerDataStorage_ReadFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_ReadFileOptions* ReadOptions, void* ClientData, const EOS_PlayerDataStorage_OnReadFileCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_ReadFile);
    return _EOS_PlayerDataStorage_ReadFile(Handle, ReadOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_HPlayerDataStorageFileTransferRequest) EOS_PlayerDataStorage_WriteFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_WriteFileOptions* WriteOptions, void* ClientData, const EOS_PlayerDataStorage_OnWriteFileCompleteCallback CompletionCallback)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_WriteFile);
    return _EOS_PlayerDataStorage_WriteFile(Handle, WriteOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState(EOS_HPlayerDataStorageFileTransferRequest Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState);
    return _EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState(Handle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_GetFilename(EOS_HPlayerDataStorageFileTransferRequest Handle, uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorageFileTransferRequest_GetFilename);
    return _EOS_PlayerDataStorageFileTransferRequest_GetFilename(Handle, FilenameStringBufferSizeBytes, OutStringBuffer, OutStringLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_CancelRequest(EOS_HPlayerDataStorageFileTransferRequest Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorageFileTransferRequest_CancelRequest);
    return _EOS_PlayerDataStorageFileTransferRequest_CancelRequest(Handle);
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_FileMetadata_Release(EOS_PlayerDataStorage_FileMetadata* FileMetadata)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_FileMetadata_Release);
    return _EOS_PlayerDataStorage_FileMetadata_Release(FileMetadata);
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorageFileTransferRequest_Release(EOS_HPlayerDataStorageFileTransferRequest PlayerDataStorageFileTransferHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorageFileTransferRequest_Release);
    return _EOS_PlayerDataStorageFileTransferRequest_Release(PlayerDataStorageFileTransferHandle);
}

////////////////////////////////////////////
// Platform
EOS_DECLARE_FUNC(void) EOS_Platform_Tick(EOS_HPlatform Handle)
{
    //LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_Tick);

    return _EOS_Platform_Tick(Handle);
}

EOS_DECLARE_FUNC(EOS_HMetrics) EOS_Platform_GetMetricsInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetMetricsInterface);
    return _EOS_Platform_GetMetricsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HAuth) EOS_Platform_GetAuthInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetAuthInterface);
    return _EOS_Platform_GetAuthInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HConnect) EOS_Platform_GetConnectInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetConnectInterface);
    return _EOS_Platform_GetConnectInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HEcom) EOS_Platform_GetEcomInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetEcomInterface);
    return _EOS_Platform_GetEcomInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HUI) EOS_Platform_GetUIInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetUIInterface);
    return _EOS_Platform_GetUIInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HFriends) EOS_Platform_GetFriendsInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetFriendsInterface);
    return _EOS_Platform_GetFriendsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HPresence) EOS_Platform_GetPresenceInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetPresenceInterface);
    return _EOS_Platform_GetPresenceInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HSessions) EOS_Platform_GetSessionsInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetSessionsInterface);
    return _EOS_Platform_GetSessionsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HLobby) EOS_Platform_GetLobbyInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetLobbyInterface);
    return _EOS_Platform_GetLobbyInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HUserInfo) EOS_Platform_GetUserInfoInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetUserInfoInterface);
    return _EOS_Platform_GetUserInfoInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HP2P) EOS_Platform_GetP2PInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetP2PInterface);
    return _EOS_Platform_GetP2PInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HPlayerDataStorage) EOS_Platform_GetPlayerDataStorageInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetPlayerDataStorageInterface);
    return _EOS_Platform_GetPlayerDataStorageInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HAchievements) EOS_Platform_GetAchievementsInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetAchievementsInterface);
    return _EOS_Platform_GetAchievementsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HStats) EOS_Platform_GetStatsInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetStatsInterface);
    return _EOS_Platform_GetStatsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HLeaderboards) EOS_Platform_GetLeaderboardsInterface(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetLeaderboardsInterface);
    return _EOS_Platform_GetLeaderboardsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetActiveCountryCode(EOS_HPlatform Handle, EOS_EpicAccountId LocalUserId, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetActiveCountryCode);
    return _EOS_Platform_GetActiveCountryCode(Handle, LocalUserId, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetActiveLocaleCode(EOS_HPlatform Handle, EOS_EpicAccountId LocalUserId, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetActiveLocaleCode);
    return _EOS_Platform_GetActiveLocaleCode(Handle, LocalUserId, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetOverrideCountryCode(EOS_HPlatform Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetOverrideCountryCode);
    return _EOS_Platform_GetOverrideCountryCode(Handle, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetOverrideLocaleCode(EOS_HPlatform Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetOverrideLocaleCode);
    return _EOS_Platform_GetOverrideLocaleCode(Handle, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_SetOverrideCountryCode(EOS_HPlatform Handle, const char* NewCountryCode)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_SetOverrideCountryCode);
    return _EOS_Platform_SetOverrideCountryCode(Handle, NewCountryCode);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_SetOverrideLocaleCode(EOS_HPlatform Handle, const char* NewLocaleCode)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_SetOverrideLocaleCode);
    return _EOS_Platform_SetOverrideLocaleCode(Handle, NewLocaleCode);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_CheckForLauncherAndRestart(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_CheckForLauncherAndRestart);
    return _EOS_Platform_CheckForLauncherAndRestart(Handle);
}

EOS_DECLARE_FUNC(EOS_HPlatform) EOS_Platform_Create(const EOS_Platform_Options* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_Create);
    hPlatform = _EOS_Platform_Create(Options);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion                     : " << Options->ApiVersion << std::endl;
    switch (Options->ApiVersion)
    {
        case EOS_PLATFORM_OPTIONS_API_007:
        {
            auto* v = reinterpret_cast<const EOS_Platform_Options007*>(Options);
            sstr << "TickBudgetInMilliseconds       : " << Options->TickBudgetInMilliseconds << std::endl;
        }

        case EOS_PLATFORM_OPTIONS_API_006:
        {
            auto* v = reinterpret_cast<const EOS_Platform_Options006*>(Options);
            sstr << "CacheDirectory                 : " << (Options->CacheDirectory == nullptr ? "" : Options->CacheDirectory) << std::endl;
        }

        case EOS_PLATFORM_OPTIONS_API_005:
        {
            auto* v = reinterpret_cast<const EOS_Platform_Options005*>(Options);
            sstr << "EncryptionKey                  : " << (Options->EncryptionKey       == nullptr ? "" : Options->EncryptionKey) << std::endl;
            sstr << "OverrideCountryCode            : " << (Options->OverrideCountryCode == nullptr ? "" : Options->OverrideCountryCode) << std::endl;
            sstr << "OverrideLocaleCode             : " << (Options->OverrideLocaleCode  == nullptr ? "" : Options->OverrideLocaleCode) << std::endl;
            sstr << "DeploymentId                   : " << (Options->DeploymentId        == nullptr ? "" : Options->DeploymentId) << std::endl;
            sstr << "Flags                          : " << Options->Flags << std::endl;
        }

        case EOS_PLATFORM_OPTIONS_API_001:
        {
            auto* v = reinterpret_cast<const EOS_Platform_Options001*>(Options);
            sstr << "Reserved                       : " << Options->Reserved << std::endl;
            sstr << "ProductId                      : " << (Options->ProductId == nullptr ? "" : Options->ProductId) << std::endl;
            sstr << "SandboxId                      : " << (Options->SandboxId == nullptr ? "" : Options->SandboxId) << std::endl;
            sstr << "ClientCredentials::ClientId    : " << (Options->ClientCredentials.ClientId     == nullptr ? "" : Options->ClientCredentials.ClientId) << std::endl;
            sstr << "ClientCredentials::ClientSecret: " << (Options->ClientCredentials.ClientSecret == nullptr ? "" : Options->ClientCredentials.ClientSecret) << std::endl;
            sstr << "bIsServer                      : " << EOS_Bool_2_str(Options->bIsServer) << std::endl;
        }
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return hPlatform;
}

EOS_DECLARE_FUNC(void) EOS_Platform_Release(EOS_HPlatform Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_Release);
    return _EOS_Platform_Release(Handle);
}

////////////////////////////////////////////
// p2p
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SendPacket(EOS_HP2P Handle, const EOS_P2P_SendPacketOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_SendPacket);
    return _EOS_P2P_SendPacket(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetNextReceivedPacketSize(EOS_HP2P Handle, const EOS_P2P_GetNextReceivedPacketSizeOptions* Options, uint32_t* OutPacketSizeBytes)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_GetNextReceivedPacketSize);
    return _EOS_P2P_GetNextReceivedPacketSize(Handle, Options, OutPacketSizeBytes);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_ReceivePacket(EOS_HP2P Handle, const EOS_P2P_ReceivePacketOptions* Options, EOS_ProductUserId* OutPeerId, EOS_P2P_SocketId* OutSocketId, uint8_t* OutChannel, void* OutData, uint32_t* OutBytesWritten)
{
    //LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_ReceivePacket);
    return _EOS_P2P_ReceivePacket(Handle, Options, OutPeerId, OutSocketId, OutChannel, OutData, OutBytesWritten);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionRequest(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionRequestOptions* Options, void* ClientData, EOS_P2P_OnIncomingConnectionRequestCallback ConnectionRequestHandler)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_AddNotifyPeerConnectionRequest);
    return _EOS_P2P_AddNotifyPeerConnectionRequest(Handle, Options, ClientData, ConnectionRequestHandler);
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionRequest(EOS_HP2P Handle, EOS_NotificationId NotificationId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_RemoveNotifyPeerConnectionRequest);
    return _EOS_P2P_RemoveNotifyPeerConnectionRequest(Handle, NotificationId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionClosed(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionClosedOptions* Options, void* ClientData, EOS_P2P_OnRemoteConnectionClosedCallback ConnectionClosedHandler)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_AddNotifyPeerConnectionClosed);
    return _EOS_P2P_AddNotifyPeerConnectionClosed(Handle, Options, ClientData, ConnectionClosedHandler);
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionClosed(EOS_HP2P Handle, EOS_NotificationId NotificationId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_RemoveNotifyPeerConnectionClosed);
    return _EOS_P2P_RemoveNotifyPeerConnectionClosed(Handle, NotificationId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_AcceptConnection(EOS_HP2P Handle, const EOS_P2P_AcceptConnectionOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_AcceptConnection);
    return _EOS_P2P_AcceptConnection(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_CloseConnection(EOS_HP2P Handle, const EOS_P2P_CloseConnectionOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_CloseConnection);
    return _EOS_P2P_CloseConnection(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_CloseConnections(EOS_HP2P Handle, const EOS_P2P_CloseConnectionsOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_CloseConnections);
    return _EOS_P2P_CloseConnections(Handle, Options);
}

EOS_DECLARE_FUNC(void) EOS_P2P_QueryNATType(EOS_HP2P Handle, const EOS_P2P_QueryNATTypeOptions* Options, void* ClientData, const EOS_P2P_OnQueryNATTypeCompleteCallback NATTypeQueriedHandler)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_QueryNATType);

    callback_wrapper* wrapper = new callback_wrapper;
    wrapper->ClientData = ClientData;
    wrapper->CbFunc = (callback_t)NATTypeQueriedHandler;
    auto f = [](const EOS_P2P_OnQueryNATTypeCompleteInfo* cbinfo) {
        callback_wrapper* wrapper = (callback_wrapper*)cbinfo->ClientData;

        const char* nattype = "";
        switch (cbinfo->NATType)
        {
            case EOS_ENATType::EOS_NAT_Unknown : nattype = "EOS_NAT_Unknown" ; break;
            case EOS_ENATType::EOS_NAT_Open    : nattype = "EOS_NAT_Open"    ; break;
            case EOS_ENATType::EOS_NAT_Moderate: nattype = "EOS_NAT_Moderate"; break;
            case EOS_ENATType::EOS_NAT_Strict  : nattype = "EOS_NAT_Strict"  ; break;
        }
        LOG(Log::LogLevel::DEBUG, "%s: %s", EOS_EResult_ToString(cbinfo->ResultCode), nattype);

        const_cast<EOS_P2P_OnQueryNATTypeCompleteInfo*>(cbinfo)->ClientData = wrapper->ClientData;
        wrapper->CbFunc((void*)cbinfo);
        delete wrapper;
    };

    return _EOS_P2P_QueryNATType(Handle, Options, wrapper, f);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetNATType(EOS_HP2P Handle, const EOS_P2P_GetNATTypeOptions* Options, EOS_ENATType* OutNATType)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_GetNATType);
    auto res = _EOS_P2P_GetNATType(Handle, Options, OutNATType);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion      : " << Options->ApiVersion << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  OutNATType: " << *OutNATType << std::endl;
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

////////////////////////////////////////////
// metrics
EOS_DECLARE_FUNC(EOS_EResult) EOS_Metrics_BeginPlayerSession(EOS_HMetrics Handle, const EOS_Metrics_BeginPlayerSessionOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Metrics_BeginPlayerSession);
    return _EOS_Metrics_BeginPlayerSession(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Metrics_EndPlayerSession(EOS_HMetrics Handle, const EOS_Metrics_EndPlayerSessionOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Metrics_EndPlayerSession);
    return _EOS_Metrics_EndPlayerSession(Handle, Options);
}

////////////////////////////////////////////
// lobby
EOS_DECLARE_FUNC(void) EOS_Lobby_CreateLobby(EOS_HLobby Handle, const EOS_Lobby_CreateLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnCreateLobbyCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_CreateLobby);
    return _EOS_Lobby_CreateLobby(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_DestroyLobby(EOS_HLobby Handle, const EOS_Lobby_DestroyLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnDestroyLobbyCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_DestroyLobby);
    return _EOS_Lobby_DestroyLobby(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_JoinLobby(EOS_HLobby Handle, const EOS_Lobby_JoinLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnJoinLobbyCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_JoinLobby);
    return _EOS_Lobby_JoinLobby(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_LeaveLobby(EOS_HLobby Handle, const EOS_Lobby_LeaveLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnLeaveLobbyCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_LeaveLobby);
    return _EOS_Lobby_LeaveLobby(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_UpdateLobbyModification(EOS_HLobby Handle, const EOS_Lobby_UpdateLobbyModificationOptions* Options, EOS_HLobbyModification* OutLobbyModificationHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_UpdateLobbyModification);
    return _EOS_Lobby_UpdateLobbyModification(Handle, Options, OutLobbyModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_UpdateLobby(EOS_HLobby Handle, const EOS_Lobby_UpdateLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnUpdateLobbyCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_UpdateLobby);
    return _EOS_Lobby_UpdateLobby(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_PromoteMember(EOS_HLobby Handle, const EOS_Lobby_PromoteMemberOptions* Options, void* ClientData, const EOS_Lobby_OnPromoteMemberCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_PromoteMember);
    return _EOS_Lobby_PromoteMember(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_KickMember(EOS_HLobby Handle, const EOS_Lobby_KickMemberOptions* Options, void* ClientData, const EOS_Lobby_OnKickMemberCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_KickMember);
    return _EOS_Lobby_KickMember(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyUpdateReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyUpdateReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyUpdateReceivedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_AddNotifyLobbyUpdateReceived);
    return _EOS_Lobby_AddNotifyLobbyUpdateReceived(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyUpdateReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RemoveNotifyLobbyUpdateReceived);
    return _EOS_Lobby_RemoveNotifyLobbyUpdateReceived(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyMemberUpdateReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyMemberUpdateReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyMemberUpdateReceivedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_AddNotifyLobbyMemberUpdateReceived);
    return _EOS_Lobby_AddNotifyLobbyMemberUpdateReceived(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived);
    return _EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyMemberStatusReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyMemberStatusReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyMemberStatusReceivedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_AddNotifyLobbyMemberStatusReceived);
    return _EOS_Lobby_AddNotifyLobbyMemberStatusReceived(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived);
    return _EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived(Handle, InId);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_SendInvite(EOS_HLobby Handle, const EOS_Lobby_SendInviteOptions* Options, void* ClientData, const EOS_Lobby_OnSendInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_SendInvite);
    return _EOS_Lobby_SendInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RejectInvite(EOS_HLobby Handle, const EOS_Lobby_RejectInviteOptions* Options, void* ClientData, const EOS_Lobby_OnRejectInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RejectInvite);
    return _EOS_Lobby_RejectInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_QueryInvites(EOS_HLobby Handle, const EOS_Lobby_QueryInvitesOptions* Options, void* ClientData, const EOS_Lobby_OnQueryInvitesCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_QueryInvites);
    return _EOS_Lobby_QueryInvites(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Lobby_GetInviteCount(EOS_HLobby Handle, const EOS_Lobby_GetInviteCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_GetInviteCount);
    return _EOS_Lobby_GetInviteCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_GetInviteIdByIndex(EOS_HLobby Handle, const EOS_Lobby_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_GetInviteIdByIndex);
    return _EOS_Lobby_GetInviteIdByIndex(Handle, Options, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CreateLobbySearch(EOS_HLobby Handle, const EOS_Lobby_CreateLobbySearchOptions* Options, EOS_HLobbySearch* OutLobbySearchHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_CreateLobbySearch);
    return _EOS_Lobby_CreateLobbySearch(Handle, Options, OutLobbySearchHandle);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyInviteReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyInviteReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyInviteReceivedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_AddNotifyLobbyInviteReceived);
    return _EOS_Lobby_AddNotifyLobbyInviteReceived(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyInviteReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RemoveNotifyLobbyInviteReceived);
    return _EOS_Lobby_RemoveNotifyLobbyInviteReceived(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandleByInviteId(EOS_HLobby Handle, const EOS_Lobby_CopyLobbyDetailsHandleByInviteIdOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_CopyLobbyDetailsHandleByInviteId);
    return _EOS_Lobby_CopyLobbyDetailsHandleByInviteId(Handle, Options, OutLobbyDetailsHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandle(EOS_HLobby Handle, const EOS_Lobby_CopyLobbyDetailsHandleOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_CopyLobbyDetailsHandle);
    return _EOS_Lobby_CopyLobbyDetailsHandle(Handle, Options, OutLobbyDetailsHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetPermissionLevel(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetPermissionLevelOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_SetPermissionLevel);
    return _EOS_LobbyModification_SetPermissionLevel(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetMaxMembers(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetMaxMembersOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_SetMaxMembers);
    return _EOS_LobbyModification_SetMaxMembers(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_AddAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_AddAttribute);
    return _EOS_LobbyModification_AddAttribute(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_RemoveAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_RemoveAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_RemoveAttribute);
    return _EOS_LobbyModification_RemoveAttribute(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddMemberAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_AddMemberAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_AddMemberAttribute);
    return _EOS_LobbyModification_AddMemberAttribute(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_RemoveMemberAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_RemoveMemberAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_RemoveMemberAttribute);
    return _EOS_LobbyModification_RemoveMemberAttribute(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetLobbyOwner(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetLobbyOwnerOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_GetLobbyOwner);
    return _EOS_LobbyDetails_GetLobbyOwner(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyInfo(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyInfoOptions* Options, EOS_LobbyDetails_Info** OutLobbyDetailsInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_CopyInfo);
    return _EOS_LobbyDetails_CopyInfo(Handle, Options, OutLobbyDetailsInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetAttributeCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetAttributeCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_GetAttributeCount);
    return _EOS_LobbyDetails_GetAttributeCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyAttributeByIndexOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_CopyAttributeByIndex);
    return _EOS_LobbyDetails_CopyAttributeByIndex(Handle, Options, OutAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByKey(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyAttributeByKeyOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_CopyAttributeByKey);
    return _EOS_LobbyDetails_CopyAttributeByKey(Handle, Options, OutAttribute);
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_GetMemberCount);
    return _EOS_LobbyDetails_GetMemberCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetMemberByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberByIndexOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_GetMemberByIndex);
    return _EOS_LobbyDetails_GetMemberByIndex(Handle, Options);
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberAttributeCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberAttributeCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_GetMemberAttributeCount);
    return _EOS_LobbyDetails_GetMemberAttributeCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberAttributeByIndexOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_CopyMemberAttributeByIndex);
    return _EOS_LobbyDetails_CopyMemberAttributeByIndex(Handle, Options, OutAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByKey(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberAttributeByKeyOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_CopyMemberAttributeByKey);
    return _EOS_LobbyDetails_CopyMemberAttributeByKey(Handle, Options, OutAttribute);
}

EOS_DECLARE_FUNC(void) EOS_LobbySearch_Find(EOS_HLobbySearch Handle, const EOS_LobbySearch_FindOptions* Options, void* ClientData, const EOS_LobbySearch_OnFindCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_Find);
    return _EOS_LobbySearch_Find(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetLobbyId(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetLobbyIdOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_SetLobbyId);
    return _EOS_LobbySearch_SetLobbyId(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetTargetUserId(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetTargetUserIdOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_SetTargetUserId);
    return _EOS_LobbySearch_SetTargetUserId(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetParameter(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetParameterOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_SetParameter);
    return _EOS_LobbySearch_SetParameter(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_RemoveParameter(EOS_HLobbySearch Handle, const EOS_LobbySearch_RemoveParameterOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_RemoveParameter);
    return _EOS_LobbySearch_RemoveParameter(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetMaxResults(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetMaxResultsOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_SetMaxResults);
    return _EOS_LobbySearch_SetMaxResults(Handle, Options);
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbySearch_GetSearchResultCount(EOS_HLobbySearch Handle, const EOS_LobbySearch_GetSearchResultCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_GetSearchResultCount);
    return _EOS_LobbySearch_GetSearchResultCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_CopySearchResultByIndex(EOS_HLobbySearch Handle, const EOS_LobbySearch_CopySearchResultByIndexOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_CopySearchResultByIndex);
    return _EOS_LobbySearch_CopySearchResultByIndex(Handle, Options, OutLobbyDetailsHandle);
}

EOS_DECLARE_FUNC(void) EOS_LobbyModification_Release(EOS_HLobbyModification LobbyModificationHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_Release);
    return _EOS_LobbyModification_Release(LobbyModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Release(EOS_HLobbyDetails LobbyHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_Release);
    return _EOS_LobbyDetails_Release(LobbyHandle);
}

EOS_DECLARE_FUNC(void) EOS_LobbySearch_Release(EOS_HLobbySearch LobbySearchHandle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_Release);
    return _EOS_LobbySearch_Release(LobbySearchHandle);
}

EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Info_Release(EOS_LobbyDetails_Info* LobbyDetailsInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_Info_Release);
    return _EOS_LobbyDetails_Info_Release(LobbyDetailsInfo);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_Attribute_Release(EOS_Lobby_Attribute* LobbyAttribute)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_Attribute_Release);
    return _EOS_Lobby_Attribute_Release(LobbyAttribute);
}

////////////////////////////////////////////
// leaderboards
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardDefinitions(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardDefinitionsOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_QueryLeaderboardDefinitions);
    return _EOS_Leaderboards_QueryLeaderboardDefinitions(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardDefinitionCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardDefinitionCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_GetLeaderboardDefinitionCount);
    return _EOS_Leaderboards_GetLeaderboardDefinitionCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions* Options, EOS_Leaderboards_Definition** OutLeaderboardDefinition)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_CopyLeaderboardDefinitionByIndex);
    auto res = _EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(Handle, Options, OutLeaderboardDefinition);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion      : " << Options->ApiVersion << std::endl;
    sstr << "LeaderboardIndex: " << Options->LeaderboardIndex << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion   : " << (*OutLeaderboardDefinition)->ApiVersion << std::endl;
        switch ((*OutLeaderboardDefinition)->ApiVersion)
        {
            case EOS_LEADERBOARDS_DEFINITION_API_001:
            {
                auto v = reinterpret_cast<EOS_Leaderboards_Definition001*>(*OutLeaderboardDefinition);
                sstr << "  LeaderboardId: " << v->LeaderboardId << std::endl;
                sstr << "  StatName     : " << v->StatName << std::endl;
                sstr << "  Aggregation  : " << v->Aggregation << std::endl;
                sstr << "  StartTime    : " << v->StartTime << std::endl;
                sstr << "  EndTime      : " << v->EndTime << std::endl;
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions* Options, EOS_Leaderboards_Definition** OutLeaderboardDefinition)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId);
    auto res = _EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(Handle, Options, OutLeaderboardDefinition);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion   : " << Options->ApiVersion << std::endl;
    sstr << "LeaderboardId: " << Options->LeaderboardId << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion   : " << (*OutLeaderboardDefinition)->ApiVersion << std::endl;
        switch ((*OutLeaderboardDefinition)->ApiVersion)
        {
            case EOS_LEADERBOARDS_DEFINITION_API_001:
            {
                auto v = reinterpret_cast<EOS_Leaderboards_Definition001*>(*OutLeaderboardDefinition);
                sstr << "  LeaderboardId: " << v->LeaderboardId << std::endl;
                sstr << "  StatName     : " << v->StatName << std::endl;
                sstr << "  Aggregation  : " << v->Aggregation << std::endl;
                sstr << "  StartTime    : " << v->StartTime << std::endl;
                sstr << "  EndTime      : " << v->EndTime << std::endl;
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardRanks(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardRanksOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_QueryLeaderboardRanks);
    return _EOS_Leaderboards_QueryLeaderboardRanks(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardRecordCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardRecordCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_GetLeaderboardRecordCount);
    return _EOS_Leaderboards_GetLeaderboardRecordCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions* Options, EOS_Leaderboards_LeaderboardRecord** OutLeaderboardRecord)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_CopyLeaderboardRecordByIndex);
    auto res = _EOS_Leaderboards_CopyLeaderboardRecordByIndex(Handle, Options, OutLeaderboardRecord);
    
    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion            : " << Options->ApiVersion << std::endl;
    sstr << "LeaderboardRecordIndex: " << Options->LeaderboardRecordIndex << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion     : " << (*OutLeaderboardRecord)->ApiVersion << std::endl;
        switch ((*OutLeaderboardRecord)->ApiVersion)
        {
            case EOS_LEADERBOARDS_LEADERBOARDRECORD_API_002:
            {
                auto v = reinterpret_cast<EOS_Leaderboards_LeaderboardRecord002*>(*OutLeaderboardRecord);
                sstr << "  UserDisplayName: " << v->UserDisplayName << std::endl;
            }
            break;

            case EOS_LEADERBOARDS_LEADERBOARDRECORD_API_001:
            {
                auto v = reinterpret_cast<EOS_Leaderboards_LeaderboardRecord001*>(*OutLeaderboardRecord);
                sstr << "  Rank           : " << v->Rank << std::endl;
                sstr << "  Score          : " << v->Score << std::endl;
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions* Options, EOS_Leaderboards_LeaderboardRecord** OutLeaderboardRecord)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_CopyLeaderboardRecordByUserId);
    auto res = _EOS_Leaderboards_CopyLeaderboardRecordByUserId(Handle, Options, OutLeaderboardRecord);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion            : " << Options->ApiVersion << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion     : " << (*OutLeaderboardRecord)->ApiVersion << std::endl;
        switch ((*OutLeaderboardRecord)->ApiVersion)
        {
            case EOS_LEADERBOARDS_LEADERBOARDRECORD_API_002:
            {
                auto v = reinterpret_cast<EOS_Leaderboards_LeaderboardRecord002*>(*OutLeaderboardRecord);
                sstr << "  UserDisplayName: " << v->UserDisplayName << std::endl;
            }
            break;

            case EOS_LEADERBOARDS_LEADERBOARDRECORD_API_001:
            {
                auto v = reinterpret_cast<EOS_Leaderboards_LeaderboardRecord001*>(*OutLeaderboardRecord);
                sstr << "  Rank           : " << v->Rank << std::endl;
                sstr << "  Score          : " << v->Score << std::endl;
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardUserScores(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardUserScoresOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_QueryLeaderboardUserScores);
    return _EOS_Leaderboards_QueryLeaderboardUserScores(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardUserScoreCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardUserScoreCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_GetLeaderboardUserScoreCount);
    return _EOS_Leaderboards_GetLeaderboardUserScoreCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions* Options, EOS_Leaderboards_LeaderboardUserScore** OutLeaderboardUserScore)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_CopyLeaderboardUserScoreByIndex);
    return _EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(Handle, Options, OutLeaderboardUserScore);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions* Options, EOS_Leaderboards_LeaderboardUserScore** OutLeaderboardUserScore)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_CopyLeaderboardUserScoreByUserId);
    return _EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(Handle, Options, OutLeaderboardUserScore);
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardDefinition_Release(EOS_Leaderboards_Definition* LeaderboardDefinition)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_LeaderboardDefinition_Release);
    return _EOS_Leaderboards_LeaderboardDefinition_Release(LeaderboardDefinition);
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardUserScore_Release(EOS_Leaderboards_LeaderboardUserScore* LeaderboardUserScore)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_LeaderboardUserScore_Release);
    return _EOS_Leaderboards_LeaderboardUserScore_Release(LeaderboardUserScore);
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardRecord_Release(EOS_Leaderboards_LeaderboardRecord* LeaderboardRecord)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_LeaderboardRecord_Release);
    return _EOS_Leaderboards_LeaderboardRecord_Release(LeaderboardRecord);
}

////////////////////////////////////////////
// friends
EOS_DECLARE_FUNC(void) EOS_Friends_QueryFriends(EOS_HFriends Handle, const EOS_Friends_QueryFriendsOptions* Options, void* ClientData, const EOS_Friends_OnQueryFriendsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_QueryFriends);
    return _EOS_Friends_QueryFriends(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Friends_SendInvite(EOS_HFriends Handle, const EOS_Friends_SendInviteOptions* Options, void* ClientData, const EOS_Friends_OnSendInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_SendInvite);
    return _EOS_Friends_SendInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Friends_AcceptInvite(EOS_HFriends Handle, const EOS_Friends_AcceptInviteOptions* Options, void* ClientData, const EOS_Friends_OnAcceptInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_AcceptInvite);
    return _EOS_Friends_AcceptInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Friends_RejectInvite(EOS_HFriends Handle, const EOS_Friends_RejectInviteOptions* Options, void* ClientData, const EOS_Friends_OnRejectInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_RejectInvite);
    return _EOS_Friends_RejectInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetFriendsCount(EOS_HFriends Handle, const EOS_Friends_GetFriendsCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_GetFriendsCount);
    return _EOS_Friends_GetFriendsCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Friends_GetFriendAtIndex(EOS_HFriends Handle, const EOS_Friends_GetFriendAtIndexOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_GetFriendAtIndex);
    return _EOS_Friends_GetFriendAtIndex(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EFriendsStatus) EOS_Friends_GetStatus(EOS_HFriends Handle, const EOS_Friends_GetStatusOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_GetStatus);
    return _EOS_Friends_GetStatus(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Friends_AddNotifyFriendsUpdate(EOS_HFriends Handle, const EOS_Friends_AddNotifyFriendsUpdateOptions* Options, void* ClientData, const EOS_Friends_OnFriendsUpdateCallback FriendsUpdateHandler)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_AddNotifyFriendsUpdate);
    return _EOS_Friends_AddNotifyFriendsUpdate(Handle, Options, ClientData, FriendsUpdateHandler);
}

EOS_DECLARE_FUNC(void) EOS_Friends_RemoveNotifyFriendsUpdate(EOS_HFriends Handle, EOS_NotificationId NotificationId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_RemoveNotifyFriendsUpdate);
    return _EOS_Friends_RemoveNotifyFriendsUpdate(Handle, NotificationId);
}

////////////////////////////////////////////
// ecom
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnership(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_QueryOwnership);
    return _EOS_Ecom_QueryOwnership(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnershipToken(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_QueryOwnershipToken);
    return _EOS_Ecom_QueryOwnershipToken(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlements(EOS_HEcom Handle, const EOS_Ecom_QueryEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_QueryEntitlements);
    return _EOS_Ecom_QueryEntitlements(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOffers(EOS_HEcom Handle, const EOS_Ecom_QueryOffersOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOffersCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_QueryOffers);
    return _EOS_Ecom_QueryOffers(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_Checkout(EOS_HEcom Handle, const EOS_Ecom_CheckoutOptions* Options, void* ClientData, const EOS_Ecom_OnCheckoutCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Checkout);
    return _EOS_Ecom_Checkout(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_RedeemEntitlements(EOS_HEcom Handle, const EOS_Ecom_RedeemEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnRedeemEntitlementsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_RedeemEntitlements);

    if (Options != nullptr)
    {
        for (uint32_t i = 0; i < Options->EntitlementIdCount; ++i)
        {
            LOG(Log::LogLevel::INFO, "EntitlementIds[%i] = %s", i, Options->EntitlementIds[i] == nullptr ? "<no id>" : Options->EntitlementIds[i]);
        }
    }

    return _EOS_Ecom_RedeemEntitlements(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetEntitlementsCount);
    auto res = _EOS_Ecom_GetEntitlementsCount(Handle, Options);
    LOG(Log::LogLevel::INFO, "Entitlements Count: %u", res);
    return res;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsByNameCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsByNameCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetEntitlementsByNameCount);
    auto res = _EOS_Ecom_GetEntitlementsByNameCount(Handle, Options);

    if(Options != nullptr)
        LOG(Log::LogLevel::INFO, "Entitlements Count (%s): %u", Options->EntitlementName == nullptr ? "<no name>": Options->EntitlementName, res);

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyEntitlementByIndex);
    auto res = _EOS_Ecom_CopyEntitlementByIndex(Handle, Options, OutEntitlement);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion      : " << Options->ApiVersion << std::endl;
    sstr << "EntitlementIndex: " << Options->EntitlementIndex << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion     : " << (*OutEntitlement)->ApiVersion << std::endl;
        switch ((*OutEntitlement)->ApiVersion)
        {
            case EOS_ECOM_ENTITLEMENT_API_002:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement002*>(*OutEntitlement);
                sstr << "  EntitlementName: " << v->EntitlementName << std::endl;
                sstr << "  EntitlementId  : " << v->EntitlementId << std::endl;
                sstr << "  CatalogItemId  : " << v->CatalogItemId << std::endl;
                sstr << "  ServerIndex    : " << v->ServerIndex << std::endl;
                sstr << "  bRedeemed      : " << EOS_Bool_2_str(v->bRedeemed) << std::endl;
                sstr << "  EndTimestamp   : " << v->EndTimestamp << std::endl;
            }
            break;

            case EOS_ECOM_ENTITLEMENT_API_001:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement001*>(*OutEntitlement);
                sstr << "  Id             : " << v->Id << std::endl;
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByNameAndIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByNameAndIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyEntitlementByNameAndIndex);
    auto res = _EOS_Ecom_CopyEntitlementByNameAndIndex(Handle, Options, OutEntitlement);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion     : " << Options->ApiVersion << std::endl;
    sstr << "EntitlementName: " << Options->EntitlementName << std::endl;
    sstr << "Index          : " << Options->Index << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion     : " << (*OutEntitlement)->ApiVersion << std::endl;
        switch ((*OutEntitlement)->ApiVersion)
        {
            case EOS_ECOM_ENTITLEMENT_API_002:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement002*>(*OutEntitlement);
                sstr << "  EntitlementName: " << v->EntitlementName << std::endl;
                sstr << "  EntitlementId  : " << v->EntitlementId << std::endl;
                sstr << "  CatalogItemId  : " << v->CatalogItemId << std::endl;
                sstr << "  ServerIndex    : " << v->ServerIndex << std::endl;
                sstr << "  bRedeemed      : " << EOS_Bool_2_str(v->bRedeemed) << std::endl;
                sstr << "  EndTimestamp   : " << v->EndTimestamp << std::endl;
            }
            break;

            case EOS_ECOM_ENTITLEMENT_API_001:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement001*>(*OutEntitlement);
                sstr << "  Id             : " << v->Id << std::endl;
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementById(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIdOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyEntitlementById);
    auto res = _EOS_Ecom_CopyEntitlementById(Handle, Options, OutEntitlement);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion   : " << Options->ApiVersion << std::endl;
    sstr << "EntitlementId: " << Options->EntitlementId << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion     : " << (*OutEntitlement)->ApiVersion << std::endl;
        switch ((*OutEntitlement)->ApiVersion)
        {
            case EOS_ECOM_ENTITLEMENT_API_002:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement002*>(*OutEntitlement);
                sstr << "  EntitlementName: " << v->EntitlementName << std::endl;
                sstr << "  EntitlementId  : " << v->EntitlementId << std::endl;
                sstr << "  CatalogItemId  : " << v->CatalogItemId << std::endl;
                sstr << "  ServerIndex    : " << v->ServerIndex << std::endl;
                sstr << "  bRedeemed      : " << EOS_Bool_2_str(v->bRedeemed) << std::endl;
                sstr << "  EndTimestamp   : " << v->EndTimestamp << std::endl;
            }
            break;

            case EOS_ECOM_ENTITLEMENT_API_001:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement001*>(*OutEntitlement);
                sstr << "  Id             : " << v->Id << std::endl;
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetOfferCount);
    return _EOS_Ecom_GetOfferCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIndexOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyOfferByIndex);
    return _EOS_Ecom_CopyOfferByIndex(Handle, Options, OutOffer);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferById(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIdOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyOfferById);
    return _EOS_Ecom_CopyOfferById(Handle, Options, OutOffer);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferItemCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferItemCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetOfferItemCount);
    return _EOS_Ecom_GetOfferItemCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferItemByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferItemByIndexOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyOfferItemByIndex);
    return _EOS_Ecom_CopyOfferItemByIndex(Handle, Options, OutItem);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemById(EOS_HEcom Handle, const EOS_Ecom_CopyItemByIdOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyItemById);
    return _EOS_Ecom_CopyItemById(Handle, Options, OutItem);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferImageInfoCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetOfferImageInfoCount);
    return _EOS_Ecom_GetOfferImageInfoCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyOfferImageInfoByIndex);
    return _EOS_Ecom_CopyOfferImageInfoByIndex(Handle, Options, OutImageInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetItemImageInfoCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetItemImageInfoCount);
    return _EOS_Ecom_GetItemImageInfoCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyItemImageInfoByIndex);
    return _EOS_Ecom_CopyItemImageInfoByIndex(Handle, Options, OutImageInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemReleaseCount(EOS_HEcom Handle, const EOS_Ecom_GetItemReleaseCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetItemReleaseCount);
    return _EOS_Ecom_GetItemReleaseCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemReleaseByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemReleaseByIndexOptions* Options, EOS_Ecom_CatalogRelease** OutRelease)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyItemReleaseByIndex);
    return _EOS_Ecom_CopyItemReleaseByIndex(Handle, Options, OutRelease);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetTransactionCount(EOS_HEcom Handle, const EOS_Ecom_GetTransactionCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetTransactionCount);
    return _EOS_Ecom_GetTransactionCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIndexOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyTransactionByIndex);
    return _EOS_Ecom_CopyTransactionByIndex(Handle, Options, OutTransaction);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionById(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIdOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyTransactionById);
    return _EOS_Ecom_CopyTransactionById(Handle, Options, OutTransaction);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_GetTransactionId(EOS_Ecom_HTransaction Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Transaction_GetTransactionId);
    return _EOS_Ecom_Transaction_GetTransactionId(Handle, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_Transaction_GetEntitlementsCount(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_GetEntitlementsCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Transaction_GetEntitlementsCount);
    return _EOS_Ecom_Transaction_GetEntitlementsCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_CopyEntitlementByIndex(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Transaction_CopyEntitlementByIndex);
    auto res = _EOS_Ecom_Transaction_CopyEntitlementByIndex(Handle, Options, OutEntitlement);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion      : " << Options->ApiVersion << std::endl;
    sstr << "EntitlementIndex: " << Options->EntitlementIndex << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion     : " << (*OutEntitlement)->ApiVersion << std::endl;
        switch ((*OutEntitlement)->ApiVersion)
        {
            case EOS_ECOM_ENTITLEMENT_API_002:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement002*>(*OutEntitlement);
                sstr << "  EntitlementName: " << v->EntitlementName << std::endl;
                sstr << "  EntitlementId  : " << v->EntitlementId << std::endl;
                sstr << "  CatalogItemId  : " << v->CatalogItemId << std::endl;
                sstr << "  ServerIndex    : " << v->ServerIndex << std::endl;
                sstr << "  bRedeemed      : " << EOS_Bool_2_str(v->bRedeemed) << std::endl;
                sstr << "  EndTimestamp   : " << v->EndTimestamp << std::endl;
            }
            break;

            case EOS_ECOM_ENTITLEMENT_API_001:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement001*>(*OutEntitlement);
                sstr << "  Id             : " << v->Id << std::endl;
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }

    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());
    return res;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_Entitlement_Release(EOS_Ecom_Entitlement* Entitlement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Entitlement_Release);
    return _EOS_Ecom_Entitlement_Release(Entitlement);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogItem_Release(EOS_Ecom_CatalogItem* CatalogItem)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CatalogItem_Release);
    return _EOS_Ecom_CatalogItem_Release(CatalogItem);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogOffer_Release(EOS_Ecom_CatalogOffer* CatalogOffer)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CatalogOffer_Release);
    return _EOS_Ecom_CatalogOffer_Release(CatalogOffer);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_KeyImageInfo_Release(EOS_Ecom_KeyImageInfo* KeyImageInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_KeyImageInfo_Release);
    return _EOS_Ecom_KeyImageInfo_Release(KeyImageInfo);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogRelease_Release(EOS_Ecom_CatalogRelease* CatalogRelease)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CatalogRelease_Release);
    return _EOS_Ecom_CatalogRelease_Release(CatalogRelease);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_Transaction_Release(EOS_Ecom_HTransaction Transaction)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Transaction_Release);
    return _EOS_Ecom_Transaction_Release(Transaction);
}

////////////////////////////////////////////
// connect
EOS_DECLARE_FUNC(void) EOS_Connect_Login(EOS_HConnect Handle, const EOS_Connect_LoginOptions* Options, void* ClientData, const EOS_Connect_OnLoginCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_Login);

    callback_wrapper* wrapper = new callback_wrapper;
    wrapper->ClientData = ClientData;
    wrapper->CbFunc = (callback_t)CompletionDelegate;
    auto f = [](const EOS_Connect_LoginCallbackInfo* cbinfo) {
        callback_wrapper* wrapper = (callback_wrapper*)cbinfo->ClientData;

        const_cast<EOS_Connect_LoginCallbackInfo*>(cbinfo)->ClientData = wrapper->ClientData;
        wrapper->CbFunc((void*)cbinfo);
        delete wrapper;
    };

    return _EOS_Connect_Login(Handle, Options, wrapper, f);
}

EOS_DECLARE_FUNC(void) EOS_Connect_CreateUser(EOS_HConnect Handle, const EOS_Connect_CreateUserOptions* Options, void* ClientData, const EOS_Connect_OnCreateUserCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_CreateUser);
    return _EOS_Connect_CreateUser(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_LinkAccount(EOS_HConnect Handle, const EOS_Connect_LinkAccountOptions* Options, void* ClientData, const EOS_Connect_OnLinkAccountCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_LinkAccount);
    return _EOS_Connect_LinkAccount(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_CreateDeviceId(EOS_HConnect Handle, const EOS_Connect_CreateDeviceIdOptions* Options, void* ClientData, const EOS_Connect_OnCreateDeviceIdCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_CreateDeviceId);
    return _EOS_Connect_CreateDeviceId(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_DeleteDeviceId(EOS_HConnect Handle, const EOS_Connect_DeleteDeviceIdOptions* Options, void* ClientData, const EOS_Connect_OnDeleteDeviceIdCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_DeleteDeviceId);
    return _EOS_Connect_DeleteDeviceId(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_QueryExternalAccountMappings(EOS_HConnect Handle, const EOS_Connect_QueryExternalAccountMappingsOptions* Options, void* ClientData, const EOS_Connect_OnQueryExternalAccountMappingsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_QueryExternalAccountMappings);
    return _EOS_Connect_QueryExternalAccountMappings(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_QueryProductUserIdMappings(EOS_HConnect Handle, const EOS_Connect_QueryProductUserIdMappingsOptions* Options, void* ClientData, const EOS_Connect_OnQueryProductUserIdMappingsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_QueryProductUserIdMappings);
    return _EOS_Connect_QueryProductUserIdMappings(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_Connect_GetExternalAccountMapping(EOS_HConnect Handle, const EOS_Connect_GetExternalAccountMappingsOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_GetExternalAccountMapping);
    return _EOS_Connect_GetExternalAccountMapping(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_GetProductUserIdMapping(EOS_HConnect Handle, const EOS_Connect_GetProductUserIdMappingOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_GetProductUserIdMapping);
    return _EOS_Connect_GetProductUserIdMapping(Handle, Options, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(int32_t) EOS_Connect_GetLoggedInUsersCount(EOS_HConnect Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_GetLoggedInUsersCount);
    return _EOS_Connect_GetLoggedInUsersCount(Handle);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_Connect_GetLoggedInUserByIndex(EOS_HConnect Handle, int32_t Index)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_GetLoggedInUserByIndex);
    return _EOS_Connect_GetLoggedInUserByIndex(Handle, Index);
}

EOS_DECLARE_FUNC(EOS_ELoginStatus) EOS_Connect_GetLoginStatus(EOS_HConnect Handle, EOS_ProductUserId LocalUserId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_GetLoginStatus);
    return _EOS_Connect_GetLoginStatus(Handle, LocalUserId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Connect_AddNotifyAuthExpiration(EOS_HConnect Handle, const EOS_Connect_AddNotifyAuthExpirationOptions* Options, void* ClientData, const EOS_Connect_OnAuthExpirationCallback Notification)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_AddNotifyAuthExpiration);
    return _EOS_Connect_AddNotifyAuthExpiration(Handle, Options, ClientData, Notification);
}

EOS_DECLARE_FUNC(void) EOS_Connect_RemoveNotifyAuthExpiration(EOS_HConnect Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_RemoveNotifyAuthExpiration);
    return _EOS_Connect_RemoveNotifyAuthExpiration(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Connect_AddNotifyLoginStatusChanged(EOS_HConnect Handle, const EOS_Connect_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Connect_OnLoginStatusChangedCallback Notification)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_AddNotifyLoginStatusChanged);
    return _EOS_Connect_AddNotifyLoginStatusChanged(Handle, Options, ClientData, Notification);
}

EOS_DECLARE_FUNC(void) EOS_Connect_RemoveNotifyLoginStatusChanged(EOS_HConnect Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_RemoveNotifyLoginStatusChanged);
    return _EOS_Connect_RemoveNotifyLoginStatusChanged(Handle, InId);
}

////////////////////////////////////////////
// auth
EOS_DECLARE_FUNC(void) EOS_Auth_Login(EOS_HAuth Handle, const EOS_Auth_LoginOptions* Options, void* ClientData, const EOS_Auth_OnLoginCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_Login);

    callback_wrapper* wrapper = new callback_wrapper;
    wrapper->ClientData = ClientData;
    wrapper->CbFunc = (callback_t)CompletionDelegate;

    auto f = [](const EOS_Auth_LoginCallbackInfo* cbinfo) {
        callback_wrapper* wrapper = (callback_wrapper*)cbinfo->ClientData;
        char buff[1024];
        int32_t size;

        size = sizeof(buff);
        memset(buff, 0, size);
        EOS_Platform_GetActiveCountryCode(hPlatform, cbinfo->LocalUserId, buff, &size);
        LOG(Log::LogLevel::DEBUG, "Active Country Code: %s", buff);

        size = sizeof(buff);
        memset(buff, 0, size);
        EOS_Platform_GetActiveLocaleCode(hPlatform, cbinfo->LocalUserId, buff, &size);
        LOG(Log::LogLevel::DEBUG, "Active Locale Code: %s", buff);

        size = sizeof(buff);
        memset(buff, 0, size);
        EOS_Platform_GetOverrideCountryCode(hPlatform, buff, &size);
        LOG(Log::LogLevel::DEBUG, "Override Country Code: %s", buff);

        size = sizeof(buff);
        memset(buff, 0, size);
        EOS_Platform_GetOverrideLocaleCode(hPlatform, buff, &size);
        LOG(Log::LogLevel::DEBUG, "Override Locale Code: %s", buff);

        const_cast<EOS_Auth_LoginCallbackInfo*>(cbinfo)->ClientData = wrapper->ClientData;
        wrapper->CbFunc((void*)cbinfo);
        delete wrapper;
    };

    return _EOS_Auth_Login(Handle, Options, wrapper, f);
}

EOS_DECLARE_FUNC(void) EOS_Auth_Logout(EOS_HAuth Handle, const EOS_Auth_LogoutOptions* Options, void* ClientData, const EOS_Auth_OnLogoutCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_Logout);
    return _EOS_Auth_Logout(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Auth_DeletePersistentAuth(EOS_HAuth Handle, const EOS_Auth_DeletePersistentAuthOptions* Options, void* ClientData, const EOS_Auth_OnDeletePersistentAuthCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_DeletePersistentAuth);
    return _EOS_Auth_DeletePersistentAuth(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Auth_VerifyUserAuth(EOS_HAuth Handle, const EOS_Auth_VerifyUserAuthOptions* Options, void* ClientData, const EOS_Auth_OnVerifyUserAuthCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_VerifyUserAuth);
    return _EOS_Auth_VerifyUserAuth(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(int32_t) EOS_Auth_GetLoggedInAccountsCount(EOS_HAuth Handle)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_GetLoggedInAccountsCount);
    return _EOS_Auth_GetLoggedInAccountsCount(Handle);
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Auth_GetLoggedInAccountByIndex(EOS_HAuth Handle, int32_t Index)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_GetLoggedInAccountByIndex);
    return _EOS_Auth_GetLoggedInAccountByIndex(Handle, Index);
}

EOS_DECLARE_FUNC(EOS_ELoginStatus) EOS_Auth_GetLoginStatus(EOS_HAuth Handle, EOS_EpicAccountId LocalUserId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_GetLoginStatus);
    return _EOS_Auth_GetLoginStatus(Handle, LocalUserId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenOld(EOS_HAuth Handle, EOS_AccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)
{
    LOG(Log::LogLevel::TRACE, "");
    static decltype(EOS_Auth_CopyUserAuthTokenOld)* _EOS_Auth_CopyUserAuthToken = (decltype(_EOS_Auth_CopyUserAuthToken))GetProcAddress(original_dll, "EOS_Auth_CopyUserAuthToken");
    return _EOS_Auth_CopyUserAuthToken(Handle, LocalUserId, OutUserAuthToken);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenNew(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)
{
    LOG(Log::LogLevel::TRACE, "");
    static decltype(EOS_Auth_CopyUserAuthTokenNew)* _EOS_Auth_CopyUserAuthToken = (decltype(_EOS_Auth_CopyUserAuthToken))GetProcAddress(original_dll, "EOS_Auth_CopyUserAuthToken");
    return _EOS_Auth_CopyUserAuthToken(Handle, Options, LocalUserId, OutUserAuthToken);
}

#ifdef _MSC_VER
#pragma optimize("", off)
#endif

#if defined(__WINDOWS_32__)
#pragma comment(linker, "/export:_EOS_Auth_CopyUserAuthToken@16=_EOS_Auth_CopyUserAuthTokenNew@16")
#pragma comment(linker, "/export:_EOS_Auth_CopyUserAuthToken@12=_EOS_Auth_CopyUserAuthTokenOld@12")
#else
EOS_DECLARE_FUNC(EOS_EResult) CLANG_GCC_DONT_OPTIMIZE EOS_Auth_CopyUserAuthToken(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)

{
    // Build rewrittable opcodes, need 14 for x64 absolute jmp and 5 for x86 relative jmp
    EOS_Auth_CopyUserAuthTokenOld(nullptr, nullptr, nullptr);
    EOS_Auth_CopyUserAuthTokenOld(nullptr, nullptr, nullptr);
    EOS_Auth_CopyUserAuthTokenOld(nullptr, nullptr, nullptr);
    EOS_Auth_CopyUserAuthTokenOld(nullptr, nullptr, nullptr);
    return EOS_EResult::EOS_NotImplemented;
}
#endif
#ifdef _MSC_VER
#pragma optimize("", on)
#endif

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedOld(EOS_HAuth Handle, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    LOG(Log::LogLevel::TRACE, "");
    static decltype(EOS_Auth_AddNotifyLoginStatusChangedOld)* _EOS_Auth_AddNotifyLoginStatusChanged = (decltype(_EOS_Auth_AddNotifyLoginStatusChanged))GetProcAddress(original_dll, "EOS_Auth_AddNotifyLoginStatusChanged");
    return _EOS_Auth_AddNotifyLoginStatusChanged(Handle, ClientData, Notification);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedNew(EOS_HAuth Handle, const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    LOG(Log::LogLevel::TRACE, "");
    static decltype(EOS_Auth_AddNotifyLoginStatusChangedNew)* _EOS_Auth_AddNotifyLoginStatusChanged = (decltype(_EOS_Auth_AddNotifyLoginStatusChanged))GetProcAddress(original_dll, "EOS_Auth_AddNotifyLoginStatusChanged");
    return _EOS_Auth_AddNotifyLoginStatusChanged(Handle, Options, ClientData, Notification);
}

#ifdef _MSC_VER
#pragma optimize("", off)
#endif
#if defined(__WINDOWS_32__)
#pragma comment(linker, "/export:_EOS_Auth_AddNotifyLoginStatusChanged@16=_EOS_Auth_AddNotifyLoginStatusChangedNew@16")
#pragma comment(linker, "/export:_EOS_Auth_AddNotifyLoginStatusChanged@12=_EOS_Auth_AddNotifyLoginStatusChangedOld@12")
#else
EOS_DECLARE_FUNC(EOS_NotificationId) CLANG_GCC_DONT_OPTIMIZE EOS_Auth_AddNotifyLoginStatusChanged(EOS_HAuth Handle, const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    // Build rewrittable opcodes, need 14 for x64 absolute jmp and 5 for x86 relative jmp
    EOS_Auth_AddNotifyLoginStatusChangedOld(nullptr, nullptr, nullptr);
    EOS_Auth_AddNotifyLoginStatusChangedOld(nullptr, nullptr, nullptr);
    EOS_Auth_AddNotifyLoginStatusChangedOld(nullptr, nullptr, nullptr);
    EOS_Auth_AddNotifyLoginStatusChangedOld(nullptr, nullptr, nullptr);
    return EOS_INVALID_NOTIFICATIONID;
}
#endif
#ifdef _MSC_VER
#pragma optimize("", on)
#endif

EOS_DECLARE_FUNC(void) EOS_Auth_RemoveNotifyLoginStatusChanged(EOS_HAuth Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_RemoveNotifyLoginStatusChanged);
    return _EOS_Auth_RemoveNotifyLoginStatusChanged(Handle, InId);
}


EOS_DECLARE_FUNC(void) EOS_Auth_Token_Release(EOS_Auth_Token* AuthToken)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_Token_Release);
    return _EOS_Auth_Token_Release(AuthToken);
}

////////////////////////////////////////////
// achievements
EOS_DECLARE_FUNC(void) EOS_Achievements_QueryDefinitions(EOS_HAchievements Handle, const EOS_Achievements_QueryDefinitionsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryDefinitionsCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_QueryDefinitions);
    return _EOS_Achievements_QueryDefinitions(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetAchievementDefinitionCount(EOS_HAchievements Handle, const EOS_Achievements_GetAchievementDefinitionCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_GetAchievementDefinitionCount);
    return _EOS_Achievements_GetAchievementDefinitionCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionByIndexOptions* Options, EOS_Achievements_Definition** OutDefinition)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyAchievementDefinitionByIndex);
    auto res = _EOS_Achievements_CopyAchievementDefinitionByIndex(Handle, Options, OutDefinition);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion           : " << Options->ApiVersion << std::endl;
    sstr << "AchievementIndex     : " << Options->AchievementIndex << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion           : " << (*OutDefinition)->ApiVersion << std::endl;
        switch ((*OutDefinition)->ApiVersion)
        {
            case EOS_ACHIEVEMENTS_COPYDEFINITIONBYINDEX_API_001:
            {
                auto* v = reinterpret_cast<EOS_Achievements_Definition001*>(*OutDefinition);
                sstr << "  AchievementId        : " << (*OutDefinition)->AchievementId << std::endl;
                sstr << "  DisplayName          : " << (*OutDefinition)->DisplayName << std::endl;
                sstr << "  Description          : " << (*OutDefinition)->Description << std::endl;
                sstr << "  LockedDisplayName    : " << (*OutDefinition)->LockedDisplayName << std::endl;
                sstr << "  LockedDescription    : " << (*OutDefinition)->LockedDescription << std::endl;
                sstr << "  HiddenDescription    : " << (*OutDefinition)->HiddenDescription << std::endl;
                sstr << "  CompletionDescription: " << (*OutDefinition)->CompletionDescription << std::endl;
                sstr << "  UnlockedIconId       : " << (*OutDefinition)->UnlockedIconId << std::endl;
                sstr << "  LockedIconId         : " << (*OutDefinition)->LockedIconId << std::endl;
                sstr << "  bIsHidden            : " << ((*OutDefinition)->bIsHidden == EOS_TRUE ? "TRUE" : "FALSE") << std::endl;

                for (int i = 0; i < (*OutDefinition)->StatThresholdsCount; ++i)
                {
                    sstr << "  Threshold[" << i << "]" << std::endl;
                    sstr << "    StatThresholds::Name     : " << (*OutDefinition)->StatThresholds[i].Name << std::endl;
                    sstr << "    StatThresholds::Threshold: " << (*OutDefinition)->StatThresholds[i].Threshold << std::endl;
                }
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionV2ByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions* Options, EOS_Achievements_DefinitionV2** OutDefinition)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyAchievementDefinitionV2ByIndex);
    auto res = _EOS_Achievements_CopyAchievementDefinitionV2ByIndex(Handle, Options, OutDefinition);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion           : " << Options->ApiVersion << std::endl;
    sstr << "AchievementIndex     : " << Options->AchievementIndex << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion           : " << (*OutDefinition)->ApiVersion << std::endl;
        switch ((*OutDefinition)->ApiVersion)
        {
            case EOS_ACHIEVEMENTS_COPYDEFINITIONV2BYINDEX_API_002:
            {
                auto* v = reinterpret_cast<EOS_Achievements_DefinitionV2002*>(*OutDefinition);
                sstr << "  AchievementId        : " << v->AchievementId << std::endl;
                sstr << "  UnlockedDisplayName  : " << v->UnlockedDisplayName << std::endl;
                sstr << "  UnlockedDescription  : " << v->UnlockedDescription << std::endl;
                sstr << "  LockedDisplayName    : " << v->LockedDisplayName << std::endl;
                sstr << "  LockedDescription    : " << v->LockedDescription << std::endl;
                sstr << "  FlavorText           : " << v->FlavorText << std::endl;
                sstr << "  UnlockedIconURL      : " << v->UnlockedIconURL << std::endl;
                sstr << "  LockedIconURL        : " << v->LockedIconURL << std::endl;
                sstr << "  bIsHidden            : " << EOS_Bool_2_str(v->bIsHidden) << std::endl;

                for (int i = 0; i < v->StatThresholdsCount; ++i)
                {
                    sstr << "  Threshold[" << i << "]" << std::endl;
                    sstr << "    Name     : " << v->StatThresholds[i].Name << std::endl;
                    sstr << "    Threshold: " << v->StatThresholds[i].Threshold << std::endl;
                }
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionByAchievementIdOptions* Options, EOS_Achievements_Definition** OutDefinition)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyAchievementDefinitionByAchievementId);

    auto res = _EOS_Achievements_CopyAchievementDefinitionByAchievementId(Handle, Options, OutDefinition);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion           : " << Options->ApiVersion << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion           : " << (*OutDefinition)->ApiVersion << std::endl;
        switch ((*OutDefinition)->ApiVersion)
        {
            case EOS_ACHIEVEMENTS_COPYDEFINITIONBYINDEX_API_001:
            {
                auto* v = reinterpret_cast<EOS_Achievements_Definition001*>(*OutDefinition);
                sstr << "  AchievementId        : " << v->AchievementId << std::endl;
                sstr << "  DisplayName          : " << v->DisplayName << std::endl;
                sstr << "  Description          : " << v->Description << std::endl;
                sstr << "  LockedDisplayName    : " << v->LockedDisplayName << std::endl;
                sstr << "  LockedDescription    : " << v->LockedDescription << std::endl;
                sstr << "  HiddenDescription    : " << v->HiddenDescription << std::endl;
                sstr << "  CompletionDescription: " << v->CompletionDescription << std::endl;
                sstr << "  UnlockedIconId       : " << v->UnlockedIconId << std::endl;
                sstr << "  LockedIconId         : " << v->LockedIconId << std::endl;
                sstr << "  bIsHidden            : " << EOS_Bool_2_str(v->bIsHidden) << std::endl;

                for (int i = 0; i < v->StatThresholdsCount; ++i)
                {
                    sstr << "  Threshold[" << i << "]" << std::endl;
                    sstr << "    Name     : " << v->StatThresholds[i].Name << std::endl;
                    sstr << "    Threshold: " << v->StatThresholds[i].Threshold << std::endl;
                }
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionV2ByAchievementIdOptions* Options, EOS_Achievements_DefinitionV2** OutDefinition)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId);
    auto res = _EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId(Handle, Options, OutDefinition);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion           : " << Options->ApiVersion << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion           : " << (*OutDefinition)->ApiVersion << std::endl;
        switch ((*OutDefinition)->ApiVersion)
        {
            case EOS_ACHIEVEMENTS_COPYDEFINITIONV2BYACHIEVEMENTID_API_002:
            {
                auto* v = reinterpret_cast<EOS_Achievements_DefinitionV2002*>(*OutDefinition);
                sstr << "  AchievementId        : " << v->AchievementId << std::endl;
                sstr << "  UnlockedDisplayName  : " << v->UnlockedDisplayName << std::endl;
                sstr << "  UnlockedDescription  : " << v->UnlockedDescription << std::endl;
                sstr << "  LockedDisplayName    : " << v->LockedDisplayName << std::endl;
                sstr << "  LockedDescription    : " << v->LockedDescription << std::endl;
                sstr << "  FlavorText           : " << v->FlavorText << std::endl;
                sstr << "  UnlockedIconURL      : " << v->UnlockedIconURL << std::endl;
                sstr << "  LockedIconURL        : " << v->LockedIconURL << std::endl;
                sstr << "  bIsHidden            : " << EOS_Bool_2_str(v->bIsHidden) << std::endl;

                for (int i = 0; i < v->StatThresholdsCount; ++i)
                {
                    sstr << "  Threshold[" << i << "]" << std::endl;
                    sstr << "    Name     : " << v->StatThresholds[i].Name << std::endl;
                    sstr << "    Threshold: " << v->StatThresholds[i].Threshold << std::endl;
                }
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(void) EOS_Achievements_QueryPlayerAchievements(EOS_HAchievements Handle, const EOS_Achievements_QueryPlayerAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_QueryPlayerAchievements);
    return _EOS_Achievements_QueryPlayerAchievements(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetPlayerAchievementCount(EOS_HAchievements Handle, const EOS_Achievements_GetPlayerAchievementCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_GetPlayerAchievementCount);
    return _EOS_Achievements_GetPlayerAchievementCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyPlayerAchievementByIndexOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyPlayerAchievementByIndex);
    return _EOS_Achievements_CopyPlayerAchievementByIndex(Handle, Options, OutAchievement);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyPlayerAchievementByAchievementId);
    return _EOS_Achievements_CopyPlayerAchievementByAchievementId(Handle, Options, OutAchievement);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_UnlockAchievements(EOS_HAchievements Handle, const EOS_Achievements_UnlockAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnUnlockAchievementsCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_UnlockAchievements);
    return _EOS_Achievements_UnlockAchievements(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetUnlockedAchievementCount(EOS_HAchievements Handle, const EOS_Achievements_GetUnlockedAchievementCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_GetUnlockedAchievementCount);
    return _EOS_Achievements_GetUnlockedAchievementCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyUnlockedAchievementByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyUnlockedAchievementByIndexOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyUnlockedAchievementByIndex);
    auto res = _EOS_Achievements_CopyUnlockedAchievementByIndex(Handle, Options, OutAchievement);

    if (res == EOS_EResult::EOS_Success)
    {
        LOG(Log::LogLevel::DEBUG, "Achievement index: %d = %s %ll", Options->AchievementIndex, (*OutAchievement)->AchievementId, (*OutAchievement)->UnlockTime);
    }

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyUnlockedAchievementByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyUnlockedAchievementByAchievementIdOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyUnlockedAchievementByAchievementId);
    auto res = _EOS_Achievements_CopyUnlockedAchievementByAchievementId(Handle, Options, OutAchievement);

    if (res == EOS_EResult::EOS_Success)
    {
        LOG(Log::LogLevel::DEBUG, "Achievement id: %s = %ll", (*OutAchievement)->AchievementId, (*OutAchievement)->UnlockTime);
    }

    return res;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlocked(EOS_HAchievements Handle, const EOS_Achievements_AddNotifyAchievementsUnlockedOptions* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_AddNotifyAchievementsUnlocked);
    return _EOS_Achievements_AddNotifyAchievementsUnlocked(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlockedV2(EOS_HAchievements Handle, const EOS_Achievements_AddNotifyAchievementsUnlockedV2Options* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallbackV2 NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_AddNotifyAchievementsUnlockedV2);
    return _EOS_Achievements_AddNotifyAchievementsUnlockedV2(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_RemoveNotifyAchievementsUnlocked(EOS_HAchievements Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_RemoveNotifyAchievementsUnlocked);
    return _EOS_Achievements_RemoveNotifyAchievementsUnlocked(Handle, InId);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_Definition_Release(EOS_Achievements_Definition* AchievementDefinition)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_Definition_Release);
    return _EOS_Achievements_Definition_Release(AchievementDefinition);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_DefinitionV2_Release(EOS_Achievements_DefinitionV2* AchievementDefinition)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_DefinitionV2_Release);
    return _EOS_Achievements_DefinitionV2_Release(AchievementDefinition);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_PlayerAchievement_Release(EOS_Achievements_PlayerAchievement* Achievement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_PlayerAchievement_Release);
    return _EOS_Achievements_PlayerAchievement_Release(Achievement);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_UnlockedAchievement_Release(EOS_Achievements_UnlockedAchievement* Achievement)
{
    LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_UnlockedAchievement_Release);
    return _EOS_Achievements_UnlockedAchievement_Release(Achievement);
}