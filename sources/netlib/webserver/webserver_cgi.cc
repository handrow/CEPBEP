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
        CgiEntry* __session;
        fd_t        __file;

    public:
        EvCgiRead(HttpServer *srv, CgiEntry* ss, fd_t file)
        : __server(srv)
        , __session(ss)
        , __file(file) {
        }

        void  Handle() {
            __server->__OnCgiFdRead(__file, __session);
        }
};

class HttpServer::EvCgiWrite : public Event::IEvent {
    private:
        HttpServer* __server;
        CgiEntry* __session;
        fd_t        __file;

    public:
        EvCgiWrite(HttpServer *srv, CgiEntry* ss, fd_t file)
        : __server(srv)
        , __session(ss)
        , __file(file) {
        }

        void  Handle() {
            __server->__OnCgiFdWrite(__file, __session);
        }
};

class HttpServer::EvCgiFdError : public Event::IEvent {
    private:
        HttpServer* __server;
        CgiEntry*   __session;
        fd_t        __file;

    public:
        EvCgiFdError(HttpServer *srv, CgiEntry* ss, fd_t file)
        : __server(srv)
        , __session(ss)
        , __file(file) {
        }

        void  Handle() {
            __server->__OnCgiFdError(__file, __session);
        }
};

Event::IEventPtr    HttpServer::__SpawnCgiReadEvent(IO::Poller::PollEvent ev, CgiEntry* ss, fd_t fd) {
    if (ev == IO::Poller::POLL_READ || ev == IO::Poller::POLL_CLOSE) {
        return new EvCgiRead(this, ss, fd);
    } else if (ev == IO::Poller::POLL_ERROR) {
        return new EvCgiFdError(this, ss, fd);
    } else {
        throw std::runtime_error("Unsupported event type for CgiReadEvent: " + Convert<std::string>(ev));
    }
}

Event::IEventPtr    HttpServer::__SpawnCgiWriteEvent(IO::Poller::PollEvent ev, CgiEntry* ss, fd_t fd) {
    if (ev == IO::Poller::POLL_WRITE) {
        return new EvCgiWrite(this, ss, fd);
    } else if (ev == IO::Poller::POLL_ERROR) {
        return new EvCgiFdError(this, ss, fd);
    } else {
        throw std::runtime_error("Unsupported event type for CgiReadEvent: " + Convert<std::string>(ev));
    }
}

bool HasBody(HttpServer::SessionCtx* ss) {
    return ss->http_req.headers.Has("Content-Length") || ss->http_req.headers.Has("Transfer-Encoding");
}

bool HttpServer::CgiAcivation(CgiEntry& cgi, bool has_body) {
    if (has_body) {
        if (pipe(cgi.fd_in) < 0)
            return false;
    }
    if (pipe(cgi.fd_out) < 0)
        return false;
    return true;
}

void HttpServer::__OnCgiRequest(SessionCtx* ss, const WebRoute& route){
    std::string  resource_path;
    bool         resource_found;
    resource_found = __FindWebFile(ss->http_req, route, &resource_path);
    if (!resource_found)
        return ss->res_code = 404, __OnHttpError(ss);

    CgiEntry cgi;
    cgi.session = ss;
    cgi.in_buf = ss->http_req.body;
    bool has_body = HasBody(ss);

    if (!CgiAcivation(cgi, has_body))
        throw std::runtime_error("pipe(): ");

    cgi.pid = fork();
    if (!cgi.pid) {
        if (has_body) {
            dup2(cgi.fd_in[1], STDIN_FILENO);
            close(cgi.fd_in[0]);
            close(cgi.fd_in[1]);
        }
        dup2(cgi.fd_out[1], STDOUT_FILENO);
        close(cgi.fd_out[0]);
        close(cgi.fd_out[1]);
        Cgi::CStringVec arg;
        arg.push_back(C::string(route.exectr));
        arg.push_back(C::string(resource_path + route.index_page));
        arg.push_back(NULL);
        execve(route.exectr.c_str(), Cgi::CastToEnvs(arg), Cgi::CastToEnvs(__env.ToEnvVector()));
        exit(1);
    }
    else if (cgi.pid < 0)
        throw std::runtime_error("fork(): ");
    if (has_body) {
        close(cgi.fd_in[1]);
        __poller.AddFd(cgi.fd_in[0], IO::Poller::POLL_READ |
                                     IO::Poller::POLL_ERROR);
    }
    close(cgi.fd_out[1]);
    __poller.AddFd(cgi.fd_out[0], IO::Poller::POLL_WRITE |
                                  IO::Poller::POLL_ERROR);
    ss->res_code = 200;
    ss->http_writer.Header().Set("Content-type", "text/html");
    __cgi_in_map[cgi.fd_in[0]] = cgi;
    __cgi_out_map[cgi.fd_out[0]] = &(__cgi_in_map[cgi.fd_in[0]]);
}

void HttpServer::__OnCgiFdRead(fd_t fd, CgiEntry* cgi) {
    SessionCtx* ss = cgi->session;
    std::string str(512, 0);
    u32 rc = read(fd, const_cast<char*>(str.c_str()), 512);
    if (!rc) {
        ss->resp_reader.Process();
        ss->resp_reader.EndRead();
        ss->resp_reader.Process();
        Http::Response resp = ss->resp_reader.GetMessage();
        ss->http_writer.Header().SetMap(resp.headers.GetMap());
        ss->http_writer.Write(resp.body);
        __OnHttpResponse(ss);
        __poller.RmFd(fd);
        __cgi_out_map.erase(fd);
        close(fd);
    }
    ss->resp_reader.Read(str);
}

void HttpServer::__OnCgiFdWrite(fd_t fd, CgiEntry* cgi) {
    static const usize  WRITE_SESSION_BUFF_SZ = 512;
    SessionCtx* ss = cgi->session;

    if (cgi->in_buf.empty()) {
        info(__system_log, "Cgi[%d]: nothing to transmit, transmittion stopped",
                            fd);

        __poller.RmEvMask(fd, IO::Poller::POLL_WRITE);
        close(fd);
    } else {
        std::string  portion = cgi->in_buf.substr(0, WRITE_SESSION_BUFF_SZ);
        isize trasmitted_bytes = ss->conn_sock.Write(portion);

        info(__system_log, "Session[%d]: transmitted %zi bytes",
                            fd, trasmitted_bytes);
        debug(__system_log, "Session[%d]: transmitted content:\n"
                            "```\n%s\n```",
                            fd, portion.c_str());

        cgi->in_buf = cgi->in_buf.substr(usize(trasmitted_bytes));
    }
}

void HttpServer::__OnCgiFdError(fd_t fd, CgiEntry* cgi) {
    SessionCtx* ss = cgi->session;
    kill(cgi->pid, SIGKILL);
    __cgi_out_map.erase(cgi->fd_out[0]);
    __cgi_in_map.erase(cgi->fd_in[0]);
    error(ss->error_log, "StaticFile[%d][%d]: file IO error, sending response 500",
                          ss->conn_sock.GetFd(), fd);
    ss->res_code = 500;
    __OnHttpError(ss);
}

}  // namespace Webserver