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
#include <thread>
#include <mutex>

#define EOS_BUILD_DLL 1

#include <eos_common.h>
#include <eos_sdk.h>
#include <eos_auth.h>
#include <eos_logging.h>
#include <eos_presence.h>
#include <eos_sessions.h>
#include <eos_achievements.h>
#include <eos_leaderboards.h>

#include <Windows.h>

#pragma comment(lib, "user32")

#include "Log.h"

#include "nlohmann/fifo_map.hpp"
#include "nlohmann/json.hpp"
#include "mini_detour/mini_detour.h"
#include "utils.h"

#include "utfcpp/utf8.h"

#define CURL_STATICLIB
#include <curl/curl.h>

static HMODULE original_dll = nullptr;

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR '\\'

static bool create_folder(std::string const& _folder)
{
    size_t pos = 0;
    struct _stat sb;

    std::wstring sub_dir;
    std::wstring folder;
    utf8::utf8to16(_folder.begin(), _folder.end(), std::back_inserter(folder));
    if (folder.empty())
        return true;

    if (folder.length() >= 3 && folder[1] == ':' && (folder[2] == '\\' || folder[2] == '/'))
        pos = 3;

    do
    {
        pos = folder.find_first_of(L"\\/", pos + 1);
        sub_dir = std::move(folder.substr(0, pos));
        if (_wstat(sub_dir.c_str(), &sb) == 0)
        {
            if (!(sb.st_mode & _S_IFDIR))
            {// A subpath in the target is not a folder
                return false;
            }
            // Folder exists
        }
        else if (CreateDirectoryW(folder.substr(0, pos).c_str(), NULL))
        {// Failed to create folder (no permission?)
}
    } while (pos != std::string::npos);

    return true;
}

#elif defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PATH_SEPARATOR '/'

static bool create_folder(std::string const& _folder)
{
    size_t pos = 0;
    struct stat sb;

    std::string sub_dir;
    std::string folder = _folder;

    do
    {
        pos = folder.find_first_of("\\/", pos + 1);
        sub_dir = std::move(folder.substr(0, pos));
        if (stat(sub_dir.c_str(), &sb) == 0)
        {
            if (!S_ISDIR(sb.st_mode))
            {// A subpath in the target is not a folder
                return false;
            }
            // Folder exists
        }
        else if (mkdir(sub_dir.c_str(), 0755) < 0)
        {// Failed to create folder (no permission?)
        }
    } while (pos != std::string::npos);

    return true;
}

#endif

#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__)
static const char* original_dll_name = "EOSSDK-Win64-Shipping.original.dll";

#define GET_PROC_ADDRESS(hModule, procName) GetProcAddress(hModule, procName)
#define ORIGINAL_FUNCTION(NAME) static decltype(NAME)* _##NAME = (decltype(_##NAME))GET_PROC_ADDRESS(original_dll, #NAME)

#define load_symbols(...)

#elif defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)

static const char* original_dll_name = "EOSSDK-Win32-Shipping.original.dll";

#if !defined(FIELD_OFFSET)
	#define FIELD_OFFSET(type, field) ( (long)(long*) &( ((type*)0)->field) )
#endif

static std::map<std::string, void*> original_exported_funcs;
static std::string original_exported_dll_name;

// Minimalistic 32bits library exported functions loader
void load_symbols(uint8_t* mem_addr)
{
    uint8_t* pAddr = (uint8_t*)mem_addr;

    original_exported_funcs.clear();

    IMAGE_DOS_HEADER   dos_header;
    IMAGE_NT_HEADERS32 nt_header;

    memcpy(&dos_header, pAddr, sizeof(dos_header));
    pAddr += dos_header.e_lfanew;
    memcpy(&nt_header, pAddr, sizeof(nt_header));

    // ----- Read the exported symbols (if any) ----- //
    if (nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress &&
        nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)
    {
        IMAGE_EXPORT_DIRECTORY exportDir;
        int rva = nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        unsigned short funcOrdinal;
        // Goto export directory structure
        pAddr = mem_addr + rva;
        memcpy(&exportDir, pAddr, sizeof(IMAGE_EXPORT_DIRECTORY));
        // Goto DLL name
        pAddr = mem_addr + exportDir.Name;// +offset;
        original_exported_dll_name = (const char*)pAddr;

        // Read exported functions that have a name
        for (unsigned int i = 0; i < exportDir.NumberOfNames; ++i)
        {
            // Goto the funcOrdinals offset
            pAddr = mem_addr + exportDir.AddressOfNameOrdinals + sizeof(unsigned short) * i;
            // Read the current func ordinal
            memcpy(&funcOrdinal, pAddr, sizeof(unsigned short));
            // Goto the nameAddr rva
            pAddr = mem_addr + exportDir.AddressOfNames + sizeof(unsigned int) * i;
            auto& addr = original_exported_funcs[(const char*)(mem_addr + *(int32_t*)pAddr)];
            // Goto the funcAddr rva
            pAddr = mem_addr + exportDir.AddressOfFunctions + sizeof(unsigned int) * i;
            addr = (void*)(mem_addr + *(int32_t*)pAddr);
        }
    }
}

// Get the proc address from its name and not its exported name
// __stdcall convention adds an '_' as prefix and "@<parameter size>" as suffix
// __fastcall convention adds an '@' as prefix and "@<parameter size>" as suffix
void* get_proc_address(HMODULE hModule, LPCSTR procName)
{
    size_t proc_len = strlen(procName);
    for (auto& proc : original_exported_funcs)
    {
        auto pos = proc.first.find(procName);
        if (pos == 0 ||                                                              // __cdecl
            (pos == 1 && proc.first[0] == '_' && proc.first[proc_len + 1] == '@') || // __stdcall
            (pos == 1 && proc.first[0] == '@' && proc.first[proc_len + 1] == '@')    // __fastcall
            )
        {
            return proc.second;
        }
    }

    return nullptr;
}

#define GET_PROC_ADDRESS(hModule, procName) get_proc_address(hModule, procName)
#define ORIGINAL_FUNCTION(NAME) static decltype(NAME)* _##NAME = (decltype(_##NAME))GET_PROC_ADDRESS(original_dll, #NAME)

#endif
static std::string exe_path;

constexpr static char leaderboards_db_file[] = "leaderboards_db.json";
constexpr static char achievements_db_file[] = "achievements_db.json";
constexpr static char achievements_file[] = "achievements.json";
constexpr static char entitlements_file[] = "entitlements.json";
constexpr static char catalog_file[] = "catalog.json";

static nlohmann::json leaderboards_db;
static nlohmann::json achievements_db;
static nlohmann::json achievements;
static nlohmann::json entitlements;
static nlohmann::json catalog;

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

            load_symbols((uint8_t*)original_dll);

            load_json(achievements_file, achievements);
            load_json(entitlements_file, entitlements);
            load_json(catalog_file, catalog);

            Log::set_loglevel(Log::LogLevel::TRACE);

            EPIC_LOG(Log::LogLevel::INFO, "Original dll load (%s): %p", original_dll_name, original_dll);
            EPIC_LOG(Log::LogLevel::INFO, "command line: %s", GetCommandLine());

            curl_global_init(CURL_GLOBAL_ALL);
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

// Since V1.0.0
static EOS_HPlatform          hPlatform = nullptr;
static EOS_HAuth              hAuth = nullptr;
static EOS_HFriends           hFriends = nullptr;
static EOS_HMetrics           hMetrics = nullptr;
static EOS_HPresence          hPresence = nullptr;
static EOS_HUserInfo          hUserInfo = nullptr;
// Since V1.1.0
static EOS_HEcom              hEcom = nullptr;
// Since V1.2.0
static EOS_HConnect           hConnect = nullptr;
static EOS_HP2P               hP2P = nullptr;
static EOS_HSessions          hSessions = nullptr;
// Since V1.3.0
static EOS_HAchievements      hAchievements = nullptr;
static EOS_HPlayerDataStorage hPlayerDataStorage = nullptr;
static EOS_HStats             hStats = nullptr;
// Since V1.4.0 
// ??
// Since V1.5.0
static EOS_HLeaderboards      hLeaderboards = nullptr;
static EOS_HLobby             hLobby = nullptr;
static EOS_HUI                hUI = nullptr;
// Since V1.8.0
static EOS_HTitleStorage      hTitleStorage = nullptr;

static EOS_EpicAccountId get_epic_account_id(int32_t index = 0)
{
    static EOS_EpicAccountId result = EOS_Auth_GetLoggedInAccountByIndex(hAuth, index);
    return result;
}

static EOS_ProductUserId get_product_user_id(int32_t index = 0)
{
    static EOS_ProductUserId result = EOS_Connect_GetLoggedInUserByIndex(hConnect, index);
    return result;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_AccountId_IsValid(EOS_AccountId AccountId);
EOS_DECLARE_FUNC(EOS_EResult) EOS_AccountId_ToString(EOS_AccountId AccountId, char* OutBuffer, int32_t* InOutBufferLength);
EOS_DECLARE_FUNC(EOS_AccountId) EOS_AccountId_FromString(const char* AccountIdString);

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenOld(EOS_HAuth Handle, EOS_AccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken);
EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenNew(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken);
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedOld(EOS_HAuth Handle, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification);
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedNew(EOS_HAuth Handle, const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification);

static void download_icon(std::string const& url, std::string filename)
{
    FILE* file_handle;
    CURL* curl_handle = curl_easy_init();
    int err;

    size_t pos = filename.find_last_of("/\\");
    if (pos != std::string::npos)
    {
        create_folder(filename.substr(0, pos));
    }
    
    {
        FILE* f;
        err = fopen_s(&f, filename.c_str(), "r");
        if (!err && f != nullptr)
        {
            fclose(f);
            return;
        }
    }

    EPIC_LOG(Log::LogLevel::INFO, "Downloading %s", filename.c_str());
    err = fopen_s(&file_handle, filename.c_str(), "wb");
    if (!err && file_handle != nullptr)
    {
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file_handle);
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_perform(curl_handle);
        fclose(file_handle);
    }
    else
    {
        EPIC_LOG(Log::LogLevel::ERR, "Failed to open file for writing: %s", filename.c_str());
    }

    curl_easy_cleanup(curl_handle);
}

static std::mutex achievements_mutex;

static void dump_achievements_definitionv1()
{
    EOS_Achievements_GetAchievementDefinitionCountOptions Options;
    Options.ApiVersion = EOS_ACHIEVEMENTS_GETACHIEVEMENTDEFINITIONCOUNT_API_LATEST;
    uint32_t count = EOS_Achievements_GetAchievementDefinitionCount(hAchievements, &Options);

    for (uint32_t i = 0; i < count; ++i)
    {
        EOS_Achievements_CopyAchievementDefinitionByIndexOptions DefinitionOptions;
        DefinitionOptions.ApiVersion = EOS_ACHIEVEMENTS_COPYDEFINITIONBYINDEX_API_LATEST;
        DefinitionOptions.AchievementIndex = i;

        EOS_Achievements_Definition* OutDefinition;
        if (EOS_Achievements_CopyAchievementDefinitionByIndex(hAchievements, &DefinitionOptions, &OutDefinition) == EOS_EResult::EOS_Success)
        {
            std::string url("achievements_images/" + std::string(OutDefinition->AchievementId) + ".jpg");
            std::string url_locked("achievements_images/" + std::string(OutDefinition->AchievementId) + "_locked.jpg");

            achievements_db[OutDefinition->AchievementId]["achievement_id"] = str_or_empty(OutDefinition->AchievementId);
            achievements_db[OutDefinition->AchievementId]["unlocked_display_name"] = str_or_empty(OutDefinition->DisplayName);
            achievements_db[OutDefinition->AchievementId]["unlocked_description"] = str_or_empty(OutDefinition->Description);
            achievements_db[OutDefinition->AchievementId]["locked_display_name"] = str_or_empty(OutDefinition->LockedDisplayName);
            achievements_db[OutDefinition->AchievementId]["locked_description"] = str_or_empty(OutDefinition->LockedDescription);
            achievements_db[OutDefinition->AchievementId]["hidden_description"] = str_or_empty(OutDefinition->HiddenDescription);
            achievements_db[OutDefinition->AchievementId]["flavor_text"] = str_or_empty("");
            achievements_db[OutDefinition->AchievementId]["completion_description"] = str_or_empty(OutDefinition->CompletionDescription);
            achievements_db[OutDefinition->AchievementId]["unlocked_icon_url"] = str_or_empty(url.c_str());
            achievements_db[OutDefinition->AchievementId]["locked_icon_url"] = str_or_empty(url_locked.c_str());
            achievements_db[OutDefinition->AchievementId]["is_hidden"] = (bool)OutDefinition->bIsHidden;

            //download_icon(OutDefinition->UnlockedIconURL, url);
            //download_icon(OutDefinition->LockedIconURL, url_locked);

            for (int i = 0; i < OutDefinition->StatThresholdsCount; ++i)
            {
                achievements_db[OutDefinition->AchievementId]["stats_thresholds"][OutDefinition->StatThresholds[i].Name]["name"] = str_or_empty(OutDefinition->StatThresholds[i].Name);
                achievements_db[OutDefinition->AchievementId]["stats_thresholds"][OutDefinition->StatThresholds[i].Name]["threshold"] = OutDefinition->StatThresholds[i].Threshold;
            }

            EOS_Achievements_Definition_Release(OutDefinition);
        }
    }

    save_json(achievements_db_file, achievements_db);
}

static void dump_achievements_definitionv2()
{
    EOS_Achievements_GetAchievementDefinitionCountOptions Options;
    Options.ApiVersion = EOS_ACHIEVEMENTS_GETACHIEVEMENTDEFINITIONCOUNT_API_LATEST;
    uint32_t count = EOS_Achievements_GetAchievementDefinitionCount(hAchievements, &Options);

    for (uint32_t i = 0; i < count; ++i)
    {
        EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions DefinitionOptions;
        DefinitionOptions.ApiVersion = EOS_ACHIEVEMENTS_COPYDEFINITIONV2BYINDEX_API_LATEST;
        DefinitionOptions.AchievementIndex = i;

        EOS_Achievements_DefinitionV2* OutDefinition;
        if (EOS_Achievements_CopyAchievementDefinitionV2ByIndex(hAchievements, &DefinitionOptions, &OutDefinition) == EOS_EResult::EOS_Success)
        {
            std::string url("achievements_images/" + std::string(OutDefinition->AchievementId) + ".jpg");
            std::string url_locked("achievements_images/" + std::string(OutDefinition->AchievementId) + "_locked.jpg");

            achievements_db[OutDefinition->AchievementId]["achievement_id"] = str_or_empty(OutDefinition->AchievementId);
            achievements_db[OutDefinition->AchievementId]["unlocked_display_name"] = str_or_empty(OutDefinition->UnlockedDisplayName);
            achievements_db[OutDefinition->AchievementId]["unlocked_description"] = str_or_empty(OutDefinition->UnlockedDescription);
            achievements_db[OutDefinition->AchievementId]["locked_display_name"] = str_or_empty(OutDefinition->LockedDisplayName);
            achievements_db[OutDefinition->AchievementId]["locked_description"] = str_or_empty(OutDefinition->LockedDescription);
            achievements_db[OutDefinition->AchievementId]["hidden_description"] = str_or_empty("");
            achievements_db[OutDefinition->AchievementId]["flavor_text"] = str_or_empty(OutDefinition->FlavorText);
            achievements_db[OutDefinition->AchievementId]["completion_description"] = str_or_empty("");
            achievements_db[OutDefinition->AchievementId]["unlocked_icon_url"] = str_or_empty(url.c_str());
            achievements_db[OutDefinition->AchievementId]["locked_icon_url"] = str_or_empty(url_locked.c_str());
            achievements_db[OutDefinition->AchievementId]["is_hidden"] = (bool)OutDefinition->bIsHidden;

            download_icon(OutDefinition->UnlockedIconURL, url);
            download_icon(OutDefinition->LockedIconURL, url_locked);

            for (int i = 0; i < OutDefinition->StatThresholdsCount; ++i)
            {
                achievements_db[OutDefinition->AchievementId]["stats_thresholds"][OutDefinition->StatThresholds[i].Name]["name"] = str_or_empty(OutDefinition->StatThresholds[i].Name);
                achievements_db[OutDefinition->AchievementId]["stats_thresholds"][OutDefinition->StatThresholds[i].Name]["threshold"] = OutDefinition->StatThresholds[i].Threshold;
            }

            EOS_Achievements_DefinitionV2_Release(OutDefinition);
        }
    }

    save_json(achievements_db_file, achievements_db);
}

