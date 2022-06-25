#include "webserver/webserver.h"

namespace Webserver {

/// Basics
HttpServer::SessionCtx*  HttpServer::__NewSessionCtx(const IO::Socket& sock, Fd listen) {

    SessionCtx* ss = new SessionCtx;

    ss->AccessLog = SystemLog_;
    ss->ErrorLOg = SystemLog_;

    ss->ConnectionSock = sock;
    ss->ListenerFileDesc = listen;

    ss->IsConnectionClosed = false;
    ss->ResponseCode = 200;

    ss->StatfilePtr.IsClosed = true;

    ss->CgiPtr = NULL;

    ss->UpdateTimeout(SessionTimeout_);

    return ss;
}

void  HttpServer::__StartSessionCtx(SessionCtx* ss) {
    WebSessions_[ss->ConnectionSock.GetFd()] = ss;
    Poller_.AddFd(ss->ConnectionSock.GetFd(), IO::Poller::POLL_READ |
                                          IO::Poller::POLL_CLOSE |
                                          IO::Poller::POLL_ERROR);
}

void  HttpServer::__DeleteSessionCtx(SessionCtx* ss) {
    __RemoveStaticFileCtx(ss);

    Poller_.RmFd(ss->ConnectionSock.GetFd());
    ss->ConnectionSock.Close();
    WebSessions_.erase(ss->ConnectionSock.GetFd());
    delete ss;
}

/// Handlers
void  HttpServer::__OnSessionRead(SessionCtx* ss) {
    static const USize  READ_SESSION_BUFF_SZ = 10000;
    const std::string  portion = ss->ConnectionSock.Read(READ_SESSION_BUFF_SZ);

    ss->UpdateTimeout(SessionTimeout_);

    if (portion.empty()) {
        return __OnSessionHup(ss);
    }

    info(SystemLog_, "Session[%d]: read %zu bytes",
                        ss->ConnectionSock.GetFd(), portion.size());
    debug(SystemLog_, "Session[%d]: read content:\n```\n%s\n```",
                         ss->ConnectionSock.GetFd(), portion.c_str());

    ss->RequestReader.Read(portion);
    ss->RequestReader.Process();

    ss->AccessLog = SystemLog_;
    ss->ErrorLOg = SystemLog_;
    ss->Server = NULL;
    ss->CgiPtr = NULL;

    if (ss->RequestReader.HasMessage()) {
        info(SystemLog_, "Session[%d]: HTTP request parsed", ss->ConnectionSock.GetFd());
        ss->Request = ss->RequestReader.GetMessage();
        ss->RequestReader.Reset();
        return __OnHttpRequest(ss);
    } else if (ss->RequestReader.HasError()) {
        info(SystemLog_, "Session[%d]: HTTP request is bad (%s), sending error",
                            ss->ConnectionSock.GetFd(),
                            ss->RequestReader.GetError().Description.c_str());
        ss->RequestReader.Reset();
        return ss->ResponseCode = 400, __OnHttpError(ss);
    }
}

void  HttpServer::__OnSessionWrite(SessionCtx* ss) {
    static const USize  WRITE_SESSION_BUFF_SZ = 10000;

    ss->UpdateTimeout(SessionTimeout_);

    if (ss->ResultBuffer.empty()) {
        info(SystemLog_, "Session[%d]: nothing to transmit, transmittion stopped",
                            ss->ConnectionSock.GetFd());

        Poller_.RmEvMask(ss->ConnectionSock.GetFd(), IO::Poller::POLL_WRITE);
        
        if (ss->IsConnectionClosed == true) {
            info(SystemLog_, "Session[%d]: connection close is set, closing connetion",
                               ss->ConnectionSock.GetFd());
            __OnSessionHup(ss);
        }

    } else {
        std::string  portion = ss->ResultBuffer.substr(0, WRITE_SESSION_BUFF_SZ);
        ISize trasmitted_bytes = ss->ConnectionSock.Write(portion);

        info(SystemLog_, "Session[%d]: transmitted %zi bytes",
                            ss->ConnectionSock.GetFd(), trasmitted_bytes);
        debug(SystemLog_, "Session[%d]: transmitted content:\n"
                            "```\n%s\n```",
                            ss->ConnectionSock.GetFd(), portion.c_str());

        ss->ResultBuffer = ss->ResultBuffer.substr(USize(trasmitted_bytes));
    }
}

void  HttpServer::__OnSessionHup(SessionCtx* ss) {
    debug(SystemLog_, "Session[%d]: closing", ss->ConnectionSock.GetFd());
    __DeleteSessionCtx(ss);
}

void  HttpServer::__OnSessionError(SessionCtx* ss) {
    error(SystemLog_, "Error occured on session (fd: %d), closing it", ss->ConnectionSock.GetFd());
    __OnSessionHup(ss);
}

/// Events
class HttpServer::EvSessionRead : public Event::IEvent {
 private:
    SessionCtx* SessionCtx_;
    HttpServer* Server_;
 public:
    EvSessionRead(HttpServer* srv, SessionCtx* ss)
    : SessionCtx_(ss)
    , Server_(srv) {
    }

    void Handle() {
        Server_->__OnSessionRead(SessionCtx_);
    }
};

class HttpServer::EvSessionWrite : public Event::IEvent {
 private:
    SessionCtx* SessionCtx_;
    HttpServer* Server_;
 public:
    EvSessionWrite(HttpServer* srv, SessionCtx* ss)
    : SessionCtx_(ss)
    , Server_(srv) {
    }

    void Handle() {
        Server_->__OnSessionWrite(SessionCtx_);
    }
};

class HttpServer::EvSessionError : public Event::IEvent {
 private:
    SessionCtx* SessionCtx_;
    HttpServer* Server_;
 public:
    EvSessionError(HttpServer* srv, SessionCtx* ss)
    : SessionCtx_(ss)
    , Server_(srv) {
    }

    void Handle() {
        Server_->__OnSessionError(SessionCtx_);
    }
};

class HttpServer::EvSessionHup : public Event::IEvent {
 private:
    SessionCtx* SessionCtx_;
    HttpServer* Server_;
 public:
    EvSessionHup(HttpServer* srv, SessionCtx* ss)
    : SessionCtx_(ss)
    , Server_(srv) {
    }

    void Handle() {
        Server_->__OnSessionHup(SessionCtx_);
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
