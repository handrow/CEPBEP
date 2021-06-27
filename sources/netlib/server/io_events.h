#ifndef NETLIB_SERVER_IO_EVENTS_H_
#define NETLIB_SERVER_IO_EVENTS_H_

#include "netlib/server/io_notifier.h"

namespace Server {

struct SessionData {

    ConnectionStream  __connection;
    /* notifier link, event loop link */
    /* virtual server pref link */

};

class EV_IO_SessionAccept : public Event::IEvent {
 public:
    virtual void Handle();
};

class EV_IO_SessionClose : public Event::IEvent {
 public:
    virtual void Handle();
};

class EV_IO_SessionRead : public Event::IEvent {
 public:
    virtual void Handle();
};

class EV_IO_SessionWrite : public Event::IEvent {
 public:
    virtual void Handle();
};

class EV_IO_SessionErr : public Event::IEvent {
 public:
    virtual void Handle();
};


class EV_IO_StaticFileRead : public Event::IEvent {
 public:
    virtual void Handle() {}
};

class EV_IO_StaticFileWrite : public Event::IEvent {
 public:
    virtual void Handle() {}
};

class EV_IO_StaticFileErr : public Event::IEvent {
 public:
    virtual void Handle() {}
};

class EV_IO_Error : public Event::IEvent {
 public:
    virtual void Handle() {}
};

}  // namespace Server

#endif  // NETLIB_SERVER_IO_EVENTS_H_