static void EOS_CALL query_achievements_complete(const EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo* Data)
{
    if (Data->ResultCode != EOS_EResult::EOS_Success)
    {
        achievements_mutex.unlock();
        return;
    }

    std::thread([]()
    {

        static int version = 0;
        if (version == 0)
        {
            if (GET_PROC_ADDRESS(original_dll, "EOS_Achievements_CopyAchievementDefinitionV2ByIndex") != nullptr)
            {
                version = 2;
            }
            else if (GET_PROC_ADDRESS(original_dll, "EOS_Achievements_CopyAchievementDefinitionByIndex") != nullptr)
            {
                version = 1;
            }
        }

        switch (version)
        {
            case 1: dump_achievements_definitionv1(); break;
            case 2: dump_achievements_definitionv2(); break;
            default: EPIC_LOG(Log::LogLevel::ERR, "Unable to find the achievement function.");
        }
    }).detach();
}

static void dump_achievements_def()
{
    if (achievements_mutex.try_lock())
    {
        EOS_Achievements_QueryDefinitionsOptions Options;
        Options.ApiVersion = EOS_ACHIEVEMENTS_QUERYDEFINITIONS_API_LATEST;
        Options.LocalUserId = get_product_user_id();
        Options.EpicUserId_DEPRECATED = get_epic_account_id();
        Options.HiddenAchievementIds_DEPRECATED = nullptr;
        Options.HiddenAchievementsCount_DEPRECATED = 0;

        EOS_Achievements_QueryDefinitions(hAchievements, &Options, nullptr, query_achievements_complete);
    }
}

static std::mutex leaderboards_mutex;

static void EOS_CALL query_leaderboards_complete(const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo* Data)
{
    if (Data->ResultCode != EOS_EResult::EOS_Success)
    {
        leaderboards_mutex.unlock();
        return;
    }

    EOS_Leaderboards_GetLeaderboardDefinitionCountOptions Options;
    Options.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDDEFINITIONCOUNT_API_LATEST;

    uint32_t count = EOS_Leaderboards_GetLeaderboardDefinitionCount(hLeaderboards, &Options);

    for (uint32_t i = 0; i < count; ++i)
    {
        EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions DefinitionOptions;
        DefinitionOptions.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYINDEX_API_LATEST;
        DefinitionOptions.LeaderboardIndex = i;

        EOS_Leaderboards_Definition *OutLeaderboard;
        if (EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(hLeaderboards, &DefinitionOptions, &OutLeaderboard) == EOS_EResult::EOS_Success)
        {
            leaderboards_db[OutLeaderboard->LeaderboardId]["leaderboard_id"] = OutLeaderboard->LeaderboardId;
            leaderboards_db[OutLeaderboard->LeaderboardId]["stat_name"]      = OutLeaderboard->StatName;
            leaderboards_db[OutLeaderboard->LeaderboardId]["aggregation"]    = static_cast<int32_t>(OutLeaderboard->Aggregation);
            leaderboards_db[OutLeaderboard->LeaderboardId]["start_time"]     = OutLeaderboard->StartTime;
            leaderboards_db[OutLeaderboard->LeaderboardId]["end_time"]       = OutLeaderboard->EndTime;

            EOS_Leaderboards_LeaderboardDefinition_Release(OutLeaderboard);
        }
    }

    save_json(leaderboards_db_file, leaderboards_db);
}

static void dump_leaderboards_def()
{
    if (leaderboards_mutex.try_lock())
    {
        EOS_Leaderboards_QueryLeaderboardDefinitionsOptions Options;
        Options.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDDEFINITIONS_API_LATEST;
        Options.StartTime = EOS_LEADERBOARDS_TIME_UNDEFINED;
        Options.EndTime = EOS_LEADERBOARDS_TIME_UNDEFINED;

        EOS_Leaderboards_QueryLeaderboardDefinitions(hLeaderboards, &Options, nullptr, query_leaderboards_complete);
    }
}

static bool set_eos_compat(int32_t compat_version)
{
    int failed = false;
#if ! defined(__WINDOWS_32__)
    if (compat_version == 1)
    {
        EPIC_LOG(Log::LogLevel::DEBUG, "Tryiing to replace EOS_Auth_CopyUserAuthToken(%p) with EOS_Auth_CopyUserAuthTokenOld(%p)", EOS_Auth_CopyUserAuthToken, EOS_Auth_CopyUserAuthTokenOld);
        if (mini_detour::replace_func((void*)EOS_Auth_CopyUserAuthToken, (void*)EOS_Auth_CopyUserAuthTokenOld) ||
            mini_detour::replace_func((void*)EOS_Auth_AddNotifyLoginStatusChanged, (void*)EOS_Auth_AddNotifyLoginStatusChangedOld))
        {
            failed = true;
        }
    }
    else
    {
        EPIC_LOG(Log::LogLevel::DEBUG, "Tryiing to replace EOS_Auth_CopyUserAuthToken(%p) with EOS_Auth_CopyUserAuthTokenNew(%p)", EOS_Auth_CopyUserAuthToken, EOS_Auth_CopyUserAuthTokenNew);
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
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Initialize);

    if (set_eos_compat(Options->ApiVersion))
    {
        EPIC_LOG(Log::LogLevel::FATAL, "Couldn't replace our dummy EOS_Auth_CopyUserAuthToken, the function will not work and thus we terminate.");
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
            sstr << "ProductName             : " << str_or_empty(Options->ProductName) << std::endl;
            sstr << "ProductVersion          : " << str_or_empty(Options->ProductVersion) << std::endl;
        }
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Shutdown()
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Shutdown);
    return _EOS_Shutdown();
}

EOS_DECLARE_FUNC(const char*) EOS_EResult_ToString(EOS_EResult Result)
{
    //EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_EResult_ToString);
    return _EOS_EResult_ToString(Result);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_EResult_IsOperationComplete(EOS_EResult Result)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_EResult_IsOperationComplete);
    return _EOS_EResult_IsOperationComplete(Result);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ByteArray_ToString(const uint8_t* ByteArray, const uint32_t Length, char* OutBuffer, uint32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ByteArray_ToString);
    return _EOS_ByteArray_ToString(ByteArray, Length, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_AccountId_IsValid(EOS_AccountId AccountId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_AccountId_IsValid);
    auto res = _EOS_AccountId_IsValid(AccountId);

    std::stringstream sstr;
    if (res == EOS_TRUE)
    {
        char buff[2048];
        int32_t size = 2048;
        EOS_AccountId_ToString(AccountId, buff, &size);

        sstr << buff;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_EpicAccountId_IsValid(EOS_EpicAccountId AccountId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_EpicAccountId_IsValid);
    auto res = _EOS_EpicAccountId_IsValid(AccountId);

    if (AccountId != (EOS_EpicAccountId)0x006E00690057002F)
    {
        auto res = _EOS_EpicAccountId_IsValid((EOS_EpicAccountId)0x006E00690057002F);
        std::stringstream sstr;

        sstr << "Epic Account Id 0x006E00690057002F: " << EOS_Bool_2_str(res) << " = ";

        if (res == EOS_TRUE)
        {
            char buff[4096];
            int32_t len = 4096;
            if (EOS_EpicAccountId_ToString(AccountId, buff, &len) == EOS_EResult::EOS_Success)
            {
                sstr << buff << std::endl;
            }
            else
            {
                sstr << "FAILED !" << std::endl;
            }
        }

        EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());
    }

    std::stringstream sstr;
    if (res == EOS_TRUE)
    {
        char buff[2048];
        int32_t size = 2048;
        EOS_EpicAccountId_ToString(AccountId, buff, &size);

        sstr << buff;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AccountId_ToString(EOS_AccountId AccountId, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_AccountId_ToString);
    auto res = _EOS_AccountId_ToString(AccountId, OutBuffer, InOutBufferLength);
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_EpicAccountId_ToString(EOS_EpicAccountId AccountId, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_EpicAccountId_ToString);
    auto res = _EOS_EpicAccountId_ToString(AccountId, OutBuffer, InOutBufferLength);
    return res;
}

EOS_DECLARE_FUNC(EOS_AccountId) EOS_AccountId_FromString(const char* AccountIdString)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_AccountId_FromString);
    auto res = _EOS_AccountId_FromString(AccountIdString);
    EPIC_LOG(Log::LogLevel::DEBUG, "%p = %s", res, AccountIdString);
    return res;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_EpicAccountId_FromString(const char* AccountIdString)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_EpicAccountId_FromString);
    auto res = _EOS_EpicAccountId_FromString(AccountIdString);
    EPIC_LOG(Log::LogLevel::DEBUG, "%p = %s", res, AccountIdString);
    return res;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_ProductUserId_IsValid(EOS_ProductUserId AccountId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProductUserId_ToString(EOS_ProductUserId AccountId, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ProductUserId_ToString);
    auto res = _EOS_ProductUserId_ToString(AccountId, OutBuffer, InOutBufferLength);
    return res;
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_ProductUserId_FromString(const char* AccountIdString)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ProductUserId_FromString);
    auto res = _EOS_ProductUserId_FromString(AccountIdString);
    EPIC_LOG(Log::LogLevel::DEBUG, "%p = %s", res, AccountIdString);
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Logging_SetCallback(EOS_LogMessageFunc Callback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Logging_SetCallback);
    return _EOS_Logging_SetCallback(Callback);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Logging_SetLogLevel(EOS_ELogCategory LogCategory, EOS_ELogLevel LogLevel)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Logging_SetLogLevel);
    return _EOS_Logging_SetLogLevel(LogCategory, LogLevel);
}

////////////////////////////////////////////
// UserInfo
EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfo(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UserInfo_QueryUserInfo);
    return _EOS_UserInfo_QueryUserInfo(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfoByDisplayName(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoByDisplayNameOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoByDisplayNameCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UserInfo_QueryUserInfoByDisplayName);
    return _EOS_UserInfo_QueryUserInfoByDisplayName(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyUserInfo(EOS_HUserInfo Handle, const EOS_UserInfo_CopyUserInfoOptions* Options, EOS_UserInfo** OutUserInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UserInfo_CopyUserInfo);
    return _EOS_UserInfo_CopyUserInfo(Handle, Options, OutUserInfo);
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_Release(EOS_UserInfo* UserInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UserInfo_Release);
    return _EOS_UserInfo_Release(UserInfo);
}

////////////////////////////////////////////
// UI
EOS_DECLARE_FUNC(void) EOS_UI_ShowFriends(EOS_HUI Handle, const EOS_UI_ShowFriendsOptions* Options, void* ClientData, const EOS_UI_OnShowFriendsCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_ShowFriends);
    return _EOS_UI_ShowFriends(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_UI_HideFriends(EOS_HUI Handle, const EOS_UI_HideFriendsOptions* Options, void* ClientData, const EOS_UI_OnHideFriendsCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_HideFriends);
    return _EOS_UI_HideFriends(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_GetFriendsVisible(EOS_HUI Handle, const EOS_UI_GetFriendsVisibleOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_GetFriendsVisible);
    return _EOS_UI_GetFriendsVisible(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_SetToggleFriendsKeyOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_SetToggleFriendsKey);
    return _EOS_UI_SetToggleFriendsKey(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_UI_EKeyCombination) EOS_UI_GetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_GetToggleFriendsKeyOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_GetToggleFriendsKey);
    return _EOS_UI_GetToggleFriendsKey(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsValidKeyCombination(EOS_HUI Handle, EOS_UI_EKeyCombination KeyCombination)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_IsValidKeyCombination);
    return _EOS_UI_IsValidKeyCombination(Handle, KeyCombination);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetDisplayPreference(EOS_HUI Handle, const EOS_UI_SetDisplayPreferenceOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_SetDisplayPreference);
    return _EOS_UI_SetDisplayPreference(Handle, Options);
}


EOS_DECLARE_FUNC(EOS_UI_ENotificationLocation) EOS_UI_GetNotificationLocationPreference(EOS_HUI Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_UI_GetNotificationLocationPreference);
    return _EOS_UI_GetNotificationLocationPreference(Handle);
}

////////////////////////////////////////////
// Stats
EOS_DECLARE_FUNC(void) EOS_Stats_IngestStat(EOS_HStats Handle, const EOS_Stats_IngestStatOptions* Options, void* ClientData, const EOS_Stats_OnIngestStatCompleteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_IngestStat);
    return _EOS_Stats_IngestStat(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Stats_QueryStats(EOS_HStats Handle, const EOS_Stats_QueryStatsOptions* Options, void* ClientData, const EOS_Stats_OnQueryStatsCompleteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_QueryStats);
    return _EOS_Stats_QueryStats(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Stats_GetStatsCount(EOS_HStats Handle, const EOS_Stats_GetStatCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_GetStatsCount);
    return _EOS_Stats_GetStatsCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByIndex(EOS_HStats Handle, const EOS_Stats_CopyStatByIndexOptions* Options, EOS_Stats_Stat** OutStat)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_CopyStatByIndex);
    return _EOS_Stats_CopyStatByIndex(Handle, Options, OutStat);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByName(EOS_HStats Handle, const EOS_Stats_CopyStatByNameOptions* Options, EOS_Stats_Stat** OutStat)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_CopyStatByName);
    return _EOS_Stats_CopyStatByName(Handle, Options, OutStat);
}

EOS_DECLARE_FUNC(void) EOS_Stats_Stat_Release(EOS_Stats_Stat* Stat)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Stats_Stat_Release);
    return _EOS_Stats_Stat_Release(Stat);
}

