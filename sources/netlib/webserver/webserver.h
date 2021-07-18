#ifndef NETLIB_WEBSERVER_WEBSERVER_H_
#define NETLIB_WEBSERVER_WEBSERVER_H_

#include <exception>

#include <list>
#include <map>
#include <set>

#include "http/http.h"
#include "http/reader.h"
#include "http/writer.h"
#include "http/mime.h"

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

 public:
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

    struct StaticFileEntry {
        IO::File    file;
        SessionCtx* session;
    };

    typedef std::set<Http::Method> MethodSet;

    struct WebRoute {
        std::string            pattern;
        std::string            root_directory;
        std::string            index_page;
        MethodSet              allowed_methods;
        bool                   listing_enabled;
    };

private:
    /*                 route                                     */
    typedef std::list< WebRoute >             WebRouteList;
    /*                fd    listen_sock                          */
    typedef std::map< fd_t, IO::Socket >      SocketFdMap;
    /*                fd    session_ctx                          */
    typedef std::map< fd_t, SessionCtx* >     SessionFdMap;
    /*                fd    file and session_ctx                 */
    typedef std::map< fd_t, StaticFileEntry > StaticFileFdMap;

 private:
    /// Event basics logic
    void                __EvaluateIoEvents();
    IO::Poller::Result  __PollEvent();
    Event::IEventPtr    __SwitchEventSpawners(IO::Poller::PollEvent pev, fd_t fd);

    Event::IEventPtr    __SpawnPollerHook();
    class  EvPollerHook;
 
    /// Listener I/O
    Event::IEventPtr    __SpawnListenerEvent(IO::Poller::PollEvent ev, IO::Socket* sock);
    class  EvListenerNewConnection;
    void                __OnListenerAccept(IO::Socket* listener_sock);

    /// Http logic
    void                __OnHttpRequest(SessionCtx* ss);
    void                __OnHttpResponse(SessionCtx* ss);
    void                __OnHttpError(SessionCtx* ss);

    const WebRoute*     __FindWebRoute(const Http::Request& req, const WebRouteList& routes_list);
    bool                __FindWebFile(const Http::Request& req, const WebRoute& route,
                                                                std::string* res_path);

    void                __OnStaticFileRequest(SessionCtx* ss, const WebRoute& route);

    /// Sessions Logic
    SessionCtx*         __NewSessionCtx(const IO::Socket& sock, Log::Logger* accessl, Log::Logger* errorl);
    void                __StartSessionCtx(SessionCtx* ss);
    void                __DeleteSessionCtx(SessionCtx* ss);

    /// Sessions I/O handling
    Event::IEventPtr    __SpawnSessionEvent(IO::Poller::PollEvent ev, SessionCtx* ss);
    class  EvSessionRead;
    class  EvSessionWrite;
    class  EvSessionError;
    class  EvSessionHup;
    void                __OnSessionRead(SessionCtx* ss);
    void                __OnSessionWrite(SessionCtx* ss);
    void                __OnSessionError(SessionCtx* ss);
    void                __OnSessionHup(SessionCtx* ss);

    /// For responsnses with static files
    void                __SendDirectoryListing(const std::string& resource_path, SessionCtx* ss);
    void                __SendStaticFileResponse(IO::File file, SessionCtx* ss);
    void                __RemoveStaticFileCtx(IO::File file);

    Event::IEventPtr    __SpawnStaticFileReadEvent(IO::Poller::PollEvent ev, IO::File file, SessionCtx* ss);
    class  EvStaticFileRead;
    class  EvStaticFileReadError;
    void                __OnStaticFileRead(IO::File file, SessionCtx* ss);
    void                __OnStaticFileReadError(IO::File file, SessionCtx* ss);
    void                __OnStaticFileReadEnd(IO::File file, SessionCtx* ss);

 public:
    void  AddListener(const IO::SockInfo& si);
    void  AddWebRoute(const WebRoute& entry);
    void  SetMimes(const Mime::MimeTypesMap& map);
    void  SetLogger(Log::Logger* a, Log::Logger* e, Log::Logger* s);
    void  ServeForever();

 private:
    Log::Logger*        __access_log;
    Log::Logger*        __system_log;
    Log::Logger*        __error_log;

    IO::Poller          __poller;
    Event::Loop         __evloop;

    SocketFdMap         __listeners_map;
    SessionFdMap        __sessions_map;
    StaticFileFdMap     __stat_files_read_map;

    WebRouteList        __routes;
    Mime::MimeTypesMap  __mime_map;
};

}

#endif  // NETLIB_SERVER_WEBSERVER_H_
