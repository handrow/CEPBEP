#ifndef NETLIB_WEBSERVER_WEBSERVER_H_
#define NETLIB_WEBSERVER_WEBSERVER_H_

#include <exception>

#include "http/http.h"
#include "http/reader.h"
#include "http/writer.h"

#include "netlib/io/socket.h"

#include "netlib/io/poller.h"

#include "netlib/event/event.h"
#include "netlib/event/loop.h"

#include "logger/logger.h"

namespace Webserver {

class HttpServer {
 public:
    static void PrintDebugInfo(IO::Poller::PollEvent p, fd_t fd, ft::Logger* l) {
        const char* poll_ev_str;
        switch (p) {
            case IO::Poller::POLL_CLOSE:        poll_ev_str = "POLL_CLOSE"; break;
            case IO::Poller::POLL_ERROR:        poll_ev_str = "POLL_ERROR"; break;
            case IO::Poller::POLL_NONE:         poll_ev_str = "POLL_NONE"; break;
            case IO::Poller::POLL_WRITE:        poll_ev_str = "POLL_WRITE"; break;
            case IO::Poller::POLL_READ:         poll_ev_str = "POLL_READ"; break;
            case IO::Poller::POLL_NOT_OPEN:     poll_ev_str = "POLL_NOT_OPEN"; break;
            default:                            poll_ev_str = "UNKNOWN"; break;
        }
        debug(l, "DEBUG_POLL_INFO: PEV='%s', PEV=0x%x, FD=%d", poll_ev_str, p, fd);
    }

    struct DebugEvent : public Event::IEvent {
        ft::Logger*           logger;
        IO::Poller::PollEvent pe;
        fd_t                  fd;

             DebugEvent(ft::Logger* l, IO::Poller::PollEvent p, fd_t f) : logger(l), pe(p), fd(f) {}
        void Handle() { HttpServer::PrintDebugInfo(pe, fd, logger); }
    };



 private:
    typedef std::map<fd_t, IO::Socket>  SocketFdMap;

 private:
    ft::Logger*     __logger;

    IO::Poller      __poller;
    Event::Loop     __evloop;

    SocketFdMap     __lstn_sockets;

 private:
    static IO::Poller::PollEvent  __MostWantedPollEvent(IO::Poller::EventSet eset);

 private:
    /// EVENTS POLLING
    void                __EvaluateIoEvents();
    Event::IEventPtr    __SpawnEvent(IO::Poller::PollEvent ev, fd_t fd);

    /// EVENTS
    class  EvLoopHook;
    Event::IEventPtr    __SpawnLoopHook();

    class  EvListenerNewConnection;
    Event::IEventPtr    __SpawnListenerEvent(IO::Poller::PollEvent ev, IO::Socket* sock);

 public:
    void  SetLogger(ft::Logger* l);
    void  AddListener(const IO::SockInfo& si);
    void  ServeForever();
};

}

#endif  // NETLIB_SERVER_WEBSERVER_H_