////////////////////////////////////////////
// Sessions
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CreateSessionModification(EOS_HSessions Handle, const EOS_Sessions_CreateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CreateSessionModification);
    return _EOS_Sessions_CreateSessionModification(Handle, Options, OutSessionModificationHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_UpdateSessionModification(EOS_HSessions Handle, const EOS_Sessions_UpdateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_UpdateSessionModification);
    return _EOS_Sessions_UpdateSessionModification(Handle, Options, OutSessionModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_UpdateSession(EOS_HSessions Handle, const EOS_Sessions_UpdateSessionOptions* Options, void* ClientData, const EOS_Sessions_OnUpdateSessionCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_UpdateSession);

    callback_wrapper* wrapper = new callback_wrapper;
    wrapper->ClientData = ClientData;
    wrapper->CbFunc = (callback_t)CompletionDelegate;
    auto f = [](const EOS_Sessions_UpdateSessionCallbackInfo *cbinfo) {
        callback_wrapper* wrapper = (callback_wrapper*)cbinfo->ClientData;

        EPIC_LOG(Log::LogLevel::DEBUG, "%s: '%s' - '%s'", EOS_EResult_ToString(cbinfo->ResultCode), cbinfo->SessionId, cbinfo->SessionName);

        const_cast<EOS_Sessions_UpdateSessionCallbackInfo*>(cbinfo)->ClientData = wrapper->ClientData;
        wrapper->CbFunc((void*)cbinfo);
        delete wrapper;
    };

    return _EOS_Sessions_UpdateSession(Handle, Options, wrapper, f);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_DestroySession(EOS_HSessions Handle, const EOS_Sessions_DestroySessionOptions* Options, void* ClientData, const EOS_Sessions_OnDestroySessionCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_DestroySession);
    return _EOS_Sessions_DestroySession(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_JoinSession(EOS_HSessions Handle, const EOS_Sessions_JoinSessionOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_JoinSession);
    return _EOS_Sessions_JoinSession(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_StartSession(EOS_HSessions Handle, const EOS_Sessions_StartSessionOptions* Options, void* ClientData, const EOS_Sessions_OnStartSessionCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_StartSession);
    return _EOS_Sessions_StartSession(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_EndSession(EOS_HSessions Handle, const EOS_Sessions_EndSessionOptions* Options, void* ClientData, const EOS_Sessions_OnEndSessionCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_EndSession);
    return _EOS_Sessions_EndSession(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RegisterPlayers(EOS_HSessions Handle, const EOS_Sessions_RegisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnRegisterPlayersCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_RegisterPlayers);
    return _EOS_Sessions_RegisterPlayers(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_UnregisterPlayers(EOS_HSessions Handle, const EOS_Sessions_UnregisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnUnregisterPlayersCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_UnregisterPlayers);
    return _EOS_Sessions_UnregisterPlayers(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_SendInvite(EOS_HSessions Handle, const EOS_Sessions_SendInviteOptions* Options, void* ClientData, const EOS_Sessions_OnSendInviteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_SendInvite);
    return _EOS_Sessions_SendInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RejectInvite(EOS_HSessions Handle, const EOS_Sessions_RejectInviteOptions* Options, void* ClientData, const EOS_Sessions_OnRejectInviteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_RejectInvite);
    return _EOS_Sessions_RejectInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_QueryInvites(EOS_HSessions Handle, const EOS_Sessions_QueryInvitesOptions* Options, void* ClientData, const EOS_Sessions_OnQueryInvitesCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_QueryInvites);
    return _EOS_Sessions_QueryInvites(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Sessions_GetInviteCount(EOS_HSessions Handle, const EOS_Sessions_GetInviteCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_GetInviteCount);
    return _EOS_Sessions_GetInviteCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_GetInviteIdByIndex(EOS_HSessions Handle, const EOS_Sessions_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_GetInviteIdByIndex);
    return _EOS_Sessions_GetInviteIdByIndex(Handle, Options, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CreateSessionSearch(EOS_HSessions Handle, const EOS_Sessions_CreateSessionSearchOptions* Options, EOS_HSessionSearch* OutSessionSearchHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CreateSessionSearch);
    return _EOS_Sessions_CreateSessionSearch(Handle, Options, OutSessionSearchHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopyActiveSessionHandle(EOS_HSessions Handle, const EOS_Sessions_CopyActiveSessionHandleOptions* Options, EOS_HActiveSession* OutSessionHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CopyActiveSessionHandle);
    return _EOS_Sessions_CopyActiveSessionHandle(Handle, Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteReceived(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteReceivedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteReceivedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_AddNotifySessionInviteReceived);
    return _EOS_Sessions_AddNotifySessionInviteReceived(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteReceived(EOS_HSessions Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_RemoveNotifySessionInviteReceived);
    return _EOS_Sessions_RemoveNotifySessionInviteReceived(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteAccepted(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteAcceptedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_AddNotifySessionInviteAccepted);
    return _EOS_Sessions_AddNotifySessionInviteAccepted(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteAccepted(EOS_HSessions Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_RemoveNotifySessionInviteAccepted);
    return _EOS_Sessions_RemoveNotifySessionInviteAccepted(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifyJoinSessionAccepted(EOS_HSessions Handle, const EOS_Sessions_AddNotifyJoinSessionAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionAcceptedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_AddNotifyJoinSessionAccepted);
    return _EOS_Sessions_AddNotifyJoinSessionAccepted(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifyJoinSessionAccepted(EOS_HSessions Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_RemoveNotifyJoinSessionAccepted);
    return _EOS_Sessions_RemoveNotifyJoinSessionAccepted(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleByInviteId(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleByInviteIdOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CopySessionHandleByInviteId);
    return _EOS_Sessions_CopySessionHandleByInviteId(Handle, Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleByUiEventId(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleByUiEventIdOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CopySessionHandleByUiEventId);
    return _EOS_Sessions_CopySessionHandleByUiEventId(Handle, Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleForPresence(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleForPresenceOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_CopySessionHandleForPresence);
    auto res = _EOS_Sessions_CopySessionHandleForPresence(Handle, Options, OutSessionHandle);
    EPIC_LOG(Log::LogLevel::DEBUG, "'%s': %p", EOS_EResult_ToString(res), OutSessionHandle);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion : " << Options->ApiVersion << std::endl;
    sstr << "LocalUserId: " << Options->LocalUserId << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        EOS_SessionDetails_CopyInfoOptions options;
        options.ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;

        EOS_SessionDetails_Info* details_infos;

        if (EOS_SessionDetails_CopyInfo(*OutSessionHandle, &options, &details_infos) == EOS_EResult::EOS_Success)
        {
            sstr << "  ApiVersion    : " << details_infos->ApiVersion << std::endl;
            switch (details_infos->ApiVersion)
            {
                case EOS_SESSIONDETAILS_INFO_API_001:
                {
                    auto v = reinterpret_cast<EOS_SessionDetails_Info001*>(details_infos);
                    sstr << "  SessionId               : " << v->SessionId << std::endl;
                    sstr << "  HostAddress             : " << v->HostAddress << std::endl;
                    sstr << "  NumOpenPublicConnections: " << v->NumOpenPublicConnections << std::endl;
                    switch (v->Settings->ApiVersion)
                    {
                        case EOS_SESSIONDETAILS_SETTINGS_API_002:
                        {
                            auto s = reinterpret_cast<const EOS_SessionDetails_Settings002*>(v->Settings);

                            sstr << "  bInvitesAllowed         : " << s->bInvitesAllowed << std::endl;
                        }
                        break;

                        case EOS_SESSIONDETAILS_SETTINGS_API_001:
                        {
                            auto s = reinterpret_cast<const EOS_SessionDetails_Settings001*>(v->Settings);

                            sstr << "  BucketId                : " << s->BucketId << std::endl;
                            sstr << "  NumPublicConnections    : " << s->NumPublicConnections << std::endl;
                            sstr << "  PermissionLevel         : " << s->PermissionLevel << std::endl;
                            sstr << "  bAllowJoinInProgress    : " << EOS_Bool_2_str(s->bAllowJoinInProgress) << std::endl;
                        }
                        break;
                    }
                }
             }
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());
    
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_IsUserInSession(EOS_HSessions Handle, const EOS_Sessions_IsUserInSessionOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_IsUserInSession);
    return _EOS_Sessions_IsUserInSession(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_DumpSessionState(EOS_HSessions Handle, const EOS_Sessions_DumpSessionStateOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Sessions_DumpSessionState);
    return _EOS_Sessions_DumpSessionState(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetBucketId(EOS_HSessionModification Handle, const EOS_SessionModification_SetBucketIdOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetBucketId);
    return _EOS_SessionModification_SetBucketId(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetHostAddress(EOS_HSessionModification Handle, const EOS_SessionModification_SetHostAddressOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetHostAddress);
    return _EOS_SessionModification_SetHostAddress(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetPermissionLevel(EOS_HSessionModification Handle, const EOS_SessionModification_SetPermissionLevelOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetPermissionLevel);
    return _EOS_SessionModification_SetPermissionLevel(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetJoinInProgressAllowed(EOS_HSessionModification Handle, const EOS_SessionModification_SetJoinInProgressAllowedOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetJoinInProgressAllowed);
    return _EOS_SessionModification_SetJoinInProgressAllowed(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetMaxPlayers(EOS_HSessionModification Handle, const EOS_SessionModification_SetMaxPlayersOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetMaxPlayers);
    return _EOS_SessionModification_SetMaxPlayers(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetInvitesAllowed(EOS_HSessionModification Handle, const EOS_SessionModification_SetInvitesAllowedOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_SetInvitesAllowed);
    return _EOS_SessionModification_SetInvitesAllowed(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_AddAttribute(EOS_HSessionModification Handle, const EOS_SessionModification_AddAttributeOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_AddAttribute);
    auto res = _EOS_SessionModification_AddAttribute(Handle, Options);
    switch (Options->SessionAttribute->ValueType)
    {
        case EOS_ESessionAttributeType::EOS_AT_BOOLEAN: EPIC_LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%d'", EOS_EResult_ToString(res)  , Options->SessionAttribute->Key, Options->SessionAttribute->Value.AsBool); break;
        case EOS_ESessionAttributeType::EOS_AT_DOUBLE : EPIC_LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%f'", EOS_EResult_ToString(res)  , Options->SessionAttribute->Key, Options->SessionAttribute->Value.AsDouble); break;
        case EOS_ESessionAttributeType::EOS_AT_INT64  : EPIC_LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%lli'", EOS_EResult_ToString(res), Options->SessionAttribute->Key, Options->SessionAttribute->Value.AsInt64); break;
        case EOS_ESessionAttributeType::EOS_AT_STRING : EPIC_LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%s'", EOS_EResult_ToString(res)  , Options->SessionAttribute->Key, Options->SessionAttribute->Value.AsUtf8); break;
    }
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_RemoveAttribute(EOS_HSessionModification Handle, const EOS_SessionModification_RemoveAttributeOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_RemoveAttribute);
    return _EOS_SessionModification_RemoveAttribute(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ActiveSession_CopyInfo(EOS_HActiveSession Handle, const EOS_ActiveSession_CopyInfoOptions* Options, EOS_ActiveSession_Info** OutActiveSessionInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ActiveSession_CopyInfo);
    auto res = _EOS_ActiveSession_CopyInfo(Handle, Options, OutActiveSessionInfo);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion : " << Options->ApiVersion << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  SessionName             : " << (*OutActiveSessionInfo)->SessionName << std::endl;
        sstr << "  State                   : " << (*OutActiveSessionInfo)->State << std::endl;
        switch ((*OutActiveSessionInfo)->SessionDetails->ApiVersion)
        {
            case EOS_SESSIONDETAILS_INFO_API_001:
            {
                auto v = reinterpret_cast<const EOS_SessionDetails_Info001*>((*OutActiveSessionInfo)->SessionDetails);

                sstr << "  HostAddress             : " << v->HostAddress << std::endl;
                sstr << "  NumOpenPublicConnections: " << v->NumOpenPublicConnections << std::endl;
                sstr << "  SessionId               : " << v->SessionId << std::endl;
                //sstr << "  : " << v->Settings << std::endl;
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(uint32_t) EOS_ActiveSession_GetRegisteredPlayerCount(EOS_HActiveSession Handle, const EOS_ActiveSession_GetRegisteredPlayerCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ActiveSession_GetRegisteredPlayerCount);
    return _EOS_ActiveSession_GetRegisteredPlayerCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_ActiveSession_GetRegisteredPlayerByIndex(EOS_HActiveSession Handle, const EOS_ActiveSession_GetRegisteredPlayerByIndexOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ActiveSession_GetRegisteredPlayerByIndex);
    return _EOS_ActiveSession_GetRegisteredPlayerByIndex(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopyInfo(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopyInfoOptions* Options, EOS_SessionDetails_Info** OutSessionInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_CopyInfo);
    return _EOS_SessionDetails_CopyInfo(Handle, Options, OutSessionInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_SessionDetails_GetSessionAttributeCount(EOS_HSessionDetails Handle, const EOS_SessionDetails_GetSessionAttributeCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_GetSessionAttributeCount);
    return _EOS_SessionDetails_GetSessionAttributeCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopySessionAttributeByIndex(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopySessionAttributeByIndexOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_CopySessionAttributeByIndex);
    return _EOS_SessionDetails_CopySessionAttributeByIndex(Handle, Options, OutSessionAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopySessionAttributeByKey(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopySessionAttributeByKeyOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_CopySessionAttributeByKey);
    return _EOS_SessionDetails_CopySessionAttributeByKey(Handle, Options, OutSessionAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetSessionId(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetSessionIdOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_SetSessionId);
    return _EOS_SessionSearch_SetSessionId(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetTargetUserId(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetTargetUserIdOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_SetTargetUserId);
    return _EOS_SessionSearch_SetTargetUserId(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetParameter(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetParameterOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_SetParameter);
    auto res = _EOS_SessionSearch_SetParameter(Handle, Options);
    switch (Options->Parameter->ValueType)
    {
        case EOS_ESessionAttributeType::EOS_AT_BOOLEAN: EPIC_LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%d'"  , EOS_EResult_ToString(res), Options->Parameter->Key, Options->Parameter->Value.AsBool); break;
        case EOS_ESessionAttributeType::EOS_AT_DOUBLE : EPIC_LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%f'"  , EOS_EResult_ToString(res), Options->Parameter->Key, Options->Parameter->Value.AsDouble); break;
        case EOS_ESessionAttributeType::EOS_AT_INT64  : EPIC_LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%lli'", EOS_EResult_ToString(res), Options->Parameter->Key, Options->Parameter->Value.AsInt64); break;
        case EOS_ESessionAttributeType::EOS_AT_STRING : EPIC_LOG(Log::LogLevel::DEBUG, "'%s': '%s' - '%s'"  , EOS_EResult_ToString(res), Options->Parameter->Key, Options->Parameter->Value.AsUtf8); break;
    }
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_RemoveParameter(EOS_HSessionSearch Handle, const EOS_SessionSearch_RemoveParameterOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_RemoveParameter);
    return _EOS_SessionSearch_RemoveParameter(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetMaxResults(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetMaxResultsOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_SetMaxResults);
    return _EOS_SessionSearch_SetMaxResults(Handle, Options);
}

EOS_DECLARE_FUNC(void) EOS_SessionSearch_Find(EOS_HSessionSearch Handle, const EOS_SessionSearch_FindOptions* Options, void* ClientData, const EOS_SessionSearch_OnFindCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_Find);

    callback_wrapper* wrapper = new callback_wrapper;
    wrapper->ClientData = ClientData;
    wrapper->CbFunc = (callback_t)CompletionDelegate;
    auto f = [](const EOS_SessionSearch_FindCallbackInfo* cbinfo) {
        callback_wrapper* wrapper = (callback_wrapper*)cbinfo->ClientData;

        EPIC_LOG(Log::LogLevel::DEBUG, "%s", EOS_EResult_ToString(cbinfo->ResultCode));

        const_cast<EOS_SessionSearch_FindCallbackInfo*>(cbinfo)->ClientData = wrapper->ClientData;
        wrapper->CbFunc((void*)cbinfo);
        delete wrapper;
    };

    return _EOS_SessionSearch_Find(Handle, Options, wrapper, f);
}

EOS_DECLARE_FUNC(uint32_t) EOS_SessionSearch_GetSearchResultCount(EOS_HSessionSearch Handle, const EOS_SessionSearch_GetSearchResultCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_GetSearchResultCount);
    auto res = _EOS_SessionSearch_GetSearchResultCount(Handle, Options);
    EPIC_LOG(Log::LogLevel::DEBUG, "%d", res);
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_CopySearchResultByIndex(EOS_HSessionSearch Handle, const EOS_SessionSearch_CopySearchResultByIndexOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_CopySearchResultByIndex);
    return _EOS_SessionSearch_CopySearchResultByIndex(Handle, Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(void) EOS_SessionModification_Release(EOS_HSessionModification SessionModificationHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionModification_Release);
    return _EOS_SessionModification_Release(SessionModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_ActiveSession_Release(EOS_HActiveSession ActiveSessionHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ActiveSession_Release);
    return _EOS_ActiveSession_Release(ActiveSessionHandle);
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Release(EOS_HSessionDetails SessionHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_Release);
    return _EOS_SessionDetails_Release(SessionHandle);
}

EOS_DECLARE_FUNC(void) EOS_SessionSearch_Release(EOS_HSessionSearch SessionSearchHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionSearch_Release);
    return _EOS_SessionSearch_Release(SessionSearchHandle);
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Attribute_Release(EOS_SessionDetails_Attribute* SessionAttribute)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_Attribute_Release);
    return _EOS_SessionDetails_Attribute_Release(SessionAttribute);
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Info_Release(EOS_SessionDetails_Info* SessionInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_SessionDetails_Info_Release);
    return _EOS_SessionDetails_Info_Release(SessionInfo);
}

EOS_DECLARE_FUNC(void) EOS_ActiveSession_Info_Release(EOS_ActiveSession_Info* ActiveSessionInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_ActiveSession_Info_Release);
    return _EOS_ActiveSession_Info_Release(ActiveSessionInfo);
}

////////////////////////////////////////////
// Presence
EOS_DECLARE_FUNC(void) EOS_Presence_QueryPresence(EOS_HPresence Handle, const EOS_Presence_QueryPresenceOptions* Options, void* ClientData, const EOS_Presence_OnQueryPresenceCompleteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_QueryPresence);
    return _EOS_Presence_QueryPresence(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_Presence_HasPresence(EOS_HPresence Handle, const EOS_Presence_HasPresenceOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_HasPresence);
    auto res = _EOS_Presence_HasPresence(Handle, Options);



    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CopyPresence(EOS_HPresence Handle, const EOS_Presence_CopyPresenceOptions* Options, EOS_Presence_Info** OutPresence)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());
    
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CreatePresenceModification(EOS_HPresence Handle, const EOS_Presence_CreatePresenceModificationOptions* Options, EOS_HPresenceModification* OutPresenceModificationHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_CreatePresenceModification);
    return _EOS_Presence_CreatePresenceModification(Handle, Options, OutPresenceModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_Presence_SetPresence(EOS_HPresence Handle, const EOS_Presence_SetPresenceOptions* Options, void* ClientData, const EOS_Presence_SetPresenceCompleteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_SetPresence);
    return _EOS_Presence_SetPresence(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyOnPresenceChanged(EOS_HPresence Handle, const EOS_Presence_AddNotifyOnPresenceChangedOptions* Options, void* ClientData, const EOS_Presence_OnPresenceChangedCallback NotificationHandler)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_AddNotifyOnPresenceChanged);
    return _EOS_Presence_AddNotifyOnPresenceChanged(Handle, Options, ClientData, NotificationHandler);
}

EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyOnPresenceChanged(EOS_HPresence Handle, EOS_NotificationId NotificationId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_RemoveNotifyOnPresenceChanged);
    return _EOS_Presence_RemoveNotifyOnPresenceChanged(Handle, NotificationId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyJoinGameAccepted(EOS_HPresence Handle, const EOS_Presence_AddNotifyJoinGameAcceptedOptions* Options, void* ClientData, const EOS_Presence_OnJoinGameAcceptedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_AddNotifyJoinGameAccepted);
    return _EOS_Presence_AddNotifyJoinGameAccepted(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyJoinGameAccepted(EOS_HPresence Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_RemoveNotifyJoinGameAccepted);
    return _EOS_Presence_RemoveNotifyJoinGameAccepted(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_GetJoinInfo(EOS_HPresence Handle, const EOS_Presence_GetJoinInfoOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_GetJoinInfo);
    auto res = _EOS_Presence_GetJoinInfo(Handle, Options, OutBuffer, InOutBufferLength);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion: " << Options->ApiVersion << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        if (OutBuffer != nullptr)
        {
            sstr << "JoinInfo: " << OutBuffer << std::endl;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetStatus(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetStatusOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_SetStatus);
    auto res = _EOS_PresenceModification_SetStatus(Handle, Options);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion: " << Options->ApiVersion << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "Status    : " << Options->Status << std::endl;
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetRawRichText(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetRawRichTextOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_SetRawRichText);
    auto res = _EOS_PresenceModification_SetRawRichText(Handle, Options);
    EPIC_LOG(Log::LogLevel::DEBUG, "%s: %s", EOS_EResult_ToString(res), Options->RichText);
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetData(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetDataOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_SetData);
    auto res = _EOS_PresenceModification_SetData(Handle, Options);
    for (int i = 0; i < Options->RecordsCount; ++i)
    {
        EPIC_LOG(Log::LogLevel::DEBUG, "%s: '%s'='%s'", EOS_EResult_ToString(res), Options->Records[i].Key, Options->Records[i].Value);
    }
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_DeleteData(EOS_HPresenceModification Handle, const EOS_PresenceModification_DeleteDataOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_DeleteData);
    return _EOS_PresenceModification_DeleteData(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetJoinInfo(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetJoinInfoOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_SetJoinInfo);
    return _EOS_PresenceModification_SetJoinInfo(Handle, Options);
}

EOS_DECLARE_FUNC(void) EOS_Presence_Info_Release(EOS_Presence_Info* PresenceInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Presence_Info_Release);
    return _EOS_Presence_Info_Release(PresenceInfo);
}

EOS_DECLARE_FUNC(void) EOS_PresenceModification_Release(EOS_HPresenceModification PresenceModificationHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PresenceModification_Release);
    return _EOS_PresenceModification_Release(PresenceModificationHandle);
}

////////////////////////////////////////////
// PlayerDataStorage
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_QueryFileOptions* QueryFileOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileCompleteCallback CompletionCallback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_QueryFile);
    return _EOS_PlayerDataStorage_QueryFile(Handle, QueryFileOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFileList(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_QueryFileListOptions* QueryFileListOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileListCompleteCallback CompletionCallback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_QueryFileList);
    return _EOS_PlayerDataStorage_QueryFileList(Handle, QueryFileListOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataByFilename(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_CopyFileMetadataByFilenameOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata** OutMetadata)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_CopyFileMetadataByFilename);
    return _EOS_PlayerDataStorage_CopyFileMetadataByFilename(Handle, CopyFileMetadataOptions, OutMetadata);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_GetFileMetadataCount(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_GetFileMetadataCountOptions* GetFileMetadataCountOptions, int32_t* OutFileMetadataCount)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_GetFileMetadataCount);
    return _EOS_PlayerDataStorage_GetFileMetadataCount(Handle, GetFileMetadataCountOptions, OutFileMetadataCount);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataAtIndex(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_CopyFileMetadataAtIndexOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata** OutMetadata)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_CopyFileMetadataAtIndex);
    return _EOS_PlayerDataStorage_CopyFileMetadataAtIndex(Handle, CopyFileMetadataOptions, OutMetadata);
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DuplicateFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DuplicateFileOptions* DuplicateOptions, void* ClientData, const EOS_PlayerDataStorage_OnDuplicateFileCompleteCallback CompletionCallback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_DuplicateFile);
    return _EOS_PlayerDataStorage_DuplicateFile(Handle, DuplicateOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DeleteFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DeleteFileOptions* DeleteOptions, void* ClientData, const EOS_PlayerDataStorage_OnDeleteFileCompleteCallback CompletionCallback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_DeleteFile);
    return _EOS_PlayerDataStorage_DeleteFile(Handle, DeleteOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_HPlayerDataStorageFileTransferRequest) EOS_PlayerDataStorage_ReadFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_ReadFileOptions* ReadOptions, void* ClientData, const EOS_PlayerDataStorage_OnReadFileCompleteCallback CompletionCallback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_ReadFile);
    return _EOS_PlayerDataStorage_ReadFile(Handle, ReadOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_HPlayerDataStorageFileTransferRequest) EOS_PlayerDataStorage_WriteFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_WriteFileOptions* WriteOptions, void* ClientData, const EOS_PlayerDataStorage_OnWriteFileCompleteCallback CompletionCallback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_WriteFile);
    return _EOS_PlayerDataStorage_WriteFile(Handle, WriteOptions, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState(EOS_HPlayerDataStorageFileTransferRequest Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState);
    return _EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState(Handle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_GetFilename(EOS_HPlayerDataStorageFileTransferRequest Handle, uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorageFileTransferRequest_GetFilename);
    return _EOS_PlayerDataStorageFileTransferRequest_GetFilename(Handle, FilenameStringBufferSizeBytes, OutStringBuffer, OutStringLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_CancelRequest(EOS_HPlayerDataStorageFileTransferRequest Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorageFileTransferRequest_CancelRequest);
    return _EOS_PlayerDataStorageFileTransferRequest_CancelRequest(Handle);
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_FileMetadata_Release(EOS_PlayerDataStorage_FileMetadata* FileMetadata)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorage_FileMetadata_Release);
    return _EOS_PlayerDataStorage_FileMetadata_Release(FileMetadata);
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorageFileTransferRequest_Release(EOS_HPlayerDataStorageFileTransferRequest PlayerDataStorageFileTransferHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_PlayerDataStorageFileTransferRequest_Release);
    return _EOS_PlayerDataStorageFileTransferRequest_Release(PlayerDataStorageFileTransferHandle);
}

////////////////////////////////////////////
// Platform
EOS_DECLARE_FUNC(void) EOS_Platform_Tick(EOS_HPlatform Handle)
{
    //EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_Tick);

    return _EOS_Platform_Tick(Handle);
}

EOS_DECLARE_FUNC(EOS_HMetrics) EOS_Platform_GetMetricsInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetMetricsInterface);
    return _EOS_Platform_GetMetricsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HAuth) EOS_Platform_GetAuthInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetAuthInterface);
    return _EOS_Platform_GetAuthInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HConnect) EOS_Platform_GetConnectInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetConnectInterface);
    return _EOS_Platform_GetConnectInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HEcom) EOS_Platform_GetEcomInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetEcomInterface);
    return _EOS_Platform_GetEcomInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HUI) EOS_Platform_GetUIInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetUIInterface);
    return _EOS_Platform_GetUIInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HFriends) EOS_Platform_GetFriendsInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetFriendsInterface);
    return _EOS_Platform_GetFriendsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HPresence) EOS_Platform_GetPresenceInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetPresenceInterface);
    return _EOS_Platform_GetPresenceInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HSessions) EOS_Platform_GetSessionsInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetSessionsInterface);
    return _EOS_Platform_GetSessionsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HLobby) EOS_Platform_GetLobbyInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetLobbyInterface);
    return _EOS_Platform_GetLobbyInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HUserInfo) EOS_Platform_GetUserInfoInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetUserInfoInterface);
    return _EOS_Platform_GetUserInfoInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HP2P) EOS_Platform_GetP2PInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetP2PInterface);
    return _EOS_Platform_GetP2PInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HPlayerDataStorage) EOS_Platform_GetPlayerDataStorageInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetPlayerDataStorageInterface);
    return _EOS_Platform_GetPlayerDataStorageInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HAchievements) EOS_Platform_GetAchievementsInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetAchievementsInterface);
    return _EOS_Platform_GetAchievementsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HStats) EOS_Platform_GetStatsInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetStatsInterface);
    return _EOS_Platform_GetStatsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_HLeaderboards) EOS_Platform_GetLeaderboardsInterface(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetLeaderboardsInterface);
    return _EOS_Platform_GetLeaderboardsInterface(Handle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetActiveCountryCode(EOS_HPlatform Handle, EOS_EpicAccountId LocalUserId, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetActiveCountryCode);
    return _EOS_Platform_GetActiveCountryCode(Handle, LocalUserId, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetActiveLocaleCode(EOS_HPlatform Handle, EOS_EpicAccountId LocalUserId, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetActiveLocaleCode);
    return _EOS_Platform_GetActiveLocaleCode(Handle, LocalUserId, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetOverrideCountryCode(EOS_HPlatform Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetOverrideCountryCode);
    return _EOS_Platform_GetOverrideCountryCode(Handle, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetOverrideLocaleCode(EOS_HPlatform Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_GetOverrideLocaleCode);
    return _EOS_Platform_GetOverrideLocaleCode(Handle, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_SetOverrideCountryCode(EOS_HPlatform Handle, const char* NewCountryCode)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_SetOverrideCountryCode);
    return _EOS_Platform_SetOverrideCountryCode(Handle, NewCountryCode);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_SetOverrideLocaleCode(EOS_HPlatform Handle, const char* NewLocaleCode)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_SetOverrideLocaleCode);
    return _EOS_Platform_SetOverrideLocaleCode(Handle, NewLocaleCode);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_CheckForLauncherAndRestart(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_CheckForLauncherAndRestart);
    return _EOS_Platform_CheckForLauncherAndRestart(Handle);
}

EOS_DECLARE_FUNC(EOS_HPlatform) EOS_Platform_Create(const EOS_Platform_Options* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_Create);
    hPlatform = _EOS_Platform_Create(Options);

    if (hPlatform != nullptr)
    {
        std::stringstream sstr;
        sstr << std::endl;
        sstr << "ApiVersion                     : " << Options->ApiVersion << std::endl;
        switch (Options->ApiVersion)
        {
            case EOS_PLATFORM_OPTIONS_API_008:
            {

            }

            case EOS_PLATFORM_OPTIONS_API_007:
            {
                auto* v = reinterpret_cast<const EOS_Platform_Options007*>(Options);
                sstr << "TickBudgetInMilliseconds       : " << Options->TickBudgetInMilliseconds << std::endl;
            }

            case EOS_PLATFORM_OPTIONS_API_006:
            {
                auto* v = reinterpret_cast<const EOS_Platform_Options006*>(Options);
                sstr << "CacheDirectory                 : " << str_or_empty(Options->CacheDirectory) << std::endl;
            }

            case EOS_PLATFORM_OPTIONS_API_005:
            {
                auto* v = reinterpret_cast<const EOS_Platform_Options005*>(Options);
                sstr << "EncryptionKey                  : " << str_or_empty(Options->EncryptionKey) << std::endl;
                sstr << "OverrideCountryCode            : " << str_or_empty(Options->OverrideCountryCode) << std::endl;
                sstr << "OverrideLocaleCode             : " << str_or_empty(Options->OverrideLocaleCode) << std::endl;
                sstr << "DeploymentId                   : " << str_or_empty(Options->DeploymentId) << std::endl;
                sstr << "Flags                          : " << Options->Flags << std::endl;
            }

            case EOS_PLATFORM_OPTIONS_API_001:
            {
                

                auto* v = reinterpret_cast<const EOS_Platform_Options001*>(Options);
                sstr << "Reserved                       : " << Options->Reserved << std::endl;
                sstr << "ProductId                      : " << str_or_empty(Options->ProductId) << std::endl;
                sstr << "SandboxId                      : " << str_or_empty(Options->SandboxId) << std::endl;
                sstr << "ClientCredentials::ClientId    : " << str_or_empty(Options->ClientCredentials.ClientId) << std::endl;
                sstr << "ClientCredentials::ClientSecret: " << str_or_empty(Options->ClientCredentials.ClientSecret) << std::endl;
                sstr << "bIsServer                      : " << EOS_Bool_2_str(Options->bIsServer) << std::endl;
            }
        }
        EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetAuthInterface") != nullptr)
        {
            hAuth = EOS_Platform_GetAuthInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetAuthInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetFriendsInterface") != nullptr)
        {
            hFriends = EOS_Platform_GetFriendsInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetFriendsInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetMetricsInterface") != nullptr)
        {
            hMetrics = EOS_Platform_GetMetricsInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetMetricsInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetPresenceInterface") != nullptr)
        {
            hPresence = EOS_Platform_GetPresenceInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetPresenceInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetUserInfoInterface") != nullptr)
        {
            hUserInfo = EOS_Platform_GetUserInfoInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetUserInfoInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetEcomInterface") != nullptr)
        {
            hEcom = EOS_Platform_GetEcomInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetEcomInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetConnectInterface") != nullptr)
        {
            hConnect = EOS_Platform_GetConnectInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetConnectInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetP2PInterface") != nullptr)
        {
            hP2P = EOS_Platform_GetP2PInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetP2PInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetSessionsInterface") != nullptr)
        {
            hSessions = EOS_Platform_GetSessionsInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetSessionsInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetAchievementsInterface") != nullptr)
        {
            hAchievements = EOS_Platform_GetAchievementsInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetAchievementsInterface");

            dump_achievements_def();
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetPlayerDataStorageInterface") != nullptr)
        {
            hPlayerDataStorage = EOS_Platform_GetPlayerDataStorageInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetPlayerDataStorageInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetStatsInterface") != nullptr)
        {
            hStats = EOS_Platform_GetStatsInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetStatsInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetLeaderboardsInterface") != nullptr)
        {
            hLeaderboards = EOS_Platform_GetLeaderboardsInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetLeaderboardsInterface");

            dump_leaderboards_def();
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetLobbyInterface") != nullptr)
        {
            hLobby = EOS_Platform_GetLobbyInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetLobbyInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetUIInterface") != nullptr)
        {
            hUI = EOS_Platform_GetUIInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetUIInterface");
        }
        if (GET_PROC_ADDRESS(original_dll, "EOS_Platform_GetTitleStorageInterface") != nullptr)
        {
            hUI = EOS_Platform_GetUIInterface(hPlatform);
            EPIC_LOG(Log::LogLevel::DEBUG, "Has EOS_Platform_GetTitleStorageInterface");
        }
    }

    return hPlatform;
}

EOS_DECLARE_FUNC(void) EOS_Platform_Release(EOS_HPlatform Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Platform_Release);
    return _EOS_Platform_Release(Handle);
}

////////////////////////////////////////////
// p2p
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SendPacket(EOS_HP2P Handle, const EOS_P2P_SendPacketOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_SendPacket);
    return _EOS_P2P_SendPacket(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetNextReceivedPacketSize(EOS_HP2P Handle, const EOS_P2P_GetNextReceivedPacketSizeOptions* Options, uint32_t* OutPacketSizeBytes)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_GetNextReceivedPacketSize);
    return _EOS_P2P_GetNextReceivedPacketSize(Handle, Options, OutPacketSizeBytes);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_ReceivePacket(EOS_HP2P Handle, const EOS_P2P_ReceivePacketOptions* Options, EOS_ProductUserId* OutPeerId, EOS_P2P_SocketId* OutSocketId, uint8_t* OutChannel, void* OutData, uint32_t* OutBytesWritten)
{
    //EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_ReceivePacket);
    return _EOS_P2P_ReceivePacket(Handle, Options, OutPeerId, OutSocketId, OutChannel, OutData, OutBytesWritten);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionRequest(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionRequestOptions* Options, void* ClientData, EOS_P2P_OnIncomingConnectionRequestCallback ConnectionRequestHandler)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_AddNotifyPeerConnectionRequest);
    return _EOS_P2P_AddNotifyPeerConnectionRequest(Handle, Options, ClientData, ConnectionRequestHandler);
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionRequest(EOS_HP2P Handle, EOS_NotificationId NotificationId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_RemoveNotifyPeerConnectionRequest);
    return _EOS_P2P_RemoveNotifyPeerConnectionRequest(Handle, NotificationId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionClosed(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionClosedOptions* Options, void* ClientData, EOS_P2P_OnRemoteConnectionClosedCallback ConnectionClosedHandler)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_AddNotifyPeerConnectionClosed);
    return _EOS_P2P_AddNotifyPeerConnectionClosed(Handle, Options, ClientData, ConnectionClosedHandler);
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionClosed(EOS_HP2P Handle, EOS_NotificationId NotificationId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_RemoveNotifyPeerConnectionClosed);
    return _EOS_P2P_RemoveNotifyPeerConnectionClosed(Handle, NotificationId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_AcceptConnection(EOS_HP2P Handle, const EOS_P2P_AcceptConnectionOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_AcceptConnection);
    return _EOS_P2P_AcceptConnection(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_CloseConnection(EOS_HP2P Handle, const EOS_P2P_CloseConnectionOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_CloseConnection);
    return _EOS_P2P_CloseConnection(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_CloseConnections(EOS_HP2P Handle, const EOS_P2P_CloseConnectionsOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_CloseConnections);
    return _EOS_P2P_CloseConnections(Handle, Options);
}

EOS_DECLARE_FUNC(void) EOS_P2P_QueryNATType(EOS_HP2P Handle, const EOS_P2P_QueryNATTypeOptions* Options, void* ClientData, const EOS_P2P_OnQueryNATTypeCompleteCallback NATTypeQueriedHandler)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
        EPIC_LOG(Log::LogLevel::DEBUG, "%s: %s", EOS_EResult_ToString(cbinfo->ResultCode), nattype);

        const_cast<EOS_P2P_OnQueryNATTypeCompleteInfo*>(cbinfo)->ClientData = wrapper->ClientData;
        wrapper->CbFunc((void*)cbinfo);
        delete wrapper;
    };

    return _EOS_P2P_QueryNATType(Handle, Options, wrapper, f);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetNATType(EOS_HP2P Handle, const EOS_P2P_GetNATTypeOptions* Options, EOS_ENATType* OutNATType)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SetRelayControl(EOS_HP2P Handle, const EOS_P2P_SetRelayControlOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_SetRelayControl);
    return _EOS_P2P_SetRelayControl(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetRelayControl(EOS_HP2P Handle, const EOS_P2P_GetRelayControlOptions* Options, EOS_ERelayControl* OutRelayControl)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_GetRelayControl);
    return _EOS_P2P_GetRelayControl(Handle, Options, OutRelayControl);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SetPortRange(EOS_HP2P Handle, const EOS_P2P_SetPortRangeOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_SetPortRange);
    return _EOS_P2P_SetPortRange(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetPortRange(EOS_HP2P Handle, const EOS_P2P_GetPortRangeOptions* Options, uint16_t* OutPort, uint16_t* OutNumAdditionalPortsToTry)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_P2P_GetPortRange);
    return _EOS_P2P_GetPortRange(Handle, Options, OutPort, OutNumAdditionalPortsToTry);
}

////////////////////////////////////////////
// metrics
EOS_DECLARE_FUNC(EOS_EResult) EOS_Metrics_BeginPlayerSession(EOS_HMetrics Handle, const EOS_Metrics_BeginPlayerSessionOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Metrics_BeginPlayerSession);
    return _EOS_Metrics_BeginPlayerSession(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Metrics_EndPlayerSession(EOS_HMetrics Handle, const EOS_Metrics_EndPlayerSessionOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Metrics_EndPlayerSession);
    return _EOS_Metrics_EndPlayerSession(Handle, Options);
}

////////////////////////////////////////////
// lobby
EOS_DECLARE_FUNC(void) EOS_Lobby_CreateLobby(EOS_HLobby Handle, const EOS_Lobby_CreateLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnCreateLobbyCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_CreateLobby);
    return _EOS_Lobby_CreateLobby(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_DestroyLobby(EOS_HLobby Handle, const EOS_Lobby_DestroyLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnDestroyLobbyCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_DestroyLobby);
    return _EOS_Lobby_DestroyLobby(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_JoinLobby(EOS_HLobby Handle, const EOS_Lobby_JoinLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnJoinLobbyCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_JoinLobby);
    return _EOS_Lobby_JoinLobby(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_LeaveLobby(EOS_HLobby Handle, const EOS_Lobby_LeaveLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnLeaveLobbyCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_LeaveLobby);
    return _EOS_Lobby_LeaveLobby(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_UpdateLobbyModification(EOS_HLobby Handle, const EOS_Lobby_UpdateLobbyModificationOptions* Options, EOS_HLobbyModification* OutLobbyModificationHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_UpdateLobbyModification);
    return _EOS_Lobby_UpdateLobbyModification(Handle, Options, OutLobbyModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_UpdateLobby(EOS_HLobby Handle, const EOS_Lobby_UpdateLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnUpdateLobbyCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_UpdateLobby);
    return _EOS_Lobby_UpdateLobby(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_PromoteMember(EOS_HLobby Handle, const EOS_Lobby_PromoteMemberOptions* Options, void* ClientData, const EOS_Lobby_OnPromoteMemberCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_PromoteMember);
    return _EOS_Lobby_PromoteMember(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_KickMember(EOS_HLobby Handle, const EOS_Lobby_KickMemberOptions* Options, void* ClientData, const EOS_Lobby_OnKickMemberCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_KickMember);
    return _EOS_Lobby_KickMember(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyUpdateReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyUpdateReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyUpdateReceivedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_AddNotifyLobbyUpdateReceived);
    return _EOS_Lobby_AddNotifyLobbyUpdateReceived(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyUpdateReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RemoveNotifyLobbyUpdateReceived);
    return _EOS_Lobby_RemoveNotifyLobbyUpdateReceived(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyMemberUpdateReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyMemberUpdateReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyMemberUpdateReceivedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_AddNotifyLobbyMemberUpdateReceived);
    return _EOS_Lobby_AddNotifyLobbyMemberUpdateReceived(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived);
    return _EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyMemberStatusReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyMemberStatusReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyMemberStatusReceivedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_AddNotifyLobbyMemberStatusReceived);
    return _EOS_Lobby_AddNotifyLobbyMemberStatusReceived(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived);
    return _EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived(Handle, InId);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_SendInvite(EOS_HLobby Handle, const EOS_Lobby_SendInviteOptions* Options, void* ClientData, const EOS_Lobby_OnSendInviteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_SendInvite);
    return _EOS_Lobby_SendInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RejectInvite(EOS_HLobby Handle, const EOS_Lobby_RejectInviteOptions* Options, void* ClientData, const EOS_Lobby_OnRejectInviteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RejectInvite);
    return _EOS_Lobby_RejectInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_QueryInvites(EOS_HLobby Handle, const EOS_Lobby_QueryInvitesOptions* Options, void* ClientData, const EOS_Lobby_OnQueryInvitesCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_QueryInvites);
    return _EOS_Lobby_QueryInvites(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Lobby_GetInviteCount(EOS_HLobby Handle, const EOS_Lobby_GetInviteCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_GetInviteCount);
    return _EOS_Lobby_GetInviteCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_GetInviteIdByIndex(EOS_HLobby Handle, const EOS_Lobby_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_GetInviteIdByIndex);
    return _EOS_Lobby_GetInviteIdByIndex(Handle, Options, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CreateLobbySearch(EOS_HLobby Handle, const EOS_Lobby_CreateLobbySearchOptions* Options, EOS_HLobbySearch* OutLobbySearchHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_CreateLobbySearch);
    return _EOS_Lobby_CreateLobbySearch(Handle, Options, OutLobbySearchHandle);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyInviteReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyInviteReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyInviteReceivedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_AddNotifyLobbyInviteReceived);
    return _EOS_Lobby_AddNotifyLobbyInviteReceived(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyInviteReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RemoveNotifyLobbyInviteReceived);
    return _EOS_Lobby_RemoveNotifyLobbyInviteReceived(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyInviteAccepted(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyInviteAcceptedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyInviteAcceptedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_AddNotifyLobbyInviteAccepted);
    return _EOS_Lobby_AddNotifyLobbyInviteAccepted(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyInviteAccepted(EOS_HLobby Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RemoveNotifyLobbyInviteAccepted);
    return _EOS_Lobby_RemoveNotifyLobbyInviteAccepted(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyJoinLobbyAccepted(EOS_HLobby Handle, const EOS_Lobby_AddNotifyJoinLobbyAcceptedOptions* Options, void* ClientData, const EOS_Lobby_OnJoinLobbyAcceptedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_AddNotifyJoinLobbyAccepted);
    return _EOS_Lobby_AddNotifyJoinLobbyAccepted(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyJoinLobbyAccepted(EOS_HLobby Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_RemoveNotifyJoinLobbyAccepted);
    return _EOS_Lobby_RemoveNotifyJoinLobbyAccepted(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandleByInviteId(EOS_HLobby Handle, const EOS_Lobby_CopyLobbyDetailsHandleByInviteIdOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_CopyLobbyDetailsHandleByInviteId);
    return _EOS_Lobby_CopyLobbyDetailsHandleByInviteId(Handle, Options, OutLobbyDetailsHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandle(EOS_HLobby Handle, const EOS_Lobby_CopyLobbyDetailsHandleOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_CopyLobbyDetailsHandle);
    return _EOS_Lobby_CopyLobbyDetailsHandle(Handle, Options, OutLobbyDetailsHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetPermissionLevel(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetPermissionLevelOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_SetPermissionLevel);
    return _EOS_LobbyModification_SetPermissionLevel(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetMaxMembers(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetMaxMembersOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_SetMaxMembers);
    return _EOS_LobbyModification_SetMaxMembers(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_AddAttributeOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_AddAttribute);
    return _EOS_LobbyModification_AddAttribute(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_RemoveAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_RemoveAttributeOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_RemoveAttribute);
    return _EOS_LobbyModification_RemoveAttribute(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddMemberAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_AddMemberAttributeOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_AddMemberAttribute);
    return _EOS_LobbyModification_AddMemberAttribute(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_RemoveMemberAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_RemoveMemberAttributeOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_RemoveMemberAttribute);
    return _EOS_LobbyModification_RemoveMemberAttribute(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetLobbyOwner(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetLobbyOwnerOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_GetLobbyOwner);
    return _EOS_LobbyDetails_GetLobbyOwner(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyInfo(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyInfoOptions* Options, EOS_LobbyDetails_Info** OutLobbyDetailsInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_CopyInfo);
    return _EOS_LobbyDetails_CopyInfo(Handle, Options, OutLobbyDetailsInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetAttributeCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetAttributeCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_GetAttributeCount);
    return _EOS_LobbyDetails_GetAttributeCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyAttributeByIndexOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_CopyAttributeByIndex);
    return _EOS_LobbyDetails_CopyAttributeByIndex(Handle, Options, OutAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByKey(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyAttributeByKeyOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_CopyAttributeByKey);
    return _EOS_LobbyDetails_CopyAttributeByKey(Handle, Options, OutAttribute);
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_GetMemberCount);
    return _EOS_LobbyDetails_GetMemberCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetMemberByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberByIndexOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_GetMemberByIndex);
    return _EOS_LobbyDetails_GetMemberByIndex(Handle, Options);
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberAttributeCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberAttributeCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_GetMemberAttributeCount);
    return _EOS_LobbyDetails_GetMemberAttributeCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberAttributeByIndexOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_CopyMemberAttributeByIndex);
    return _EOS_LobbyDetails_CopyMemberAttributeByIndex(Handle, Options, OutAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByKey(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberAttributeByKeyOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_CopyMemberAttributeByKey);
    return _EOS_LobbyDetails_CopyMemberAttributeByKey(Handle, Options, OutAttribute);
}

EOS_DECLARE_FUNC(void) EOS_LobbySearch_Find(EOS_HLobbySearch Handle, const EOS_LobbySearch_FindOptions* Options, void* ClientData, const EOS_LobbySearch_OnFindCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_Find);
    return _EOS_LobbySearch_Find(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetLobbyId(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetLobbyIdOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_SetLobbyId);
    return _EOS_LobbySearch_SetLobbyId(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetTargetUserId(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetTargetUserIdOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_SetTargetUserId);
    return _EOS_LobbySearch_SetTargetUserId(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetParameter(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetParameterOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_SetParameter);
    return _EOS_LobbySearch_SetParameter(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_RemoveParameter(EOS_HLobbySearch Handle, const EOS_LobbySearch_RemoveParameterOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_RemoveParameter);
    return _EOS_LobbySearch_RemoveParameter(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetMaxResults(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetMaxResultsOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_SetMaxResults);
    return _EOS_LobbySearch_SetMaxResults(Handle, Options);
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbySearch_GetSearchResultCount(EOS_HLobbySearch Handle, const EOS_LobbySearch_GetSearchResultCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_GetSearchResultCount);
    return _EOS_LobbySearch_GetSearchResultCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_CopySearchResultByIndex(EOS_HLobbySearch Handle, const EOS_LobbySearch_CopySearchResultByIndexOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_CopySearchResultByIndex);
    return _EOS_LobbySearch_CopySearchResultByIndex(Handle, Options, OutLobbyDetailsHandle);
}

EOS_DECLARE_FUNC(void) EOS_LobbyModification_Release(EOS_HLobbyModification LobbyModificationHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyModification_Release);
    return _EOS_LobbyModification_Release(LobbyModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Release(EOS_HLobbyDetails LobbyHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_Release);
    return _EOS_LobbyDetails_Release(LobbyHandle);
}

EOS_DECLARE_FUNC(void) EOS_LobbySearch_Release(EOS_HLobbySearch LobbySearchHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbySearch_Release);
    return _EOS_LobbySearch_Release(LobbySearchHandle);
}

EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Info_Release(EOS_LobbyDetails_Info* LobbyDetailsInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_LobbyDetails_Info_Release);
    return _EOS_LobbyDetails_Info_Release(LobbyDetailsInfo);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_Attribute_Release(EOS_Lobby_Attribute* LobbyAttribute)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Lobby_Attribute_Release);
    return _EOS_Lobby_Attribute_Release(LobbyAttribute);
}

////////////////////////////////////////////
// leaderboards
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardDefinitions(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardDefinitionsOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_QueryLeaderboardDefinitions);
    return _EOS_Leaderboards_QueryLeaderboardDefinitions(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardDefinitionCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardDefinitionCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_GetLeaderboardDefinitionCount);
    return _EOS_Leaderboards_GetLeaderboardDefinitionCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions* Options, EOS_Leaderboards_Definition** OutLeaderboardDefinition)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions* Options, EOS_Leaderboards_Definition** OutLeaderboardDefinition)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardRanks(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardRanksOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_QueryLeaderboardRanks);
    return _EOS_Leaderboards_QueryLeaderboardRanks(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardRecordCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardRecordCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_GetLeaderboardRecordCount);
    return _EOS_Leaderboards_GetLeaderboardRecordCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions* Options, EOS_Leaderboards_LeaderboardRecord** OutLeaderboardRecord)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions* Options, EOS_Leaderboards_LeaderboardRecord** OutLeaderboardRecord)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardUserScores(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardUserScoresOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_QueryLeaderboardUserScores);
    return _EOS_Leaderboards_QueryLeaderboardUserScores(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardUserScoreCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardUserScoreCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_GetLeaderboardUserScoreCount);
    return _EOS_Leaderboards_GetLeaderboardUserScoreCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions* Options, EOS_Leaderboards_LeaderboardUserScore** OutLeaderboardUserScore)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_CopyLeaderboardUserScoreByIndex);
    return _EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(Handle, Options, OutLeaderboardUserScore);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions* Options, EOS_Leaderboards_LeaderboardUserScore** OutLeaderboardUserScore)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_CopyLeaderboardUserScoreByUserId);
    return _EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(Handle, Options, OutLeaderboardUserScore);
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardDefinition_Release(EOS_Leaderboards_Definition* LeaderboardDefinition)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_LeaderboardDefinition_Release);
    return _EOS_Leaderboards_LeaderboardDefinition_Release(LeaderboardDefinition);
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardUserScore_Release(EOS_Leaderboards_LeaderboardUserScore* LeaderboardUserScore)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_LeaderboardUserScore_Release);
    return _EOS_Leaderboards_LeaderboardUserScore_Release(LeaderboardUserScore);
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardRecord_Release(EOS_Leaderboards_LeaderboardRecord* LeaderboardRecord)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Leaderboards_LeaderboardRecord_Release);
    return _EOS_Leaderboards_LeaderboardRecord_Release(LeaderboardRecord);
}

////////////////////////////////////////////
// friends
EOS_DECLARE_FUNC(void) EOS_Friends_QueryFriends(EOS_HFriends Handle, const EOS_Friends_QueryFriendsOptions* Options, void* ClientData, const EOS_Friends_OnQueryFriendsCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_QueryFriends);
    return _EOS_Friends_QueryFriends(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Friends_SendInvite(EOS_HFriends Handle, const EOS_Friends_SendInviteOptions* Options, void* ClientData, const EOS_Friends_OnSendInviteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_SendInvite);
    return _EOS_Friends_SendInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Friends_AcceptInvite(EOS_HFriends Handle, const EOS_Friends_AcceptInviteOptions* Options, void* ClientData, const EOS_Friends_OnAcceptInviteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_AcceptInvite);
    return _EOS_Friends_AcceptInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Friends_RejectInvite(EOS_HFriends Handle, const EOS_Friends_RejectInviteOptions* Options, void* ClientData, const EOS_Friends_OnRejectInviteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_RejectInvite);
    return _EOS_Friends_RejectInvite(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetFriendsCount(EOS_HFriends Handle, const EOS_Friends_GetFriendsCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_GetFriendsCount);
    return _EOS_Friends_GetFriendsCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Friends_GetFriendAtIndex(EOS_HFriends Handle, const EOS_Friends_GetFriendAtIndexOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_GetFriendAtIndex);
    return _EOS_Friends_GetFriendAtIndex(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EFriendsStatus) EOS_Friends_GetStatus(EOS_HFriends Handle, const EOS_Friends_GetStatusOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_GetStatus);
    return _EOS_Friends_GetStatus(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Friends_AddNotifyFriendsUpdate(EOS_HFriends Handle, const EOS_Friends_AddNotifyFriendsUpdateOptions* Options, void* ClientData, const EOS_Friends_OnFriendsUpdateCallback FriendsUpdateHandler)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_AddNotifyFriendsUpdate);
    return _EOS_Friends_AddNotifyFriendsUpdate(Handle, Options, ClientData, FriendsUpdateHandler);
}

EOS_DECLARE_FUNC(void) EOS_Friends_RemoveNotifyFriendsUpdate(EOS_HFriends Handle, EOS_NotificationId NotificationId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Friends_RemoveNotifyFriendsUpdate);
    return _EOS_Friends_RemoveNotifyFriendsUpdate(Handle, NotificationId);
}

////////////////////////////////////////////
// ecom
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnership(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_QueryOwnership);

    callback_wrapper* wrapper = new callback_wrapper;
    wrapper->ClientData = ClientData;
    wrapper->CbFunc = (callback_t)CompletionDelegate;
    auto f = [](const EOS_Ecom_QueryOwnershipCallbackInfo* cbinfo) {
        callback_wrapper* wrapper = (callback_wrapper*)cbinfo->ClientData;

        if (cbinfo->ResultCode == EOS_EResult::EOS_Success)
        {
            std::stringstream sstr;
            sstr << std::endl;
            for (int i = 0; i < cbinfo->ItemOwnershipCount; ++i)
            {
                sstr << "ItemOwnership[" << i << "]: " << cbinfo->ItemOwnership[i].Id << " = " << cbinfo->ItemOwnership[i].OwnershipStatus << std::endl;
                const_cast<EOS_Ecom_ItemOwnership*>(cbinfo->ItemOwnership)[i].OwnershipStatus = EOS_EOwnershipStatus::EOS_OS_Owned;
            }
            EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());
        }

        const_cast<EOS_Ecom_QueryOwnershipCallbackInfo*>(cbinfo)->ClientData = wrapper->ClientData;
        wrapper->CbFunc((void*)cbinfo);
        delete wrapper;
    };

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion      : " << Options->ApiVersion << std::endl;
    switch (Options->ApiVersion)
    {
        case EOS_ECOM_QUERYOWNERSHIP_API_002:
        {
            auto* v = reinterpret_cast<const EOS_Ecom_QueryOwnershipOptions002*>(Options);
            sstr << "CatalogNamespace: " << str_or_empty(v->CatalogNamespace) << std::endl;
        }

        case EOS_ECOM_QUERYOWNERSHIP_API_001:
        {
            auto *v = reinterpret_cast<const EOS_Ecom_QueryOwnershipOptions001*>(Options);
            for (int i = 0; i < v->CatalogItemIdCount; ++i)
            {
                sstr << "CatalogItemIds[" << i << "]: " << str_or_empty(v->CatalogItemIds[i]) << std::endl;
                catalog[Options->CatalogItemIds[i]]["owned"] = true;
            }
        }
    }
    save_json(catalog_file, catalog);

    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return _EOS_Ecom_QueryOwnership(Handle, Options, wrapper, f);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnershipToken(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_QueryOwnershipToken);
    return _EOS_Ecom_QueryOwnershipToken(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlements(EOS_HEcom Handle, const EOS_Ecom_QueryEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_QueryEntitlements);
    return _EOS_Ecom_QueryEntitlements(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOffers(EOS_HEcom Handle, const EOS_Ecom_QueryOffersOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOffersCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_QueryOffers);
    return _EOS_Ecom_QueryOffers(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_Checkout(EOS_HEcom Handle, const EOS_Ecom_CheckoutOptions* Options, void* ClientData, const EOS_Ecom_OnCheckoutCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Checkout);
    return _EOS_Ecom_Checkout(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_RedeemEntitlements(EOS_HEcom Handle, const EOS_Ecom_RedeemEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnRedeemEntitlementsCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_RedeemEntitlements);

    if (Options != nullptr)
    {
        for (uint32_t i = 0; i < Options->EntitlementIdCount; ++i)
        {
            EPIC_LOG(Log::LogLevel::INFO, "EntitlementIds[%i] = %s", i, str_or_empty(Options->EntitlementIds[i]));
        }
    }

    return _EOS_Ecom_RedeemEntitlements(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetEntitlementsCount);
    auto res = _EOS_Ecom_GetEntitlementsCount(Handle, Options);
    EPIC_LOG(Log::LogLevel::INFO, "Entitlements Count: %u", res);
    return res;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsByNameCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsByNameCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetEntitlementsByNameCount);
    auto res = _EOS_Ecom_GetEntitlementsByNameCount(Handle, Options);

    if(Options != nullptr)
        EPIC_LOG(Log::LogLevel::INFO, "Entitlements Count (%s): %u", str_or_empty(Options->EntitlementName), res);

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
                sstr << "  EntitlementName: " << str_or_empty(v->EntitlementName) << std::endl;
                sstr << "  EntitlementId  : " << str_or_empty(v->EntitlementId) << std::endl;
                sstr << "  CatalogItemId  : " << str_or_empty(v->CatalogItemId) << std::endl;
                sstr << "  ServerIndex    : " << v->ServerIndex << std::endl;
                sstr << "  bRedeemed      : " << EOS_Bool_2_str(v->bRedeemed) << std::endl;
                sstr << "  EndTimestamp   : " << v->EndTimestamp << std::endl;
                
                entitlements[v->EntitlementId]["entitlement_name"] = str_or_empty(v->EntitlementName);
                entitlements[v->EntitlementId]["entitlement_id"]   = str_or_empty(v->EntitlementId);
                entitlements[v->EntitlementId]["catalog_item_id"]  = str_or_empty(v->CatalogItemId);
                entitlements[v->EntitlementId]["redeemed"]         = (bool)v->bRedeemed;
            }
            break;

            case EOS_ECOM_ENTITLEMENT_API_001:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement001*>(*OutEntitlement);
                sstr << "  Id             : " << v->Id << std::endl;

                entitlements[v->Id]["entitlement_name"] = "";
                entitlements[v->Id]["entitlement_id"]   = str_or_empty(v->Id);
                entitlements[v->Id]["catalog_item_id"]  = "";
                entitlements[v->Id]["redeemed"]         = true;
            }
            break;
        }
        save_json(entitlements_file, entitlements);
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByNameAndIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByNameAndIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyEntitlementByNameAndIndex);
    auto res = _EOS_Ecom_CopyEntitlementByNameAndIndex(Handle, Options, OutEntitlement);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion     : " << Options->ApiVersion << std::endl;
    sstr << "EntitlementName: " << str_or_empty(Options->EntitlementName) << std::endl;
    sstr << "Index          : " << Options->Index << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion     : " << (*OutEntitlement)->ApiVersion << std::endl;
        switch ((*OutEntitlement)->ApiVersion)
        {
            case EOS_ECOM_ENTITLEMENT_API_002:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement002*>(*OutEntitlement);
                sstr << "  EntitlementName: " << str_or_empty(v->EntitlementName) << std::endl;
                sstr << "  EntitlementId  : " << str_or_empty(v->EntitlementId) << std::endl;
                sstr << "  CatalogItemId  : " << str_or_empty(v->CatalogItemId) << std::endl;
                sstr << "  ServerIndex    : " << v->ServerIndex << std::endl;
                sstr << "  bRedeemed      : " << EOS_Bool_2_str(v->bRedeemed) << std::endl;
                sstr << "  EndTimestamp   : " << v->EndTimestamp << std::endl;

                //entitlements[v->EntitlementId]["entitlement_name"] = v->EntitlementName;
                //entitlements[v->EntitlementId]["entitlement_id"] = v->EntitlementId;
                //entitlements[v->EntitlementId]["catalog_item_id"] = v->CatalogItemId;
                //entitlements[v->EntitlementId]["redeemed"] = (bool)v->bRedeemed;
            }
            break;

            case EOS_ECOM_ENTITLEMENT_API_001:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement001*>(*OutEntitlement);
                sstr << "  Id             : " << str_or_empty(v->Id) << std::endl;

                //entitlements[v->Id]["entitlement_name"] = "";
                //entitlements[v->Id]["entitlement_id"] = v->Id;
                //entitlements[v->Id]["catalog_item_id"] = "";
                //entitlements[v->Id]["redeemed"] = true;
            }
            break;
        }
        save_json(entitlements_file, entitlements);
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementById(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIdOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyEntitlementById);
    auto res = _EOS_Ecom_CopyEntitlementById(Handle, Options, OutEntitlement);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion   : " << Options->ApiVersion << std::endl;
    sstr << "EntitlementId: " << str_or_empty(Options->EntitlementId) << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion     : " << (*OutEntitlement)->ApiVersion << std::endl;
        switch ((*OutEntitlement)->ApiVersion)
        {
            case EOS_ECOM_ENTITLEMENT_API_002:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement002*>(*OutEntitlement);
                sstr << "  EntitlementName: " << str_or_empty(v->EntitlementName) << std::endl;
                sstr << "  EntitlementId  : " << str_or_empty(v->EntitlementId) << std::endl;
                sstr << "  CatalogItemId  : " << str_or_empty(v->CatalogItemId) << std::endl;
                sstr << "  ServerIndex    : " << v->ServerIndex << std::endl;
                sstr << "  bRedeemed      : " << EOS_Bool_2_str(v->bRedeemed) << std::endl;
                sstr << "  EndTimestamp   : " << v->EndTimestamp << std::endl;

                entitlements[v->EntitlementId]["entitlement_name"] = str_or_empty(v->EntitlementName);
                entitlements[v->EntitlementId]["entitlement_id"]   = str_or_empty(v->EntitlementId);
                entitlements[v->EntitlementId]["catalog_item_id"]  = str_or_empty(v->CatalogItemId);
                entitlements[v->EntitlementId]["redeemed"]         = (bool)v->bRedeemed;
            }
            break;

            case EOS_ECOM_ENTITLEMENT_API_001:
            {
                auto v = reinterpret_cast<EOS_Ecom_Entitlement001*>(*OutEntitlement);
                sstr << "  Id             : " << v->Id << std::endl;

                entitlements[v->Id]["entitlement_name"] = "";
                entitlements[v->Id]["entitlement_id"]   = str_or_empty(v->Id);
                entitlements[v->Id]["catalog_item_id"]  = "";
                entitlements[v->Id]["redeemed"]         = true;
            }
            break;
        }
        save_json(entitlements_file, entitlements);
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetOfferCount);
    return _EOS_Ecom_GetOfferCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIndexOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyOfferByIndex);
    return _EOS_Ecom_CopyOfferByIndex(Handle, Options, OutOffer);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferById(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIdOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyOfferById);
    return _EOS_Ecom_CopyOfferById(Handle, Options, OutOffer);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferItemCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferItemCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetOfferItemCount);
    return _EOS_Ecom_GetOfferItemCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferItemByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferItemByIndexOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyOfferItemByIndex);
    return _EOS_Ecom_CopyOfferItemByIndex(Handle, Options, OutItem);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemById(EOS_HEcom Handle, const EOS_Ecom_CopyItemByIdOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyItemById);
    return _EOS_Ecom_CopyItemById(Handle, Options, OutItem);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferImageInfoCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetOfferImageInfoCount);
    return _EOS_Ecom_GetOfferImageInfoCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyOfferImageInfoByIndex);
    return _EOS_Ecom_CopyOfferImageInfoByIndex(Handle, Options, OutImageInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetItemImageInfoCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetItemImageInfoCount);
    return _EOS_Ecom_GetItemImageInfoCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyItemImageInfoByIndex);
    return _EOS_Ecom_CopyItemImageInfoByIndex(Handle, Options, OutImageInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemReleaseCount(EOS_HEcom Handle, const EOS_Ecom_GetItemReleaseCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetItemReleaseCount);
    return _EOS_Ecom_GetItemReleaseCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemReleaseByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemReleaseByIndexOptions* Options, EOS_Ecom_CatalogRelease** OutRelease)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyItemReleaseByIndex);
    return _EOS_Ecom_CopyItemReleaseByIndex(Handle, Options, OutRelease);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetTransactionCount(EOS_HEcom Handle, const EOS_Ecom_GetTransactionCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_GetTransactionCount);
    return _EOS_Ecom_GetTransactionCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIndexOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyTransactionByIndex);
    return _EOS_Ecom_CopyTransactionByIndex(Handle, Options, OutTransaction);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionById(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIdOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CopyTransactionById);
    return _EOS_Ecom_CopyTransactionById(Handle, Options, OutTransaction);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_GetTransactionId(EOS_Ecom_HTransaction Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Transaction_GetTransactionId);
    return _EOS_Ecom_Transaction_GetTransactionId(Handle, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_Transaction_GetEntitlementsCount(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_GetEntitlementsCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Transaction_GetEntitlementsCount);
    return _EOS_Ecom_Transaction_GetEntitlementsCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_CopyEntitlementByIndex(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
                sstr << "  EntitlementName: " << str_or_empty(v->EntitlementName) << std::endl;
                sstr << "  EntitlementId  : " << str_or_empty(v->EntitlementId) << std::endl;
                sstr << "  CatalogItemId  : " << str_or_empty(v->CatalogItemId) << std::endl;
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
        save_json(entitlements_file, entitlements);
    }
    else
    {
        sstr << "Failed" << std::endl;
    }

    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());
    return res;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_Entitlement_Release(EOS_Ecom_Entitlement* Entitlement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Entitlement_Release);
    return _EOS_Ecom_Entitlement_Release(Entitlement);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogItem_Release(EOS_Ecom_CatalogItem* CatalogItem)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CatalogItem_Release);
    return _EOS_Ecom_CatalogItem_Release(CatalogItem);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogOffer_Release(EOS_Ecom_CatalogOffer* CatalogOffer)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CatalogOffer_Release);
    return _EOS_Ecom_CatalogOffer_Release(CatalogOffer);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_KeyImageInfo_Release(EOS_Ecom_KeyImageInfo* KeyImageInfo)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_KeyImageInfo_Release);
    return _EOS_Ecom_KeyImageInfo_Release(KeyImageInfo);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogRelease_Release(EOS_Ecom_CatalogRelease* CatalogRelease)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_CatalogRelease_Release);
    return _EOS_Ecom_CatalogRelease_Release(CatalogRelease);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_Transaction_Release(EOS_Ecom_HTransaction Transaction)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Ecom_Transaction_Release);
    return _EOS_Ecom_Transaction_Release(Transaction);
}

