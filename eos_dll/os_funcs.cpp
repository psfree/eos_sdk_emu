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

LOCAL_API std::chrono::microseconds get_uptime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - get_boottime());
}

LOCAL_API std::vector<ipv4_addr> const& get_broadcasts()
{
    static std::vector<ipv4_addr> broadcasts;

    if (broadcasts.empty())
    {
        std::vector<iface_ip_t> const& ifaces_ip = get_ifaces_ip();

        PortableAPI::ipv4_addr addr;

        for (auto& iface : ifaces_ip)
        {
            addr.set_ip(iface.ip | (~iface.mask));
            broadcasts.emplace_back(addr);
            APP_LOG(Log::LogLevel::INFO, "%s", addr.to_string().c_str());
        }
    }

    return broadcasts;
}

LOCAL_API bool is_iface_ip(const sockaddr* sock_addr, int namelen)
{
    if (namelen == 0)
        return false;


    auto const& ifaces = get_ifaces_ip();

    if (sock_addr->sa_family == AF_INET)
    {
        uint32_t target_ip = utils::Endian::net_swap(((struct sockaddr_in*)sock_addr)->sin_addr.s_addr);

        //ipv4_addr addr, mask, target;

        for (auto& iface : ifaces)
        {
            //addr.set_ip(iface.ip);
            //mask.set_ip(iface.mask);
            //target.set_ip(target_ip);
            //
            //APP_LOG(Log::LogLevel::INFO, "Checking %s | %s against %s", addr.to_string().c_str(), mask.to_string().c_str(), target.to_string().c_str());

            if ((iface.ip & iface.mask) == (target_ip & iface.mask))
            {
                return true;
            }
        }
    }

    return false;
}

