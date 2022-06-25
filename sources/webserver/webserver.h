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

#include "io/socket.h"

#include "io/poller.h"

#include "event/event.h"
#include "event/loop.h"

#include "config/config.h"

#include "logger/logger.h"
#include "cgi/cgi.h"
#include "cgi/response_reader.h"

namespace Webserver {

class HttpServer {
 public:
    static void PrintDebugInfo(IO::Poller::PollEvent p, Fd fd, Log::Logger* l) {
        const char* PollerEventStr;
        switch (p) {
            case IO::Poller::POLL_CLOSE:        PollerEventStr = "POLL_CLOSE"; break;
            case IO::Poller::POLL_ERROR:        PollerEventStr = "POLL_ERROR"; break;
            case IO::Poller::POLL_NONE:         PollerEventStr = "POLL_NONE"; break;
            case IO::Poller::POLL_WRITE:        PollerEventStr = "POLL_WRITE"; break;
            case IO::Poller::POLL_READ:         PollerEventStr = "POLL_READ"; break;
            case IO::Poller::POLL_NOT_OPEN:     PollerEventStr = "POLL_NOT_OPEN"; break;
            default:                            PollerEventStr = "UNKNOWN"; break;
        }
        debug(l, "DEBUG_POLL_INFO: PEV='%s', PEV=0x%x, FD=%d", PollerEventStr, p, fd);
    }

    struct DebugEvent : public Event::IEvent {
        Log::Logger*           Logger;
        IO::Poller::PollEvent  PollerEv;
        Fd                     FileD;

             DebugEvent(Log::Logger* l, IO::Poller::PollEvent p, Fd f) : Logger(l), PollerEv(p), FileD(f) {}
        void Handle() {
            // HttpServer::PrintDebugInfo(pe, fd, logger);
        }
    };

 public:
    struct UploadReq {
        std::string FileName;
        std::string FileContent;
    };

    struct StaticFile {
        bool        IsClosed;
        IO::File    File;
    };

    struct VirtualServer;
    struct CgiEntry;

    struct SessionCtx {
        VirtualServer*       Server;
        IO::Socket           ConnectionSock;
        std::string          ResultBuffer;
        Http::RequestReader  RequestReader;

        Log::Logger*         AccessLog;
        Log::Logger*         ErrorLOg;

        Http::ResponseWriter  ResponseWriter;
        Http::Request         Request;


        bool                  IsConnectionClosed;
        int                   ResponseCode;

        CgiEntry*             CgiPtr;
        StaticFile            StatfilePtr;
        Fd                    ListenerFileDesc;
        UInt64                TimeoutMs_;

        void  UpdateTimeout(UInt64 timeoutMs) { TimeoutMs_ = tv_to_msec(GetTimeOfDay()) + timeoutMs; }
    };

    struct CgiEntry {
        IO::File              FileDescIn;
        IO::File              FileDescOut;
        pid_t                 Pid;
        std::string           InBuffer;
        Cgi::ResponseReader   CgiReader;
        Http::Response        CgiResponse;
    };

    typedef std::set<Http::Method> MethodSet;

    struct WebRedirect {
        bool                Enabled;
        std::string         Location;
        int                 Code;
    };

    struct WebRoute {
        std::string            Pattern;
        std::string            RootDir;
        std::string            IndexPage;          // if empty, index page is disabled
        WebRedirect            Redirect;            // if empty, redirection is disabled
        MethodSet              AllowedMethods;
        bool                   CgiEnabled;
        bool                   UploadEnabled;
        bool                   ListingEnabled;
    };

public:
    /*                 route                                     */
    typedef std::list< WebRoute >             WebRouteList;
    /*                fd    listen_sock                          */
    typedef std::map< Fd, IO::Socket >      SocketFdMap;
    /*                fd    session_ctx                          */
    typedef std::map< Fd, SessionCtx* >     SessionFdMap;
    /*               errcode   page_path                         */
    typedef std::map< int,     std::string >  ErrpageMap;

    typedef std::map< std::string, std::string > CgiDriverMap;

    struct VirtualServer {
        typedef std::list<std::string> HostnameList;

        HostnameList        Hostnames;
        Mime::MimeTypesMap  MimeMap;
        WebRouteList        Routes;
        ErrpageMap          ErrorPages;

        Log::Logger*        AccessLog;
        Log::Logger*        ErrorLog;
    };

private:
    /*                    listen_fd   server                     */
    typedef std::multimap< Fd,     VirtualServer > VirtualServerMap;
    typedef std::map< pid_t, CgiEntry >              CgiPidMap;

 private:
    /// Event basics logic
    void                EvaluateIoEvents();
    IO::Poller::Result  PollEvent();
    Event::IEventPtr    SwitchEventSpawners(IO::Poller::PollEvent pev, Fd fd);

    Event::IEventPtr    SpawnPollerHook();
    class  EvPollerHook;
 
    /// Timeout
    Event::IEventPtr    SpawnTimeoutHook();
    class  EvTimeoutHook;

    void                EvaluateIoTimeouts();
    void                OnSessionTimeout(SessionCtx* ss);

