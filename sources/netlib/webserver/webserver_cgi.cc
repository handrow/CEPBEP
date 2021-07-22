#include "netlib/webserver/webserver.h"

#include "common/file.h"

#include "dirent.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace Webserver {

void HttpServer::__EvaluateCgiWorkers() {
    for (CgiPidMap::iterator it = __cgi_pid_map.begin();
                             it != __cgi_pid_map.end();
                            ++it) {
        CgiEntry& ce = it->second;

        int rc = waitpid(ce.pid, NULL, WNOHANG);
        // not dead
        if (rc == 0)
            continue;
        
        if (rc < 0) {
            __StopCgiWorker(&ce);
        }

        // dead
        __cgi_pid_map.erase(it--);
    }
}

Cgi::CStringVec
HttpServer::__FillCgiMetavars(SessionCtx* ss, const std::string& filepath) {
    Cgi::Metavars   metavar;    
    metavar.AddHttpHeaders(ss->http_req.headers);
    metavar.AddEnvs(__envs);
    metavar.GetMapRef()["REMOTE_ADDR"] = std::string(ss->conn_sock.GetSockInfo().addr_BE);
    metavar.GetMapRef()["CONTENT_TYPE"] = ss->http_req.headers.Get("Content-Type");
    metavar.GetMapRef()["CONTENT_LENGTH"] = ss->http_req.headers.Get("Content-Length");
    metavar.GetMapRef()["QUERY_STRING"] = ss->http_req.uri.query_str;
    metavar.GetMapRef()["GATEWAY_INTERFACE"] = "CGI/1.1";
    metavar.GetMapRef()["SERVER_PORT"] = u16(__listeners_map[ss->__listener_fd].GetSockInfo().port_BE);
    metavar.GetMapRef()["SERVER_PROTOCOL"] = Http::ProtocolVersionToString(ss->http_req.version);
    metavar.GetMapRef()["SERVER_SOFTWARE"] = "webserv/1.0.0";
    metavar.GetMapRef()["REQUEST_METHOD"] = Http::MethodToString(ss->http_req.method);
    metavar.GetMapRef()["SCRIPT_NAME"] = filepath;
    metavar.GetMapRef()["SERVER_NAME"] = ss->http_req.headers.Get("Host");
    return metavar.ToEnvVector();
}

bool  HttpServer::__AvaibleCgiDriver(const std::string& filepath) {
    std::string fileext = GetFileExt(filepath);
    return __cgi_drivers.count(fileext) > 0;
}

std::string  HttpServer::__GetCgiDriver(const std::string& filepath) {
    std::string fileext = GetFileExt(filepath);

    if (__cgi_drivers.count(fileext) > 0) {
        return __cgi_drivers[fileext];
    }

    return "";
}

void  HttpServer::__CgiWorker(fd_t ipip[], fd_t opip[], SessionCtx* ss, const std::string& filepath) {
    if (dup2(ipip[0], STDIN_FILENO) < 0)
        exit(1);
    if (dup2(opip[1], STDOUT_FILENO) < 0)
        exit(1);

    close(ipip[0]);
    close(ipip[1]);
    close(opip[0]);
    close(opip[1]);

    std::string cgi_driver = __GetCgiDriver(filepath);

    Cgi::CStringVec arg_vec;
    arg_vec.push_back(C::string());
    arg_vec.push_back(C::string(filepath));
    arg_vec.push_back(NULL);

    Cgi::CStringVec env_vec = __FillCgiMetavars(ss, filepath);

    execve(cgi_driver.c_str(), Cgi::CastToEnvs(arg_vec), Cgi::CastToEnvs(env_vec));
    exit(1);
}

void  HttpServer::__HandleCgiRequest(SessionCtx* ss, const std::string& filepath) {
    CgiEntry    cgi;
    debug(__system_log, "BEGIN");
    cgi.in_buf = ss->http_req.body;

    fd_t ipip[2] = {-1, -1};
    fd_t opip[2] = {-1, -1};

    if (pipe(ipip) < 0)
        return ss->res_code = 500, __OnHttpError(ss);

    if (pipe(opip) < 0)
        return close(ipip[0]), close(ipip[1]),
               ss->res_code = 500, __OnHttpError(ss);

    cgi.pid = fork();
    if (cgi.pid == 0) {
        // cgi worker
        return __CgiWorker(ipip, opip, ss, filepath);
    }
    // cgi watcher

    if (cgi.pid < 0) {
        close(ipip[0]);
        close(ipip[1]);
        close(opip[0]);
        close(opip[1]);
        return ss->res_code = 500, __OnHttpError(ss);
    }

    cgi.fd_in = IO::File(ipip[1]);
    close(ipip[0]);
    cgi.fd_out = IO::File(opip[0]);
    close(opip[1]);

    /// Add cgi to our inner structures
    __cgi_pid_map[cgi.pid] = cgi;
    ss->__link_cgi = &__cgi_pid_map[cgi.pid];

    __cgi_fd_map[cgi.fd_in.GetFd()] = ss;
    __cgi_fd_map[cgi.fd_out.GetFd()] = ss;

    debug(__system_log, "BEGIN");

    if (!cgi.in_buf.empty())
        __poller.AddFd(cgi.fd_in.GetFd(), IO::Poller::POLL_WRITE);
    __poller.AddFd(cgi.fd_out.GetFd(), IO::Poller::POLL_READ);
}

