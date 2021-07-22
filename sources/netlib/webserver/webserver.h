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

#include "config/config.h"

#include "logger/logger.h"
#include "cgi/cgi.h"
#include "cgi/response_reader.h"

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
        void Handle() {
            // HttpServer::PrintDebugInfo(pe, fd, logger);
        }
    };

 public:
    struct StaticFile {
        bool        closed;
        IO::File    file;
    };

    struct VirtualServer;
    struct CgiEntry;

    struct SessionCtx {
        VirtualServer*       server;
        IO::Socket           conn_sock;
        std::string          res_buff;
        Http::RequestReader  req_rdr;

        Log::Logger*         access_log;
        Log::Logger*         error_log;

        Http::ResponseWriter  http_writer;
        Http::Request         http_req;

        CgiEntry*             cgi;

        bool                  conn_close;
        int                   res_code;

        StaticFile            __link_stfile;
        fd_t                  __listener_fd;
        u64                   __timeout_ms;

        void  UpdateTimeout(u64 timeout_ms) { __timeout_ms = tv_to_msec(GetTimeOfDay()) + timeout_ms; }
    };

    struct CgiEntry {
        IO::File              fd_in;
        IO::File              fd_out;
        u32                   pid;
        SessionCtx*           session;
        std::string           in_buf;
        Cgi::ResponseReader   cgi_rdr;
    };

    typedef std::set<Http::Method> MethodSet;

    struct WebRedirect {
        bool                enabled;
        std::string         location;
        int                 code;
    };

    struct CgiOptions {
        std::string path_to_driver;
    };

    struct WebRoute {
        std::string            pattern;
        std::string            root_directory;
        std::string            index_page;          // if empty, index page is disabled
        WebRedirect            reditect;            // if empty, redirection is disabled
        bool                   cgi_enabled;
        MethodSet              allowed_methods;
        bool                   listing_enabled;
    };

public:
    /*                 route                                     */
    typedef std::list< WebRoute >             WebRouteList;
    /*                fd    listen_sock                          */
    typedef std::map< fd_t, IO::Socket >      SocketFdMap;
    /*                fd    session_ctx                          */
    typedef std::map< fd_t, SessionCtx* >     SessionFdMap;
    /*               errcode   page_path                         */
    typedef std::map< int,     std::string >  ErrpageMap;

    struct VirtualServer {
        typedef std::list<std::string> Hostnames;

        Hostnames           hostnames;
        Mime::MimeTypesMap  mime_map;
        WebRouteList        routes;
        ErrpageMap          errpages;

        Log::Logger*        access_log;
        Log::Logger*        error_log;
    };

