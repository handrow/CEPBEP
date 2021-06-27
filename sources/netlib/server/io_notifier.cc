#include "netlib/server/io_notifier.h"
#include "netlib/server/io_events.h"

namespace Server {

fd_t  IoNotifier::WatchFor(const ConnectionStream& connection) {
    const fd_t                       fd = GetFd(connection);
    ConnectionsFdMap::value_type     ins_pair = std::make_pair(fd, connection);
    insert_res<ConnectionsFdMap>::t  res = __connections.insert(ins_pair);

    if (res.second == true) {
        __io_poller.AddFd(fd);
        return fd;
    }
    return -1;
}

fd_t  IoNotifier::WatchFor(const ListenerStream& lstn) {
    const fd_t                     fd = GetFd(lstn);
    ListenersFdMap::value_type     ins_pair = std::make_pair(fd, lstn);
    insert_res<ListenersFdMap>::t  res = __listeners.insert(ins_pair);

    if (res.second == true) {
        __io_poller.AddFd(fd);
        return fd;
    }
    return -1;
}

fd_t  IoNotifier::WatchFor(const StaticFileStream& st_file) {
    const fd_t                       fd = GetFd(st_file);
    StaticFilesFdMap::value_type     ins_pair = std::make_pair(fd, st_file);
    insert_res<StaticFilesFdMap>::t  res = __static_files.insert(ins_pair);

    if (res.second == true) {
        __io_poller.AddFd(fd);
        return fd;
    }
    return -1;
}

void  IoNotifier::StopWatchFor(fd_t fd) {
    __io_poller.RmFd(fd);

    __listeners.erase(fd);
    __connections.erase(fd);
    __static_files.erase(fd);
}

void  IoNotifier::AddPollerFlag(fd_t fd, IO::Poller::PollEvent pev) {
    __io_poller.AddEvMask(fd, pev);
}

void  IoNotifier::RmPollerFlag(fd_t fd, IO::Poller::PollEvent pev) {
    __io_poller.RmEvMask(fd, pev);
}

void  IoNotifier::ResetPollerFlags(fd_t fd) {
    __io_poller.SetEvMask(fd, 0x0);
}

ConnectionStream    IoNotifier::GetConnection(fd_t fd) const {
    ConnectionsFdMap::const_iterator it = __connections.find(fd);
    if (it == __connections.end()) {
        return it->second;
    }

    return (ConnectionStream){ .ios = IO::Socket() };  // TODO(mcottomn): Use default constructor
}

StaticFileStream IoNotifier::GetStaticFile(fd_t fd) const {
    StaticFilesFdMap::const_iterator it = __static_files.find(fd);
    if (it == __static_files.end()) {
        return it->second;
    }

    return (StaticFileStream){ .ios = IO::File() };  // TODO(mcottomn): Use default constructor
}

ListenerStream   IoNotifier::GetListener(fd_t fd) const {
    ListenersFdMap::const_iterator it = __listeners.find(fd);
    if (it == __listeners.end()) {
        return it->second;
    }

    return (ListenerStream){ .ios = IO::Socket() };  // TODO(mcottomn): Use default constructor
}

void  IoNotifier::Process() {
    if (__evloop) {
        __io_poller.SetPollTimeout(u32(__evloop->GetTimeToNextEventMS()));

        Error err;
        IO::Poller::Result pres = __io_poller.Poll(&err);

        if (err.IsError()) {
            // TODO(mcottomn): What should we do?
            throw std::runtime_error(err.message);
        }

        Event::IEventPtr ev = __CreateEvent(pres);
        if (ev != NULL) {
            __evloop->PushEvent(ev);
        }
    }
}

void  IoNotifier::LinkWithEvLoop(Event::Loop* loop) {
    __evloop = loop;
    __evloop->AddLoopHook(new LoopHook(this));
}

namespace {
IO::Poller::EventSet  MostWantedPollEvent(IO::Poller::EventSet poll_evset) {
    if      (poll_evset & IO::Poller::POLL_ERROR)
        return IO::Poller::POLL_ERROR;
    else if (poll_evset & IO::Poller::POLL_CLOSE)
        return IO::Poller::POLL_CLOSE;
    else if (poll_evset & IO::Poller::POLL_WRITE)
        return IO::Poller::POLL_WRITE;
    else if (poll_evset & IO::Poller::POLL_READ)
        return IO::Poller::POLL_READ;
    else if (poll_evset == IO::Poller::POLL_NONE)
        return IO::Poller::POLL_NONE;
    else
        return IO::Poller::POLL_ERROR;
}
}  // namespace

Event::IEventPtr     IoNotifier::__CreateEvent(const IO::Poller::Result& pres) {
    const IO::Poller::EventSet single_ev = MostWantedPollEvent(pres.ev);
    Event::IEventPtr           event = NULL;

    switch (single_ev) {
    case IO::Poller::POLL_ERROR:
        if        (__listeners.count(pres.fd)) {
            event = new EV_IO_Error;

        } else if (__connections.count(pres.fd)) {
            event = new EV_IO_SessionErr;

        } else if (__static_files.count(pres.fd)) {
            event = new EV_IO_StaticFileErr;

        }
        break;

    case IO::Poller::POLL_CLOSE:
        if        (__listeners.count(pres.fd)) {
            event = new EV_IO_Error;

        } else if (__connections.count(pres.fd)) {
            event = new EV_IO_SessionClose;

        } else if (__static_files.count(pres.fd)) {
            event = new EV_IO_StaticFileErr;

        }
        break;

    case IO::Poller::POLL_WRITE:
        if        (__listeners.count(pres.fd)) {
            event = new EV_IO_Error;

        } else if (__connections.count(pres.fd)) {
            event = new EV_IO_SessionWrite;

        } else if (__static_files.count(pres.fd)) {
            event = new EV_IO_StaticFileWrite;
        }
        break;

    case IO::Poller::POLL_READ:
        if        (__listeners.count(pres.fd)) {
            event = new EV_IO_SessionAccept;

        } else if (__connections.count(pres.fd)) {
            event = new EV_IO_SessionRead;

        } else if (__static_files.count(pres.fd)) {
            event = new EV_IO_StaticFileRead;

        }
        break;
    }

    return event;
}

}  // namespace Server
