#ifndef NETLIB_SERVER_CS_EVENTS_H_
#define NETLIB_SERVER_CS_EVENTS_H_

#include "http/http.h"

#include "netlib/server/io_notifier.h"

namespace Server {

class EV_CS_NewHttpReq : public Event::IEvent {
 public:
    virtual void Handle();
};

class EV_CS_NewHttpRes : public Event::IEvent {
 public:
    virtual void Handle();
};

}  // namespace Server

#endif  // NETLIB_SERVER_CS_EVENTS_H_