void  HttpServer::__StopCgiWorker(CgiEntry* ce) {
    kill(ce->pid, SIGKILL);
}

void  HttpServer::__StopCgiRead(CgiEntry* ce) {
    __poller.RmFd(ce->fd_out.GetFd());
    __cgi_fd_map.erase(ce->fd_out.GetFd());
}

void  HttpServer::__StopCgiWrite(CgiEntry* ce) {
    __poller.RmFd(ce->fd_in.GetFd());
    __cgi_fd_map.erase(ce->fd_in.GetFd());
}

void  HttpServer::__OnCgiResponse(SessionCtx* ss) {
    CgiEntry& ce = *ss->__link_cgi;
    __StopCgiRead(&ce);
    ss->http_writer.Write(ce.cgi_res.body);
    ss->http_writer.Header().SetMap(ce.cgi_res.headers.GetMap());
    return ss->res_code = ce.cgi_res.code, __OnHttpResponse(ss);
}

void  HttpServer::__OnCgiOutput(SessionCtx* ss) {
    static const usize READ_BUF_SZ = 10000;
    CgiEntry& ce = *ss->__link_cgi;
    std::string portion = ce.fd_out.Read(READ_BUF_SZ);

    if (portion.empty()) {
        ce.cgi_rdr.EndRead();
    } else {
        ce.cgi_rdr.Read(portion);
    }

    ce.cgi_rdr.Process();

    if (ce.cgi_rdr.HasError())
        return __OnCgiError(ss);

    if (ce.cgi_rdr.HasMessage()) {
        ce.cgi_res = ce.cgi_rdr.GetMessage();
        return __OnCgiResponse(ss);
    }
}

void  HttpServer::__OnCgiInput(SessionCtx* ss) {
    CgiEntry& ce = *ss->__link_cgi;
    static const usize  WRITE_BUFF_SZ = 10000;

    if (ce.in_buf.empty()) {
        __StopCgiWrite(ss->__link_cgi);
    } else {
        std::string  portion = ce.in_buf.substr(0, WRITE_BUFF_SZ);
        isize trasmitted_bytes = ce.fd_in.Write(portion);

        ce.in_buf = ce.in_buf.substr(trasmitted_bytes);
    }
}

void  HttpServer::__OnCgiError(SessionCtx* ss) {
    __StopCgiWrite(ss->__link_cgi);
    __StopCgiRead(ss->__link_cgi);
    __StopCgiWorker(ss->__link_cgi);

    return ss->res_code = 500, __OnHttpError(ss);
}

class HttpServer::EvCgiRead : public Event::IEvent {
 private:
    HttpServer* __server;
    SessionCtx* __session;

 public:
    EvCgiRead(HttpServer *srv, SessionCtx* ss)
    : __server(srv)
    , __session(ss)
    {
    }

    void  Handle() {
        __server->__OnCgiOutput(__session);
    }
};

class HttpServer::EvCgiWrite : public Event::IEvent {
 private:
    HttpServer* __server;
    SessionCtx* __session;

 public:
    EvCgiWrite(HttpServer *srv, SessionCtx* ss)
    : __server(srv)
    , __session(ss)
    {
    }

    void  Handle() {
        __server->__OnCgiInput(__session);
    }
};

class HttpServer::EvCgiError : public Event::IEvent {
 private:
    HttpServer* __server;
    SessionCtx* __session;

 public:
    EvCgiError(HttpServer *srv, SessionCtx* ss)
    : __server(srv)
    , __session(ss)
    {
    }

    void  Handle() {
        __server->__OnCgiError(__session);
    }
};

class HttpServer::EvCgiHook : public Event::IEvent {
 private:
    HttpServer* __server;

 public:
    EvCgiHook(HttpServer *srv) : __server(srv) {
    }

    void  Handle() {
        __server->__EvaluateCgiWorkers();
    }
};

Event::IEventPtr    HttpServer::__SpawnCgiEvent(IO::Poller::PollEvent ev, SessionCtx* ss) {
    if (ev == IO::Poller::POLL_READ || ev == IO::Poller::POLL_CLOSE) {
        return new EvCgiRead(this, ss);
    } else if (ev == IO::Poller::POLL_WRITE) {
        return new EvCgiWrite(this, ss);
    } else if (ev == IO::Poller::POLL_ERROR) {
        return new EvCgiError(this, ss);
    } else {
        throw std::runtime_error("Unsupported event type for CgiReadEvent: " + Convert<std::string>(ev));
    }
}

Event::IEventPtr    HttpServer::__SpawnCgiHook() {
    return new EvCgiHook(this);
}

}  // namespace Webserver
