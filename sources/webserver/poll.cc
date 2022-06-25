#include "webserver/webserver.h"

#include "logger/logger.h"

namespace Webserver {

class HttpServer::EvPollerHook: public Event::IEvent {
 private:
    HttpServer*     HttpServer_;

 public:
    explicit EvPollerHook(HttpServer* serv) : HttpServer_(serv) { }
    void     Handle() { HttpServer_->EvaluateIoEvents(); }
};

Event::IEventPtr  HttpServer::SpawnPollerHook() {
    return new EvPollerHook(this);
}

namespace {
IO::Poller::PollEvent  MostWantedPollEvent(IO::Poller::EventSet eset) {
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

IO::Poller::Result   HttpServer::PollEvent() {
    Error err;
    IO::Poller::Result res = Poller_.Poll(&err);

    debug(SystemLog_, "Poller gained event_set 0x%.6x on fd %d:\n"
                        ">    POLL_NOT_OPEN: %d\n"
                        ">        POLL_READ: %d\n"
                        ">       POLL_WRITE: %d\n"
                        ">       POLL_ERROR: %d\n"
                        ">       POLL_CLOSE: %d\n"
                        ">        POLL_PRIO: %d",
                          res.EvSet, res.FileDesc,
                          bool(res.EvSet & IO::Poller::POLL_NOT_OPEN),
                          bool(res.EvSet & IO::Poller::POLL_READ),
                          bool(res.EvSet & IO::Poller::POLL_WRITE),
                          bool(res.EvSet & IO::Poller::POLL_ERROR),
                          bool(res.EvSet & IO::Poller::POLL_CLOSE),
                          bool(res.EvSet & IO::Poller::POLL_PRIO));

    if (err.IsError()) {
        error(SystemLog_, "Poll error: %s (%d)", err.Description.c_str(), err.ErrorCode);
        return res.EvSet = 0x0, res.FileDesc = -1, res;
    }

    res.EvSet = MostWantedPollEvent(res.EvSet);

    return res;
}

Event::IEventPtr HttpServer::SwitchEventSpawners(IO::Poller::PollEvent pev, Fd fd) {
    SocketFdMap::iterator  lstn_sock_it = Listeners_.find(fd);
    if (lstn_sock_it != Listeners_.end()) {
        debug(SystemLog_, "Poller fd %d is LISTENER_EVENT", fd);
        return SpawnListenerEvent(pev, &lstn_sock_it->second);
    }

    SessionFdMap::iterator  session_it = WebSessions_.find(fd);
    if (session_it != WebSessions_.end()) {
        debug(SystemLog_, "Poller fd %d is SESSION_EVENT", fd);
        return SpawnSessionEvent(pev, session_it->second);
    }

    SessionFdMap::iterator  stat_file_rd_it = StatfileSessions_.find(fd);
    if (stat_file_rd_it != StatfileSessions_.end()) {
        debug(SystemLog_, "Poller fd %d is STATIC_FILE_EVENT", fd);
        return SpawnStaticFileReadEvent(pev, stat_file_rd_it->second);
    }

    SessionFdMap::iterator  cgi_it = CgiSessions_.find(fd);
    if (cgi_it != CgiSessions_.end()) {
        return SpawnCgiEvent(pev, cgi_it->second);
    }
    return new DebugEvent(SystemLog_, pev, fd);
}

void HttpServer::EvaluateIoEvents() {
    const UInt64 MAX_TIMEOUT_MS = 1500;
    const UInt64 poller_timeout = std::min(EventLoop_.GetTimeToNextEventMS(), MAX_TIMEOUT_MS);
    
    Poller_.SetPollTimeout(poller_timeout);

    IO::Poller::Result res = PollEvent();

    Event::IEventPtr ev = SwitchEventSpawners(IO::Poller::PollEvent(res.EvSet), res.FileDesc);

    EventLoop_.PushEvent(ev);
}

}  // namespace Webserver