////////////////////////////////////////////
// connect
EOS_DECLARE_FUNC(void) EOS_Connect_Login(EOS_HConnect Handle, const EOS_Connect_LoginOptions* Options, void* ClientData, const EOS_Connect_OnLoginCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_CreateUser);
    return _EOS_Connect_CreateUser(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_LinkAccount(EOS_HConnect Handle, const EOS_Connect_LinkAccountOptions* Options, void* ClientData, const EOS_Connect_OnLinkAccountCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_LinkAccount);
    return _EOS_Connect_LinkAccount(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_CreateDeviceId(EOS_HConnect Handle, const EOS_Connect_CreateDeviceIdOptions* Options, void* ClientData, const EOS_Connect_OnCreateDeviceIdCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_CreateDeviceId);
    return _EOS_Connect_CreateDeviceId(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_DeleteDeviceId(EOS_HConnect Handle, const EOS_Connect_DeleteDeviceIdOptions* Options, void* ClientData, const EOS_Connect_OnDeleteDeviceIdCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_DeleteDeviceId);
    return _EOS_Connect_DeleteDeviceId(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_QueryExternalAccountMappings(EOS_HConnect Handle, const EOS_Connect_QueryExternalAccountMappingsOptions* Options, void* ClientData, const EOS_Connect_OnQueryExternalAccountMappingsCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_QueryExternalAccountMappings);
    return _EOS_Connect_QueryExternalAccountMappings(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_QueryProductUserIdMappings(EOS_HConnect Handle, const EOS_Connect_QueryProductUserIdMappingsOptions* Options, void* ClientData, const EOS_Connect_OnQueryProductUserIdMappingsCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_QueryProductUserIdMappings);
    return _EOS_Connect_QueryProductUserIdMappings(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_Connect_GetExternalAccountMapping(EOS_HConnect Handle, const EOS_Connect_GetExternalAccountMappingsOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_GetExternalAccountMapping);
    return _EOS_Connect_GetExternalAccountMapping(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_GetProductUserIdMapping(EOS_HConnect Handle, const EOS_Connect_GetProductUserIdMappingOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_GetProductUserIdMapping);
    return _EOS_Connect_GetProductUserIdMapping(Handle, Options, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(int32_t) EOS_Connect_GetLoggedInUsersCount(EOS_HConnect Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_GetLoggedInUsersCount);
    return _EOS_Connect_GetLoggedInUsersCount(Handle);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_Connect_GetLoggedInUserByIndex(EOS_HConnect Handle, int32_t Index)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_GetLoggedInUserByIndex);
    return _EOS_Connect_GetLoggedInUserByIndex(Handle, Index);
}

EOS_DECLARE_FUNC(EOS_ELoginStatus) EOS_Connect_GetLoginStatus(EOS_HConnect Handle, EOS_ProductUserId LocalUserId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_GetLoginStatus);
    return _EOS_Connect_GetLoginStatus(Handle, LocalUserId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Connect_AddNotifyAuthExpiration(EOS_HConnect Handle, const EOS_Connect_AddNotifyAuthExpirationOptions* Options, void* ClientData, const EOS_Connect_OnAuthExpirationCallback Notification)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_AddNotifyAuthExpiration);
    return _EOS_Connect_AddNotifyAuthExpiration(Handle, Options, ClientData, Notification);
}

EOS_DECLARE_FUNC(void) EOS_Connect_RemoveNotifyAuthExpiration(EOS_HConnect Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_RemoveNotifyAuthExpiration);
    return _EOS_Connect_RemoveNotifyAuthExpiration(Handle, InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Connect_AddNotifyLoginStatusChanged(EOS_HConnect Handle, const EOS_Connect_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Connect_OnLoginStatusChangedCallback Notification)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_AddNotifyLoginStatusChanged);
    return _EOS_Connect_AddNotifyLoginStatusChanged(Handle, Options, ClientData, Notification);
}

EOS_DECLARE_FUNC(void) EOS_Connect_RemoveNotifyLoginStatusChanged(EOS_HConnect Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Connect_RemoveNotifyLoginStatusChanged);
    return _EOS_Connect_RemoveNotifyLoginStatusChanged(Handle, InId);
}

////////////////////////////////////////////
// auth
EOS_DECLARE_FUNC(void) EOS_Auth_Login(EOS_HAuth Handle, const EOS_Auth_LoginOptions* Options, void* ClientData, const EOS_Auth_OnLoginCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
        EPIC_LOG(Log::LogLevel::DEBUG, "Active Country Code: %s", buff);

        size = sizeof(buff);
        memset(buff, 0, size);
        EOS_Platform_GetActiveLocaleCode(hPlatform, cbinfo->LocalUserId, buff, &size);
        EPIC_LOG(Log::LogLevel::DEBUG, "Active Locale Code: %s", buff);

        size = sizeof(buff);
        memset(buff, 0, size);
        EOS_Platform_GetOverrideCountryCode(hPlatform, buff, &size);
        EPIC_LOG(Log::LogLevel::DEBUG, "Override Country Code: %s", buff);

        size = sizeof(buff);
        memset(buff, 0, size);
        EOS_Platform_GetOverrideLocaleCode(hPlatform, buff, &size);
        EPIC_LOG(Log::LogLevel::DEBUG, "Override Locale Code: %s", buff);

        const_cast<EOS_Auth_LoginCallbackInfo*>(cbinfo)->ClientData = wrapper->ClientData;
        wrapper->CbFunc((void*)cbinfo);
        delete wrapper;
    };

    return _EOS_Auth_Login(Handle, Options, wrapper, f);
}

EOS_DECLARE_FUNC(void) EOS_Auth_Logout(EOS_HAuth Handle, const EOS_Auth_LogoutOptions* Options, void* ClientData, const EOS_Auth_OnLogoutCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_Logout);
    return _EOS_Auth_Logout(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Auth_DeletePersistentAuth(EOS_HAuth Handle, const EOS_Auth_DeletePersistentAuthOptions* Options, void* ClientData, const EOS_Auth_OnDeletePersistentAuthCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_DeletePersistentAuth);
    return _EOS_Auth_DeletePersistentAuth(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Auth_VerifyUserAuth(EOS_HAuth Handle, const EOS_Auth_VerifyUserAuthOptions* Options, void* ClientData, const EOS_Auth_OnVerifyUserAuthCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_VerifyUserAuth);
    return _EOS_Auth_VerifyUserAuth(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(int32_t) EOS_Auth_GetLoggedInAccountsCount(EOS_HAuth Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_GetLoggedInAccountsCount);
    return _EOS_Auth_GetLoggedInAccountsCount(Handle);
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Auth_GetLoggedInAccountByIndex(EOS_HAuth Handle, int32_t Index)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_GetLoggedInAccountByIndex);
    return _EOS_Auth_GetLoggedInAccountByIndex(Handle, Index);
}

EOS_DECLARE_FUNC(EOS_ELoginStatus) EOS_Auth_GetLoginStatus(EOS_HAuth Handle, EOS_EpicAccountId LocalUserId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_GetLoginStatus);
    return _EOS_Auth_GetLoginStatus(Handle, LocalUserId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenOld(EOS_HAuth Handle, EOS_AccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    static decltype(EOS_Auth_CopyUserAuthTokenOld)* _EOS_Auth_CopyUserAuthToken = (decltype(_EOS_Auth_CopyUserAuthToken))GET_PROC_ADDRESS(original_dll, "EOS_Auth_CopyUserAuthToken");
    auto res = _EOS_Auth_CopyUserAuthToken(Handle, LocalUserId, OutUserAuthToken);

    std::stringstream sstr;
    sstr << std::endl;
    char buff[EOS_EPICACCOUNTID_MAX_LENGTH + 1] = {};
    int32_t len = EOS_EPICACCOUNTID_MAX_LENGTH + 1;
    EOS_EpicAccountId_ToString(LocalUserId, buff, &len);

    sstr << "ApiVersion : None" << std::endl;
    sstr << "LocalUserId: " << buff << std::endl;

    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion           : " << (*OutUserAuthToken)->ApiVersion << std::endl;
        switch ((*OutUserAuthToken)->ApiVersion)
        {
            case EOS_AUTH_TOKEN_API_002:
            {
                EOS_Auth_Token002* p = (EOS_Auth_Token002*)*OutUserAuthToken;
                sstr << "  RefreshToken    :" << p->RefreshToken << std::endl;
                sstr << "  RefreshExpiresIn:" << p->RefreshExpiresIn << std::endl;
                sstr << "  RefreshExpiresAt:" << p->RefreshExpiresAt << std::endl;
            }
            case EOS_AUTH_TOKEN_API_001:
            {
                EOS_Auth_Token001* p = (EOS_Auth_Token001*)*OutUserAuthToken;
                sstr << "  App            :" << p->App << std::endl;
                sstr << "  ClientId       :" << p->ClientId << std::endl;
                sstr << "  AccessToken    :" << p->AccessToken << std::endl;
                sstr << "  ExpiresIn      :" << p->ExpiresIn << std::endl;
                sstr << "  ExpiresAt      :" << p->ExpiresAt << std::endl;
                sstr << "  AuthType       :" << p->AuthType << std::endl;
            }
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenNew(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    static decltype(EOS_Auth_CopyUserAuthTokenNew)* _EOS_Auth_CopyUserAuthToken = (decltype(_EOS_Auth_CopyUserAuthToken))GET_PROC_ADDRESS(original_dll, "EOS_Auth_CopyUserAuthToken");
    auto res = _EOS_Auth_CopyUserAuthToken(Handle, Options, LocalUserId, OutUserAuthToken);

    std::stringstream sstr;
    sstr << std::endl;
    char buff[EOS_EPICACCOUNTID_MAX_LENGTH + 1] = {};
    int32_t len = EOS_EPICACCOUNTID_MAX_LENGTH + 1;
    EOS_EpicAccountId_ToString(LocalUserId, buff, &len);

    sstr << "ApiVersion : " << Options->ApiVersion << std::endl;
    sstr << "LocalUserId: " << buff << std::endl;

    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion           : " << (*OutUserAuthToken)->ApiVersion << std::endl;
        switch ((*OutUserAuthToken)->ApiVersion)
        {
            case EOS_AUTH_TOKEN_API_002:
            {
                EOS_Auth_Token002* p = (EOS_Auth_Token002*)*OutUserAuthToken;
                sstr << "  RefreshToken    :" << p->RefreshToken << std::endl;
                sstr << "  RefreshExpiresIn:" << p->RefreshExpiresIn << std::endl;
                sstr << "  RefreshExpiresAt:" << p->RefreshExpiresAt << std::endl;
            }
            case EOS_AUTH_TOKEN_API_001:
            {
                EOS_Auth_Token001* p = (EOS_Auth_Token001*)*OutUserAuthToken;
                sstr << "  App            :" << p->App << std::endl;
                sstr << "  ClientId       :" << p->ClientId << std::endl;
                sstr << "  AccessToken    :" << p->AccessToken << std::endl;
                sstr << "  ExpiresIn      :" << p->ExpiresIn << std::endl;
                sstr << "  ExpiresAt      :" << p->ExpiresAt << std::endl;
                sstr << "  AuthType       :" << p->AuthType << std::endl;
            }
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
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
    EPIC_LOG(Log::LogLevel::TRACE, "");
    static decltype(EOS_Auth_AddNotifyLoginStatusChangedOld)* _EOS_Auth_AddNotifyLoginStatusChanged = (decltype(_EOS_Auth_AddNotifyLoginStatusChanged))GET_PROC_ADDRESS(original_dll, "EOS_Auth_AddNotifyLoginStatusChanged");
    return _EOS_Auth_AddNotifyLoginStatusChanged(Handle, ClientData, Notification);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedNew(EOS_HAuth Handle, const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    static decltype(EOS_Auth_AddNotifyLoginStatusChangedNew)* _EOS_Auth_AddNotifyLoginStatusChanged = (decltype(_EOS_Auth_AddNotifyLoginStatusChanged))GET_PROC_ADDRESS(original_dll, "EOS_Auth_AddNotifyLoginStatusChanged");
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
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_RemoveNotifyLoginStatusChanged);
    return _EOS_Auth_RemoveNotifyLoginStatusChanged(Handle, InId);
}


EOS_DECLARE_FUNC(void) EOS_Auth_Token_Release(EOS_Auth_Token* AuthToken)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Auth_Token_Release);
    return _EOS_Auth_Token_Release(AuthToken);
}

////////////////////////////////////////////
// achievements
EOS_DECLARE_FUNC(void) EOS_Achievements_QueryDefinitions(EOS_HAchievements Handle, const EOS_Achievements_QueryDefinitionsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryDefinitionsCompleteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_QueryDefinitions);
    return _EOS_Achievements_QueryDefinitions(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetAchievementDefinitionCount(EOS_HAchievements Handle, const EOS_Achievements_GetAchievementDefinitionCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_GetAchievementDefinitionCount);
    return _EOS_Achievements_GetAchievementDefinitionCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionByIndexOptions* Options, EOS_Achievements_Definition** OutDefinition)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
                sstr << "  AchievementId        : " << str_or_empty((*OutDefinition)->AchievementId        ) << std::endl;
                sstr << "  DisplayName          : " << str_or_empty((*OutDefinition)->DisplayName          ) << std::endl;
                sstr << "  Description          : " << str_or_empty((*OutDefinition)->Description          ) << std::endl;
                sstr << "  LockedDisplayName    : " << str_or_empty((*OutDefinition)->LockedDisplayName    ) << std::endl;
                sstr << "  LockedDescription    : " << str_or_empty((*OutDefinition)->LockedDescription    ) << std::endl;
                sstr << "  HiddenDescription    : " << str_or_empty((*OutDefinition)->HiddenDescription    ) << std::endl;
                sstr << "  CompletionDescription: " << str_or_empty((*OutDefinition)->CompletionDescription) << std::endl;
                sstr << "  UnlockedIconId       : " << str_or_empty((*OutDefinition)->UnlockedIconId       ) << std::endl;
                sstr << "  LockedIconId         : " << str_or_empty((*OutDefinition)->LockedIconId         ) << std::endl;
                sstr << "  bIsHidden            : " << EOS_Bool_2_str((*OutDefinition)->bIsHidden)           << std::endl;

                for (int i = 0; i < (*OutDefinition)->StatThresholdsCount; ++i)
                {
                    sstr << "  StatThresholds[" << i << "]" << std::endl;
                    sstr << "    Name     : " << str_or_empty((*OutDefinition)->StatThresholds[i].Name) << std::endl;
                    sstr << "    Threshold: " << (*OutDefinition)->StatThresholds[i].Threshold << std::endl;
                }
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionV2ByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions* Options, EOS_Achievements_DefinitionV2** OutDefinition)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
            case EOS_ACHIEVEMENTS_DEFINITIONV2_API_002:
            {
                auto* v = reinterpret_cast<EOS_Achievements_DefinitionV2002*>(*OutDefinition);
                sstr << "  AchievementId        : " << str_or_empty(v->AchievementId      ) << std::endl;
                sstr << "  UnlockedDisplayName  : " << str_or_empty(v->UnlockedDisplayName) << std::endl;
                sstr << "  UnlockedDescription  : " << str_or_empty(v->UnlockedDescription) << std::endl;
                sstr << "  LockedDisplayName    : " << str_or_empty(v->LockedDisplayName  ) << std::endl;
                sstr << "  LockedDescription    : " << str_or_empty(v->LockedDescription  ) << std::endl;
                sstr << "  FlavorText           : " << str_or_empty(v->FlavorText         ) << std::endl;
                sstr << "  UnlockedIconURL      : " << str_or_empty(v->UnlockedIconURL    ) << std::endl;
                sstr << "  LockedIconURL        : " << str_or_empty(v->LockedIconURL      ) << std::endl;
                sstr << "  bIsHidden            : " << EOS_Bool_2_str(v->bIsHidden)         << std::endl;

                for (int i = 0; i < v->StatThresholdsCount; ++i)
                {
                    sstr << "  StatThresholds[" << i << "]" << std::endl;
                    sstr << "    Name     : " << str_or_empty(v->StatThresholds[i].Name) << std::endl;
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
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionByAchievementIdOptions* Options, EOS_Achievements_Definition** OutDefinition)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
                sstr << "  AchievementId        : " << str_or_empty(v->AchievementId        ) << std::endl;
                sstr << "  DisplayName          : " << str_or_empty(v->DisplayName          ) << std::endl;
                sstr << "  Description          : " << str_or_empty(v->Description          ) << std::endl;
                sstr << "  LockedDisplayName    : " << str_or_empty(v->LockedDisplayName    ) << std::endl;
                sstr << "  LockedDescription    : " << str_or_empty(v->LockedDescription    ) << std::endl;
                sstr << "  HiddenDescription    : " << str_or_empty(v->HiddenDescription    ) << std::endl;
                sstr << "  CompletionDescription: " << str_or_empty(v->CompletionDescription) << std::endl;
                sstr << "  UnlockedIconId       : " << str_or_empty(v->UnlockedIconId       ) << std::endl;
                sstr << "  LockedIconId         : " << str_or_empty(v->LockedIconId         ) << std::endl;
                sstr << "  bIsHidden            : " << EOS_Bool_2_str(v->bIsHidden)           << std::endl;

                for (int i = 0; i < v->StatThresholdsCount; ++i)
                {
                    sstr << "  Threshold[" << i << "]" << std::endl;
                    sstr << "    Name     : " << str_or_empty(v->StatThresholds[i].Name) << std::endl;
                    sstr << "    Threshold: " << v->StatThresholds[i].Threshold << std::endl;
                }
            }
            break;
        }

        save_json(achievements_db_file, achievements_db);
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionV2ByAchievementIdOptions* Options, EOS_Achievements_DefinitionV2** OutDefinition)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
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
            case EOS_ACHIEVEMENTS_DEFINITIONV2_API_LATEST:
            {
                auto* v = reinterpret_cast<EOS_Achievements_DefinitionV2002*>(*OutDefinition);
                sstr << "  AchievementId        : " << str_or_empty(v->AchievementId      ) << std::endl;
                sstr << "  UnlockedDisplayName  : " << str_or_empty(v->UnlockedDisplayName) << std::endl;
                sstr << "  UnlockedDescription  : " << str_or_empty(v->UnlockedDescription) << std::endl;
                sstr << "  LockedDisplayName    : " << str_or_empty(v->LockedDisplayName  ) << std::endl;
                sstr << "  LockedDescription    : " << str_or_empty(v->LockedDescription  ) << std::endl;
                sstr << "  FlavorText           : " << str_or_empty(v->FlavorText         ) << std::endl;
                sstr << "  UnlockedIconURL      : " << str_or_empty(v->UnlockedIconURL    ) << std::endl;
                sstr << "  LockedIconURL        : " << str_or_empty(v->LockedIconURL      ) << std::endl;
                sstr << "  bIsHidden            : " << EOS_Bool_2_str(v->bIsHidden)         << std::endl;

                for (int i = 0; i < v->StatThresholdsCount; ++i)
                {
                    sstr << "  Threshold[" << i << "]" << std::endl;
                    sstr << "    Name     : " << str_or_empty(v->StatThresholds[i].Name) << std::endl;
                    sstr << "    Threshold: " << v->StatThresholds[i].Threshold << std::endl;
                }
            }
            break;
        }

        save_json(achievements_db_file, achievements_db);
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(void) EOS_Achievements_QueryPlayerAchievements(EOS_HAchievements Handle, const EOS_Achievements_QueryPlayerAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_QueryPlayerAchievements);
    return _EOS_Achievements_QueryPlayerAchievements(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetPlayerAchievementCount(EOS_HAchievements Handle, const EOS_Achievements_GetPlayerAchievementCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_GetPlayerAchievementCount);
    return _EOS_Achievements_GetPlayerAchievementCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyPlayerAchievementByIndexOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyPlayerAchievementByIndex);
    auto res = _EOS_Achievements_CopyPlayerAchievementByIndex(Handle, Options, OutAchievement);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion           : " << Options->ApiVersion << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion           : " << (*OutAchievement)->ApiVersion << std::endl;
        switch ((*OutAchievement)->ApiVersion)
        {
            case EOS_ACHIEVEMENTS_PLAYERACHIEVEMENT_API_002:
            {
                auto* v = reinterpret_cast<EOS_Achievements_PlayerAchievement002*>(*OutAchievement);

                sstr << "  DisplayName  : " << str_or_empty(v->DisplayName) << std::endl;
                sstr << "  Description  : " << str_or_empty(v->Description) << std::endl;
                sstr << "  IconURL      : " << str_or_empty(v->IconURL    ) << std::endl;
                sstr << "  FlavorText   : " << str_or_empty(v->FlavorText ) << std::endl;
            }

            case EOS_ACHIEVEMENTS_PLAYERACHIEVEMENT_API_001:
            {
                auto* v = reinterpret_cast<EOS_Achievements_PlayerAchievement001*>(*OutAchievement);
                sstr << "  AchievementId: " << str_or_empty(v->AchievementId) << std::endl;
                sstr << "  Progress     : " << v->Progress << std::endl;
                sstr << "  UnlockTime   : " << v->UnlockTime << std::endl;

                for (int i = 0; i < v->StatInfoCount; ++i)
                {
                    sstr << "  StatInfo[" << i << "]:" << std::endl;
                    switch (v->StatInfo[i].ApiVersion)
                    {
                        case EOS_ACHIEVEMENTS_PLAYERSTATINFO_API_001:
                        {
                            sstr << "    Name          :" << str_or_empty(v->StatInfo[i].Name) << std::endl;
                            sstr << "    CurrentValue  :" << v->StatInfo[i].CurrentValue << std::endl;
                            sstr << "    ThresholdValue:" << v->StatInfo[i].ThresholdValue << std::endl;
                        }
                    }
                }
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());
    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyPlayerAchievementByAchievementId);
    auto res = _EOS_Achievements_CopyPlayerAchievementByAchievementId(Handle, Options, OutAchievement);

    std::stringstream sstr;
    sstr << std::endl;
    sstr << "ApiVersion           : " << Options->ApiVersion << std::endl;
    if (res == EOS_EResult::EOS_Success)
    {
        sstr << "  ApiVersion           : " << (*OutAchievement)->ApiVersion << std::endl;
        switch ((*OutAchievement)->ApiVersion)
        {
            case EOS_ACHIEVEMENTS_PLAYERACHIEVEMENT_API_002:
            {
                auto* v = reinterpret_cast<EOS_Achievements_PlayerAchievement002*>(*OutAchievement);

                sstr << "  DisplayName  : " << str_or_empty(v->DisplayName) << std::endl;
                sstr << "  Description  : " << str_or_empty(v->Description) << std::endl;
                sstr << "  IconURL      : " << str_or_empty(v->IconURL    ) << std::endl;
                sstr << "  FlavorText   : " << str_or_empty(v->FlavorText ) << std::endl;
            }

            case EOS_ACHIEVEMENTS_PLAYERACHIEVEMENT_API_001:
            {
                auto* v = reinterpret_cast<EOS_Achievements_PlayerAchievement001*>(*OutAchievement);
                sstr << "  AchievementId: " << str_or_empty(v->AchievementId) << std::endl;
                sstr << "  Progress     : " << v->Progress << std::endl;
                sstr << "  UnlockTime   : " << v->UnlockTime << std::endl;

                for (int i = 0; i < v->StatInfoCount; ++i)
                {
                    sstr << "  StatInfo[" << i << "]:" << std::endl;
                    switch (v->StatInfo[i].ApiVersion)
                    {
                        case EOS_ACHIEVEMENTS_PLAYERSTATINFO_API_001:
                        {
                            sstr << "    Name          :" << str_or_empty(v->StatInfo[i].Name) << std::endl;
                            sstr << "    CurrentValue  :" << v->StatInfo[i].CurrentValue << std::endl;
                            sstr << "    ThresholdValue:" << v->StatInfo[i].ThresholdValue << std::endl;
                        }
                    }
                }
            }
            break;
        }
    }
    else
    {
        sstr << "Failed" << std::endl;
    }
    EPIC_LOG(Log::LogLevel::DEBUG, "%s", sstr.str().c_str());

    return res;
}

