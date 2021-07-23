#include "netlib/webserver/webserver.h"

namespace Webserver {

void  HttpServer::__OnListenerAccept(IO::Socket* lstn_sock) {
    Error err;
    IO::Socket conn = IO::Socket::AcceptNewConnection(lstn_sock, &err);
    if (err.IsError()) {
        error(__system_log, "ServerSock[%d]: acception failed: %s (%d)", lstn_sock->GetFd(), err.message.c_str(), err.errcode);
    } else {
        info(__system_log, "ServerSock[%d]: accepted new connection:\n"
                        ">  connection_info: (%s:%u)\n"
                        ">      server_info: (%s:%u)",
                                lstn_sock->GetFd(),
                                std::string(conn.GetSockInfo().addr_BE).c_str(),
                                u16(conn.GetSockInfo().port_BE),
                                std::string(lstn_sock->GetSockInfo().addr_BE).c_str(),
                                u16(lstn_sock->GetSockInfo().port_BE));

        SessionCtx* session = __NewSessionCtx(conn, lstn_sock->GetFd());
        info(__system_log, "ServerSock[%d]: session (fd: %d) created",
                                lstn_sock->GetFd(),
                                session->conn_sock.GetFd());

        __StartSessionCtx(session);
    }
}

class HttpServer::EvListenerNewConnection : public Event::IEvent {
 private:
    HttpServer* __http_server;
    IO::Socket* __sock;
 public:
    explicit EvListenerNewConnection(HttpServer* server, IO::Socket* sock)
    : __http_server(server)
    , __sock(sock) {
    }

    void Handle() {
        __http_server->__OnListenerAccept(__sock);
    }
};

Event::IEventPtr    HttpServer::__SpawnListenerEvent(IO::Poller::PollEvent ev, IO::Socket* sock) {
    if (ev == IO::Poller::POLL_READ)
        return new EvListenerNewConnection(this, sock);
    return new DebugEvent(__system_log, ev, sock->GetFd());
}

}  // namespace Webserver
