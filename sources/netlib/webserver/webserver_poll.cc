#include "netlib/webserver/webserver.h"

#include "logger/logger.h"

namespace Webserver {

class HttpServer::EvPollerHook: public Event::IEvent {
 private:
    HttpServer*     __http_server;

 public:
    explicit EvPollerHook(HttpServer* serv) : __http_server(serv) { }
    void     Handle() { __http_server->__EvaluateIoEvents(); }
};

Event::IEventPtr  HttpServer::__SpawnPollerHook() {
    return new EvPollerHook(this);
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

    debug(__system_log, "Poller gained event_set 0x%.6x on fd %d:\n"
                        ">    POLL_NOT_OPEN: %d\n"
                        ">        POLL_READ: %d\n"
                        ">       POLL_WRITE: %d\n"
                        ">       POLL_ERROR: %d\n"
                        ">       POLL_CLOSE: %d\n"
                        ">        POLL_PRIO: %d",
                          res.ev, res.fd,
                          bool(res.ev & IO::Poller::POLL_NOT_OPEN),
                          bool(res.ev & IO::Poller::POLL_READ),
                          bool(res.ev & IO::Poller::POLL_WRITE),
                          bool(res.ev & IO::Poller::POLL_ERROR),
                          bool(res.ev & IO::Poller::POLL_CLOSE),
                          bool(res.ev & IO::Poller::POLL_PRIO));

    if (err.IsError())
        throw std::runtime_error("Poll failed: " + err.message);

    res.ev = __MostWantedPollEvent(res.ev);

    return res;
}

Event::IEventPtr HttpServer::__SwitchEventSpawners(IO::Poller::PollEvent pev, fd_t fd) {
    SocketFdMap::iterator  lstn_sock_it = __listeners_map.find(fd);
    if (lstn_sock_it != __listeners_map.end()) {
        return __SpawnListenerEvent(pev, &lstn_sock_it->second);
    }

    SessionFdMap::iterator  session_it = __sessions_map.find(fd);
    if (session_it != __sessions_map.end()) {
        return __SpawnSessionEvent(pev, session_it->second);
    }

    StaticFileFdMap::iterator  stat_file_rd_it = __stat_files_read_map.find(fd);
    if (stat_file_rd_it != __stat_files_read_map.end()) {
        return __SpawnStaticFileReadEvent(pev, stat_file_rd_it->second.file, stat_file_rd_it->second.session);
    }

    CgiInFdMap::iterator  cgi_in_fd = __cgi_in_map.find(fd);
    if (cgi_in_fd != __cgi_in_map.end()) {
        return __SpawnCgiWriteEvent(pev, &(cgi_in_fd->second), fd);
    }

    CgiOutFdMap::iterator  cgi_out_fd = __cgi_out_map.find(fd);
    if (cgi_out_fd != __cgi_out_map.end()) {
        return __SpawnCgiReadEvent(pev, cgi_out_fd->second, fd);
    }
    return new DebugEvent(__system_log, pev, fd);
}

void HttpServer::__EvaluateIoEvents() {
    __poller.SetPollTimeout(__evloop.GetTimeToNextEventMS());

    IO::Poller::Result res = __PollEvent();

    Event::IEventPtr ev = __SwitchEventSpawners(IO::Poller::PollEvent(res.ev), res.fd);

    __evloop.PushEvent(ev);
}

}  // namespace Webserver
