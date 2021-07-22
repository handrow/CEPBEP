#include "netlib/webserver/webserver.h"

#include "common/file.h"

#include "dirent.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace Webserver {

class HttpServer::EvCgiRead : public Event::IEvent {
    private:
        HttpServer* __server;
        CgiEntry* __cgi_entry;

    public:
        EvCgiRead(HttpServer *srv, CgiEntry* cgi)
        : __server(srv)
        , __cgi_entry(cgi)
        {
        }

        void  Handle() {
            __server->__OnCgiFdRead(__cgi_entry);
        }
};

class HttpServer::EvCgiWrite : public Event::IEvent {
    private:
        HttpServer* __server;
        CgiEntry* __cgi_entry;

    public:
        EvCgiWrite(HttpServer *srv, CgiEntry* cgi)
        : __server(srv)
        , __cgi_entry(cgi)
        {
        }

        void  Handle() {
            __server->__OnCgiFdWrite(__cgi_entry);
        }
};

class HttpServer::EvCgiFdError : public Event::IEvent {
    private:
        HttpServer* __server;
        CgiEntry*   __cgi_entry;
        fd_t        __fd;

    public:
        EvCgiFdError(HttpServer *srv, CgiEntry* cgi, fd_t fd)
        : __server(srv)
        , __cgi_entry(cgi)
        , __fd(fd)
        {
        }

        void  Handle() {
            __server->__OnCgiFdError(__cgi_entry, __fd);
        }
};

class HttpServer::EvCgiCheckPid : public Event::IEvent {
    private:
        HttpServer* __server;

    public:
        EvCgiCheckPid(HttpServer *srv)
        : Event::IEvent(5000)
        , __server(srv)
        {
        }

        void  Handle() {
            __server->CgiCheckPid();
        }
};

Event::IEventPtr    HttpServer::__SpawnCgiEvent(IO::Poller::PollEvent ev, CgiEntry* cgi, fd_t fd) {
    if (ev == IO::Poller::POLL_READ || ev == IO::Poller::POLL_CLOSE) {
        return new EvCgiRead(this, cgi);
    } else if (ev == IO::Poller::POLL_WRITE) {
        return new EvCgiWrite(this, cgi);
    } else if (ev == IO::Poller::POLL_ERROR) {
        return new EvCgiFdError(this, cgi, fd);
    } else {
        throw std::runtime_error("Unsupported event type for CgiReadEvent: " + Convert<std::string>(ev));
    }
}

Event::IEventPtr    HttpServer::__SpawnCgiPidCheckHook() {
    return new EvCgiCheckPid(this);
}

Cgi::Envs HttpServer::__FillEnv(SessionCtx* ss) {
    Cgi::Metavars filled;
    filled.AddEnvs(__env);
    filled.AddHttpHeaders(ss->http_req.headers);
    filled.GetMapRef()["AUTH_TYPE"] = ss->http_req.headers.Get("Authorization").substr(0, ' ');
    filled.GetMapRef()["CONTENT_TYPE"] = ss->http_req.headers.Get("Content-Length");
    filled.GetMapRef()["CONTENT_TYPE"] = ss->http_req.headers.Get("Content-Type");
    filled.GetMapRef()["GATEWAY_INTERFACE"] = "1.1";
    filled.GetMapRef()["PATH_INFO"] = ss->http_req.uri.path;
    filled.GetMapRef()["PATH_INFO"] = ss->http_req.uri.path;
    filled.GetMapRef()["QUERY_STRING"] = ss->http_req.uri.query_str;
    filled.GetMapRef()["REMOTE_ADDR"] = std::string(ss->conn_sock.GetSockInfo().addr_BE);
    filled.GetMapRef()["REMOTE_HOST"] = "";
    filled.GetMapRef()["REMOTE_USER"] = "";
    filled.GetMapRef()["REQUEST_METHOD"] = ss->http_req.method;
    filled.GetMapRef()["SCRIPT_NAME"] = ss->http_req.uri.path;
    filled.GetMapRef()["SERVER_NAME"] = "";
    filled.GetMapRef()["SERVER_PORT"] = "";
    filled.GetMapRef()["SERVER_PROTOCOL"] = "HTTP/ 1.1";
    filled.GetMapRef()["SERVER_SOFTWARE"] = "CEPBEP";
    return Cgi::CastToEnvs(filled.ToEnvVector());
}

