#include "netlib/webserver/webserver.h"

namespace Webserver {

void HttpServer::SetLogger(Log::Logger* l) {
    __logger = l;
}

void HttpServer::AddListener(const IO::SockInfo& si) {
    Error err;
    IO::Socket sock = IO::Socket::CreateListenSocket(si, &err);

    if (err.IsError())
        throw std::runtime_error("Socket creation failed: " + err.message);

    __lstn_sockets[sock.GetFd()] = sock;
    __poller.AddFd(sock.GetFd(), IO::Poller::POLL_READ);
}

void HttpServer::ServeForever() {
    __evloop.AddDefaultEvent(__SpawnLoopHook());
    __evloop.Run();
}

}  // namespace Webserver