LOCAL_API bool is_lan_ip(const sockaddr* addr, int namelen)
{
    if (!namelen) return false;

    if (addr->sa_family == AF_INET) {
        struct sockaddr_in* addr_in = (struct sockaddr_in*)addr;
        unsigned char ip[4];
        memcpy(ip, &addr_in->sin_addr, sizeof(ip));
        //APP_LOG(Log::LogLevel::DEBUG, "CHECK LAN IP %hhu.%hhu.%hhu.%hhu:%u", ip[0], ip[1], ip[2], ip[3], ntohs(addr_in->sin_port));
        if (is_iface_ip(addr, namelen)) return true;
        if (ip[0] == 127) return true;
        if (ip[0] == 10) return true;
        if (ip[0] == 192 && ip[1] == 168) return true;
        if (ip[0] == 169 && ip[1] == 254 && ip[2] != 0) return true;
        if (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) return true;
        if ((ip[0] == 100) && ((ip[1] & 0xC0) == 0x40)) return true;
        if (ip[0] == 239) return true; //multicast
        if (ip[0] == 0) return true; //Current network
        if (ip[0] == 192 && (ip[1] == 18 || ip[1] == 19)) return true; //Used for benchmark testing of inter-network communications between two separate subnets.
        if (ip[0] >= 224) return true; //ip multicast (224 - 239) future use (240.0.0.0�255.255.255.254) broadcast (255.255.255.255)
    }
    else if (addr->sa_family == AF_INET6) {
        struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)addr;
        unsigned char ip[16];
        unsigned char zeroes[16] = {};
        memcpy(ip, &addr_in6->sin6_addr, sizeof(ip));
        //APP_LOG(Log::LogLevel::DEBUG, "CHECK LAN IP6 %hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu...%hhu", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7], ip[15]);
        if (is_iface_ip(addr, namelen)) return true;
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

#if defined(__WINDOWS__)

#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

decltype(sendto)*             _sendto             = sendto;
decltype(connect)*            _connect            = connect;
decltype(WSAConnect)*         _WSAConnect         = WSAConnect;
decltype(WinHttpOpenRequest)* _WinHttpOpenRequest = WinHttpOpenRequest;
decltype(WinHttpConnect)*     _WinHttpConnect     = WinHttpConnect;

static int WINAPI Mysendto(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
    if (is_lan_ip(to, tolen)) {
        return _sendto(s, buf, len, flags, to, tolen);
    }
    else
    {
        return len;
    }
}

static int WINAPI Myconnect(SOCKET s, const sockaddr* addr, int namelen)
{
    if (is_lan_ip(addr, namelen)) {
        return _connect(s, addr, namelen);
    }
    else
    {
        WSASetLastError(WSAECONNREFUSED);
        return SOCKET_ERROR;
    }
}

static int WINAPI MyWSAConnect(SOCKET s, const sockaddr* addr, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS)
{
    if (is_lan_ip(addr, namelen))
    {
        return _WSAConnect(s, addr, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);
    }
    else
    {
        WSASetLastError(WSAECONNREFUSED);
        return SOCKET_ERROR;
    }
}

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
    else
    {
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
    if (dwFlags & WINHTTP_FLAG_SECURE)
    {
        dwFlags &= ~(WINHTTP_FLAG_SECURE);
    }

    return _WinHttpOpenRequest(hConnect, pwszVerb, pwszObjectName, pwszVersion, pwszReferrer, ppwszAcceptTypes, dwFlags);
}

void shared_library_load(void* hmodule)
{
    ::hmodule = hmodule;

    std::fstream log("cmdline.txt", std::ios::out | std::ios::trunc);
    log << GetCommandLine();

    Socket::InitSocket();
}

void shared_library_unload(void* hmodule)
{

}

LOCAL_API void disable_online_networking()
{
    mini_detour::transaction_begin();

    if (mini_detour::detour_func((void**)&_sendto           , (void*)&Mysendto))
        APP_LOG(Log::LogLevel::WARN, "Failed to hook sendto");
    if(mini_detour::detour_func((void**)&_connect           , (void*)&Myconnect))
        APP_LOG(Log::LogLevel::WARN, "Failed to hook connect");
    if(mini_detour::detour_func((void**)&_WSAConnect        , (void*)&MyWSAConnect))
        APP_LOG(Log::LogLevel::WARN, "Failed to hook wsaconnect");
    if(mini_detour::detour_func((void**)&_WinHttpConnect    , (void*)&MyWinHttpConnect))
        APP_LOG(Log::LogLevel::WARN, "Failed to hook winhttpconnect");
    if(mini_detour::detour_func((void**)&_WinHttpOpenRequest, (void*)&MyWinHttpOpenRequest))
        APP_LOG(Log::LogLevel::WARN, "Failed to hook winhttpopenrequest");

    mini_detour::transaction_commit();
}

LOCAL_API void enable_online_networking()
{
    mini_detour::transaction_begin();
    mini_detour::unhook_func((void**)&_sendto            , &sendto);
    mini_detour::unhook_func((void**)&_connect           , &connect);
    mini_detour::unhook_func((void**)&_WSAConnect        , &WSAConnect);
    mini_detour::unhook_func((void**)&_WinHttpConnect    , &WinHttpConnect);
    mini_detour::unhook_func((void**)&_WinHttpOpenRequest, &WinHttpOpenRequest);
    mini_detour::transaction_commit();
}

LOCAL_API std::chrono::system_clock::time_point get_boottime()
{
    static std::chrono::system_clock::time_point boottime(std::chrono::system_clock::now() - std::chrono::milliseconds(GetTickCount64()));
    return boottime;
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
    std::string path;
    std::wstring wpath(4096, '\0');

    DWORD size = GetModuleFileNameW(nullptr, &wpath[0], wpath.length());
    utf8::utf16to8(wpath.begin(), wpath.begin() + size, std::back_inserter(path));

    APP_LOG(Log::LogLevel::INFO, "%s", path.c_str());
    return path;
}

LOCAL_API std::string get_module_path()
{
    std::string path;
    std::wstring wpath(4096, '\0');

    DWORD size = GetModuleFileNameW((HINSTANCE)hmodule, &wpath[0], wpath.length());
    utf8::utf16to8(wpath.begin(), wpath.begin() + size, std::back_inserter(path));

    APP_LOG(Log::LogLevel::INFO, "%s", path.c_str());
    return path;
}

LOCAL_API void* get_module_handle(std::string const& name)
{
    std::wstring wname;
    utf8::utf8to16(name.begin(), name.end(), std::back_inserter(wname));

    return GetModuleHandleW(wname.c_str());
}

LOCAL_API std::vector<iface_ip_t> const& get_ifaces_ip()
{
    static std::vector<iface_ip_t> ifaces;

    if (ifaces.empty())
    {
        IP_ADAPTER_ADDRESSES* pAdaptersAddresses = nullptr;
        ULONG ulOutBufLen = 0;

        if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_ALL_INTERFACES, nullptr, pAdaptersAddresses, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
        {
            pAdaptersAddresses = (IP_ADAPTER_ADDRESSES*)malloc(ulOutBufLen);

            if (pAdaptersAddresses == nullptr)
                return ifaces;
        }

        if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_ALL_INTERFACES, nullptr, pAdaptersAddresses, &ulOutBufLen) == NO_ERROR)
        {
            for(IP_ADAPTER_ADDRESSES* pAdapterAddress = pAdaptersAddresses; pAdapterAddress != nullptr; pAdapterAddress = pAdapterAddress->Next)
            {
                if (pAdapterAddress->OperStatus != IfOperStatusUp)
                    continue;

                for (PIP_ADAPTER_UNICAST_ADDRESS_LH pAddr = pAdapterAddress->FirstUnicastAddress; pAddr != nullptr; pAddr = pAddr->Next)
                {
                    if (pAddr->Address.lpSockaddr->sa_family == AF_INET)
                    {
                        const sockaddr_in* sock_addr = reinterpret_cast<const sockaddr_in*>(pAddr->Address.lpSockaddr);
                        uint32_t ip = sock_addr->sin_addr.s_addr;
                        uint32_t mask = 0;
                        for (int i = 0; i < pAddr->OnLinkPrefixLength; ++i)
                        {// Set netmask from cidr
                            mask <<= 1;
                            mask |= 1;
                        }

                        if (sock_addr->sin_addr.s_addr != 0 && pAddr->OnLinkPrefixLength != 0)
                        {
                            ifaces.emplace_back(iface_ip_t{ utils::Endian::net_swap(ip), utils::Endian::net_swap(mask) });
                        }
                    }
                    //else if (pAddr->Address.lpSockaddr->sa_family == AF_INET6)
                    //{
                    //  const sockaddr_in6* sock_addr = reinterpret_cast<const sockaddr_in6*>(pAddr->Address.lpSockaddr);
                    //
                    //}
                }
            }
        }

        free(pAdaptersAddresses);
    }

    return ifaces;
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


    std::string self = "/proc/self/map_files/";
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
            std::string path = self + ep->d_name;
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

    APP_LOG(Log::LogLevel::INFO, "%s", exec_path.c_str());
    return exec_path;
}

