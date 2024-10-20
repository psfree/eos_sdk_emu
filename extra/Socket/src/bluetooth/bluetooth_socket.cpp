/* Copyright (C) 2019-2020 Nemirtingas
 * This file is part of Socket.
 *
 * Socket is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Socket is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Socket.  If not, see <https://www.gnu.org/licenses/>
 */

#ifndef __APPLE__

#ifdef __WINDOWS__
#define INITGUID
#endif

#include <socket/bluetooth/bluetooth_socket.h>
#include <inttypes.h>  // for SCNx8
#include <codecvt>

#define __UUID128_PRINTF_FORMAT__ \
"%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "-"\
"%02" SCNx8 "%02" SCNx8 "-"\
"%02" SCNx8 "%02" SCNx8 "-"\
"%02" SCNx8 "%02" SCNx8 "-"\
"%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8

constexpr uint8_t PortableAPI::BluetoothSocket::bth_base_uuid[16];

#if defined(__LINUX__) && !defined(__BLUEZ_4__)
#include <iomanip>      // for setfill and setw
#include <dbus/dbus.h> // for bluez

class LOCAL_API BluezDBusSystem
{
private:
    DBusConnection *_conn;
    DBusError _err;

    BluezDBusSystem() :_conn(nullptr) {}

    BluezDBusSystem( BluezDBusSystem const& ) = delete;
    BluezDBusSystem( BluezDBusSystem && ) = delete;
    BluezDBusSystem& operator=( BluezDBusSystem const& ) = delete;
    BluezDBusSystem& operator=( BluezDBusSystem && ) = delete;

public:
    static BluezDBusSystem& Inst()
    {
        static BluezDBusSystem _inst;

        if (_inst._conn == nullptr)
        {
            dbus_error_init(&_inst._err);
            _inst._conn = dbus_bus_get(DBUS_BUS_SYSTEM, &_inst._err);
        }

        return _inst;
    }

    operator DBusConnection*() { return _conn; }
    operator DBusError*() { return &_err; }

    const char* message() const { return _err.message; }

    ~BluezDBusSystem()
    {
        if (_conn != nullptr)
            dbus_connection_unref(_conn);
    }
};
#endif

using namespace PortableAPI;

Uuid::Uuid():
    _type(Uuid::type::none),
    _uuid{}
{}

Uuid::Uuid(uuid_t const& uuid)
{
    set_uuid128(uuid);
}

bool Uuid::is_valid_uuid(std::string const& struuid)
{
    // V�rification de la taille et de la pr�sence des s�parateurs aux bons endroits
    if(   struuid.length() != 36
        || struuid[8] != '-' || struuid[13] != '-'
        || struuid[18] != '-' || struuid[23] != '-' )
    {
        return false;
    }
    const char *ptr = struuid.c_str();

    for( int i = 0; i < 36; ++i, ++ptr )
    {
        // Si i n'est pas sur un '-'
        if (i != 8 && i != 13 && i != 18 && i != 23)
        {
            // si le caract�re n'est pas entre 0-9 et A-Z
            if (((*ptr < '0') || (*ptr > '9')) &&
                ((*ptr < 'A') || (*ptr > 'F')) &&
                ((*ptr < 'a') || (*ptr > 'f')))
            {
                return false;
            }
        }
    }
    return true;
}

#if defined(__WINDOWS__)

///////////////////////////////////////////////////////////////////////////////
// Uuid Class

void Uuid::uuid16_to_uuid128(uint16_t uuid)
{
    uuid32_to_uuid128(uuid);
}

void Uuid::uuid32_to_uuid128(uint32_t uuid)
{
    _type = Uuid::type::uuid128;
    //             Bluetooth Base UUID
    memcpy(&_uuid, BluetoothSocket::bth_base_uuid, sizeof(BluetoothSocket::bth_base_uuid));
    _uuid.Data1 = uuid;
}

void Uuid::uuid128_to_uuid16(uuid_t const& uuid)
{
    _type = Uuid::type::uuid16;
    memset(&_uuid, 0, sizeof(_uuid));
    _uuid.Data1 = static_cast<uint16_t>(uuid.Data1);
}

void Uuid::uuid128_to_uuid32(uuid_t const& uuid)
{
    _type = Uuid::type::uuid32;
    memset(&_uuid, 0, sizeof(_uuid));
    _uuid.Data1 = uuid.Data1;
}

void Uuid::from_string(std::string const& struuid)
{
    // si c'est une uuid
    if (is_valid_uuid(struuid))
    {
        _type = Uuid::type::uuid128;
        uint8_t* uuid8 = reinterpret_cast<uint8_t*>(&_uuid);

        sscanf_s(struuid.c_str(), __UUID128_PRINTF_FORMAT__,
            &uuid8[3], &uuid8[2], &uuid8[1], &uuid8[0],
            &uuid8[5], &uuid8[4],
            &uuid8[7], &uuid8[6],
            &uuid8[8], &uuid8[9],
            &uuid8[10], &uuid8[11], &uuid8[12], &uuid8[13], &uuid8[14], &uuid8[15]);
    }
    else
    {
        memset(&_uuid, 0, sizeof(_uuid));
    }
}

std::string Uuid::to_string()
{
    char str[37] = { 0 };
    if (_type == Uuid::type::uuid128)
    {
        const uint8_t* datas = reinterpret_cast<const uint8_t*>(&_uuid);
        snprintf(str, 37, __UUID128_PRINTF_FORMAT__,
            datas[3], datas[2], datas[1], datas[0],
            datas[5], datas[4],
            datas[7], datas[6],
            datas[8], datas[9],
            datas[10], datas[11], datas[12], datas[13], datas[14], datas[15]);
    }
    return std::string(str);
}

void Uuid::set_uuid128(uuid_t const& uuid)
{
    _type = Uuid::type::uuid128;
    memcpy(&_uuid, &uuid, sizeof(uuid));
}

bool Uuid::operator ==(Uuid const& other)
{
    if (_type != other._type)
        return false;

    return memcmp(&_uuid, &other._uuid, sizeof(_uuid)) == 0;
}

