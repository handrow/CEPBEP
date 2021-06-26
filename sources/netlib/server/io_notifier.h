#ifndef NETLIB_SERVER_IO_NOTIFIER_H_
#define NETLIB_SERVER_IO_NOTIFIER_H_

#include <map>

#include "common/types.h"

#include "netlib/io/poller.h"

#include "netlib/event/event.h"
#include "netlib/event/loop.h"

#include "netlib/server/preferences.h"
#include "netlib/server/io.h"

namespace Server {

class IoNotifier {
 private:
    typedef std::map<fd_t, ListenerStream>      ListenersFdMap;
    typedef std::map<fd_t, SessionStream>       SessionsFdMap;
    typedef std::map<fd_t, StaticFileStream>    StaticFilesFdMap;

 private:
    class LoopHook : public Event::IEvent {
     private:
        IoNotifier*     __self;

     public:

        LoopHook(IoNotifier* notif)
        : __self(notif) {
        }

        void Handle() {
            __self->Process();
        }
    };

 private:
    ListenersFdMap      __listeners;
    SessionsFdMap       __sessions;
    StaticFilesFdMap    __static_files;
    IO::Poller          __io_poller;
    Event::Loop*        __evloop;

 private:
    Event::IEventPtr    __CreateEvent(const IO::Poller::Result& pres) const;

 public:
    void Process();

    fd_t WatchFor(const SessionStream& session);
    fd_t WatchFor(const StaticFileStream& sfile);
    fd_t WatchFor(const ListenerStream& listener);

    void StopWatchFor(fd_t fd);

    SessionStream    GetSession(fd_t fd) const;
    StaticFileStream GetStaticFile(fd_t fd) const;
    ListenerStream   GetListener(fd_t fd) const;

    void AddPollerFlag(fd_t fd, IO::Poller::PollEvent pev);
    void RmPollerFlag(fd_t fd, IO::Poller::PollEvent pev);
    void ResetPollerFlags(fd_t fd);

    void LinkWithEvLoop(Event::Loop* evloop);
};

}  // namespace Server

#endif  // NETLIB_SERVER_EVENT_H_
