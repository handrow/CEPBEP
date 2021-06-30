#include "netlib/webserver/webserver.h"

#include "logger/logger.h"

namespace Webserver {

Event::IEventPtr  HttpServer::__SpawnEvent(IO::Poller::PollEvent pev, fd_t fd) {
    // if listener then call spawner for listeners
    SocketFdMap::iterator sock_map_it = __lstn_sockets.find(fd);
    if (sock_map_it != __lstn_sockets.end()) {
        return __SpawnListenerEvent(pev, &(sock_map_it->second));
    }
    return new DebugEvent(__logger, pev, fd);
}

class HttpServer::EvLoopHook: public Event::IEvent {
 private:
    HttpServer*     __http_server;

 public:
    explicit EvLoopHook(HttpServer* serv) : __http_server(serv) { }
    void     Handle() { __http_server->__EvaluateIoEvents(); }
};

Event::IEventPtr  HttpServer::__SpawnLoopHook() {
    return new EvLoopHook(this);
}

IO::Poller::PollEvent  HttpServer::__MostWantedPollEvent(IO::Poller::EventSet eset) {
    if (eset & IO::Poller::POLL_ERROR)
        return IO::Poller::POLL_ERROR;

    if (eset & IO::Poller::POLL_CLOSE)
        return IO::Poller::POLL_CLOSE;

    if (eset & IO::Poller::POLL_WRITE)
        return IO::Poller::POLL_WRITE;

    if (eset & IO::Poller::POLL_READ)
        return IO::Poller::POLL_READ;

    return IO::Poller::POLL_NONE;
}

void HttpServer::__EvaluateIoEvents() {
    __poller.SetPollTimeout(__evloop.GetTimeToNextEventMS());

    Error err;
    IO::Poller::Result res = __poller.Poll(&err);

    if (err.IsError())
        throw std::runtime_error("Poll failed: " + err.message);

    IO::Poller::PollEvent most_wanted_pev = __MostWantedPollEvent(res.ev);

    Event::IEventPtr ev = __SpawnEvent(most_wanted_pev, res.fd);
    __evloop.PushEvent(ev);
}

}  // namespace Webserver