bool Uuid::operator !=(Uuid const& other)
{
    return !(*this == other);
}

uuid_t const& Uuid::get_native_uuid() const
{
    return _uuid;
}

///////////////////////////////////////////////////////////////////////////////
// BluetoothSocket Class

int BluetoothSocket::inet_pton(BluetoothSocket::address_family family, std::string const& str_addr, void* out_buf)
{
    if (family != BluetoothSocket::address_family::bth)
        return 0;

    sockaddr_rc sockaddr;
    socklen_t lg = sizeof(sockaddr_rc);

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring waddr = converter.from_bytes(str_addr);

    if (WSAStringToAddressW(const_cast<wchar_t*>(waddr.c_str()), static_cast<uint32_t>(BluetoothSocket::address_family::bth), nullptr, (LPSOCKADDR)&sockaddr, (socklen_t*)&lg))
        return 0;

    *reinterpret_cast<BTH_ADDR*>(out_buf) = sockaddr.btAddr;
    return 1;
}

void BluetoothSocket::inet_ntop(BluetoothSocket::address_family family, const void* addr, std::string& str_addr)
{
    if (family != BluetoothSocket::address_family::bth)
        throw error_in_value("Error in family, cannot parse addr");

    wchar_t wstr[128];
    wchar_t* wtmp = wstr;
    unsigned long wstr_len = (sizeof(wstr) / sizeof(*wstr)) - 1;

    sockaddr_rc rcaddr;
    rcaddr.btAddr = *reinterpret_cast<const BTH_ADDR*>(addr);
    rcaddr.addressFamily = static_cast<uint32_t>(BluetoothSocket::address_family::bth);

    if (WSAAddressToStringW((LPSOCKADDR)&rcaddr, sizeof(rcaddr), nullptr, wstr, &wstr_len) != 0)
    {
        str_addr.clear();
        throw error_in_value("Error in value, cannot parse addr");
    }
    // Windows renvoie l'adresse d'une facon chelou :
    // (XX:XX:XX:XX:XX:XX):XX:XX:XX
    // alors on cr�e une sous-chaine
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    str_addr = std::move(converter.to_bytes(wstr+1, wstr+18));
}

