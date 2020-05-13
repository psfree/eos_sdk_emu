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

#include "os_funcs.h"

using namespace PortableAPI;

static void* hmodule;

LOCAL_API std::string get_path_folder(std::string const& path)
{
    return std::string(path.begin(), path.begin() + path.find_last_of(PATH_SEPARATOR));
}

LOCAL_API std::chrono::microseconds get_uptime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - get_boottime());
}

#if defined(__WINDOWS__)

#include "../overlay/Base_Hook.h"

decltype(CreateProcessA)* _CreateProcessA = CreateProcessA;
decltype(CreateProcessW)* _CreateProcessW = CreateProcessW;

BOOL WINAPI MyCreateProcessA(
    _In_opt_    LPCSTR lpApplicationName,
    _Inout_opt_ LPSTR lpCommandLine,
    _In_opt_    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    _In_opt_    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_        BOOL bInheritHandles,
    _In_        DWORD dwCreationFlags,
    _In_opt_    LPVOID lpEnvironment,
    _In_opt_    LPCSTR lpCurrentDirectory,
    _In_        LPSTARTUPINFOA lpStartupInfo,
    _Out_       LPPROCESS_INFORMATION lpProcessInformation)
{
    return _CreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

BOOL WINAPI MyCreateProcessW(
    _In_opt_    LPCWSTR lpApplicationName,
    _Inout_opt_ LPWSTR lpCommandLine,
    _In_opt_    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    _In_opt_    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_        BOOL bInheritHandles,
    _In_        DWORD dwCreationFlags,
    _In_opt_    LPVOID lpEnvironment,
    _In_opt_    LPCWSTR lpCurrentDirectory,
    _In_        LPSTARTUPINFOW lpStartupInfo,
    _Out_       LPPROCESS_INFORMATION lpProcessInformation
)
{
    return _CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

static bool is_lan_ip(const sockaddr* addr, int namelen)
{
    if (!namelen) return false;

    if (addr->sa_family == AF_INET) {
        struct sockaddr_in* addr_in = (struct sockaddr_in*)addr;
        unsigned char ip[4];
        memcpy(ip, &addr_in->sin_addr, sizeof(ip));
        //LOG(Log::LogLevel::DEBUG, "CHECK LAN IP %hhu.%hhu.%hhu.%hhu:%u", ip[0], ip[1], ip[2], ip[3], ntohs(addr_in->sin_port));
        if (ip[0] == 127) return true;
        if (ip[0] == 10) return true;
        if (ip[0] == 192 && ip[1] == 168) return true;
        if (ip[0] == 169 && ip[1] == 254 && ip[2] != 0) return true;
        if (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) return true;
        if ((ip[0] == 100) && ((ip[1] & 0xC0) == 0x40)) return true;
        if (ip[0] == 239) return true; //multicast
        if (ip[0] == 0) return true; //Current network
        if (ip[0] == 192 && (ip[1] == 18 || ip[1] == 19)) return true; //Used for benchmark testing of inter-network communications between two separate subnets.
        if (ip[0] >= 224) return true; //ip multicast (224 - 239) future use (240.0.0.0–255.255.255.254) broadcast (255.255.255.255)
    }
    else if (addr->sa_family == AF_INET6) {
        struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)addr;
        unsigned char ip[16];
        unsigned char zeroes[16] = {};
        memcpy(ip, &addr_in6->sin6_addr, sizeof(ip));
        //LOG(Log::LogLevel::DEBUG, "CHECK LAN IP6 %hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu...%hhu", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7], ip[15]);
        if (((ip[0] == 0xFF) && (ip[1] < 3) && (ip[15] == 1)) ||
            ((ip[0] == 0xFE) && ((ip[1] & 0xC0) == 0x80))) return true;
        if (memcmp(zeroes, ip, sizeof(ip)) == 0) return true;
        if (memcmp(zeroes, ip, sizeof(ip) - 1) == 0 && ip[15] == 1) return true;
        if (ip[0] == 0xff) return true; //multicast
        if (ip[0] == 0xfc) return true; //unique local
        if (ip[0] == 0xfd) return true; //unique local
        //TODO: ipv4 mapped?
    }

    return false;
}

int (WINAPI* _sendto)(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen) = sendto;
int (WINAPI* _connect)(SOCKET s, const sockaddr* addr, int namelen) = connect;
int (WINAPI* _WSAConnect)(SOCKET s, const sockaddr* addr, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS) = WSAConnect;

static int WINAPI Mysendto(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
    if (is_lan_ip(to, tolen)) {
        return _sendto(s, buf, len, flags, to, tolen);
    }
    else {
        return len;
    }
}

static int WINAPI Myconnect(SOCKET s, const sockaddr* addr, int namelen)
{
    if (is_lan_ip(addr, namelen)) {
        return _connect(s, addr, namelen);
    }
    else {
        WSASetLastError(WSAECONNREFUSED);
        return SOCKET_ERROR;
    }
}

static int WINAPI MyWSAConnect(SOCKET s, const sockaddr* addr, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS)
{
    if (is_lan_ip(addr, namelen)) {
        return _WSAConnect(s, addr, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);
    }
    else {
        WSASetLastError(WSAECONNREFUSED);
        return SOCKET_ERROR;
    }
}

#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")
decltype(WinHttpOpenRequest)* _WinHttpOpenRequest = WinHttpOpenRequest;
decltype(WinHttpConnect)* _WinHttpConnect = WinHttpConnect;

HINTERNET WINAPI MyWinHttpConnect(
    IN HINTERNET     hSession,
    IN LPCWSTR       pswzServerName,
    IN INTERNET_PORT nServerPort,
    IN DWORD         dwReserved
) {
    struct sockaddr_in ip4;
    struct sockaddr_in6 ip6;
    ip4.sin_family = AF_INET;
    ip6.sin6_family = AF_INET6;

    if ((InetPtonW(AF_INET, pswzServerName, &(ip4.sin_addr)) && is_lan_ip((sockaddr*)&ip4, sizeof(ip4))) || (InetPtonW(AF_INET6, pswzServerName, &(ip6.sin6_addr)) && is_lan_ip((sockaddr*)&ip6, sizeof(ip6)))) {
        return _WinHttpConnect(hSession, pswzServerName, nServerPort, dwReserved);
    }
    else {
        return _WinHttpConnect(hSession, L"127.1.33.7", nServerPort, dwReserved);
    }
}

HINTERNET WINAPI MyWinHttpOpenRequest(
    IN HINTERNET hConnect,
    IN LPCWSTR   pwszVerb,
    IN LPCWSTR   pwszObjectName,
    IN LPCWSTR   pwszVersion,
    IN LPCWSTR   pwszReferrer,
    IN LPCWSTR* ppwszAcceptTypes,
    IN DWORD     dwFlags
) {
    if (dwFlags & WINHTTP_FLAG_SECURE) {
        dwFlags &= ~(WINHTTP_FLAG_SECURE);
    }

    return _WinHttpOpenRequest(hConnect, pwszVerb, pwszObjectName, pwszVersion, pwszReferrer, ppwszAcceptTypes, dwFlags);
}

Base_Hook debug;

void shared_library_load(void* hmodule)
{
    ::hmodule = hmodule;

    debug.BeginHook();

    debug.HookFuncs(
        std::make_pair<void**, void*>((void**)&_CreateProcessA, (void*)MyCreateProcessA),
        std::make_pair<void**, void*>((void**)&_CreateProcessW, (void*)MyCreateProcessW),
        std::make_pair<void**, void*>((void**)&_WinHttpOpenRequest, (void*)MyWinHttpOpenRequest),
        std::make_pair<void**, void*>((void**)&_WinHttpConnect, (void*)MyWinHttpConnect),
        std::make_pair<void**, void*>((void**)&_sendto, (void*)Mysendto),
        std::make_pair<void**, void*>((void**)&_connect, (void*)Myconnect),
        std::make_pair<void**, void*>((void**)&_WSAConnect, (void*)MyWSAConnect)
    );

    debug.EndHook();

    std::fstream log("cmdline.txt", std::ios::out | std::ios::trunc);
    log << GetCommandLine();

    Socket::InitSocket();
}

void shared_library_unload(void* hmodule)
{

}

LOCAL_API std::chrono::system_clock::time_point get_boottime()
{
    static std::chrono::system_clock::time_point boottime(std::chrono::system_clock::now() - std::chrono::milliseconds(GetTickCount64()));
    return boottime;
}

LOCAL_API std::string process_path(std::string const& path)
{
    std::string canonicalized_path(path);
    size_t pos;
    size_t size;

    std::replace(canonicalized_path.begin(), canonicalized_path.end(), '/', '\\');

    while ((pos = canonicalized_path.find("\\\\")) != std::string::npos)
        canonicalized_path.replace(pos, 2, "\\");

    while ((pos = canonicalized_path.find("\\.\\")) != std::string::npos)
        canonicalized_path.replace(pos, 3, "\\");

    while ((pos = canonicalized_path.find("\\..")) != std::string::npos)
    {
        if (pos == 0)
            size = 3;
        else
        {
            size_t parent_pos = canonicalized_path.rfind("\\", pos - 1);
            if (parent_pos == std::string::npos)
            {
                size = pos + 3;
                pos = 0;
            }
            else
            {
                size = 3 + pos - parent_pos;
                pos = parent_pos;
            }
        }

        canonicalized_path.replace(pos, size, "");
    }

    while ((pos = canonicalized_path.find("\\.")) != std::string::npos)
        canonicalized_path.replace(pos, 2, "");

    return canonicalized_path;
}

LOCAL_API std::string canonical_path(std::string const& path)
{
    WCHAR pathout[4096];

    std::wstring wide;
    utf8::utf8to16(path.begin(), path.end(), std::back_inserter(wide));

    DWORD ret = GetFullPathNameW(wide.c_str(), sizeof(pathout) / sizeof(*pathout), pathout, nullptr);
    if (ret > 0)
    {
        std::string res;
        utf8::utf16to8(pathout, pathout + ret, std::back_inserter(res));
        return res;
    }

    throw std::exception("Failed retrieve canonincal path\n");
}

LOCAL_API std::string get_env_var(std::string const& var)
{
    WCHAR env_variable[1024];

    std::wstring wide;
    utf8::utf8to16(var.begin(), var.end(), std::back_inserter(wide));

    DWORD ret = GetEnvironmentVariableW(wide.c_str(), env_variable, sizeof(env_variable)/sizeof(*env_variable));
    if (ret <= 0)
        return std::string();

    env_variable[ret] = 0;
    std::string res;
    utf8::utf16to8(env_variable, env_variable+ret, std::back_inserter(res));
    return res;
}

LOCAL_API std::string get_userdata_path()
{
    std::string user_appdata_path;
    CHAR szPath[MAX_PATH] = {};

    HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, szPath);

    if (SUCCEEDED(hr))
        user_appdata_path = szPath;

    return user_appdata_path;
}

