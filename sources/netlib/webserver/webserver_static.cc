#include "netlib/webserver/webserver.h"

namespace Webserver {

/// Basics
void  HttpServer::__SendStaticFileResponse(IO::File file, SessionCtx* ss) {
    __stat_files_read_map[file.GetFd()] = (StaticFileEntry){
        .file = file,
        .session = ss
    };

    __poller.AddFd(file.GetFd(), IO::Poller::POLL_READ |
                                 IO::Poller::POLL_ERROR);
}

void  HttpServer::__RemoveStaticFileCtx(IO::File file) {
    __poller.RmFd(file.GetFd());
    __stat_files_read_map.erase(file.GetFd());
    file.Close();
}

/// Handlers
void  HttpServer::__OnStaticFileRead(IO::File file, SessionCtx* ss) {
    const static usize READ_FILE_BUF_SZ = 512;

    std::string file_part = file.Read(READ_FILE_BUF_SZ);

    info(__system_log, "StaticFile[%d][%d]: read %zu bytes",
                         ss->conn_sock.GetFd(), file.GetFd(), file_part.size());

    debug(__system_log, "StaticFile[%d][%d]: read content:\n"
                         "```\n%s\n```",
                          ss->conn_sock.GetFd(), file.GetFd(), file_part.c_str());

    if (file_part.empty()) {
        __OnStaticFileReadEnd(file, ss);
    } else {
        ss->http_writer.Write(file_part);
    }
}

void  HttpServer::__OnStaticFileReadError(IO::File file, SessionCtx* ss) {
    __RemoveStaticFileCtx(file);
    error(__system_log, "StaticFile[%d][%d]: file IO error, sending response 500",
                          ss->conn_sock.GetFd(), file.GetFd());
    ss->res_code = 500;
    __OnHttpError(ss);
}

void  HttpServer::__OnStaticFileReadEnd(IO::File file, SessionCtx* ss) {
    __RemoveStaticFileCtx(file);
    info(__system_log, "StaticFile[%d][%d]: nothing to read, file closed, preparing HTTP response",
                        ss->conn_sock.GetFd(), file.GetFd());
    __OnHttpResponse(ss);
}


/// Events
class HttpServer::EvStaticFileRead : public Event::IEvent {
 private:
    HttpServer* __server;
    SessionCtx* __session;
    IO::File    __file;

 public:
    EvStaticFileRead(HttpServer *srv, SessionCtx* ss, IO::File file)
    : __server(srv)
    , __session(ss)
    , __file(file) {
    }

    void  Handle() {
        __server->__OnStaticFileRead(__file, __session);
    }
};

class HttpServer::EvStaticFileReadError : public Event::IEvent {
 private:
    HttpServer* __server;
    SessionCtx* __session;
    IO::File    __file;

 public:
    EvStaticFileReadError(HttpServer *srv, SessionCtx* ss, IO::File file)
    : __server(srv)
    , __session(ss)
    , __file(file) {
    }

    void  Handle() {
        __server->__OnStaticFileReadError(__file, __session);
    }
};

Event::IEventPtr  HttpServer::__SpawnStaticFileReadEvent(IO::Poller::PollEvent pev, IO::File file, SessionCtx* ss) {
    if (pev == IO::Poller::POLL_READ) {
        return new EvStaticFileRead(this, ss, file);
    } else if (pev == IO::Poller::POLL_ERROR) {
        return new EvStaticFileReadError(this, ss, file);
    } else {
        throw std::runtime_error("Unsupported event type for StaticFileEvent: " + Convert<std::string>(pev));
    }
}

}  // namespace Webserver
