#include "netlib/webserver/webserver.h"

namespace Webserver {

void  HttpServer::SetLogger(Log::Logger* a, Log::Logger* e, Log::Logger* s) {
    __access_log = a;
    __error_log = e;
    __system_log = s;
}

void  HttpServer::AddWebRoute(const WebRoute& entry) {
    __routes.push_back(entry);
}

void  HttpServer::AddListener(const IO::SockInfo& si) {
    Error err;
    IO::Socket sock = IO::Socket::CreateListenSocket(si, &err);

    if (err.IsError())
        throw std::runtime_error("Socket creation failed: " + err.message);

    __listeners_map[sock.GetFd()] = sock;
    __poller.AddFd(sock.GetFd(), IO::Poller::POLL_READ);
}

void  HttpServer::SetMimes(const Mime::MimeTypesMap& map) {
    __mime_map = map;
}

void  HttpServer::ServeForever() {
    __evloop.AddDefaultEvent(__SpawnPollerHook());
    __evloop.Run();
}

}  // namespace Webserver
