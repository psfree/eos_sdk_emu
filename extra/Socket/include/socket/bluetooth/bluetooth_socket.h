/* Copyright (C) Nemirtingas
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

#ifndef __BLUETOOTH_SOCKET_INCLUDED__
#define __BLUETOOTH_SOCKET_INCLUDED__

#include "../common/socket.h"

#if defined(UTILS_OS_LINUX)

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#ifdef __BLUEZ_4__
typedef sdp_session_t* service_t;
#else
typedef std::string service_t;
#endif

#elif defined(UTILS_OS_WINDOWS)

#include <ws2bth.h>
#include <bluetoothapis.h>
#include <initguid.h>

#pragma comment(lib, "Bthprops.lib")

typedef GUID uuid_t;
typedef BTH_ADDR bdaddr_t;
typedef SOCKADDR_BTH sockaddr_rc;
typedef WSAQUERYSETW service_t;

#endif

#if defined(UTILS_OS_WINDOWS) || defined(UTILS_OS_LINUX)

#include <list>

namespace PortableAPI
{
#define SOCKET_EXCEPTION_CLASS(x) \
class x : public socket_exception \
{\
public:\
    x(const char* mywhat):socket_exception(mywhat){}\
    x(std::string const& mywhat):socket_exception(mywhat){}\
}

    SOCKET_EXCEPTION_CLASS(sdp_service_exception);

#undef SOCKET_EXCEPTION_CLASS

    ////////////
    /// @brief Class with address and name about a device
    ////////////
    class BluetoothDevice
    {
    public:
        bdaddr_t addr;
        std::string name;

        BluetoothDevice()                            = default;
        BluetoothDevice(BluetoothDevice const&)      = default;
        BluetoothDevice(BluetoothDevice &&) noexcept = default;

        BluetoothDevice& operator=(BluetoothDevice const&)      = default;
        BluetoothDevice& operator=(BluetoothDevice &&) noexcept = default;
    };

    ////////////
    /// @brief Utility Class for manipulating uuids
    ////////////
    class Uuid
    {
    public:
        enum class type : uint8_t
        {
            none,
            uuid16,
            uuid32,
            uuid128
        };

    private:
        Uuid::type _type;
        uuid_t _uuid;

    public:
        ////////////
        /// @brief Converts a bluetooth uuid16 to uuid128
        /// @param[in] uuid uuid16
        /// @return 
        ////////////
        void uuid16_to_uuid128(uint16_t uuid);
        ////////////
        /// @brief Converts a bluetooth uuid32 to uuid128
        /// @param[in] uuid uuid32
        /// @return 
        ////////////
        void uuid32_to_uuid128(uint32_t uuid);
        ////////////
        /// @brief Converts a bluetooth uuid128 to uuid16
        /// @param[in] uuid uuid128
        /// @return 
        ////////////
        void uuid128_to_uuid16(uuid_t const& uuid);
        ////////////
        /// @brief Converts a bluetooth uuid128 to uuid32
        /// @param[in] uuid uuid128
        /// @return 
        ////////////
        void uuid128_to_uuid32(uuid_t const& uuid);
        ////////////
        /// @brief Checks if the string is a valid uuid
        ///        Must be formated like so: 01234567-89AB-CDEF-0123-456789ABCDEF
        /// @param[in] struuid A string representation of an UUID
        /// @return Is valid
        ////////////
        static bool is_valid_uuid(std::string const& struuid);
        ////////////
        /// @brief Set the internal value from the string
        ///        Must be formated like to: 01234567-89AB-CDEF-0123-456789ABCDEF
        /// @param[in] struuid A string representation of an UUID
        /// @return 
        ////////////
        void from_string(std::string const& struuid);
        ////////////
        /// @brief Transforms the internal UUID into a human readable string
        /// @return UUID as string
        ////////////
        std::string to_string();
        ////////////
        /// @brief Sets the UUID from the native type
        /// @param[in] uuid The native uuid
        /// @return 
        ////////////
        void set_uuid128(uuid_t const& uuid);
        ////////////
        /// @brief Check if the other uuid equals us
        /// @param[in] other The other UUID to compare to
        /// @return Is equal
        ////////////
        bool operator ==(Uuid const& other);
        ////////////
        /// @brief Check if the other uuid equals us
        /// @param[in] other The other UUID to compare to
        /// @return Is not equal
        ////////////
        bool operator !=(Uuid const& other);
        ////////////
        /// @brief Get the native uuid
        /// @return The native uuid
        ////////////
        uuid_t const& get_native_uuid() const;
        ////////////
        /// @brief 
        ////////////
        Uuid();
        ////////////
        /// @brief Build the UUID from the native type
        /// @param[in] uuid The native uuid
        /// @return 
        ////////////
        Uuid(uuid_t const& uuid);
    };

    ////////////
    /// @brief A wrapper class for 'C' network & socket Bluetooth functions
    ////////////
    class BluetoothSocket : public Socket
    {
    public:
        /////////////
        /// @brief Bluetooth network address family enum 
        ////////////
        enum class address_family
        {
#if defined(UTILS_OS_WINDOWS)
            bth = AF_BTH,
#elif defined(UTILS_OS_LINUX)
            bth = AF_BLUETOOTH,
#endif
        };

        ////////////
        /// @brief Bluetooth network protocols enum
        ////////////
        enum class protocols
        {
#if defined(UTILS_OS_WINDOWS)
            rfcomm = BTHPROTO_RFCOMM,
            l2cap = BTHPROTO_L2CAP,
#elif defined(UTILS_OS_LINUX)
            rfcomm = BTPROTO_RFCOMM,
            l2cap = BTPROTO_L2CAP,
#endif
        };

#if defined(UTILS_OS_WINDOWS)
        //                                                                                 Here, Windows is Host ordered
        static constexpr uint8_t bth_base_uuid[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
#elif defined(UTILS_OS_LINUX)
        //                                                                                 Here, Linux is big endian
        static constexpr uint8_t bth_base_uuid[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
#endif

        ////////////
        /// @brief Wrapper for 'C' inet_pton function. Transforms a string to its binary representation
        /// @param[in]  family   The address family. Must be BluetoothSocket::adress_family::bth
        /// @param[in]  str_addr A string that contains the text representation of the address to convert to numeric binary form.
        /// @param[out] out_buf  A pointer to a buffer in which to store the numeric binary representation of the address. The address is returned in network byte order.
        /// @return 1 on success
        ////////////
        static int inet_pton(BluetoothSocket::address_family family, std::string const& str_addr, void* out_buf);
        ////////////
        /// @brief Wrapper for 'C' inet_pton function. Transforms a binary representation to a human readable string
        /// @param[in]  family   The address family. Must be BluetoothSocket::adress_family::bth
        /// @param[in]  addr     A pointer to a buffer in which 
        /// @param[out] str_addr A string in which to store the representation of the address.
        /// @return 
        ////////////
        static void inet_ntop(BluetoothSocket::address_family family, const void* addr, std::string& str_addr);
        ////////////
        /// @brief Starts a Bluetooth scan to detect nearby devices
        /// @param[in]  flushCache Flush the cache before scanning
        /// @return A list of BluetoothDevices
        ////////////
        static std::list<BluetoothDevice> scan(bool flushCache = true);
        ////////////
        /// @brief Query the remote device's SDP about the UUID's port
        /// @param[in]  uuid    The remote service UUID
        /// @param[in]  bthaddr The remote addr
        /// @return The service port or -1 if the function failed or if there is no service
        ////////////
        static int scan_open_port_from_uuid(Uuid const&uuid, bdaddr_t const&bthaddr);
        ////////////
        /// @brief Register an uuid/port pair into the Bluetooth SDP
        ///        Can throw a sdp_service_exception
        /// @param[inout] The service handle
        /// @param[in]    The service uuid
        /// @param[in]    The service port
        /// @param[in]    The service name
        /// @param[in]    The service provider
        /// @param[in]    The service description
        /// @return        
        ////////////
        static void register_sdp_service(service_t & service, uuid_t const& uuid, uint8_t port, std::string const&srv_name, std::string const&srv_prov, std::string const&srv_desc);
        ////////////
        /// @brief Unregister a SDP record
        ///        Can throw a sdp_service_exception
        /// @param[inout] The service handle
        /// @return        
        ////////////
        static void unregister_sdp_service(service_t &service);
    };

    ////////////
    /// @brief A SDP Record class
    ////////////
    class SDPService
    {
        friend class BluetoothSocket;

        bool _registered;
        service_t *_service;
        Uuid _uuid;
        std::string _name;
        std::string _description;
        std::string _provider;

        SDPService(SDPService const&) = delete;
        SDPService& operator =(SDPService const&) = delete;

    public:
        SDPService();
        SDPService(SDPService &&) noexcept;
        SDPService& operator =(SDPService &&) noexcept;
        ~SDPService();

        void registerService(Uuid const& uuid, uint8_t port, std::string const&name, std::string const&prov, std::string const&desc);
        void unregisterService();
        bool is_registered() const;
        Uuid const& get_uuid() const;
        std::string const& get_name() const;
        std::string const& get_description() const;
        std::string const& get_provider() const;
    };
}
#endif

#endif//__BLUETOOTH_SOCKET_INCLUDED__
