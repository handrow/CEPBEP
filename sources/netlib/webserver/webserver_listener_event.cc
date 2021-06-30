#include "netlib/webserver/webserver.h"

namespace Webserver {

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
        Error err;
        IO::Socket conn = IO::Socket::AcceptNewConnection(__sock, &err);
        debug(__http_server->__logger, "Accepted new connection from %s:%d on %s:%d",
                                       std::string(conn.GetSockInfo().addr_BE).c_str(),
                                       u16(conn.GetSockInfo().port_BE),
                                       std::string(__sock->GetSockInfo().addr_BE).c_str(),
                                       u16(__sock->GetSockInfo().port_BE));
                    
        conn.Close();
    }
};

Event::IEventPtr    HttpServer::__SpawnListenerEvent(IO::Poller::PollEvent ev, IO::Socket* sock) {
    if (ev == IO::Poller::POLL_READ)
        return new EvListenerNewConnection(this, sock);
    return new DebugEvent(__logger, ev, sock->GetFd());
}

}  // namespace Webserver
