#include "netlib/webserver/webserver.h"

namespace Webserver {

void  HttpServer::__OnListenerAccept(IO::Socket* lstn_sock) {
    Error err;
    IO::Socket conn = IO::Socket::AcceptNewConnection(lstn_sock, &err);
    if (err.IsError()) {
        error(SystemLog_, "ServerSock[%d]: acception failed: %s (%d)", lstn_sock->GetFd(), err.Description.c_str(), err.ErrorCode);
    } else {
        info(SystemLog_, "ServerSock[%d]: accepted new connection:\n"
                        ">  connection_info: (%s:%u)\n"
                        ">      server_info: (%s:%u)",
                                lstn_sock->GetFd(),
                                std::string(conn.GetSockInfo().Addr_BE).c_str(),
                                UInt16(conn.GetSockInfo().Port_BE),
                                std::string(lstn_sock->GetSockInfo().Addr_BE).c_str(),
                                UInt16(lstn_sock->GetSockInfo().Port_BE));

        SessionCtx* session = __NewSessionCtx(conn, lstn_sock->GetFd());
        info(SystemLog_, "ServerSock[%d]: session (fd: %d) created",
                                lstn_sock->GetFd(),
                                session->ConnectionSock.GetFd());

        __StartSessionCtx(session);
    }
}

class HttpServer::EvListenerNewConnection : public Event::IEvent {
 private:
    HttpServer* HttpServer_;
    IO::Socket* Socket_;
 public:
    explicit EvListenerNewConnection(HttpServer* server, IO::Socket* sock)
    : HttpServer_(server)
    , Socket_(sock) {
    }

    void Handle() {
        HttpServer_->__OnListenerAccept(Socket_);
    }
};

Event::IEventPtr    HttpServer::__SpawnListenerEvent(IO::Poller::PollEvent ev, IO::Socket* sock) {
    if (ev == IO::Poller::POLL_READ)
        return new EvListenerNewConnection(this, sock);
    return new DebugEvent(SystemLog_, ev, sock->GetFd());
}

}  // namespace Webserver