std::list<BluetoothDevice> BluetoothSocket::scan(bool flushCache)
{
    std::list<BluetoothDevice> devices;
    BluetoothDevice device;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    HANDLE hLookup;
    union
    {
        char buffer[4096];
        double __unused;
    };
    WSAQUERYSETW wsaq = { 0 };
    wsaq.dwSize = sizeof(WSAQUERYSETW);
    wsaq.dwNameSpace = NS_BTH;
    wsaq.lpcsaBuffer = 0;
    int flags = LUP_CONTAINERS;
    if (flushCache)
        flags |= LUP_FLUSHCACHE;
    // D�marrage de la recherche de service
    if (WSALookupServiceBeginW(&wsaq, flags, &hLookup))
    {
        switch (WSAGetLastError())
        {
            case WSANOTINITIALISED: throw wsa_not_initialised("A successful WSAStartup call must occur before using this function.");
            case WSAEINVAL: throw error_in_value("One or more parameters were missing or invalid for this provider.");
            // Pas de probl�me avec la recherche mais il n'y a pas de p�riph�riques visibles.
            case WSASERVICE_NOT_FOUND: return devices;
            default: throw socket_exception("scan exception.");
        }
    }

    LPWSAQUERYSETW pwsaResults = (LPWSAQUERYSETW)buffer;
    DWORD dwSize = sizeof(buffer);
    memset(pwsaResults, 0, sizeof(WSAQUERYSETW));
    pwsaResults->dwSize = sizeof(WSAQUERYSETW);
    pwsaResults->dwNameSpace = NS_BTH;
    pwsaResults->lpBlob = NULL;

    // R�cup�ration de l'appareil
    while(!WSALookupServiceNextW(hLookup, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pwsaResults))
    {
        if (pwsaResults->dwNumberOfCsAddrs != 1)
            break;
        // Affectation de l'adresse
        device.addr = ((sockaddr_rc *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;
        // Affectation du nom de l'appareil
        if (pwsaResults->lpszServiceInstanceName != nullptr && *(pwsaResults->lpszServiceInstanceName))
        {
            device.name = converter.to_bytes(pwsaResults->lpszServiceInstanceName);
        }
        else
            device.name = "[inconnu]";

        devices.emplace_back(device);
    }

    int32_t err = WSAGetLastError();
    switch (err)
    {
        case WSANOTINITIALISED: 
            WSALookupServiceEnd(hLookup);
            throw wsa_not_initialised("A successful WSAStartup call must occur before using this function.");
        case WSAEINVAL: 
            WSALookupServiceEnd(hLookup);
            throw error_in_value("One or more required parameters were invalid or missing.");
#ifdef WSAENOMORE
        case WSAENOMORE: break;
#endif
#ifdef WSA_E_NO_MORE
        case WSA_E_NO_MORE: break;
#endif
        default: 
            WSALookupServiceEnd(hLookup);
            throw socket_exception("scan exception: " + std::to_string(err));
    }

    // Lib�rer les ressources de recherche
    WSALookupServiceEnd(hLookup);

    return devices;
}

int sdp_get_proto_port(SDP_ELEMENT_DATA& protocol_container, WORD proto_uuid)
{
    int port = -1;
    // HBLUETOOTH_CONTAINER_ELEMENT is actually just a PBYTE with offset to the next SDP type
    HBLUETOOTH_CONTAINER_ELEMENT hProtocolContainer = NULL;
    SDP_ELEMENT_DATA protocol_sequence;
    SDP_ELEMENT_DATA elem;

    while (BluetoothSdpGetContainerElementData(protocol_container.data.sequence.value, protocol_container.data.sequence.length, &hProtocolContainer, &protocol_sequence) == ERROR_SUCCESS)
    {
        HBLUETOOTH_CONTAINER_ELEMENT hProtocolSequence = NULL;
        int proto_port = -1;
        uuid_t uuid = { 0 };
        while (BluetoothSdpGetContainerElementData(protocol_sequence.data.sequence.value, protocol_sequence.data.sequence.length, &hProtocolSequence, &elem) == ERROR_SUCCESS)
        {
            switch(elem.type)
            {
                case SDP_TYPE_UUID:
                    switch(elem.specificType)
                    {
                        case SDP_ST_UUID16:
                            uuid.Data1 = elem.data.uuid16;
                            break;

                        case SDP_ST_UUID32:
                            uuid.Data1 = elem.data.uuid32;
                            break;

                        case SDP_ST_UUID128:
                            uuid = elem.data.uuid128;
                            break;
                    }
                    break;

                case SDP_TYPE_UINT:
                    switch (elem.specificType)
                    {
                        case SDP_ST_UINT8 : proto_port = elem.data.uint8 ; break;
                        case SDP_ST_UINT16: proto_port = elem.data.uint16; break;
                    }
                    break;

                case SDP_TYPE_INT:
                    switch (elem.specificType)
                    {
                        case SDP_ST_INT8 : proto_port = elem.data.int8 ; break;
                        case SDP_ST_INT16: proto_port = elem.data.int16; break;
                    }
                    break;
            }
        }
        if (uuid.Data1 == proto_uuid)
        {
            port = proto_port;
            break;
        }
    }

    return port;
}

int BluetoothSocket::scan_open_port_from_uuid(Uuid const& uuid, bdaddr_t const& addr)
{
    int port = -1;

    WSAQUERYSETW* querySet = new WSAQUERYSETW;
    if (querySet == nullptr)
        return port;

    HANDLE hLookup;
    GUID protocol = RFCOMM_PROTOCOL_UUID;
    std::string str_addr;
    BluetoothSocket::inet_ntop(BluetoothSocket::address_family::bth, &addr, str_addr);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring addressAsString = converter.from_bytes("(" + str_addr + ")");
    DWORD flags = LUP_FLUSHCACHE | LUP_RETURN_BLOB;
    int result;

    memset(querySet, 0, sizeof(*querySet));
    querySet->dwSize = sizeof(*querySet);
    querySet->lpServiceClassId = &protocol;
    querySet->dwNameSpace = NS_BTH;
    querySet->lpszContext = (LPWSTR)addressAsString.c_str();

    if (WSALookupServiceBeginW(querySet, flags, &hLookup) == ERROR_SUCCESS)
    {
        DWORD bufferLength = sizeof(WSAQUERYSETW);
        WSAQUERYSETW* pResults = reinterpret_cast<WSAQUERYSETW*>(new char[bufferLength]);
        if (pResults != nullptr)
        {
            while (1)
            {
                result = WSALookupServiceNextW(hLookup, flags, &bufferLength, pResults);

                if (result != ERROR_SUCCESS && WSAGetLastError() == WSAEFAULT && bufferLength > 0)
                {
                    delete[] reinterpret_cast<char*>(pResults);
                    pResults = reinterpret_cast<WSAQUERYSETW*>(new char[bufferLength]);
                    result = WSALookupServiceNextW(hLookup, flags, &bufferLength, pResults);
                }

                if (result != ERROR_SUCCESS)
                    break;

                if (pResults->lpBlob)
                {
                    const BLOB* pBlob = (BLOB*)pResults->lpBlob;

                    SDP_ELEMENT_DATA elem;

                    if (BluetoothSdpGetAttributeValue(pBlob->pBlobData, pBlob->cbSize, SDP_ATTRIB_SERVICE_ID, &elem) == ERROR_SUCCESS)
                    {
                        if (elem.type == SDP_TYPE_UUID)
                        {
                            Uuid sdp_uuid;

                            switch (elem.specificType)
                            {
                                case SDP_ST_UUID16 : sdp_uuid.uuid16_to_uuid128(elem.data.uuid16); break;
                                case SDP_ST_UUID32 : sdp_uuid.uuid32_to_uuid128(elem.data.uuid32); break;
                                case SDP_ST_UUID128: sdp_uuid.set_uuid128(elem.data.uuid128); break;
                            }

                            if (sdp_uuid == uuid)
                            {
                                SDP_ELEMENT_DATA protocol_container;

                                // Open the decriptor list sequence
                                if (BluetoothSdpGetAttributeValue(pBlob->pBlobData, pBlob->cbSize, SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST, &protocol_container) == ERROR_SUCCESS)
                                {
                                    port = sdp_get_proto_port(protocol_container, RFCOMM_PROTOCOL_UUID16);
                                }

                                // If you need theses, just uncomment these lines & use the values
                                //if (BluetoothSdpGetAttributeValue(pBlob->pBlobData, pBlob->cbSize, 0x0100, &elem) == ERROR_SUCCESS)
                                //{
                                //    if (elem.type == SDP_TYPE_STRING)// Service Name
                                //    {
                                //        std::string().assign((char*)elem.data.string.value, elem.data.string.length);
                                //    }
                                //}
                                //if (BluetoothSdpGetAttributeValue(pBlob->pBlobData, pBlob->cbSize, 0x0101, &elem) == ERROR_SUCCESS)
                                //{
                                //    if (elem.type == SDP_TYPE_STRING)// Service Provider
                                //    {
                                //        std::string().assign((char*)elem.data.string.value, elem.data.string.length);
                                //    }
                                //}
                                //if (BluetoothSdpGetAttributeValue(pBlob->pBlobData, pBlob->cbSize, 0x0102, &elem) == ERROR_SUCCESS)
                                //{
                                //    if (elem.type == SDP_TYPE_STRING)// Service Description
                                //    {
                                //        std::string().assign((char*)elem.data.string.value, elem.data.string.length);
                                //    }
                                //}
                            }
                        }
                    }
                }
            }

            delete[] reinterpret_cast<char*>(pResults);
        }
        WSALookupServiceEnd(hLookup);
    }

    delete querySet;
    return port;
}

void BluetoothSocket::register_sdp_service(service_t & service, uuid_t const& uuid, uint8_t port, std::string const&srv_name, std::string const&srv_prov, std::string const&srv_desc)
{
    memset(&service, 0, sizeof(service_t));
    service.dwSize = sizeof(service_t);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wstr;
    WCHAR* lpwstr;

    wstr = std::move(converter.from_bytes(srv_name));
    lpwstr = new WCHAR[wstr.length()+1];
    wstr.copy(lpwstr, wstr.length());
    wstr[wstr.length()] = L'\0';
    service.lpszServiceInstanceName = lpwstr;

    wstr = std::move(converter.from_bytes(srv_desc));
    lpwstr = new WCHAR[wstr.length() + 1];
    wstr.copy(lpwstr, wstr.length());
    wstr[wstr.length()] = L'\0';
    service.lpszComment = lpwstr;

    uuid_t* privateuuid = new uuid_t(uuid);

    service.lpServiceClassId = privateuuid;
    service.dwNumberOfCsAddrs = 1;
    service.dwNameSpace = NS_BTH;
    // cr�ation de l'adresse du serveur
    sockaddr_rc *addr = new sockaddr_rc;
    memset(addr, 0, sizeof(*addr));
    addr->addressFamily = static_cast<uint16_t>(BluetoothSocket::address_family::bth);
    addr->port = port;
    // cr�ation des information de service
    CSADDR_INFO *csAddr = new CSADDR_INFO;
    memset(csAddr, 0, sizeof(CSADDR_INFO));
    csAddr->LocalAddr.iSockaddrLength = sizeof(*addr);
    csAddr->LocalAddr.lpSockaddr = reinterpret_cast<sockaddr*>(addr);
    csAddr->iSocketType = static_cast<uint32_t>(Socket::types::stream);
    csAddr->iProtocol = static_cast<uint32_t>(BluetoothSocket::protocols::rfcomm);
    service.lpcsaBuffer = csAddr;
    // Essayer d'enregistrer le service
    if (WSASetServiceW(&service, RNRSERVICE_REGISTER, 0))
    {
        delete addr;
        delete csAddr;
        delete privateuuid;
        service.lpcsaBuffer = nullptr;

        int32_t err = WSAGetLastError();
        switch (err)
        {
            case WSANOTINITIALISED: throw wsa_not_initialised("A successful WSAStartup call must occur before using this function.");
            case WSAEINVAL: throw error_in_value("One or more required parameters were invalid or missing.");
            default: throw sdp_service_exception("register_sdp_service exception: " + std::to_string(err));
        }
    }
}

void BluetoothSocket::unregister_sdp_service(service_t &service)
{
    int res = WSASetServiceW(&service, RNRSERVICE_DELETE, 0);

    delete   service.lpcsaBuffer->LocalAddr.lpSockaddr;
    delete   service.lpcsaBuffer;
    delete   service.lpServiceClassId;
    delete[] service.lpszServiceInstanceName;
    delete[] service.lpszComment;
    memset(&service, 0, sizeof(service_t));

    if (res)
    {
        res = WSAGetLastError();
        switch (res)
        {
            case WSANOTINITIALISED: throw wsa_not_initialised("A successful WSAStartup call must occur before using this function.");
            case WSAEINVAL: throw error_in_value("One or more required parameters were invalid or missing.");
            default: throw sdp_service_exception("register_sdp_service exception: " + std::to_string(res));
        }
    }
}

#elif defined(__LINUX__)

///////////////////////////////////////////////////////////////////////////////
// Uuid Class

void Uuid::uuid16_to_uuid128(uint16_t uuid)
{
    uuid32_to_uuid128(uuid);
}

void Uuid::uuid32_to_uuid128(uint32_t uuid)
{
    _type = Uuid::type::uuid128;
    _uuid.type = SDP_UUID128;
    //            Bluetooth Base UUID
    memcpy(&_uuid.value, BluetoothSocket::bth_base_uuid, sizeof(BluetoothSocket::bth_base_uuid));
    // UUID is always stored big-endian (on Linux)
    _uuid.value.uuid32 = utils::Endian::net_swap(uuid);
}

void Uuid::uuid128_to_uuid16(uuid_t const& uuid)
{
    _type = Uuid::type::uuid16;
    memset(&_uuid, 0, sizeof(_uuid));
    _uuid.type = SDP_UUID16;
    _uuid.value.uuid16 = uuid.value.uuid16;
}

void Uuid::uuid128_to_uuid32(uuid_t const& uuid)
{
    _type = Uuid::type::uuid32;
    memset(&_uuid, 0, sizeof(_uuid));
    _uuid.type = SDP_UUID32;
    _uuid.value.uuid32 = uuid.value.uuid32;
}

void Uuid::from_string(std::string const& struuid)
{
    // si c'est une uuid
    if (is_valid_uuid(struuid))
    {
        _type = Uuid::type::uuid128;
        uint8_t* datas = _uuid.value.uuid128.data;
        _uuid.type = SDP_UUID128;
        sscanf(struuid.c_str(), __UUID128_PRINTF_FORMAT__,
            &datas[0], &datas[1], &datas[2], &datas[3],
            &datas[4], &datas[5],
            &datas[6], &datas[7],
            &datas[8], &datas[9],
            &datas[10], &datas[11], &datas[12], &datas[13], &datas[14], &datas[15]);
    }
    else
    {
        memset(&_uuid, 0, sizeof(_uuid));
    }
}

std::string Uuid::to_string()
{
    char str[37] = { 0 };
    if (_type == Uuid::type::uuid128)
    {
        uint8_t const* datas = _uuid.value.uuid128.data;
        snprintf(str, 37, __UUID128_PRINTF_FORMAT__,
            datas[0], datas[1], datas[2], datas[3],
            datas[4], datas[5],
            datas[6], datas[7],
            datas[8], datas[9],
            datas[10], datas[11], datas[12], datas[13], datas[14], datas[15]);
    }
    return std::string(str);
}

void Uuid::set_uuid128(uuid_t const& uuid)
{
    _type = Uuid::type::uuid128;
    memcpy(&_uuid, &uuid, sizeof(uuid));
}

bool Uuid::operator ==(Uuid const& other)
{
    if (_type != other._type)
        return false;

    return memcmp(&_uuid, &other._uuid, sizeof(_uuid)) == 0;
}

bool Uuid::operator !=(Uuid const& other)
{
    return !(*this == other);
}

uuid_t const& Uuid::get_native_uuid() const
{
    return _uuid;
}

///////////////////////////////////////////////////////////////////////////////
// BluetoothSocket Class

int BluetoothSocket::inet_pton(BluetoothSocket::address_family family, std::string const& str_addr, void* out_buf)
{
    if (family != BluetoothSocket::address_family::bth)
        return 0;

    if (str2ba(str_addr.c_str(), reinterpret_cast<bdaddr_t*>(out_buf)) == -1)
        return 0;

    return 1;
}

void BluetoothSocket::inet_ntop(BluetoothSocket::address_family family, const void* addr, std::string& str_addr)
{
    if (family != BluetoothSocket::address_family::bth)
        throw error_in_value("Error in family, cannot parse addr");

    str_addr.clear();
    str_addr.resize(17);
    if(ba2str(reinterpret_cast<const bdaddr_t*>(addr), &str_addr[0]) != 17 )
        throw error_in_value("Error in value, cannot parse addr");
}

int BluetoothSocket::scan_open_port_from_uuid(Uuid const& _uuid, bdaddr_t const& addr)
{
    bdaddr_t Any = { { 0 } };
    int port = -1;
    sdp_session_t *session = 0;
    sdp_list_t *response_list, *search_list, *attrid_list;
    uint32_t range = 0x0000FFFF;
    uuid_t uuid = _uuid.get_native_uuid();

    // connexion au sdp de l'appareil � l'adresse bthaddr
    session = sdp_connect(&Any, &addr, 0);
    // si on s'est connect�
    if (session)
    {
        // cr�ation des donn�es uuid
        search_list = sdp_list_append(0, &uuid);
        // cr�ation des donn�es de plage
        attrid_list = sdp_list_append(0, &range);
        // recherche du service
        if (!sdp_service_search_attr_req(session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list))
        {
            sdp_list_t *proto_list;
            sdp_list_t *r = response_list;
            // pour tout les services
            for (; r; r = r->next)
            {
                // r�cup�ration de l'enregistrement du service
                sdp_record_t *rec = (sdp_record_t*)r->data;
                // r�cup�ration des protocols
                if (sdp_get_access_protos(rec, &proto_list) == 0)
                {
                    // r�cup�ration du canal ouvert
                    port = sdp_get_proto_port(proto_list, RFCOMM_UUID);
                    // lib�ration des donn�es
                    sdp_list_free(proto_list, 0);
                }
                // lib�ration des donn�es du service
                sdp_record_free(rec);
            }
        }
        // lib�ration des donn�es de recherches
        sdp_list_free(response_list, 0);
        sdp_list_free(search_list, 0);
        sdp_list_free(attrid_list, 0);
        // fermeture de la connexin
        sdp_close(session);
    }

    return port;
}

#ifdef __BLUEZ_4__
list<BluetoothDevice> BluetoothSocket::scan(bool flushCache)
{
    inquiry_info *iinfo;
    int dev_id, sock;
    int trouve;
    char nom[255];
    BluetoothDevice device;
    list<BluetoothDevice> devices;

    // NULL permet de r�cup�rer le 1er dispositif Bluetooth libre
    // Nous pourrions sp�cifier 0 pour le 1er dispositif, mais si il y a plusieurs
    // dispositifs, il se pourrait qu'il ai un Identifiant diff�rent tel que 1
    // retourne l'ID du dispositif bluetooth libre
    dev_id = hci_get_route(NULL);
    if (dev_id < 0)
    {
        throw socket_exception("No local bluetooth device available.");
    }

    // Ouvrir la connexion du dispositif bluetooth
    sock = hci_open_dev(dev_id);
    if (sock < 0)
    {
        throw socket_exception("Unable to open local bluetooth device.");
    }

    // allocation d'autant inquiry_info que d'appareils potentiellement trouvable, ici 255 appareils maxi
    iinfo = new inquiry_info[255];

    // Fonction hci_inquiry
    // Usage : active la d�couverte Bluetooth et r�cup�re le nombre d'appareils actuellement visible en Bluetooth

    // int hci_inquiry(int dev_id, int len, int max_rsp, const uint8_t *lap, inquiry_info **ii, long flags);
    // dev_id : dispositif trouv� gr�ce � hci_get_route, si dev_id = -1, la fonction recherchera toute seule
    // un dispositif.
    // len     : 1.28 * len = nombre de secondes de la d�couverte, la norme pr�voit 5 � 15 secondes de d�couverte
    //             1.28 * 8 = 10.24s, qui est ~ la moyenne
    // max_rsp: nb d'appareils maxi (�galement la taille de ii)
    // lap     : nombre de fois que la d�couverte recommence si il y a une erreur???
    // ii      : structure de requ�te
    // flags  : IREQ_CACHE_FLUSH = vide le buffer des anciennes d�couvertes, permet de remettre � jour les
    // appareils actuellement pr�sent.
    // Retourne le nombre d'appareils trouv�s.
    trouve = hci_inquiry(dev_id, 8, 255, NULL, &iinfo, flushCache ? IREQ_CACHE_FLUSH : 0);
    if (trouve > 0)
    {
        for (int i = 0; i < trouve; ++i)
        {
            // r�cup�re le nom de l'appareil ayant l'adresse bdaddr
            if (hci_read_remote_name(sock, &iinfo[i]->bdaddr, 255, nom, 0) < 0)
                strcpy(nom, "[inconnu]");

            device.name = nom;
            device.addr = iinfo[i]->bdaddr;

            devices.push_back(device);
        }
    }
    delete[] iinfo;
    hci_close_dev(sock);
    return devices;
}

void BluetoothSocket::register_sdp_service(service_t & service, uuid_t const& uuid, uint8_t port, std::string const&srv_name, std::string const&srv_prov, std::string const&srv_desc)
{
    bdaddr_t addr_any = { { 0 } };
    bdaddr_t addr_local = { { 0, 0, 0, 0xFF, 0xFF, 0xFF } };
    bool r = false;

    // d�marrer une connexion avec le sdp
    service = sdp_connect(&addr_any, &addr_local, SDP_RETRY_IF_BUSY);
    if (service == 0)
        throw sdp_service_exception("Can't connect to local SDP server.");

    // les UUID du service
    uuid_t root_uuid, l2cap_uuid, rfcomm_uuid;
    // les listes de donn�es n�cessaire � l'enregistrement du service
    sdp_list_t *l2cap_list = 0,
        *rfcomm_list = 0,
        *root_list = 0,
        *proto_list = 0,
        *access_proto_list = 0;
    // le canal du service qui est ouvert
    sdp_data_t *channel = 0;
    // le service
    sdp_record_t *record = sdp_record_alloc();

    // affecter l'uuid du service
    sdp_set_service_id(record, uuid);

    // cr�er l'uuid permettant de rechercher le service
    sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
    // affectation de l'uuid
    root_list = sdp_list_append(0, &root_uuid);
    // mettre l'enregistrement du service visible par tous
    sdp_set_browse_groups(record, root_list);

    // cr�ation de l'uuid L2CAP
    sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
    // cr�er les informations L2CAP de l'uuid
    l2cap_list = sdp_list_append(0, &l2cap_uuid);
    // cr�er les informations du protocol L2CAP pour le service
    proto_list = sdp_list_append(0, l2cap_list);

    // cr�er l'uuid RFCOMM
    sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
    // cr�er l'information du canal du service RFCOMM
    channel = sdp_data_alloc(SDP_UINT8, &port);
    // cr�er les informations de l'uuid du RFCOMM
    rfcomm_list = sdp_list_append(0, &rfcomm_uuid);
    // ajouter les information du canal ouvert pour le RFCOMM
    sdp_list_append(rfcomm_list, channel);
    // ajout des informations RFCOMM � la liste
    sdp_list_append(proto_list, rfcomm_list);

    // cr�er les informations d'acc�s du service
    access_proto_list = sdp_list_append(0, proto_list);
    // mettre � jour les informations des protocol de l'enregistrement du service
    sdp_set_access_protos(record, access_proto_list);
    // concactenner toutes les informations du service
    sdp_set_info_attr(record, srv_name.c_str(), srv_prov.c_str(), srv_desc.c_str());

    // enregistrer le service
    if (sdp_record_register(service, record, 0))
        r = true;

    // lib�ration des donn�es
    sdp_data_free(channel);
    sdp_list_free(l2cap_list, 0);
    sdp_list_free(rfcomm_list, 0);
    sdp_list_free(root_list, 0);
    sdp_list_free(access_proto_list, 0);
    sdp_record_free(record);

    if (r)
    {
        sdp_close(service);
        throw sdp_service_exception("Unable to register SDP service.");
    }
}

void BluetoothSocket::unregister_sdp_service(service_t &service)
{
    sdp_close(service);
}

#else
#include <iostream>
std::list<BluetoothDevice> BluetoothSocket::scan(bool flushCache)
{
    (void)flushCache;

    BluezDBusSystem& bus = BluezDBusSystem::Inst();
    DBusMessage *msg, *ret;
    DBusMessageIter iter;
    // initialise the errors

    if( (msg = dbus_message_new_method_call("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects")) == nullptr )
    {
        throw socket_exception("New method call error (" + std::string(bus.message()) + ')');
    }

    if( (ret = dbus_connection_send_with_reply_and_block(bus, msg, DBUS_TIMEOUT_INFINITE, bus)) == nullptr )
    {
        throw socket_exception("Send (" + std::string(bus.message()) + ')');
    }

    dbus_message_iter_init(ret, &iter);
    char *tmp, *tmp2;
    std::string adapter_path;
    BluetoothDevice device;
    std::list<BluetoothDevice> devices;
    bool found_device;
    // a{oa{sa{sv}}}
    // Pour chaque valeur du tableau de dictionnaire
    for( int type; (type = dbus_message_iter_get_arg_type (&iter)) != DBUS_TYPE_INVALID; dbus_message_iter_next (&iter))
    {
        DBusMessageIter Dict1;
        // On r�cup�re la valeur du tableau (par exemple a[0], a[1], etc...)
        dbus_message_iter_recurse(&iter, &Dict1);
        // Pour chaque valeur du tableau
        for( int type; (type = dbus_message_iter_get_arg_type(&Dict1)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict1) )
        {
            DBusMessageIter DictEntry1;
            // On rentre dans le dictionnaire
            dbus_message_iter_recurse(&Dict1, &DictEntry1);
            // Pour chaque entr�e du dictionnaire (� un object_path on associe un tableau)
            // Par exemple o["toto"], o["/org/bluez/hci0"], etc...)
            for( int type; (type = dbus_message_iter_get_arg_type(&DictEntry1)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&DictEntry1) )
            {
                // On r�cup�re la cl� du dictionnaire
                dbus_message_iter_get_basic(&DictEntry1, &tmp);
                dbus_message_iter_next(&DictEntry1);
                DBusMessageIter Dict2;
                // On entre dans le tableau de dictionnaires
                dbus_message_iter_recurse(&DictEntry1, &Dict2);
                for( int type; (type = dbus_message_iter_get_arg_type(&Dict2)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict2) )
                {
                    DBusMessageIter DictEntry2;
                    // On rentre dans le dictionnaire
                    dbus_message_iter_recurse(&Dict2, &DictEntry2);
                    // Pour chaque entr�e du dictionnaire (� une string on associe un tableau)
                    for( int type; (type = dbus_message_iter_get_arg_type(&DictEntry2)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&DictEntry2) )
                    {
                        // On r�cup�re la cl� du dictionnaire
                        dbus_message_iter_get_basic(&DictEntry2, &tmp2);
                        dbus_message_iter_next(&DictEntry2);
                        if( !strcmp(tmp2, "org.bluez.Adapter1") )
                        {
                            adapter_path = tmp;
                            break;
                        }
                    }
                    if( !adapter_path.empty() )
                        break;
                }
                if( !adapter_path.empty() )
                    break;
            }
            if( !adapter_path.empty() )
                break;
        }
        if( !adapter_path.empty() )
            break;
    }

    if( (msg = dbus_message_new_method_call("org.bluez", adapter_path.c_str(), "org.bluez.Adapter1", "StartDiscovery")) == nullptr )
    {
        throw socket_exception("New method call error (" + std::string(bus.message()) + ')');
    }

    if( (ret = dbus_connection_send_with_reply_and_block(bus, msg, DBUS_TIMEOUT_INFINITE, bus)) == nullptr )
    {
        throw socket_exception("Send (" + std::string(bus.message()) + ')');
    }
    
    sleep(10);
    
    // Pour chaque valeur du tableau de dictionnaire
    for( int type; (type = dbus_message_iter_get_arg_type (&iter)) != DBUS_TYPE_INVALID; dbus_message_iter_next (&iter))
    {
        DBusMessageIter Dict1;
        // On r�cup�re la valeur du tableau (par exemple a[0], a[1], etc...)
        dbus_message_iter_recurse(&iter, &Dict1);
        // Pour chaque valeur du tableau
        for( int type; (type = dbus_message_iter_get_arg_type(&Dict1)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict1) )
        {
            DBusMessageIter DictEntry1;
            // On rentre dans le dictionnaire
            dbus_message_iter_recurse(&Dict1, &DictEntry1);
            // Pour chaque entr�e du dictionnaire (� un object_path on associe un tableau)
            // Par exemple o["toto"], o["/org/bluez/hci0"], etc...)
            for( int type; (type = dbus_message_iter_get_arg_type(&DictEntry1)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&DictEntry1) )
            {
                // On r�cup�re la cl� du dictionnaire
                dbus_message_iter_get_basic(&DictEntry1, &tmp);
                dbus_message_iter_next(&DictEntry1);
                DBusMessageIter Dict2;
                // On entre dans le tableau de dictionnaires
                dbus_message_iter_recurse(&DictEntry1, &Dict2);
                for( int type; (type = dbus_message_iter_get_arg_type(&Dict2)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict2) )
                {
                    DBusMessageIter DictEntry2;
                    // On rentre dans le dictionnaire
                    dbus_message_iter_recurse(&Dict2, &DictEntry2);
                    // Pour chaque entr�e du dictionnaire (� une string on associe un tableau)
                    for( int type; (type = dbus_message_iter_get_arg_type(&DictEntry2)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&DictEntry2) )
                    {
                        // On r�cup�re la cl� du dictionnaire
                        dbus_message_iter_get_basic(&DictEntry2, &tmp2);
                        dbus_message_iter_next(&DictEntry2);
                        DBusMessageIter Dict3;
                        // On entre dans le tableau de dictionnaires
                        dbus_message_iter_recurse(&DictEntry2, &Dict3);
                        if( !strcmp(tmp2, "org.bluez.Device1") )
                        {
                            device.addr = {0};
                            device.name.clear();
                            found_device = false;
                            DBusMessageIter Dict3;
                            // On entre dans le tableau de dictionnaires
                            dbus_message_iter_recurse(&DictEntry2, &Dict3);
                            for( int type; (type = dbus_message_iter_get_arg_type(&Dict3)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict3) )
                            {
                                DBusMessageIter DictEntry3;
                                // On rentre dans le dictionnaire
                                dbus_message_iter_recurse(&Dict3, &DictEntry3);
                                dbus_message_iter_get_basic(&DictEntry3, &tmp);
                                dbus_message_iter_next(&DictEntry3);
                                if( !strcmp(tmp, "Address") )
                                {
                                    DBusMessageIter variant;
                                    dbus_message_iter_recurse(&DictEntry3, &variant);     
                                    dbus_message_iter_get_basic(&variant, &tmp);
                                    BluetoothSocket::inet_pton(BluetoothSocket::address_family::bth, tmp, &device.addr);
                                    found_device = true;
                                    
                                }
                                else if( !strcmp(tmp, "Name") )
                                {
                                    DBusMessageIter variant;
                                    dbus_message_iter_recurse(&DictEntry3, &variant);     
                                    dbus_message_iter_get_basic(&variant, &tmp);
                                    device.name = tmp;
                                    found_device = true;
                                }
                            }
                            if( found_device )
                            {
                                devices.emplace_back(device);
                            }
                        }
                    }
                }
            }
        }
    }
    
    if( (msg = dbus_message_new_method_call("org.bluez", adapter_path.c_str(), "org.bluez.Adapter1", "StopDiscovery")) == nullptr )
    {
        throw socket_exception("New method call error (" + std::string(bus.message()) + ')');
    }

    if( (ret = dbus_connection_send_with_reply_and_block(bus, msg, DBUS_TIMEOUT_INFINITE, bus)) == nullptr )
    {
        throw socket_exception("Send (" + std::string(bus.message()) + ')');
    }
    
    /*
    // org.bluez.Device1
    // Attr: Address
    // Attr: Name
    if( interface == "org.bluez.Device1" )
    {
        DBusMessageIter Dict3;
        // On entre dans le tableau de dictionnaires
        dbus_message_iter_recurse(&DictEntry2, &Dict3);
        for( int type; (type = dbus_message_iter_get_arg_type(&Dict3)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict3) )
        {
            DBusMessageIter DictEntry3;
            // On rentre dans le dictionnaire
            dbus_message_iter_recurse(&Dict3, &DictEntry3);
            dbus_message_iter_get_basic(&DictEntry3, &tmp);
            attr = tmp;
            dbus_message_iter_next(&DictEntry3);
        }
    }
    */
    return devices;
}

void BluetoothSocket::register_sdp_service(service_t & service, uuid_t const& uuid, uint8_t port, std::string const&srv_name, std::string const& srv_prov, std::string const& srv_desc)
{
    BluezDBusSystem& bus = BluezDBusSystem::Inst();
    DBusMessage *msg, *ret;
    DBusMessageIter iter, dict, entry, variant;
    if( (msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.ProfileManager1", "RegisterProfile")) == nullptr )
    {
        throw sdp_service_exception("New method call error (" + std::string(bus.message()) + ')');
    }
    std::string sdp_session = "/PortableAPI/";
    sdp_session += srv_name;
    std::string struuid = Uuid(uuid).to_string();
    const char* recordkey = "ServiceRecord";
    const char *tmp;

    std::stringstream sstr;
    sstr  << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"   << std::endl
          << "<record>"                                      << std::endl
          << "    <attribute id=\"0x0003\">"                 << std::endl // SDP_ATTRIB_SERVICE_ID
          << "        <uuid value=\"" << struuid << "\" />"  << std::endl
          << "    </attribute>"                              << std::endl 
          << "    <attribute id=\"0x0004\">"                 << std::endl // SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST
          << "        <sequence>"                            << std::endl
          << "            <sequence>"                        << std::endl
          << "                <uuid value=\"0x0100\" />"     << std::endl
          << "            </sequence>"                       << std::endl
          << "            <sequence>"                        << std::endl
          << "                <uuid value=\"0x0003\" />"     << std::endl // RFCOMM_PROTOCOL_UUID16 == 0x0003
          << "                <uint8 value=\"0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint16_t>(port) << std::dec << "\" />" << std::endl
          << "            </sequence>"                       << std::endl
          << "        </sequence>"                           << std::endl
          << "    </attribute>"                              << std::endl
          << "    <attribute id=\"0x0005\">"                 << std::endl // SDP_ATTRIB_BROWSE_GROUP_LIST
          << "        <sequence>"                            << std::endl
          << "            <uuid value=\"0x1002\" />"         << std::endl
          << "        </sequence>"                           << std::endl
          << "    </attribute>"                              << std::endl
          << "    <attribute id=\"0x0100\">"                 << std::endl
          << "        <text value=\"" << srv_name << "\" />" << std::endl
          << "    </attribute>"                              << std::endl
          << "    <attribute id=\"0x0101\">"                 << std::endl
          << "        <text value=\"" << srv_prov << "\" />" << std::endl
          << "    </attribute>"                              << std::endl
          << "    <attribute id=\"0x0102\">"                 << std::endl
          << "        <text value=\"" << srv_desc << "\" />" << std::endl
          << "    </attribute>"                              << std::endl
          << "</record>";
    std::string sdprecord(sstr.str());
    // Cr�ation des param�tres de la fonction pour dbus
    dbus_message_iter_init_append(msg, &iter);
    // On ajoute aux param�tres le nom du service
    tmp = sdp_session.c_str();
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &tmp);
    // On ajoute aux param�tres l'uuid du service
    tmp = struuid.c_str();
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &tmp);
    // On cr�� un tableau de dictionnaires pour les autres param�tres
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
        DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
            DBUS_TYPE_STRING_AS_STRING
            DBUS_TYPE_VARIANT_AS_STRING
        DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
        &dict);
    // On cr�� une entr�e dans le dictionnaire
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, 0, &entry);
    // On affecte la valeur de la cl� du dictionnaire
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &recordkey);
    // On cr�� un variant de type string
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant);
    // On affecte la valeur du variant
    tmp = sdprecord.c_str();
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &tmp);
    // On ferme le variant
    dbus_message_iter_close_container(&entry, &variant);
    // On ferme l'entr�e du dictionnaire
    dbus_message_iter_close_container(&dict, &entry);
    // On ferme le tableau de dictionnaires
    dbus_message_iter_close_container(&iter, &dict);

    if( (ret = dbus_connection_send_with_reply_and_block(bus, msg, DBUS_TIMEOUT_INFINITE, bus)) == nullptr )
    {
        throw sdp_service_exception("Unable to register SDP service!");
    }

    service = std::move(sdp_session);
}