void HttpServer::__RmCgiFd(IO::File& fd) {
    if (close(fd.GetFd()) != -1) {
        __poller.RmFd(fd.GetFd());
        __cgi_fd_map.erase(fd.GetFd());
    }
    errno = 0;
}

void HttpServer::__CgiIOStart(CgiEntry* cgi, const std::string& resource_path, const WebRoute& route) {
    int pipe_in[2];
    int pipe_out[2];

    if (pipe(pipe_in) < 0)
        throw std::runtime_error("pipe(): ");
    if (pipe(pipe_out) < 0)
        throw std::runtime_error("pipe(): ");
    cgi->pid = fork();
    if (!cgi->pid) {
        if (dup2(pipe_in[0], STDIN_FILENO) < 0)
            throw std::runtime_error("dup2(): ");
        close(pipe_in[0]);
        close(pipe_in[1]);
        if (dup2(pipe_out[1], STDOUT_FILENO) < 0)
            throw std::runtime_error("dup2(): ");
        close(pipe_out[0]);
        close(pipe_out[1]);
        Cgi::CStringVec arg;
        arg.push_back(C::string(__cgi_options.path_to_driver));
        arg.push_back(C::string(resource_path + route.index_page));
        arg.push_back(NULL);
        execve(__cgi_options.path_to_driver.size() ? __cgi_options.path_to_driver.c_str() : Cgi::CastToEnvs(arg)[0], Cgi::CastToEnvs(arg), __FillEnv(cgi->session));
        exit(1);
    }
    else if (cgi->pid < 0)
        throw std::runtime_error("fork(): ");

    close(pipe_in[0]);
    __poller.AddFd(pipe_in[1], IO::Poller::POLL_WRITE);
    close(pipe_out[1]);
    __poller.AddFd(pipe_out[0], IO::Poller::POLL_READ);

    cgi->fd_in = IO::File(pipe_in[1]);
    cgi->fd_out = IO::File(pipe_out[0]);
    __cgi_fd_map[pipe_in[1]] = cgi;
    __cgi_fd_map[pipe_out[0]] = cgi;
}

void HttpServer::__CgiOutStart(CgiEntry* cgi, const std::string& resource_path, const WebRoute& route) {
    int pipe_out[2];

    if (pipe(pipe_out) < 0)
        throw std::runtime_error("pipe(): ");
    cgi->pid = fork();
    if (!cgi->pid) {
        if (dup2(pipe_out[1], STDOUT_FILENO) < 0)
            throw std::runtime_error("dup2(): ");
        close(pipe_out[0]);
        close(pipe_out[1]);
        Cgi::CStringVec arg;
        arg.push_back(C::string(__cgi_options.path_to_driver));
        arg.push_back(C::string(resource_path + route.index_page));
        arg.push_back(NULL);
        execve(__cgi_options.path_to_driver.size() ? __cgi_options.path_to_driver.c_str() : Cgi::CastToEnvs(arg)[0], Cgi::CastToEnvs(arg), __FillEnv(cgi->session));
        exit(1);
    }
    else if (cgi->pid < 0)
        throw std::runtime_error("fork(): ");
    close(pipe_out[1]);
    __poller.AddFd(pipe_out[0], IO::Poller::POLL_READ);

    cgi->fd_out = IO::File(pipe_out[0]);
    __cgi_fd_map[pipe_out[0]] = cgi;
}

