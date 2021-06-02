#ifndef NETLIB_SOCKET_H_
#define NETLIB_SOCKET_H_

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <cerrno>

#include "common/types.h"
#include "common/error.h"

namespace Ft {
namespace Netlib {

struct IpAddrV4 {
    u32 __raw_addr;

    explicit IpAddrV4(u32 addr);
    explicit IpAddrV4(const std::string& str);
    explicit operator std::string() const;
    explicit operator u32() const;
};

// TODO(mcottomn): Unit test for this and IPv4 Addr
struct SockInfo {
    IpAddrV4 addr;
    u16 port_LE;

    explicit SockInfo(const struct sockaddr_in& sin);
    explicit operator struct sockaddr_in() const;
    SockInfo(IpAddrV4 addr, u16 port);
};

class Socket {
 protected:
    SockInfo    __info;
    fd_t        __fd;

 public:
                    Socket(fd_t fd, const SockInfo& sinfo);
                    ~Socket();

    SockInfo        GetSockInfo() const;
    fd_t            FileNo() const;
    Error           AddFlag(u32 fd_flag);
    void            Close();
};

class ListenSocket: public Socket {
 protected:
    bool    __is_listening;

 public:
            ListenSocket();

    Error Listen(u32 ipaddr_v4, u16 port_LE);
    Error Accept(ConnectionSocket& sock);
};

class ConnectionSocket: public Socket {
    friend class ListenSocket;  // Can be only created by ListenSocket

 protected:
                    ConnectionSocket(fd_t fd, const SockInfo& con_info);

 public:
    std::string     Read(usize nbytes);
    isize           Write(const std::string& s);
};

}  // namespace Netlib
}  // namespace Ft

#endif  // NETLIB_SOCKET_H_