void BluetoothSocket::unregister_sdp_service(service_t &service)
{
    BluezDBusSystem& bus = BluezDBusSystem::Inst();
    DBusMessage *msg, *ret;
    DBusMessageIter iter;
    const char *tmp = service.c_str();
    if( (msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.ProfileManager1", "UnregisterProfile")) == nullptr )
    {
        throw sdp_service_exception("New method call error (" + std::string(bus.message()) + ')');
    } 
    // Cr�ation des param�tres de la fonction pour dbus
    dbus_message_iter_init_append(msg, &iter);
    // On ajoute aux param�tres le nom du service
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &tmp);
    if( (ret = dbus_connection_send_with_reply_and_block(bus, msg, DBUS_TIMEOUT_INFINITE, bus)) == nullptr )
    {
        throw sdp_service_exception("Unable to unregister SDP service!");
    }
}

#endif
#endif

SDPService::SDPService():
    _registered(false),
    _service(new service_t)
{}

SDPService::SDPService(SDPService &&other) noexcept
{
    _registered = other._registered; other._registered = false;
    _service = other._service; other._service = nullptr;
    _uuid = other._uuid;
    _name = std::move(other._name);
    _description = std::move(other._name);
    _provider = std::move(other._provider);
}

SDPService& SDPService::operator =(SDPService &&other) noexcept
{
    _registered = other._registered; other._registered = false;
    _service = other._service; other._service = nullptr;
    _uuid = other._uuid;
    _name = std::move(other._name);
    _description = std::move(other._name);
    _provider = std::move(other._provider);
    return *this;
}

SDPService::~SDPService()
{
    if (_registered)
        unregisterService();

    if( _service != nullptr )
        delete _service;
}

void SDPService::registerService(Uuid const& uuid, uint8_t port, std::string const&name, std::string const& provider, std::string const& description)
{
    if (_registered)
        throw error_in_value("Service already registered.");
    
    BluetoothSocket::register_sdp_service(*_service, uuid.get_native_uuid(), port, name, provider, description);

    _uuid = uuid;
    _name = name;
    _registered = true;
}

void SDPService::unregisterService()
{
    if (!_registered)
        throw error_in_value("Service not registered.");

    BluetoothSocket::unregister_sdp_service(*_service);
    _registered = false;
}

bool SDPService::is_registered() const { return _registered; }
Uuid const& SDPService::get_uuid() const { return _uuid; }
std::string const& SDPService::get_name() const { return _name; }
std::string const& SDPService::get_description() const { return _description; }
std::string const& SDPService::get_provider() const { return _provider; }

#endif//__APPLE__