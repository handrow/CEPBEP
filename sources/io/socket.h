#ifndef NETLIB_IO_SOCKET_H_
#define NETLIB_IO_SOCKET_H_

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common/types.h"

#include "io/errors.h"
#include "io/file.h"

namespace IO {

// IpAddrV4(0x7f000001) -> [0x7f, 0x0, 0x0, 0x1]
// IpAddrV4("127.0.0.1") -> [0x7f, 0x0, 0x0, 0x1]
// std::string(ipaddr) -> "127.0.0.1"
// UInt32(ipaddr) -> 0x7f000001

struct IpAddrV4 {
    union {
        UInt32 raw;
        UInt8  bytes[4];
    } Val;

             IpAddrV4(UInt32 addr);                // NOLINT(*)
             IpAddrV4(const std::string& str);  // NOLINT(*)
    operator std::string() const;
    operator UInt32() const;
};

struct Port {
    union {
        UInt16 raw;
        UInt8  bytes[2];
    } Val;

             Port(UInt16 addr);                    // NOLINT(*)
             Port(const std::string& str);      // NOLINT(*)
    operator std::string() const;
    operator UInt16() const;

};

struct SockInfo {
    IpAddrV4    Addr_BE;
    Port        Port_BE;

    explicit SockInfo(IpAddrV4 ip = UInt32(-1), Port port = UInt16(-1));
    explicit SockInfo(const sockaddr_in& sin);
    operator struct sockaddr_in() const;
    bool operator==(const SockInfo& si) const;
    bool operator!=(const SockInfo& si) const;
};

class Socket : public File {
 protected:
    SockInfo  Info_;

 public:
    explicit Socket(const SockInfo& sinfo = SockInfo(), Fd fd = -1);
    SockInfo GetSockInfo() const;

    static Socket CreateListenSocket(const SockInfo& info, Error* err);
    static Socket AcceptNewConnection(Socket* listen_sock, Error* err);

};

}  // namespace IO

#endif  // NETLIB_IO_SOCKET_H_
