#include "netlib/webserver/webserver.h"

namespace Webserver {

/// Basics
void  HttpServer::__AddStaticFileCtx(IO::File file, SessionCtx* ss) {
    __static_files_map[file.GetFd()] = (StaticFileEntry){
        .file = file,
        .session = ss
    };

    __poller.AddFd(file.GetFd(), IO::Poller::POLL_READ |
                                 IO::Poller::POLL_ERROR);
}

void  HttpServer::__RemoveStaticFileCtx(IO::File file) {
    file.Close();
    __static_files_map.erase(file.GetFd());
}

/// Handlers
void  HttpServer::__OnStaticFileRead(IO::File file, SessionCtx* ss) {
    const static usize READ_FILE_BUF_SZ = 512;

    std::string file_part = file.Read(READ_FILE_BUF_SZ);
    info(ss->error_log, "Read from static file (fd: %d) %zu bytes",
                         file.GetFd(), file_part.size());

    if (file_part.empty()) {
        __RemoveStaticFileCtx(file);
        info(ss->error_log, "Stopped reading, file (fd: %d) has ended", file.GetFd());
        __OnStaticFileEnd(ss);
    } else {
        ss->http_writer.Write(file_part);
    }
}

void  HttpServer::__OnStaticFileError(IO::File file, SessionCtx* ss) {
    error(ss->error_log, "File IO error, sending 500: (fd: %d)", file.GetFd());
    __RemoveStaticFileCtx(file);
    ss->res_code = 500;
    __OnHttpError(ss);
}

void  HttpServer::__OnStaticFileEnd(SessionCtx* ss) {
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

class HttpServer::EvStaticFileError : public Event::IEvent {
 private:
    HttpServer* __server;
    SessionCtx* __session;
    IO::File    __file;

 public:
    EvStaticFileError(HttpServer *srv, SessionCtx* ss, IO::File file)
    : __server(srv)
    , __session(ss)
    , __file(file) {
    }

    void  Handle() {
        __server->__OnStaticFileError(__file, __session);
    }
};

Event::IEventPtr  HttpServer::__SpawnStaticFileEvent(IO::Poller::PollEvent pev, IO::File file, SessionCtx* ss) {
    if (pev == IO::Poller::POLL_READ) {
        return new EvStaticFileRead(this, ss, file);
    } else if (pev == IO::Poller::POLL_ERROR) {
        return new EvStaticFileError(this, ss, file);
    } else {
        throw std::runtime_error("Unsupported event type for StaticFileEvent: " + Convert<std::string>(pev));
    }
}

}  // namespace Webserver
