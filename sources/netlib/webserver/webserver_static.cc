#include "netlib/webserver/webserver.h"

namespace Webserver {

/// Basics
void  HttpServer::__SendStaticFileResponse(IO::File file, SessionCtx* ss) {
    StaticFile stfile = {
        .closed = false,
        .file = file
    };
    ss->__link_stfile = stfile;
    __stat_files_read_map[file.GetFd()] = ss;

    __poller.AddFd(file.GetFd(), IO::Poller::POLL_READ |
                                 IO::Poller::POLL_ERROR);
}

void  HttpServer::__RemoveStaticFileCtx(SessionCtx* ss) {
    StaticFile* stfile = &ss->__link_stfile;
    if (!stfile->closed) {
        __poller.RmFd(stfile->file.GetFd());
        __stat_files_read_map.erase(stfile->file.GetFd());
        stfile->file.Close();
        stfile->closed = true;
    }
}

/// Handlers
void  HttpServer::__OnStaticFileRead(SessionCtx* ss) {
    const static usize READ_FILE_BUF_SZ = 512;
    StaticFile& stfile = ss->__link_stfile;

    std::string file_part = stfile.file.Read(READ_FILE_BUF_SZ);

    info(__system_log, "StaticFile[%d][%d]: read %zu bytes",
                         ss->conn_sock.GetFd(), stfile.file.GetFd(), file_part.size());

    debug(__system_log, "StaticFile[%d][%d]: read content:\n"
                         "```\n%s\n```",
                          ss->conn_sock.GetFd(), stfile.file.GetFd(), file_part.c_str());

    if (file_part.empty()) {
        __OnStaticFileReadEnd(ss);
    } else {
        ss->http_writer.Write(file_part);
    }
}

void  HttpServer::__OnStaticFileReadError(SessionCtx* ss) {
    StaticFile& stfile = ss->__link_stfile;
    error(__system_log, "StaticFile[%d][%d]: file IO error, sending response 500",
                          ss->conn_sock.GetFd(), stfile.file.GetFd());
    __RemoveStaticFileCtx(ss);
    ss->res_code = 500;
    __OnHttpError(ss);
}

void  HttpServer::__OnStaticFileReadEnd(SessionCtx* ss) {
    StaticFile& stfile = ss->__link_stfile;
    info(__system_log, "StaticFile[%d][%d]: nothing to read, file closed, preparing HTTP response",
                        ss->conn_sock.GetFd(), stfile.file.GetFd());
    __RemoveStaticFileCtx(ss);
    __OnHttpResponse(ss);
}


/// Events
class HttpServer::EvStaticFileRead : public Event::IEvent {
 private:
    HttpServer* __server;
    SessionCtx* __session;

 public:
    EvStaticFileRead(HttpServer *srv, SessionCtx* ss)
    : __server(srv)
    , __session(ss) {
    }

    void  Handle() {
        __server->__OnStaticFileRead(__session);
    }
};

class HttpServer::EvStaticFileReadError : public Event::IEvent {
 private:
    HttpServer* __server;
    SessionCtx* __session;

 public:
    EvStaticFileReadError(HttpServer *srv, SessionCtx* ss)
    : __server(srv)
    , __session(ss) {
    }

    void  Handle() {
        __server->__OnStaticFileReadError(__session);
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
