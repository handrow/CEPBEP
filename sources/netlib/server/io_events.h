#ifndef NETLIB_SERVER_IO_EVENTS_H_
#define NETLIB_SERVER_IO_EVENTS_H_

#include "netlib/server/io_notifier.h"

namespace Server {

struct ListenerEventData {
    ListenerStream* __lstn;
    IoNotifier*     __notifier;
    Event::Loop*    __evloop;

    ListenerEventData(ListenerStream* lstn, IoNotifier* notifier, Event::Loop* lp)
    : __lstn(lstn)
    , __notifier(notifier)
    , __evloop(lp) {
    }
};

struct SessionEventData {
    static const usize SOCKET_READ_BUFF_SZ = 1024ull;
    static const usize SOCKET_WRITE_BUFF_SZ = 1024ull;

    SessionStream*   __session;
    IoNotifier*      __notifier;
    Event::Loop*     __evloop;

    SessionEventData(SessionStream* session, IoNotifier* notifier, Event::Loop* lp)
    : __session(session)
    , __notifier(notifier)
    , __evloop(lp) {
    }
};

class EV_IO_SessionAccept : public Event::IEvent, private ListenerEventData {
 public:
    EV_IO_SessionAccept(ListenerStream* lstn, IoNotifier* notifier, Event::Loop* lp)
    : ListenerEventData(lstn, notifier, lp) {
    }

    virtual void Handle();
};

class EV_IO_SessionClose : public Event::IEvent, private SessionEventData {
 public:
    EV_IO_SessionClose(SessionStream* session, IoNotifier* notifier, Event::Loop* lp)
    : SessionEventData(session, notifier, lp) {
    }

    virtual void Handle();
};

class EV_IO_SessionRead : public Event::IEvent, private SessionEventData {
 public:
    EV_IO_SessionRead(SessionStream* session, IoNotifier* notifier, Event::Loop* lp)
    : SessionEventData(session, notifier, lp) {
    }

    virtual void Handle();
};

class EV_IO_SessionWrite : public Event::IEvent, private SessionEventData {
 public:
    EV_IO_SessionWrite(SessionStream* session, IoNotifier* notifier, Event::Loop* lp)
    : SessionEventData(session, notifier, lp) {
    }

    virtual void Handle();
};

class EV_IO_SessionErr : public Event::IEvent, private SessionEventData {
 public:
    EV_IO_SessionErr(SessionStream* session, IoNotifier* notifier, Event::Loop* lp)
    : SessionEventData(session, notifier, lp) {
    }

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