    /// CGI hooks
    Event::IEventPtr    SpawnCgiHook();
    class  EvCgiHook;
    void                EvaluateCgiWorkers();

    /// Listener I/O
    Event::IEventPtr    SpawnListenerEvent(IO::Poller::PollEvent ev, IO::Socket* sock);
    class  EvListenerNewConnection;
    void                OnListenerAccept(IO::Socket* listener_sock);

    /// Http logic
    void                OnHttpRequest(SessionCtx* ss);
    void                OnHttpResponse(SessionCtx* ss);
    void                OnHttpRedirect(SessionCtx* ss, const std::string& location, int code = 302);
    void                OnHttpError(SessionCtx* ss, bool reset = true);

    const WebRoute*     FindWebRoute(const Http::Request& req, const WebRouteList& routes_list);

    void                HandleStaticFile(SessionCtx* ss, const std::string& file_path);
    void                HandleBadMethod(SessionCtx* ss, const WebRoute& route);
    void                HandleDirectoryResource(SessionCtx* ss, const WebRoute& route,
                                                                  const std::string& fpath);
    void                HandleDeleteFile(SessionCtx* ss, const std::string& filepath);
    
    /// Sessions Logic
    SessionCtx*         NewSessionCtx(const IO::Socket& sock, Fd fd);
    void                StartSessionCtx(SessionCtx* ss);
    void                DeleteSessionCtx(SessionCtx* ss);

    VirtualServer*      GetVirtualServer(Fd lfd, const std::string& hostname);

    /// Sessions I/O handling
    Event::IEventPtr    SpawnSessionEvent(IO::Poller::PollEvent ev, SessionCtx* ss);
    class  EvSessionRead;
    class  EvSessionWrite;
    class  EvSessionError;
    class  EvSessionHup;
    void                OnSessionRead(SessionCtx* ss);
    void                OnSessionWrite(SessionCtx* ss);
    void                OnSessionError(SessionCtx* ss);
    void                OnSessionHup(SessionCtx* ss);

    /// For responsnses with static files
    void                SendDirectoryListing(const std::string& resource_path, SessionCtx* ss);
    void                SendStaticFileResponse(IO::File file, SessionCtx* ss);
    void                RemoveStaticFileCtx(SessionCtx* stfile);

    Event::IEventPtr    SpawnStaticFileReadEvent(IO::Poller::PollEvent ev, SessionCtx* ss);
    class  EvStaticFileRead;
    class  EvStaticFileReadError;
    void                OnStaticFileRead(SessionCtx* ss);
    void                OnStaticFileReadError(SessionCtx* ss);
    void                OnStaticFileReadEnd(SessionCtx* ss);

    IO::File            GetErrPage(int errcode, SessionCtx* ss);
    void                SendDefaultErrPage(SessionCtx* ss);

    /// CGI
    Event::IEventPtr    SpawnCgiEvent(IO::Poller::PollEvent ev, SessionCtx* ss);
    class  EvCgiRead;
    class  EvCgiError;
    class  EvCgiWrite;
    class  EvCgiHup;

    Cgi::CStringVec     FillCgiMetavars(SessionCtx* ss, const std::string& filepath);
    bool                AvaibleCgiDriver(const std::string& filepath);
    std::string         GetCgiDriver(const std::string& filepath);
    void                HandleCgiRequest(SessionCtx* ss, const std::string& filepath);
    void                StopCgiWorker(CgiEntry* ce);
    void                StopCgiRead(CgiEntry* ce);
    void                StopCgiWrite(CgiEntry* ce);
    void                OnCgiResponse(SessionCtx* ss);
    void                OnCgiOutput(SessionCtx* ss);
    void                OnCgiInput(SessionCtx* ss);
    void                OnCgiError(SessionCtx* ss);
    void                OnCgiHup(SessionCtx* ss);
    void                CgiWorker(Fd ipip[2], Fd opip[2],
                                    SessionCtx* ss, const std::string& filepath);

    bool                IsUpload(SessionCtx* ss, const WebRoute& rt);
    void                HandleUploadRequest(SessionCtx* ss, const WebRoute& rt);
    void                OnUploadEnd(SessionCtx* ss, const WebRoute& route,
                                      const std::list<UploadReq>& files);

 public:
    void  SetTimeout(UInt64 msec);
    void  SetSystemLogger(Log::Logger* s);
    void  AddVritualServer(const IO::SockInfo& si, const VirtualServer& vs);

    void  Config(const Config::Category& cat, Cgi::Envs evs);

    void  ServeForever();

 private:
    Log::Logger*        SystemLog_;

    IO::Poller          Poller_;
    Event::Loop         EventLoop_;

    VirtualServerMap    VirtualServers_;
    SocketFdMap         Listeners_;
    SessionFdMap        WebSessions_;
    SessionFdMap        StatfileSessions_;
    SessionFdMap        CgiSessions_;
    CgiPidMap           CgiPids_;

    Cgi::Envs           Envs_;
    CgiDriverMap        CgiDrivers_;
    UInt64              SessionTimeout_;
    USize               MaxBodySize_;
};

}

#endif  // NETLIB_SERVER_WEBSERVER_H_
