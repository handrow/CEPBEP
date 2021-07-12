#include "netlib/webserver/webserver.h"

#include "logger/logger.h"

namespace Webserver {

/// Basics
HttpServer::SessionCtx*  HttpServer::__NewSessionCtx(const IO::Socket& sock,
                                                     Log::Logger* accessl,
                                                     Log::Logger* errorl) {

    SessionCtx* ss = new SessionCtx;

    ss->access_log = accessl;
    ss->error_log = errorl;
    ss->conn_sock = sock;

    ss->conn_close = false;
    ss->res_code = 200;

    return ss;
}

void  HttpServer::__AddSessionCtx(SessionCtx* ss) {
    __sessions_map[ss->conn_sock.GetFd()] = ss;
    __poller.AddFd(ss->conn_sock.GetFd(), IO::Poller::POLL_READ |
                                          IO::Poller::POLL_CLOSE |
                                          IO::Poller::POLL_ERROR);
}

void  HttpServer::__RemoveSessionCtx(SessionCtx* ss) {
    __poller.RmFd(ss->conn_sock.GetFd());
    ss->conn_sock.Close();
    __sessions_map.erase(ss->conn_sock.GetFd());
    delete ss;
}


/// Http On Message
void  HttpServer::__OnHttpRequest(SessionCtx* ss) {
    ss->http_req = ss->req_rdr.GetMessage();

    info(ss->access_log, "Http Request INFO:\n"
                         ">  SESSION_FD: %d\n"
                         ">      METHOD: %s\n"
                         ">     VERSION: %s\n"
                         ">         URI: %s\n",
                           ss->conn_sock.GetFd(),
                           Http::MethodToString(ss->http_req.method).c_str(),
                           Http::ProtocolVersionToString(ss->http_req.version).c_str(),
                           ss->http_req.uri.ToString().c_str());
    
    ss->http_writer.Write("Hello guys, it is text/plain.\n");
    ss->http_writer.Write("But I think it's not bad at all!!!\n");
    ss->http_writer.Write("It's better then an error :)\n");
    __OnHttpResponse(ss);
}

void  HttpServer::__OnHttpError(SessionCtx* ss) {
    // TODO
    ss->http_writer.Reset();
    ss->http_writer.Write("Error occured: " + Convert<std::string>(ss->res_code) + ".\n");
    ss->http_writer.Write("Good luck with it!\n");
    __OnHttpResponse(ss);
}

void  HttpServer::__OnHttpResponse(SessionCtx* ss) {
    if (ss->http_req.version == Http::HTTP_1_1) {
        // Set default connection
        if (ss->conn_close == true) {
            ss->http_writer.Header().Set("Connection", "close");
        } else if (!ss->http_writer.Header().Has("Connection")) {
            ss->http_writer.Header().Set("Connection", "keep-alive");
        }
    } else if (ss->http_req.version == Http::HTTP_1_0) {
        ss->conn_close = true;
    }

    // Set default Content-Type
    if (!ss->http_writer.Header().Has("Content-Type")) {
        ss->http_writer.Header().Set("Content-Type", "text/plain");
    }

    // Set server name
    ss->http_writer.Header().Set("Server", "not-ngnix/1.16");

    // Append to response buffer and enable writing in poller
    ss->res_buff += ss->http_writer.SendToString(ss->res_code, ss->http_req.version);
    ss->http_writer.Reset();

    __poller.AddEvMask(ss->conn_sock.GetFd(), IO::Poller::POLL_WRITE);
}


/// Handlers
void  HttpServer::__OnSessionRead(SessionCtx* ss) {
    static const usize  READ_SESSION_BUFF_SZ = 512;
    const std::string  portion = ss->conn_sock.Read(READ_SESSION_BUFF_SZ);

    debug(__system_log, "Accepted new bytes portion (size: %zu) on session (fd: %d):\n```\n%s\n```",
                        portion.size(), ss->conn_sock.GetFd(), portion.c_str());

    ss->req_rdr.Read(portion);
    ss->req_rdr.Process();

    if (ss->req_rdr.HasMessage()) {
        debug(__system_log, "New HTTP request raised on session (fd: %d)", ss->conn_sock.GetFd());
        __OnHttpRequest(ss);
    } else if (ss->req_rdr.HasError()) {
        debug(__system_log, "Bad request raised on session (fd: %d)", ss->conn_sock.GetFd());
        // __OnError
    }
}

void  HttpServer::__OnSessionWrite(SessionCtx* ss) {
    static const usize  WRITE_SESSION_BUFF_SZ = 512;

    if (ss->res_buff.empty()) {
        debug(__system_log, "Transmittion is stopped, response buffer is empty on session (fd: %d)",
                            ss->conn_sock.GetFd());
        __poller.RmEvMask(ss->conn_sock.GetFd(), IO::Poller::POLL_WRITE);
        
        if (ss->conn_close == true)
            __OnSessionHup(ss);

    } else {
        std::string  portion = ss->res_buff.substr(0, WRITE_SESSION_BUFF_SZ);
        isize trasmitted_bytes = ss->conn_sock.Write(portion);

        debug(__system_log, "Transmitted %zi bytes on session (fd: %d):\n```\n%s\n```",
                            trasmitted_bytes, ss->conn_sock.GetFd(), portion.c_str());

        ss->res_buff = ss->res_buff.substr(usize(trasmitted_bytes));
    }
}

void  HttpServer::__OnSessionHup(SessionCtx* ss) {
    debug(__system_log, "Session (fd: %d) is closed;", ss->conn_sock.GetFd());
    __RemoveSessionCtx(ss);
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
        throw std::runtime_error("Unsupported poller event type for SessionEventSpawner");
    }
}

}  // namespace