void HttpServer::__OnCgiRequest(SessionCtx* ss, const WebRoute& route){
    std::string  resource_path;
    bool         resource_found;
    resource_found = __FindWebFile(ss->http_req, route, &resource_path);
    if (!resource_found)
        return ss->res_code = 404, __OnHttpError(ss);
    signal(SIGPIPE, SIG_IGN);

    __cgi_list.push_back(CgiEntry());
    __cgi_list.back().session = ss;
    __cgi_list.back().in_buf = ss->http_req.body;
    ss->cgi = &__cgi_list.back();

    if (ss->http_req.body.size() != 0)
        __CgiIOStart(&__cgi_list.back(), resource_path, route);
    else
        __CgiOutStart(&__cgi_list.back(), resource_path, route);
}

void HttpServer::__OnCgiFdRead(CgiEntry* cgi) {
    std::string str(512, 0);
    u32 rc = read(cgi->fd_out.GetFd(), const_cast<char*>(str.c_str()), 512);
    if (!rc) {
        SessionCtx* ss = cgi->session;
        cgi->cgi_rdr.EndRead();
        cgi->cgi_rdr.Process();
        if (cgi->cgi_rdr.HasError())
            __OnCgiFdError(cgi, cgi->fd_out.GetFd());
        Http::Response resp = cgi->cgi_rdr.GetMessage();
        ss->http_writer.Header().SetMap(resp.headers.GetMap());
        ss->http_writer.Write(resp.body);
        __OnHttpResponse(ss);
        __RmCgiFd(cgi->fd_out);
    }
    cgi->cgi_rdr.Read(str);
    cgi->cgi_rdr.Process();
    if (cgi->cgi_rdr.HasError())
        __OnCgiFdError(cgi, cgi->fd_out.GetFd());
}

void HttpServer::__OnCgiFdWrite(CgiEntry* cgi) {
    static const usize  WRITE_SESSION_BUFF_SZ = 512;

    if (cgi->in_buf.empty()) {
        info(__system_log, "Session[%d]Cgi: nothing to transmit, transmittion stopped",
                            cgi->fd_in.GetFd());
        __RmCgiFd(cgi->fd_in);
    } else {
        std::string  portion = cgi->in_buf.substr(0, WRITE_SESSION_BUFF_SZ);
        isize trasmitted_bytes = cgi->fd_in.Write(portion);

        info(__system_log, "Session[%d]Cgi: transmitted %zi bytes",
                            cgi->fd_in.GetFd(), trasmitted_bytes);
        debug(__system_log, "Session[%d]Cgi: transmitted content:\n"
                            "```\n%s\n```",
                            cgi->fd_in.GetFd(), portion.c_str());

        cgi->in_buf = cgi->in_buf.substr(usize(trasmitted_bytes));
    }
}

void HttpServer::__OnCgiFdError(CgiEntry* cgi, fd_t fd) {
    SessionCtx* ss = cgi->session;
    kill(cgi->pid, SIGKILL);
    __RmCgiFd(cgi->fd_out);
    __RmCgiFd(cgi->fd_in);
    error(ss->error_log, "Session[%d] Cgi[%d] : file IO error, sending response 500",
                         ss->conn_sock.GetFd(), fd);
    ss->res_code = 500;
    __OnHttpError(ss);
}

void HttpServer::CgiCheckPid() {
    for (CgiList::iterator  cgi_it = __cgi_list.begin(); cgi_it != __cgi_list.end(); ++cgi_it) {
        CgiEntry* cgi = &*cgi_it;
        int rc = waitpid(cgi->pid, NULL, WNOHANG);
        if (rc) {
            SessionCtx* ss = cgi->session;
            debug(__system_log, "Cgi deleted");
            if (rc < 0)
                error(ss->error_log, "Session[%d] Cgi: waitpid fail() : %s",
                                    ss->conn_sock.GetFd()), strerror(errno);
            __RmCgiFd(cgi->fd_out);
            __RmCgiFd(cgi->fd_in);
            cgi_it = __cgi_list.erase(cgi_it);
        }
    }
}

}  // namespace Webserver