LOCAL_API void* get_module_handle(std::string const& name)
{
    std::string const self("/proc/self/map_files/");
    DIR* dir;
    struct dirent* dir_entry;
    char buff[4096];
    size_t buff_len = 4095;

    void* res = nullptr;

    dir = opendir(self.c_str());
    if (dir != nullptr)
    {
        while((dir_entry = readdir(dir)) != nullptr)
        {
            ssize_t name_len = readlink((self + dir_entry->d_name).c_str(), buff, buff_len);
            if (name_len > 0)
            {
                buff[name_len] = '\0';
                char* pos = strrchr(buff, '/');
                if (pos != nullptr)
                {
                    ++pos;
                    if (strncmp(pos, name.c_str(), name.length()) == 0)
                    {
                        res = dlopen(buff, RTLD_NOW);
                        if(res != nullptr)
                        {// Like Windows' GetModuleHandle, we don't want to increment the ref counter
                            dlclose(res);
                        }
                        break;
                    }
                }
            }
        }

        closedir(dir);
    }

    return res;
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

    return exec_path;
}

LOCAL_API void* get_module_handle(std::string const& name)
{
    void* res = nullptr;

    task_dyld_info dyld_info;
    task_t t;
    pid_t pid = getpid();
    task_for_pid(mach_task_self(), pid, &t);
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;

    if (task_info(t, TASK_DYLD_INFO, reinterpret_cast<task_info_t>(&dyld_info), &count) == KERN_SUCCESS)
    {
        const char* pos;
        dyld_all_image_infos *dyld_img_infos = reinterpret_cast<dyld_all_image_infos*>(dyld_info.all_image_info_addr);
        for (int i = 0; i < dyld_img_infos->infoArrayCount; ++i)
        {
            pos = strrchr(dyld_img_infos->infoArray[i].imageFilePath, '/');
            if(pos != nullptr)
            {
                ++pos;
                if(strncmp(pos, name.c_str(), name.length()) == 0)
                {
                    res = dlopen(dyld_img_infos->infoArray[i].imageFilePath, RTLD_NOW);
                    if(res != nullptr)
                    {// Like Windows' GetModuleHandle, we don't want to increment the ref counter
                        dlclose(res);
                    }
                }
                break;
            }
        }
    }

    return res;
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

decltype(sendto)*  _sendto  = sendto;
decltype(connect)* _connect = connect;

static int Mysendto(int s, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
    if (is_lan_ip(to, tolen)) {
        return _sendto(s, buf, len, flags, to, tolen);
    }
    else {
        return len;
    }
}

static int Myconnect(int s, const sockaddr* addr, int namelen)
{
    if (is_lan_ip(addr, namelen)) {
        return _connect(s, addr, namelen);
    }
    else {
        errno = ECONNREFUSED;
        return -1;
    }
}

void shared_library_load(void* hmodule)
{
    ::hmodule = hmodule;
}

void shared_library_unload(void* hmodule)
{

}

LOCAL_API void disable_online_networking()
{
    mini_detour::transaction_begin();
    if (mini_detour::detour_func((void**)&_sendto, (void*)&Mysendto))
        APP_LOG(Log::LogLevel::WARN, "Failed to hook sendto");
    if (mini_detour::detour_func((void**)&_connect, (void*)&Myconnect))
        APP_LOG(Log::LogLevel::WARN, "Failed to hook connect");
    mini_detour::transaction_commit();
}

LOCAL_API void enable_online_networking()
{
    mini_detour::transaction_begin();
    mini_detour::unhook_func((void**)&_sendto , (void*)&sendto);
    mini_detour::unhook_func((void**)&_connect, (void*)&connect);
    mini_detour::transaction_commit();
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

    APP_LOG(Log::LogLevel::INFO, "%s", library_path.c_str());
    return library_path;
}

LOCAL_API std::vector<iface_ip_t> const& get_ifaces_ip()
{
    static std::vector<iface_ip_t> ifaces;

    if (ifaces.empty())
    {
        ifaddrs* ifaces_list;
        ifaddrs* pIface;

        if (getifaddrs(&ifaces_list) == 0)
        {
            const sockaddr_in* sock_addr;
            for (pIface = ifaces_list; pIface != nullptr; pIface = pIface->ifa_next)
            {
                if (pIface->ifa_addr->sa_family == AF_INET)
                {
                    sock_addr = reinterpret_cast<const sockaddr_in*>(pIface->ifa_addr);
                    if (sock_addr->sin_addr.s_addr != 0 && pIface->ifa_netmask != nullptr)
                    {
                        uint32_t ip = reinterpret_cast<const sockaddr_in*>(pIface->ifa_addr)->sin_addr.s_addr;
                        uint32_t mask = reinterpret_cast<const sockaddr_in*>(pIface->ifa_netmask)->sin_addr.s_addr;

                        ifaces.emplace_back(iface_ip_t{ utils::Endian::net_swap(ip), utils::Endian::net_swap(mask) });
                    }
                }
                // IPV6
                //else if (pIface->ifa_addr->sa_family == AF_INET6)
                //{
                //    const sockaddr_in6* addr = reinterpret_cast<const sockaddr_in6*>(pIface->ifa_addr);));
                //}
            }
            freeifaddrs(ifaces_list);
        }
    }

    return ifaces;
}

#endif