EOS_DECLARE_FUNC(void) EOS_Achievements_UnlockAchievements(EOS_HAchievements Handle, const EOS_Achievements_UnlockAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnUnlockAchievementsCompleteCallback CompletionDelegate)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_UnlockAchievements);
    return _EOS_Achievements_UnlockAchievements(Handle, Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetUnlockedAchievementCount(EOS_HAchievements Handle, const EOS_Achievements_GetUnlockedAchievementCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_GetUnlockedAchievementCount);
    return _EOS_Achievements_GetUnlockedAchievementCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyUnlockedAchievementByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyUnlockedAchievementByIndexOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyUnlockedAchievementByIndex);
    auto res = _EOS_Achievements_CopyUnlockedAchievementByIndex(Handle, Options, OutAchievement);

    if (res == EOS_EResult::EOS_Success)
    {
        EPIC_LOG(Log::LogLevel::DEBUG, "Achievement index: %d = %s %ll", Options->AchievementIndex, (*OutAchievement)->AchievementId, (*OutAchievement)->UnlockTime);
    }

    return res;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyUnlockedAchievementByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyUnlockedAchievementByAchievementIdOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_CopyUnlockedAchievementByAchievementId);
    auto res = _EOS_Achievements_CopyUnlockedAchievementByAchievementId(Handle, Options, OutAchievement);

    if (res == EOS_EResult::EOS_Success)
    {
        EPIC_LOG(Log::LogLevel::DEBUG, "Achievement id: %s = %ll", (*OutAchievement)->AchievementId, (*OutAchievement)->UnlockTime);
    }

    return res;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlocked(EOS_HAchievements Handle, const EOS_Achievements_AddNotifyAchievementsUnlockedOptions* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallback NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_AddNotifyAchievementsUnlocked);
    return _EOS_Achievements_AddNotifyAchievementsUnlocked(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlockedV2(EOS_HAchievements Handle, const EOS_Achievements_AddNotifyAchievementsUnlockedV2Options* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallbackV2 NotificationFn)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_AddNotifyAchievementsUnlockedV2);
    return _EOS_Achievements_AddNotifyAchievementsUnlockedV2(Handle, Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_RemoveNotifyAchievementsUnlocked(EOS_HAchievements Handle, EOS_NotificationId InId)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_RemoveNotifyAchievementsUnlocked);
    return _EOS_Achievements_RemoveNotifyAchievementsUnlocked(Handle, InId);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_Definition_Release(EOS_Achievements_Definition* AchievementDefinition)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_Definition_Release);
    return _EOS_Achievements_Definition_Release(AchievementDefinition);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_DefinitionV2_Release(EOS_Achievements_DefinitionV2* AchievementDefinition)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_DefinitionV2_Release);
    return _EOS_Achievements_DefinitionV2_Release(AchievementDefinition);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_PlayerAchievement_Release(EOS_Achievements_PlayerAchievement* Achievement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_PlayerAchievement_Release);
    return _EOS_Achievements_PlayerAchievement_Release(Achievement);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_UnlockedAchievement_Release(EOS_Achievements_UnlockedAchievement* Achievement)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_Achievements_UnlockedAchievement_Release);
    return _EOS_Achievements_UnlockedAchievement_Release(Achievement);
}