LOCAL_API std::string get_executable_path()
{
    std::string exec_path;

    char* pgm_path;
    _get_pgmptr(&pgm_path);

    exec_path = pgm_path;
    exec_path = exec_path.substr(0, exec_path.rfind(PATH_SEPARATOR) + 1);
    LOG(Log::LogLevel::INFO, "%s", exec_path.c_str());
    return exec_path;
}

LOCAL_API std::string get_module_path()
{
    std::string program_path;
    char DllPath[MAX_PATH] = { 0 };
    GetModuleFileNameA((HINSTANCE)hmodule, DllPath, MAX_PATH-1);
    program_path = DllPath;
    program_path = program_path.substr(0, program_path.rfind(PATH_SEPARATOR)+1);
    return program_path;
}

LOCAL_API std::vector<ipv4_addr> get_broadcasts()
{
    IP_ADAPTER_INFO* pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    unsigned long ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    std::vector<ipv4_addr> broadcasts;

    ipv4_addr addr, netmask;
    addr.set_addr(ipv4_addr::broadcast_addr);

    broadcasts.push_back(addr);

    if (pAdapterInfo == nullptr)
        return broadcasts;

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);

        if (pAdapterInfo == nullptr)
            return broadcasts;
    }

    int ret;

    if ((ret = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
    {
        IP_ADAPTER_INFO* pAdapter = pAdapterInfo;

        while (pAdapter != nullptr)
        {
            unsigned long iface_ip = 0, subnet_mask = 0;

            netmask.from_string(pAdapter->IpAddressList.IpMask.String);
            addr.from_string(pAdapter->IpAddressList.IpAddress.String);

            int ip = addr.get_ip();
            int mask = netmask.get_ip();
            if (ip != 0)
            {
                addr.set_ip(ip | (~mask));
                broadcasts.push_back(addr);
            }

            pAdapter = pAdapter->Next;
        }
    }

    free(pAdapterInfo);

    return broadcasts;
}

LOCAL_API bool create_folder(std::string const& _folder)
{
    size_t pos = 0;
    struct _stat sb;

    std::wstring sub_dir;
    std::wstring folder;
    utf8::utf8to16(_folder.begin(), _folder.end(), std::back_inserter(folder));
    if (folder.empty())
        return true;

    if (folder.length() >= 3 && folder[1] == ':' && folder[2] == PATH_SEPARATOR)
        pos = 3;

    do
    {
        pos = folder.find_first_of(L"\\/", pos + 1);
        sub_dir = std::move(folder.substr(0, pos));
        if(_wstat(sub_dir.c_str(), &sb) == 0)
        {
            if(!(sb.st_mode & _S_IFDIR))
            {// A subpath in the target is not a folder
                return false;
            }
            // Folder exists
        }
        else if(CreateDirectoryW(folder.substr(0, pos).c_str(), NULL) )
        {// Failed to create folder (no permission?)
        }
    } while (pos != std::string::npos);

    return true;
}

LOCAL_API bool delete_file(std::string const& _path)
{
    std::wstring path;
    utf8::utf8to16(_path.begin(), _path.end(), std::back_inserter(path));
    return DeleteFileW(path.c_str()) == TRUE;
}

static std::vector<std::wstring> list_files(std::wstring const& path, bool recursive)
{
    std::vector<std::wstring> files;
    WIN32_FIND_DATAW hfind_data;
    HANDLE hfind = INVALID_HANDLE_VALUE;

    std::wstring search_path = path;

    if (*path.rbegin() != L'\\')
        search_path += L'\\';

    search_path += L'*';

    // Start iterating over the files in the path directory.
    hfind = FindFirstFileW(search_path.c_str(), &hfind_data);
    if (hfind != INVALID_HANDLE_VALUE)
    {
        search_path.pop_back();
        do // Managed to locate and create an handle to that folder.
        {
            if (wcscmp(L".", hfind_data.cFileName) == 0
                || wcscmp(L"..", hfind_data.cFileName) == 0)
                continue;

            if (hfind_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (recursive)
                {
                    std::wstring dir_name = hfind_data.cFileName;

                    std::vector<std::wstring> sub_files = std::move(list_files(search_path + dir_name, true));
                    std::transform(sub_files.begin(), sub_files.end(), std::back_inserter(files), [&dir_name](std::wstring& file_name)
                    {
                        return dir_name + L'\\' + file_name;
                    });
                }
            }
            else
            {
                files.emplace_back(hfind_data.cFileName);
            }
        } while (FindNextFileW(hfind, &hfind_data) == TRUE);
        FindClose(hfind);
    }

    return files;
}

LOCAL_API std::vector<std::string> list_files(std::string const& path, bool recursive)
{
    std::vector<std::string> files;
    std::wstring wpath;
    utf8::utf8to16(path.begin(), path.end(), std::back_inserter(wpath));
    std::vector<std::wstring> wfiles = std::move(list_files(wpath, recursive));
    
    std::transform(wfiles.begin(), wfiles.end(), std::back_inserter(files), [](std::wstring const& wfile_name)
    {
        std::string file_name;
        utf8::utf16to8(wfile_name.begin(), wfile_name.end(), std::back_inserter(file_name));
        return file_name;
    });

    return files;
}

#elif defined(__LINUX__) || defined(__APPLE__)
#ifdef __LINUX__

LOCAL_API std::chrono::system_clock::time_point get_boottime()
{
    static bool has_boottime = false;
    static std::chrono::system_clock::time_point boottime(std::chrono::seconds(0));
    if (!has_boottime)
    {
        std::ifstream uptime_file("/proc/uptime");

        double uptime;
        if (uptime_file)
        {// Get uptime (millisecond resolution)
            uptime_file >> uptime;
            uptime_file.close();
        }
        else
        {// If we can't open /proc/uptime, fallback to sysinfo (second resolution)
            struct sysinfo infos;
            if (sysinfo(&infos) != 0)
                return boottime;

            uptime = infos.uptime;
        }

        std::chrono::system_clock::time_point now_tp = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point uptime_tp(std::chrono::milliseconds(static_cast<uint64_t>(uptime * 1000)));

        boottime = std::chrono::system_clock::time_point(now_tp - uptime_tp);
        has_boottime = true;
    }

    return boottime;
}

LOCAL_API std::string get_executable_path()
{
    std::string exec_path("./");

    void* hexecutable = dlopen(nullptr, RTLD_NOW);
    if(hexecutable == nullptr)
        return exec_path;


    std::string self = "/proc/self/map_files";
    DIR* dir;
    struct dirent* ep;

    dir = opendir(self.c_str());
    if(dir == nullptr)
    {
        dlclose(hexecutable);
        return exec_path;
    }

    while ((ep = readdir(dir)))
    {
        if (strcmp(ep->d_name, ".")  == 0
         || strcmp(ep->d_name, "..") == 0)
            continue;

        std::string bounds(ep->d_name);
        std::stringstream sstr(bounds);
        size_t pos = bounds.find('-');
        if (pos == std::string::npos)
            continue;

        bounds[pos] = ' ';
        void *start, *end;

        sstr >> start >> end;

        if( start <= hexecutable && hexecutable <= end )
        {
            std::string path = self + PATH_SEPARATOR + ep->d_name;
            char link[1024] = {};
            if (readlink(path.c_str(), link, sizeof(link)) > 0)
            {
                exec_path = link;
                break;
            }
        }
    }

    closedir(dir);
    dlclose(hexecutable);
    
    exec_path = exec_path.substr(0, exec_path.rfind(PATH_SEPARATOR) + 1);
    LOG(Log::LogLevel::INFO, "%s", exec_path.c_str());
    return exec_path;
}

LOCAL_API std::string get_userdata_path()
{
    std::string user_appdata_path;

    /* $XDG_DATA_HOME defines the base directory relative to which user specific data files should be stored.
    If $XDG_DATA_HOME is either not set or empty, a default equal to $HOME/.local/share should be used. */
    char* datadir = getenv("XDG_DATA_HOME");
    if (datadir != nullptr)
    {
        user_appdata_path = datadir;
    }
    else
    {
        char* homedir = getenv("HOME");
        if (homedir != nullptr)
            user_appdata_path = std::move(std::string(homedir) + "/.local/share");
    }
    return user_appdata_path;
}

#else

LOCAL_API std::chrono::system_clock::time_point get_boottime()
{
    static bool has_boottime = false;
    static std::chrono::system_clock::time_point boottime(std::chrono::seconds(0));
    if (!has_boottime)
    {
        struct timeval boottime_tv;
        size_t len = sizeof(boottime_tv);
        int mib[2] = { CTL_KERN, KERN_BOOTTIME };
        if (sysctl(mib, sizeof(mib)/sizeof(*mib), &boottime_tv, &len, nullptr, 0) < 0)
            return boottime;

        boottime = std::chrono::system_clock::time_point(
            std::chrono::seconds(boottime_tv.tv_sec) +
            std::chrono::microseconds(boottime_tv.tv_usec));
        has_boottime = true;
    }

    return boottime;
}

LOCAL_API std::string get_executable_path()
{
    std::string exec_path("./");

    task_dyld_info dyld_info;
    task_t t;
    pid_t pid = getpid();
    task_for_pid(mach_task_self(), pid, &t);
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;

    if (task_info(t, TASK_DYLD_INFO, reinterpret_cast<task_info_t>(&dyld_info), &count) == KERN_SUCCESS)
    {
        dyld_all_image_infos *dyld_img_infos = reinterpret_cast<dyld_all_image_infos*>(dyld_info.all_image_info_addr);
        for (int i = 0; i < dyld_img_infos->infoArrayCount; ++i)
        {// For now I don't know how to be sure to get the executable path
         // but looks like the 1st entry is the executable path
            exec_path = dyld_img_infos->infoArray[i].imageFilePath;
            break;
        }
    }

    exec_path = exec_path.substr(0, exec_path.rfind(PATH_SEPARATOR) + 1);
    return exec_path;
}

LOCAL_API std::string get_userdata_path()
{
    std::string user_appdata_path;

    char* homedir = getenv("HOME");
    if (homedir != nullptr)
        user_appdata_path = std::move(std::string(homedir));

    return user_appdata_path;
}

#endif

void shared_library_load(void* hmodule)
{
    ::hmodule = hmodule;
}

void shared_library_unload(void* hmodule)
{

}

LOCAL_API std::string process_path(std::string const& path)
{
    std::string canonicalized_path(path);
    size_t pos;
    size_t size;

    while ((pos = canonicalized_path.find("//")) != std::string::npos)
        canonicalized_path.replace(pos, 2, "/");

    while ((pos = canonicalized_path.find("/./")) != std::string::npos)
        canonicalized_path.replace(pos, 3, "/");

    while ((pos = canonicalized_path.find("/..")) != std::string::npos)
    {
        if (pos == 0)
            size = 3;
        else
        {
            size_t parent_pos = canonicalized_path.rfind("/", pos - 1);
            if (parent_pos == std::string::npos)
            {
                size = pos + 3;
                pos = 0;
            }
            else
            {
                size = 3 + pos - parent_pos;
                pos = parent_pos;
            }
        }

        canonicalized_path.replace(pos, size, "");
    }

    while ((pos = canonicalized_path.find("/.")) != std::string::npos)
        canonicalized_path.replace(pos, 2, "");

    return canonicalized_path;
}

LOCAL_API std::string canonical_path(std::string const& path)
{
    return process_path(path);
}

LOCAL_API std::string get_env_var(std::string const& name)
{
    char* env = getenv(name.c_str());
    return (env == nullptr ? std::string() : std::string(env));
}

LOCAL_API std::string get_module_path()
{
    std::string library_path = "./";
    
    Dl_info infos;
    dladdr(hmodule, &infos);
    library_path = infos.dli_fname;

    LOG(Log::LogLevel::INFO, "%s", library_path.c_str());
    return library_path;
}

LOCAL_API std::vector<ipv4_addr> get_broadcasts()
{
    /* Not sure how many platforms this will run on,
     * so it's wrapped in __linux for now.
     * Definitely won't work like this on Windows...
     */

    std::vector<ipv4_addr> broadcasts;

    static constexpr auto max_broadcasts = 32;
    uint32_t sock = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return broadcasts;

    /* Configure ifconf for the ioctl call. */
    struct ifreq i_faces[max_broadcasts];
    memset(i_faces, 0, sizeof(struct ifreq) * max_broadcasts);

    struct ifconf ifconf;
    ifconf.ifc_buf = (char*)i_faces;
    ifconf.ifc_len = sizeof(i_faces);

    if (ioctl(sock, SIOCGIFCONF, &ifconf) < 0)
    {
        close(sock);
        return broadcasts;
    }

    /* ifconf.ifc_len is set by the ioctl() to the actual length used;
     * on usage of the complete array the call should be repeated with
     * a larger array, not done (640kB and 16 interfaces shall be
     * enough, for everybody!)
     */
    int i, count = ifconf.ifc_len / sizeof(struct ifreq);

    for (i = 0; i < count; ++i)
    {
        /* there are interfaces with are incapable of broadcast */
        if (ioctl(sock, SIOCGIFBRDADDR, &i_faces[i]) < 0)
            continue;

        /* moot check: only AF_INET returned (backwards compat.) */
        if (i_faces[i].ifr_broadaddr.sa_family != AF_INET)
            continue;

        struct sockaddr_in* sock4 = (struct sockaddr_in*) & i_faces[i].ifr_broadaddr;

        if (sock4->sin_addr.s_addr == 0)
            continue;

        ipv4_addr addr;
        addr.set_ip(sock4->sin_addr.s_addr);
        broadcasts.emplace_back(std::move(addr));

        if (broadcasts.size() >= max_broadcasts)
            break;
    }

    close(sock);

    return broadcasts;
}

LOCAL_API bool create_folder(std::string const& _folder)
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

LOCAL_API bool delete_file(std::string const& path)
{
    return unlink(path.c_str()) == 0;
}

LOCAL_API std::vector<std::string> list_files(std::string const& path, bool recursive)
{
    std::vector<std::string> files;

    std::string search_path = path;

    if (*path.rbegin() != PATH_SEPARATOR)
        search_path += PATH_SEPARATOR;

    DIR* dir = opendir(search_path.c_str());
    struct dirent* entry;

    if (dir == nullptr)
        return files;

    while ((entry = readdir(dir)) != nullptr)
    {
        if (strcmp(entry->d_name, ".")  == 0
         || strcmp(entry->d_name, "..") == 0)
            continue;

        if(entry->d_type == DT_DIR)
        {
            if (recursive)
            {
                std::string dir_name = entry->d_name;
                std::vector<std::string> sub_files = std::move(list_files(search_path + dir_name, true));
                std::transform(sub_files.begin(), sub_files.end(), std::back_inserter(files), [&dir_name](std::string& file_name)
                {
                    return dir_name + PATH_SEPARATOR + file_name;
                });
            }
        }
        else if(entry->d_type == DT_REG)
        {
            files.emplace_back(entry->d_name);
        }
    }

    closedir(dir);   

    return files;
}

#endif
