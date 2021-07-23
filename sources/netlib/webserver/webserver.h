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
    struct UploadReq {
        std::string filename;
        std::string file_content;
    };

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


        bool                  conn_close;
        int                   res_code;

        CgiEntry*             __link_cgi;
        StaticFile            __link_stfile;
        fd_t                  __listener_fd;
        u64                   __timeout_ms;

        void  UpdateTimeout(u64 timeout_ms) { __timeout_ms = tv_to_msec(GetTimeOfDay()) + timeout_ms; }
    };

    struct CgiEntry {
        IO::File              fd_in;
        IO::File              fd_out;
        pid_t                 pid;
        std::string           in_buf;
        Cgi::ResponseReader   cgi_rdr;
        Http::Response        cgi_res;
    };

    typedef std::set<Http::Method> MethodSet;

    struct WebRedirect {
        bool                enabled;
        std::string         location;
        int                 code;
    };

    struct WebRoute {
        std::string            pattern;
        std::string            root_directory;
        std::string            index_page;          // if empty, index page is disabled
        WebRedirect            reditect;            // if empty, redirection is disabled
        MethodSet              allowed_methods;
        bool                   cgi_enabled;
        bool                   upload_enabled;
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

    typedef std::map< std::string, std::string > CgiDriverMap;

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
    typedef std::map< pid_t, CgiEntry >              CgiPidMap;

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

    /// CGI hooks
    Event::IEventPtr    __SpawnCgiHook();
    class  EvCgiHook;
    void                __EvaluateCgiWorkers();

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

    /// CGI
    Event::IEventPtr    __SpawnCgiEvent(IO::Poller::PollEvent ev, SessionCtx* ss);
    class  EvCgiRead;
    class  EvCgiError;
    class  EvCgiWrite;
    class  EvCgiHup;

    Cgi::CStringVec     __FillCgiMetavars(SessionCtx* ss, const std::string& filepath);
    bool                __AvaibleCgiDriver(const std::string& filepath);
    std::string         __GetCgiDriver(const std::string& filepath);
    void                __HandleCgiRequest(SessionCtx* ss, const std::string& filepath);
    void                __StopCgiWorker(CgiEntry* ce);
    void                __StopCgiRead(CgiEntry* ce);
    void                __StopCgiWrite(CgiEntry* ce);
    void                __OnCgiResponse(SessionCtx* ss);
    void                __OnCgiOutput(SessionCtx* ss);
    void                __OnCgiInput(SessionCtx* ss);
    void                __OnCgiError(SessionCtx* ss);
    void                __OnCgiHup(SessionCtx* ss);
    void                __CgiWorker(fd_t ipip[2], fd_t opip[2],
                                    SessionCtx* ss, const std::string& filepath);

    bool                __IsUpload(SessionCtx* ss, const WebRoute& rt);
    void                __HandleUploadRequest(SessionCtx* ss, const WebRoute& rt);
    void                __OnUploadEnd(SessionCtx* ss, const WebRoute& route,
                                      const std::list<UploadReq>& files);

 public:
    void  SetTimeout(u64 msec);
    void  SetSystemLogger(Log::Logger* s);
    void  AddVritualServer(const IO::SockInfo& si, const VirtualServer& vs);

    void  Config(const Config::Category& cat, Cgi::Envs evs);

    void  ServeForever();

 private:
    Log::Logger*        __system_log;

    IO::Poller          __poller;
    Event::Loop         __evloop;

    VirtualServerMap    __vservers_map;
    SocketFdMap         __listeners_map;
    SessionFdMap        __sessions_map;
    SessionFdMap        __stat_files_read_map;
    SessionFdMap        __cgi_fd_map;
    CgiPidMap           __cgi_pid_map;

    Cgi::Envs           __envs;
    CgiDriverMap        __cgi_drivers;
    u64                 __session_timeout;
};

}

#endif  // NETLIB_SERVER_WEBSERVER_H_
