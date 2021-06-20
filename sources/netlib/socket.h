#ifndef NETLIB_SOCKET_H_
#define NETLIB_SOCKET_H_

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
#include <cerrno>
#include <fcntl.h>

#include "common/types.h"
#include "common/error.h"

namespace Netlib {

enum NetLibErrorCode {

    NET_NO_ERR = 10000,
    NET_SYSTEM_ERR,
    NET_BAD_FAMILY_ERR
};

// IpAddrV4(0x7f000001) -> [0x7f, 0x0, 0x0, 0x1]
// IpAddrV4("127.0.0.1") -> [0x7f, 0x0, 0x0, 0x1]
// std::string(ipaddr) -> "127.0.0.1"
// u32(ipaddr) -> 0x7f000001

struct IpAddrV4 {
    union {
        u32 raw;
        u8  bytes[4];
    } __val;

             IpAddrV4(u32 addr);                // NOLINT(*)
             IpAddrV4(const std::string& str);  // NOLINT(*)
    operator std::string() const;
    operator u32() const;
};

struct Port {
    union {
        u16 raw;
        u8  bytes[2];
    } __val;

             Port(u16 addr);                    // NOLINT(*)
             Port(const std::string& str);      // NOLINT(*)
    operator std::string() const;
    operator u16() const;

};

struct SockInfo {
    IpAddrV4    addr_BE;
    Port        port_BE;

    explicit SockInfo(IpAddrV4 ip = u32(-1), Port port = u16(-1));
    explicit SockInfo(const sockaddr_in& sin);
    operator struct sockaddr_in() const;
};

class Socket {
 protected:
    SockInfo  __info;
    fd_t      __fd;

 public:
    explicit Socket(const SockInfo& sinfo = SockInfo(), fd_t fd = -1);

    static Socket CreateListenSocket(const SockInfo& info, Error* err);
    static Socket AcceptNewConnection(Socket* listen_sock, Error* err);

    void        AddFileFlag(int flag, Error* err);
    void        Close();
    fd_t        GetFd();
    std::string Read(usize nbytes, Error* err);
    isize       Write(const std::string& s, Error* err);

};

}  // namespace Netlib

#endif  // NETLIB_SOCKET_H_
