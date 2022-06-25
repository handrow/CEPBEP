#include "common/string_utils.h"
#include "io/socket.h"

namespace IO {

// IPADDRV4

IpAddrV4::IpAddrV4(UInt32 addr) {
    Val.raw = htonl(addr);
}

IpAddrV4::IpAddrV4(const std::string& str) {
    Val.raw = inet_addr(str.c_str());
}

IpAddrV4::operator std::string() const {
    struct in_addr l;
    l.s_addr = Val.raw;
    return inet_ntoa(l);
}

IpAddrV4::operator UInt32() const {
    return ntohl(Val.raw);
}

// PORT

Port::Port(UInt16 addr) {
    Val.raw = htons(addr);
}

Port::Port(const std::string& str) {
    UInt16 i = Convert<UInt16>(str);
    Val.raw = htons(i);
}

Port::operator std::string() const {
    return Convert<std::string>(ntohs(Val.raw));
}

Port::operator UInt16() const {
    return ntohs(Val.raw);
}

// SOCKIONFO

SockInfo::SockInfo(IpAddrV4 ip, Port port)
: Addr_BE(ip)
, Port_BE(port) {
}

SockInfo::SockInfo(const sockaddr_in& sin)
: Addr_BE(ntohl(sin.sin_addr.s_addr))
, Port_BE(ntohs(sin.sin_port)) {
}

SockInfo::operator sockaddr_in() const {
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = Port_BE.Val.raw;
    saddr.sin_addr.s_addr = Addr_BE.Val.raw;
    return saddr;
}

bool SockInfo::operator==(const SockInfo& si) const {
    return (Addr_BE == si.Addr_BE) && (Port_BE == si.Port_BE);
}

bool SockInfo::operator!=(const SockInfo& si) const {
    return !(*this == si);
}

// SOCKET

Socket::Socket(const SockInfo& sinfo, Fd fd) : File(fd), Info_(sinfo) {
}

Socket Socket::CreateListenSocket(const SockInfo& sinfo, Error* err) {
    Socket             sock;
    int                rc;

    rc = socket(AF_INET, SOCK_STREAM, 0);
    if (rc < 0)
        return *err = SystemError(errno), sock;
    sock.Fd_ = rc;

    sockaddr_in addr = sinfo;
    rc = bind(sock.Fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (rc < 0)
        return *err = SystemError(errno), sock;
    sock.Info_ = sinfo;

    rc = listen(sock.Fd_, 0);
    if (rc < 0)
        return *err = SystemError(errno), sock;

    return sock;
}

SockInfo Socket::GetSockInfo() const {
    return Info_;
}

Socket Socket::AcceptNewConnection(Socket* listen_sock, Error* err) {
    SockInfo    sinfo;
    int         rc;

    socklen_t   slen;
    sockaddr    sa;
    rc = accept(listen_sock->Fd_, &sa, &slen);

    if (rc < 0)
        *err = SystemError(errno);
    else if (sa.sa_family != AF_INET)
        *err = Error(NET_BAD_FAMILY_ERR, "Not AF_INET connection");
    else
        sinfo = SockInfo(*reinterpret_cast<sockaddr_in *>(&sa));

    return Socket(sinfo, rc);
}

}  // namespace IO
