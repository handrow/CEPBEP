#ifndef NETLIB_SERVER_CS_EVENTS_H_
#define NETLIB_SERVER_CS_EVENTS_H_

#include "http/http.h"

#include "netlib/server/io_notifier.h"

namespace Server {

class EV_CS_NewHttpReq : public Event::IEvent, private SessionEventData {
 private:
    Http::Request __req;

 public:
    EV_CS_NewHttpReq(const Http::Request& req, SessionStream* session, IoNotifier* notf, Event::Loop* lp)
    : SessionEventData(session, notf, lp)
    , __req(req) {
    }

    virtual void Handle();
};

class EV_CS_NewHttpRes : public Event::IEvent, private SessionEventData {
 private:
    Http::Response __res;

 public:
    EV_CS_NewHttpRes(const Http::Response& res, SessionStream* session, IoNotifier* notf, Event::Loop* lp)
    : SessionEventData(session, notf, lp)
    , __res(res) {
    }

    virtual void Handle();
};

}  // namespace Server

#endif  // NETLIB_SERVER_CS_EVENTS_H_
