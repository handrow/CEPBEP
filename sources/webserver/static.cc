#include "webserver/webserver.h"

namespace Webserver {

/// Basics
void  HttpServer::__SendStaticFileResponse(IO::File file, SessionCtx* ss) {
    StaticFile stfile = {
        .IsClosed = false,
        .File = file
    };
    ss->StatfilePtr = stfile;
    StatfileSessions_[file.GetFd()] = ss;

    Poller_.AddFd(file.GetFd(), IO::Poller::POLL_READ |
                                 IO::Poller::POLL_ERROR);
}

void  HttpServer::__RemoveStaticFileCtx(SessionCtx* ss) {
    StaticFile* stfile = &ss->StatfilePtr;
    if (!stfile->IsClosed) {
        Poller_.RmFd(stfile->File.GetFd());
        StatfileSessions_.erase(stfile->File.GetFd());
        stfile->File.Close();
        stfile->IsClosed = true;
    }
}

/// Handlers
void  HttpServer::__OnStaticFileRead(SessionCtx* ss) {
    const static USize READ_FILE_BUF_SZ = 10000;
    StaticFile& stfile = ss->StatfilePtr;

    std::string file_part = stfile.File.Read(READ_FILE_BUF_SZ);

    info(SystemLog_, "StaticFile[%d][%d]: read %zu bytes",
                         ss->ConnectionSock.GetFd(), stfile.File.GetFd(), file_part.size());

    debug(SystemLog_, "StaticFile[%d][%d]: read content:\n"
                         "```\n%s\n```",
                          ss->ConnectionSock.GetFd(), stfile.File.GetFd(), file_part.c_str());

    if (file_part.empty()) {
        __OnStaticFileReadEnd(ss);
    } else {
        ss->ResponseWriter.Write(file_part);
    }
}

void  HttpServer::__OnStaticFileReadError(SessionCtx* ss) {
    StaticFile& stfile = ss->StatfilePtr;
    error(SystemLog_, "StaticFile[%d][%d]: file IO error, sending response 500",
                          ss->ConnectionSock.GetFd(), stfile.File.GetFd());
    __RemoveStaticFileCtx(ss);
    ss->ResponseCode = 500;
    __OnHttpError(ss);
}

void  HttpServer::__OnStaticFileReadEnd(SessionCtx* ss) {
    StaticFile& stfile = ss->StatfilePtr;
    info(SystemLog_, "StaticFile[%d][%d]: nothing to read, file closed, preparing HTTP response",
                        ss->ConnectionSock.GetFd(), stfile.File.GetFd());
    __RemoveStaticFileCtx(ss);
    __OnHttpResponse(ss);
}


/// Events
class HttpServer::EvStaticFileRead : public Event::IEvent {
 private:
    HttpServer* Server_;
    SessionCtx* SessionCtx_;

 public:
    EvStaticFileRead(HttpServer *srv, SessionCtx* ss)
    : Server_(srv)
    , SessionCtx_(ss) {
    }

    void  Handle() {
        Server_->__OnStaticFileRead(SessionCtx_);
    }
};

class HttpServer::EvStaticFileReadError : public Event::IEvent {
 private:
    HttpServer* Server_;
    SessionCtx* SessionCtx_;

 public:
    EvStaticFileReadError(HttpServer *srv, SessionCtx* ss)
    : Server_(srv)
    , SessionCtx_(ss) {
    }

    void  Handle() {
        Server_->__OnStaticFileReadError(SessionCtx_);
    }
};

Event::IEventPtr  HttpServer::__SpawnStaticFileReadEvent(IO::Poller::PollEvent pev, SessionCtx* ss) {
    if (pev == IO::Poller::POLL_READ) {
        return new EvStaticFileRead(this, ss);
    } else if (pev == IO::Poller::POLL_ERROR) {
        return new EvStaticFileReadError(this, ss);
    } else {
        throw std::runtime_error("Unsupported event type for StaticFileEvent: " + Convert<std::string>(pev));
    }
}

}  // namespace Webserver