////////////////////////////////////////////
// titlestorage
EOS_DECLARE_FUNC(void) EOS_TitleStorage_QueryFile(EOS_HTitleStorage Handle, const EOS_TitleStorage_QueryFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileCompleteCallback CompletionCallback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorage_QueryFile);
    return _EOS_TitleStorage_QueryFile(Handle, Options, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(void) EOS_TitleStorage_QueryFileList(EOS_HTitleStorage Handle, const EOS_TitleStorage_QueryFileListOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileListCompleteCallback CompletionCallback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorage_QueryFileList);
    return _EOS_TitleStorage_QueryFileList(Handle, Options, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_CopyFileMetadataByFilename(EOS_HTitleStorage Handle, const EOS_TitleStorage_CopyFileMetadataByFilenameOptions* Options, EOS_TitleStorage_FileMetadata** OutMetadata)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorage_CopyFileMetadataByFilename);
    return _EOS_TitleStorage_CopyFileMetadataByFilename(Handle, Options, OutMetadata);
}

EOS_DECLARE_FUNC(uint32_t) EOS_TitleStorage_GetFileMetadataCount(EOS_HTitleStorage Handle, const EOS_TitleStorage_GetFileMetadataCountOptions* Options)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorage_GetFileMetadataCount);
    return _EOS_TitleStorage_GetFileMetadataCount(Handle, Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_CopyFileMetadataAtIndex(EOS_HTitleStorage Handle, const EOS_TitleStorage_CopyFileMetadataAtIndexOptions* Options, EOS_TitleStorage_FileMetadata** OutMetadata)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorage_CopyFileMetadataAtIndex);
    return _EOS_TitleStorage_CopyFileMetadataAtIndex(Handle, Options, OutMetadata);
}

