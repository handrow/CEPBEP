#include "webserver/webserver.h"

namespace Webserver {

void                HttpServer::EvaluateIoTimeouts() {
    UInt64  currentMS = tv_to_msec(GetTimeOfDay());
    SessionFdMap::iterator  it = WebSessions_.begin();

    while (it != WebSessions_.end()) {
        SessionFdMap::iterator del = it;
        ++it;

        if (currentMS >= del->second->TimeoutMs_)
            OnSessionTimeout(del->second);
    }
}

void                HttpServer::OnSessionTimeout(SessionCtx* ss) {
    debug(SystemLog_, "Session[%d]: Timeout occured, disconnecting", ss->ConnectionSock.GetFd());
    DeleteSessionCtx(ss);
}

class  HttpServer::EvTimeoutHook : public Event::IEvent {
 private:
    HttpServer*     Server_;

 public:
    EvTimeoutHook(HttpServer* s) : Server_(s) {
    }

    void Handle() {
        Server_->EvaluateIoTimeouts();
    }
};

Event::IEventPtr    HttpServer::SpawnTimeoutHook() {
    return new EvTimeoutHook(this);
}

}  // namespace Webserver
