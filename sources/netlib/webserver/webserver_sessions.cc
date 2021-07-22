#include "netlib/webserver/webserver.h"

namespace Webserver {

/// Basics
HttpServer::SessionCtx*  HttpServer::__NewSessionCtx(const IO::Socket& sock, fd_t listen) {

    SessionCtx* ss = new SessionCtx;

    ss->access_log = __system_log;
    ss->error_log = __system_log;

    ss->conn_sock = sock;
    ss->__listener_fd = listen;

    ss->conn_close = false;
    ss->res_code = 200;

    ss->__link_stfile.closed = true;

    ss->__link_cgi = NULL;

    ss->UpdateTimeout(__session_timeout);

    return ss;
}

void  HttpServer::__StartSessionCtx(SessionCtx* ss) {
    __sessions_map[ss->conn_sock.GetFd()] = ss;
    __poller.AddFd(ss->conn_sock.GetFd(), IO::Poller::POLL_READ |
                                          IO::Poller::POLL_CLOSE |
                                          IO::Poller::POLL_ERROR);
}

void  HttpServer::__DeleteSessionCtx(SessionCtx* ss) {
    __RemoveStaticFileCtx(ss);

    __poller.RmFd(ss->conn_sock.GetFd());
    ss->conn_sock.Close();
    __sessions_map.erase(ss->conn_sock.GetFd());
    delete ss;
}

/// Handlers
void  HttpServer::__OnSessionRead(SessionCtx* ss) {
    static const usize  READ_SESSION_BUFF_SZ = 10000;
    const std::string  portion = ss->conn_sock.Read(READ_SESSION_BUFF_SZ);

    ss->UpdateTimeout(__session_timeout);

    if (portion.empty()) {
        return __OnSessionHup(ss);
    }

    info(__system_log, "Session[%d]: read %zu bytes",
                        ss->conn_sock.GetFd(), portion.size());
    debug(__system_log, "Session[%d]: read content:\n```\n%s\n```",
                         ss->conn_sock.GetFd(), portion.c_str());

    ss->req_rdr.Read(portion);
    ss->req_rdr.Process();

    if (ss->req_rdr.HasMessage()) {
        info(__system_log, "Session[%d]: HTTP request parsed", ss->conn_sock.GetFd());
        ss->server = NULL;
        ss->access_log = NULL;
        ss->error_log = NULL;
        ss->__link_cgi = NULL;
        __OnHttpRequest(ss);
    } else if (ss->req_rdr.HasError()) {
        info(__system_log, "Session[%d]: HTTP request is bad (%s), sending error",
                            ss->conn_sock.GetFd(),
                            ss->req_rdr.GetError().message.c_str());
        ss->res_code = 400;
        __OnHttpError(ss);
    }
}

void  HttpServer::__OnSessionWrite(SessionCtx* ss) {
    static const usize  WRITE_SESSION_BUFF_SZ = 10000;

    ss->UpdateTimeout(__session_timeout);

    if (ss->res_buff.empty()) {
        info(__system_log, "Session[%d]: nothing to transmit, transmittion stopped",
                            ss->conn_sock.GetFd());

        __poller.RmEvMask(ss->conn_sock.GetFd(), IO::Poller::POLL_WRITE);
        
        if (ss->conn_close == true) {
            info(__system_log, "Session[%d]: connection close is set, closing connetion",
                               ss->conn_sock.GetFd());
            __OnSessionHup(ss);
        }

    } else {
        std::string  portion = ss->res_buff.substr(0, WRITE_SESSION_BUFF_SZ);
        isize trasmitted_bytes = ss->conn_sock.Write(portion);

        info(__system_log, "Session[%d]: transmitted %zi bytes",
                            ss->conn_sock.GetFd(), trasmitted_bytes);
        debug(__system_log, "Session[%d]: transmitted content:\n"
                            "```\n%s\n```",
                            ss->conn_sock.GetFd(), portion.c_str());

        ss->res_buff = ss->res_buff.substr(usize(trasmitted_bytes));
    }
}

void  HttpServer::__OnSessionHup(SessionCtx* ss) {
    debug(__system_log, "Session[%d]: closing", ss->conn_sock.GetFd());
    __DeleteSessionCtx(ss);
}

void  HttpServer::__OnSessionError(SessionCtx* ss) {
    error(__system_log, "Error occured on session (fd: %d), closing it", ss->conn_sock.GetFd());
    __OnSessionHup(ss);
}

/// Events
class HttpServer::EvSessionRead : public Event::IEvent {
 private:
    SessionCtx* __session;
    HttpServer* __server;
 public:
    EvSessionRead(HttpServer* srv, SessionCtx* ss)
    : __session(ss)
    , __server(srv) {
    }

    void Handle() {
        __server->__OnSessionRead(__session);
    }
};

class HttpServer::EvSessionWrite : public Event::IEvent {
 private:
    SessionCtx* __session;
    HttpServer* __server;
 public:
    EvSessionWrite(HttpServer* srv, SessionCtx* ss)
    : __session(ss)
    , __server(srv) {
    }

    void Handle() {
        __server->__OnSessionWrite(__session);
    }
};

class HttpServer::EvSessionError : public Event::IEvent {
 private:
    SessionCtx* __session;
    HttpServer* __server;
 public:
    EvSessionError(HttpServer* srv, SessionCtx* ss)
    : __session(ss)
    , __server(srv) {
    }

    void Handle() {
        __server->__OnSessionError(__session);
    }
};

class HttpServer::EvSessionHup : public Event::IEvent {
 private:
    SessionCtx* __session;
    HttpServer* __server;
 public:
    EvSessionHup(HttpServer* srv, SessionCtx* ss)
    : __session(ss)
    , __server(srv) {
    }

    void Handle() {
        __server->__OnSessionHup(__session);
    }
};

Event::IEventPtr  HttpServer::__SpawnSessionEvent(IO::Poller::PollEvent pev, SessionCtx* ss) {
    if (pev == IO::Poller::POLL_READ) {
        return new EvSessionRead(this, ss);
    } else if (pev == IO::Poller::POLL_WRITE) {
        return new EvSessionWrite(this, ss);
    } else if (pev == IO::Poller::POLL_ERROR) {
        return new EvSessionError(this, ss);
    } else if (pev == IO::Poller::POLL_CLOSE) {
        return new EvSessionHup(this, ss);
    } else {
        throw std::runtime_error("Unsupported poller event type for SessionEventSpawner: " + Convert<std::string>(pev));
    }
}

}  // namespace Webserver
