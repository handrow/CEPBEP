#include "netlib/webserver/webserver.h"

#include "logger/logger.h"

namespace Webserver {

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

namespace {
IO::Poller::PollEvent  __MostWantedPollEvent(IO::Poller::EventSet eset) {
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
}  // namespace

IO::Poller::Result   HttpServer::__PollEvent() {
    Error err;
    IO::Poller::Result res = __poller.Poll(&err);

    if (err.IsError())
        throw std::runtime_error("Poll failed: " + err.message);

    res.ev = __MostWantedPollEvent(res.ev);

    return res;
}

Event::IEventPtr HttpServer::__ChooseAndSpawnEvent(IO::Poller::PollEvent pev, fd_t fd) {
    SocketFdMap::iterator  lstn_sock_it = __listeners_map.find(fd);
    if (lstn_sock_it != __listeners_map.end()) {
        return __SpawnListenerEvent(pev, &lstn_sock_it->second);
    }

    SessionFdMap::iterator  session_it = __sessions_map.find(fd);
    if (session_it != __sessions_map.end()) {
        return __SpawnSessionEvent(pev, session_it->second);
    }

    return new DebugEvent(__system_log, pev, fd);
}

void HttpServer::__EvaluateIoEvents() {
    __poller.SetPollTimeout(__evloop.GetTimeToNextEventMS());

    IO::Poller::Result res = __PollEvent();

    Event::IEventPtr ev = __ChooseAndSpawnEvent(IO::Poller::PollEvent(res.ev),
                                                res.fd);

    __evloop.PushEvent(ev);
}

}  // namespace Webserver
