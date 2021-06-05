#include "socket.h"

namespace NetLib {

//IPADDRV4

IpAddrV4::IpAddrV4(u32 addr) {
    __val.raw = htonl(addr);
}

IpAddrV4::IpAddrV4(const std::string& str) {
    __val.raw = inet_addr(str.c_str());
}

IpAddrV4::operator std::string() const {
    struct in_addr l;
    l.s_addr = __val.raw;
    return inet_ntoa(l);
}

IpAddrV4::operator u32() const {
    return ntohl(__val.raw);
}

//PORT

Port::Port(u16 addr) {
    __val.raw = htons(addr);
}

Port::Port(const std::string& str) {
    u16 i = atoi(str.c_str());
    __val.raw = htons(i);
}

Port::operator std::string() const {
    return std::to_string(ntohs(__val.raw));
}

Port::operator u16() const {
    return ntohs(__val.raw);
}

//SOCKIONFO

SockInfo::SockInfo(IpAddrV4 ip, Port port)
: addr_BE(ip)
, port_BE(port) {
}

SockInfo::SockInfo(const sockaddr_in& sin)
: addr_BE(sin.sin_addr.s_addr)
, port_BE(sin.sin_port) {
}

SockInfo::operator sockaddr_in() const {
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = port_BE.__val.raw;
    saddr.sin_addr.s_addr = addr_BE.__val.raw;
    return saddr;
}

//SOCKET

Socket::Socket(const SockInfo& sinfo, fd_t fd) : __info(sinfo), __fd(fd) {}

Socket Socket::CreateListenSocket(const SockInfo& sinfo, Error* err) {
    Socket             sock;
    int                rc;

    rc = socket(AF_INET, SOCK_STREAM, 0);
    if (rc < 0)
        return *err = SystemError(errno), sock;
    sock.__fd = rc;

    sockaddr_in addr = sinfo;
    rc = bind(sock.__fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (rc < 0)
        return *err = SystemError(errno), sock;
    sock.__info = sinfo;

    rc = listen(sock.__fd, 0);
    if (rc < 0)
        return *err = SystemError(errno), sock;

    return sock;
}

Socket Socket::AcceptNewConnection(Socket* listen_sock, Error* err) {
    SockInfo    sinfo;
    int         rc;

    socklen_t   slen;
    sockaddr    sa;
    rc = accept(listen_sock->__fd, &sa, &slen);

    if (rc < 0)
        *err = SystemError(errno);
    else if (sa.sa_family != AF_INET)
        *err = Error(NET_BAD_FAMILY_ERR, "Not AF_INET connection");
    else
        sinfo = SockInfo(*reinterpret_cast<sockaddr_in *>(&sa));

    return Socket(sinfo, rc);
}

void    Socket::AddFileFlag(int flag, Error* err) {
    int flags = fcntl(__fd, F_GETFL);
    if (flags < 0)
        *err = SystemError(errno);
    else if (fcntl(__fd, F_SETFL, flags | flag) < 0)
        *err = SystemError(errno);
}

void    Socket::Close() {
    close(__fd);
}

fd_t    Socket::GetFd() {
    return __fd;
}

std::string Socket::Read(usize nbytes, Error* err) {
    isize rc;
    std::string s;

    s.reserve(nbytes);

    rc = read(__fd, const_cast<char*>(s.data()), nbytes);
    if (rc < 0)
        *err = Error(NET_SYSTEM_ERR, "Syscall error: read");
    s.resize(rc);
    return s;
}

isize   Socket::Write(const std::string& s, Error* err) {
    isize rc = write(__fd, const_cast<char*>(s.data()), s.size());

    if (rc < 0)
        *err = Error(NET_SYSTEM_ERR, "Syscall error: write");
    return rc;
}

}  // namespace Socket