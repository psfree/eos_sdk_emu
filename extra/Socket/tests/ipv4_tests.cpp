#include <iostream>
#include <iomanip>
#include <vector>

#include <socket/ipv4/tcp_socket.h>
#include <socket/ipv4/udp_socket.h>
#include <socket/common/poll.h>

using namespace PortableAPI;

int test_addr_format()
{
    ipv4_addr addr;
    std::string res;
    std::string expected;

    addr.set_port(0);
    addr.set_any_addr();

    res = addr.to_string(false);
    expected = "0.0.0.0";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    res = addr.to_string(true);
    expected = "0.0.0.0:0";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    addr.set_port(10000);

    res = addr.to_string(false);
    expected = "0.0.0.0";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    res = addr.to_string(true);
    expected = "0.0.0.0:10000";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    addr.set_port(9999);
    addr.set_ip(0x7f000002);

    res = addr.to_string(false);
    expected = "127.0.0.2";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    res = addr.to_string(true);
    expected = "127.0.0.2:9999";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    addr.from_string("1.5.9.12:1548");

    res = addr.to_string(false);
    expected = "1.5.9.12";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    res = addr.to_string(true);
    expected = "1.5.9.12:1548";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    addr.set_broadcast_addr();

    res = addr.to_string(false);
    expected = "255.255.255.255";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    res = addr.to_string(true);
    expected = "255.255.255.255:1548";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    addr.set_loopback_addr();

    res = addr.to_string(false);
    expected = "127.0.0.1";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    res = addr.to_string(true);
    expected = "127.0.0.1:1548";
    if (res != expected) {
        std::cerr << __LINE__ << " Failed: " << res << " != " << expected << std::endl;
        return -1;
    }
    else {
        std::cerr << __LINE__ << " Success: " << res << " == " << expected << std::endl;
    }

    return 0;
}

int test_tcp()
{
    tcp_socket server, client1, client2;
    ipv4_addr addr;

    addr.set_loopback_addr();
    addr.set_port(45678);

    try {
        server.bind(addr);
        std::cerr << __LINE__ << " bind successful" << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to bind: " << e.what() << std::endl;
        return -1;
    }
    try {
        server.listen(5);
        std::cerr << __LINE__ << " listen successful" << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to listen:" << e.what() << std::endl;
        return -1;
    }
    try {
        client1.connect(addr);
        std::cerr << __LINE__ << " connect successful" << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to connect: " << e.what() << std::endl;
        return -1;
    }
    try {
        client2 = server.accept();
        std::cerr << __LINE__ << " accept successful" << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to accept: " << e.what() << std::endl;
        return -1;
    }
    try {
        size_t res = client1.send("Hello from client1", 19);
        size_t expected = 19;
        if (res != expected) {
            std::cerr << __LINE__ << " Failed to send the data: sent " << res << ", wanted " << expected << std::endl;
            return -1;
        }

        std::cerr << __LINE__ << " send successful" << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to send: " << e.what() << std::endl;
        return -1;
    }
    try {
        std::vector<uint8_t> buffer(4096, 0);
        size_t res = client2.recv(&buffer[0], buffer.size());
        size_t expected = 19;
        if (res != expected) {
            std::cerr << __LINE__ << " Failed to recv the data: recv " << res << ", wanted " << expected << std::endl;
            return -1;
        }

        std::cerr << __LINE__ << " recv successful: " << (const char*)(buffer.data()) << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to send: " << e.what() << std::endl;
        return -1;
    }
    try {
        size_t res = client2.send("Hello from client2", 19);
        size_t expected = 19;
        if (res != expected) {
            std::cerr << __LINE__ << " Failed to send the data: sent " << res << ", wanted " << expected << std::endl;
            return -1;
        }

        std::cerr << __LINE__ << " send successful" << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to send: " << e.what() << std::endl;
        return -1;
    }
    try {
        std::vector<uint8_t> buffer(4096, 0);
        size_t res = client1.recv(&buffer[0], buffer.size());
        size_t expected = 19;
        if (res != expected) {
            std::cerr << __LINE__ << " Failed to recv the data: recv " << res << ", wanted " << expected << std::endl;
            return -1;
        }

        std::cerr << __LINE__ << " recv successful: " << (const char*)(buffer.data()) << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to send: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

int test_udp()
{
    udp_socket sock1, sock2;
    ipv4_addr addr, remote_addr;

    addr.set_loopback_addr();
    addr.set_port(45678);

    try {
        sock1.bind(addr);
        std::cerr << __LINE__ << " bind successful" << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to bind: " << e.what() << std::endl;
        return -1;
    }
    try {
        size_t res = sock2.sendto(addr, "Hello from sock2", 17);
        size_t expected = 17;
        if (res != expected) {
            std::cerr << __LINE__ << " Failed to sendto the data: sent " << res << ", wanted " << expected << std::endl;
            return -1;
        }

        std::cerr << __LINE__ << " sendto successful" << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to send: " << e.what() << std::endl;
        return -1;
    }
    try {
        std::vector<uint8_t> buff(4096, 0);
        size_t res = sock1.recvfrom(addr, &buff[0], buff.size());
        size_t expected = 17;
        if (res != expected) {
            std::cerr << __LINE__ << " Failed to recvfrom the data: received " << res << ", wanted " << expected << std::endl;
            return -1;
        }

        std::cerr << __LINE__ << " recvfrom " << addr.to_string(true) << " successful: " << (const char*)(buff.data()) << std::endl;
    }
    catch (socket_exception& e) {
        std::cerr << __LINE__ << " Failed to recvfrom: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if (test_addr_format())
    {
        std::cerr << "Failed at addr format" << std::endl;
        return -1;
    }
    std::cerr << "Format tests ok" << std::endl << std::endl;
    if (test_tcp())
    {
        std::cerr << "Failed at tcp" << std::endl;
        return -1;
    }
    std::cerr << "TCP tests ok" << std::endl << std::endl;
    if (test_udp())
    {
        std::cerr << "Failed at udp" << std::endl;
        return -1;
    }
    std::cerr << "UDP tests ok" << std::endl << std::endl;

    std::cerr << "Tests successful" << std::endl;
    return 0;
}