EOS_DECLARE_FUNC(EOS_HTitleStorageFileTransferRequest) EOS_TitleStorage_ReadFile(EOS_HTitleStorage Handle, const EOS_TitleStorage_ReadFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnReadFileCompleteCallback CompletionCallback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorage_ReadFile);
    return _EOS_TitleStorage_ReadFile(Handle, Options, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_DeleteCache(EOS_HTitleStorage Handle, const EOS_TitleStorage_DeleteCacheOptions* Options, void* ClientData, const EOS_TitleStorage_OnDeleteCacheCompleteCallback CompletionCallback)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorage_DeleteCache);
    return _EOS_TitleStorage_DeleteCache(Handle, Options, ClientData, CompletionCallback);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_GetFileRequestState(EOS_HTitleStorageFileTransferRequest Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorageFileTransferRequest_GetFileRequestState);
    return _EOS_TitleStorageFileTransferRequest_GetFileRequestState(Handle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_GetFilename(EOS_HTitleStorageFileTransferRequest Handle, uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorageFileTransferRequest_GetFilename);
    return _EOS_TitleStorageFileTransferRequest_GetFilename(Handle, FilenameStringBufferSizeBytes, OutStringBuffer, OutStringLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_CancelRequest(EOS_HTitleStorageFileTransferRequest Handle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorageFileTransferRequest_CancelRequest);
    return _EOS_TitleStorageFileTransferRequest_CancelRequest(Handle);
}

EOS_DECLARE_FUNC(void) EOS_TitleStorage_FileMetadata_Release(EOS_TitleStorage_FileMetadata* FileMetadata)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorage_FileMetadata_Release);
    return _EOS_TitleStorage_FileMetadata_Release(FileMetadata);
}

EOS_DECLARE_FUNC(void) EOS_TitleStorageFileTransferRequest_Release(EOS_HTitleStorageFileTransferRequest TitleStorageFileTransferHandle)
{
    EPIC_LOG(Log::LogLevel::TRACE, "");
    ORIGINAL_FUNCTION(EOS_TitleStorageFileTransferRequest_Release);
    return _EOS_TitleStorageFileTransferRequest_Release(TitleStorageFileTransferHandle);
}