private:
    /*                    listen_fd   server                     */
    typedef std::multimap< fd_t,     VirtualServer > VirtualServerMap;

    typedef std::map< fd_t, CgiEntry* >       CgiFdMap;

    typedef std::list< CgiEntry >       CgiList;

 private:
    /// Event basics logic
    void                __EvaluateIoEvents();
    IO::Poller::Result  __PollEvent();
    Event::IEventPtr    __SwitchEventSpawners(IO::Poller::PollEvent pev, fd_t fd);

    Event::IEventPtr    __SpawnPollerHook();
    class  EvPollerHook;
 
    /// Timeout
    Event::IEventPtr    __SpawnTimeoutHook();
    class  EvTimeoutHook;

    void                __EvaluateIoTimeouts();
    void                __OnSessionTimeout(SessionCtx* ss);

    /// Listener I/O
    Event::IEventPtr    __SpawnListenerEvent(IO::Poller::PollEvent ev, IO::Socket* sock);
    class  EvListenerNewConnection;
    void                __OnListenerAccept(IO::Socket* listener_sock);

    /// Http logic
    void                __OnHttpRequest(SessionCtx* ss);
    void                __OnHttpResponse(SessionCtx* ss);
    void                __OnHttpRedirect(SessionCtx* ss, const std::string& location, int code = 302);
    void                __OnHttpError(SessionCtx* ss, bool reset = true);

    const WebRoute*     __FindWebRoute(const Http::Request& req, const WebRouteList& routes_list);

    void                __HandleStaticFile(SessionCtx* ss, const std::string& file_path);
    void                __HandleBadMethod(SessionCtx* ss, const WebRoute& route);
    void                __HandleDirectoryResource(SessionCtx* ss, const WebRoute& route,
                                                                  const std::string& fpath);

    /// Sessions Logic
    SessionCtx*         __NewSessionCtx(const IO::Socket& sock, fd_t fd);
    void                __StartSessionCtx(SessionCtx* ss);
    void                __DeleteSessionCtx(SessionCtx* ss);

    VirtualServer*      __GetVirtualServer(fd_t lfd, const std::string& hostname);

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
    void                __RemoveStaticFileCtx(SessionCtx* stfile);

    Event::IEventPtr    __SpawnStaticFileReadEvent(IO::Poller::PollEvent ev, SessionCtx* ss);
    class  EvStaticFileRead;
    class  EvStaticFileReadError;
    void                __OnStaticFileRead(SessionCtx* ss);
    void                __OnStaticFileReadError(SessionCtx* ss);
    void                __OnStaticFileReadEnd(SessionCtx* ss);

    IO::File            __GetErrPage(int errcode, SessionCtx* ss);
    void                __SendDefaultErrPage(SessionCtx* ss);

//CGI
    void                __OnCgiRequest(SessionCtx* ss, const WebRoute& route);
    // Event::IEventPtr    __SpawnCgiWriteEvent(IO::Poller::PollEvent ev, IO::File file, SessionCtx* ss);
    // Event::IEventPtr    __SpawnCgiReadEvent(IO::Poller::PollEvent ev, IO::File file, SessionCtx* ss);
    // class  EvCgiWriteError;
    void                __OnCgiFdRead(CgiEntry* cgi);
    void                __OnCgiFdError(CgiEntry* cgi, fd_t fd);
    void                __OnCgiFdWrite(CgiEntry* cgi);
    void                CgiCheckPid();
    void                __RmCgiFd(IO::File& fd);
    // void                __OnCgiFdReadEnd(IO::File file, CgiEntry* ss);
    void                __CgiIOStart(CgiEntry* cgi, const std::string& resource_path, const WebRoute& route);
    void                __CgiOutStart(CgiEntry* cgi, const std::string& resource_path, const WebRoute& route);
    Event::IEventPtr    __SpawnCgiEvent(IO::Poller::PollEvent ev, CgiEntry* cgi, fd_t fd);
    Event::IEventPtr    __SpawnCgiPidCheckHook();
    Cgi::Envs           __FillEnv(SessionCtx* ss);
    class  EvCgiRead;
    class  EvCgiFdError;
    class  EvCgiWrite;
    class  EvCgiCheckPid;
    // void                __OnCgiFdWriteError(IO::File file, SessionCtx* ss);
    // void                __OnCgiFdWriteEnd(IO::File file, SessionCtx* ss);
///-----
 public:
    void  SetTimeout(u64 msec);
    void  SetSystemLogger(Log::Logger* s);
    void  AddVritualServer(const IO::SockInfo& si, const VirtualServer& vs);

    void  Config(const Config::Category& cat);

    void  ServeForever();

 private:
    Log::Logger*        __system_log;

    IO::Poller          __poller;
    Event::Loop         __evloop;

    VirtualServerMap    __vservers_map;
    SocketFdMap         __listeners_map;
    SessionFdMap        __sessions_map;
    SessionFdMap        __stat_files_read_map;
    CgiInFdMap          __cgi_in_map;
    CgiOutFdMap         __cgi_out_map;

    u64                 __session_timeout;
};

}

#endif  // NETLIB_SERVER_WEBSERVER_H_
