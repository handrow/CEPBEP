#include "netlib/webserver/webserver.h"

namespace Webserver {

// void  HttpServer::SetLogger(Log::Logger* a, Log::Logger* e, Log::Logger* s) {
//     __access_log = a;
//     __error_log = e;
//     __system_log = s;
// }

// void  HttpServer::AddWebRoute(const WebRoute& entry) {
//     __routes.push_back(entry);
// }

// void  HttpServer::AddListener(const IO::SockInfo& si) {
//     Error err;
//     IO::Socket sock = IO::Socket::CreateListenSocket(si, &err);

//     if (err.IsError())
//         throw std::runtime_error("Socket creation failed: " + err.message);

//     __listeners_map[sock.GetFd()] = sock;
//     __poller.AddFd(sock.GetFd(), IO::Poller::POLL_READ);
// }

// void  HttpServer::SetMimes(const Mime::MimeTypesMap& map) {
//     __mime_map = map;
// }

void  HttpServer::SetSystemLogger(Log::Logger* logger) {
    __system_log = logger;
}

void  HttpServer::AddVritualServer(const IO::SockInfo& si, const VirtualServer& vs) {
    VirtualServerMap::iterator vit = __vservers_map.begin();
    for (;vit != __vservers_map.end(); ++vit) {
        fd_t lfd = vit->first;

        if (__listeners_map[lfd].GetSockInfo() == si) {
            // listener already exists
            __vservers_map.insert(std::make_pair(lfd, vs));
            return;
        }
    }

    // create new listener
    Error err;
    IO::Socket sock = IO::Socket::CreateListenSocket(si, &err);

    if (err.IsError())
        throw std::runtime_error("Socket creation failed: " + err.message);

    __listeners_map[sock.GetFd()] = sock;
    __poller.AddFd(sock.GetFd(), IO::Poller::POLL_READ);
    __vservers_map.insert( std::make_pair(sock.GetFd(), vs) );
}

void  HttpServer::SetTimeout(u64 msec) {
    __session_timeout = msec;
}

void  HttpServer::ServeForever() {
    __evloop.AddDefaultEvent(__SpawnPollerHook());
    __evloop.AddDefaultEvent(__SpawnTimeoutHook());
    __evloop.Run();
}

}  // namespace Webserver
