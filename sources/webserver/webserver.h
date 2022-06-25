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
    void                __EvaluateIoEvents();
    IO::Poller::Result  __PollEvent();
    Event::IEventPtr    __SwitchEventSpawners(IO::Poller::PollEvent pev, Fd fd);

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
    void                __HandleDeleteFile(SessionCtx* ss, const std::string& filepath);
    
    /// Sessions Logic
    SessionCtx*         __NewSessionCtx(const IO::Socket& sock, Fd fd);
    void                __StartSessionCtx(SessionCtx* ss);
    void                __DeleteSessionCtx(SessionCtx* ss);

    VirtualServer*      __GetVirtualServer(Fd lfd, const std::string& hostname);

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
    void                __CgiWorker(Fd ipip[2], Fd opip[2],
                                    SessionCtx* ss, const std::string& filepath);

    bool                __IsUpload(SessionCtx* ss, const WebRoute& rt);
    void                __HandleUploadRequest(SessionCtx* ss, const WebRoute& rt);
    void                __OnUploadEnd(SessionCtx* ss, const WebRoute& route,
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
