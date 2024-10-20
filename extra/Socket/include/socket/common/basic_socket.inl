

///////////////////////////////////////////////////////////////////////////////
// connected_socket class
///////////////////////////////////////////////////////////////////////////////
template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
connected_socket<Addr, family, type, proto>::connected_socket():
    basic_socket(Socket::socket(family, type, proto))
{
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
connected_socket<Addr, family, type, proto>::connected_socket(Socket::socket_t s):
    basic_socket(s)
{
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
inline void connected_socket<Addr, family, type, proto>::socket()
{
    basic_socket::socket(family, type, proto);
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
inline typename connected_socket<Addr, family, type, proto>::myaddr_t const& connected_socket<Addr, family, type, proto>::get_addr() const
{
    return _addr;
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
inline void connected_socket<Addr, family, type, proto>::listen(int waiting_socks)
{
    Socket::listen(*_sock, waiting_socks);
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
inline void connected_socket<Addr, family, type, proto>::connect(myaddr_t const& addr)
{
    myaddr_t new_addr(addr);
    try
    {
        Socket::connect(*_sock, new_addr);
        _addr = std::move(new_addr);
    }
    catch (would_block&)
    {
        _addr = std::move(new_addr);
        throw;
    }
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
inline typename connected_socket<Addr, family, type, proto>::mytype_t connected_socket<Addr, family, type, proto>::accept()
{
    myaddr_t addr;
    mytype_t res(Socket::accept(*_sock, addr));
    res._addr = std::move(addr);
    return res;
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
inline void connected_socket<Addr, family, type, proto>::bind(myaddr_t const& addr)
{
    Socket::bind(*_sock, addr);
    _addr = std::move(addr);
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
inline size_t connected_socket<Addr, family, type, proto>::recv(void* buffer, size_t len, Socket::socket_flags flags)
{
    return Socket::recv(*_sock, buffer, len, flags);
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
inline size_t connected_socket<Addr, family, type, proto>::send(const void* buffer, size_t len, Socket::socket_flags flags)
{
    return Socket::send(*_sock, buffer, len, flags);
}

///////////////////////////////////////////////////////////////////////////////
// unconnected_socket class
///////////////////////////////////////////////////////////////////////////////
template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
unconnected_socket<Addr, family, type, proto>::unconnected_socket() :
    basic_socket(Socket::socket(family, type, proto))
{
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
inline typename unconnected_socket<Addr, family, type, proto>::myaddr_t const& unconnected_socket<Addr, family, type, proto>::get_addr() const
{
    return _addr;
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
void unconnected_socket<Addr, family, type, proto>::socket()
{
    basic_socket::socket(family, type, proto);
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
void unconnected_socket<Addr, family, type, proto>::bind(myaddr_t const& addr)
{
    Socket::bind(*_sock, addr);
    _addr = std::move(addr);
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
size_t unconnected_socket<Addr, family, type, proto>::recvfrom(basic_addr& addr, void* buffer, size_t len, Socket::socket_flags flags)
{
    return Socket::recvfrom(*_sock, addr, buffer, len, flags);
}

template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
size_t unconnected_socket<Addr, family, type, proto>::sendto(const basic_addr& addr, const void* buffer, size_t len, Socket::socket_flags flags)
{
    return Socket::sendto(*_sock, addr, buffer, len, flags);
}