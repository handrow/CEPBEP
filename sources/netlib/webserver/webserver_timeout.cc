#include "netlib/webserver/webserver.h"

namespace Webserver {

void                HttpServer::__EvaluateIoTimeouts() {
    u64  current_time_ms = tv_to_msec(GetTimeOfDay());
    SessionFdMap::iterator  it = __sessions_map.begin();

    while (it != __sessions_map.end()) {
        SessionFdMap::iterator del = it;
        ++it;

        if (current_time_ms >= del->second->__timeout_ms)
            __OnSessionTimeout(del->second);
    }
}

void                HttpServer::__OnSessionTimeout(SessionCtx* ss) {
    debug(__system_log, "Session[%d]: Timeout occured, disconnecting", ss->conn_sock.GetFd());
    __DeleteSessionCtx(ss);
}

class  HttpServer::EvTimeoutHook : public Event::IEvent {
 private:
    HttpServer*     __server;

 public:
    EvTimeoutHook(HttpServer* s) : __server(s) {
    }

    void Handle() {
        __server->__EvaluateIoTimeouts();
    }
};

Event::IEventPtr    HttpServer::__SpawnTimeoutHook() {
    return new EvTimeoutHook(this);
}

}  // namespace Webserver
