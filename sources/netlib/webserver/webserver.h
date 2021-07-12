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
    static void PrintDebugInfo(IO::Poller::PollEvent p, fd_t fd, Log::Logger* l) {
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
        Log::Logger*           logger;
        IO::Poller::PollEvent pe;
        fd_t                  fd;

             DebugEvent(Log::Logger* l, IO::Poller::PollEvent p, fd_t f) : logger(l), pe(p), fd(f) {}
        void Handle() { HttpServer::PrintDebugInfo(pe, fd, logger); }
    };

 private:
    struct SessionCtx {
        IO::Socket           conn_sock;
        std::string          res_buff;
        Http::RequestReader  req_rdr;

        Log::Logger*         access_log;
        Log::Logger*         error_log;

        Http::ResponseWriter  http_writer;
        Http::Request         http_req;

        bool                  conn_close;
        int                   res_code;
    };

    typedef std::map< fd_t, IO::Socket >   SocketFdMap;
    typedef std::map< fd_t, SessionCtx* >  SessionFdMap;

 private:
    SessionCtx*         __NewSessionCtx(const IO::Socket& sock, Log::Logger* accessl, Log::Logger* errorl);
    void                __AddSessionCtx(SessionCtx* ss);
    void                __RemoveSessionCtx(SessionCtx* ss);

    /// EVENTS POLLING
    void                __EvaluateIoEvents();
    IO::Poller::Result  __PollEvent();
    Event::IEventPtr    __ChooseAndSpawnEvent(IO::Poller::PollEvent pev, fd_t fd);

    /// EVENTS
    class  EvLoopHook;
    Event::IEventPtr    __SpawnLoopHook();
 
    Event::IEventPtr    __SpawnListenerEvent(IO::Poller::PollEvent ev, IO::Socket* sock);
    class  EvListenerNewConnection;

    Event::IEventPtr    __SpawnSessionEvent(IO::Poller::PollEvent ev, SessionCtx* ss);
    class  EvSessionRead;
    class  EvSessionWrite;
    class  EvSessionError;
    class  EvSessionHup;
    void                __OnSessionRead(SessionCtx* ss);
    void                __OnSessionWrite(SessionCtx* ss);
    void                __OnSessionError(SessionCtx* ss);
    void                __OnSessionHup(SessionCtx* ss);

    void                __OnHttpRequest(SessionCtx* ss);
    void                __OnHttpResponse(SessionCtx* ss);
    void                __OnHttpError(SessionCtx* ss);

 public:
    void  SetLogger(Log::Logger* a, Log::Logger* e, Log::Logger* s);
    void  AddListener(const IO::SockInfo& si);
    void  ServeForever();

 private:
    Log::Logger*     __access_log;
    Log::Logger*     __system_log;
    Log::Logger*     __error_log;

    IO::Poller       __poller;
    Event::Loop      __evloop;

    SocketFdMap      __listeners_map;
    SessionFdMap     __sessions_map;
};

}

#endif  // NETLIB_SERVER_WEBSERVER_H_
