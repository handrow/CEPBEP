#include "webserver/webserver.h"

#include "common/file.h"

#include <csignal>

#include "dirent.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace Webserver {

void HttpServer::EvaluateCgiWorkers() {
    for (CgiPidMap::iterator it = CgiPids_.begin();
                             it != CgiPids_.end();) {
        CgiEntry& ce = it->second;

        int rc = waitpid(ce.Pid, NULL, WNOHANG);
        // not dead
        if (rc == 0) {
            ++it;
            continue;
        }
        
        if (rc < 0) {
            debug(SystemLog_, "Cgi[%d]: waitpid error, killing worker", ce.Pid);
            StopCgiWorker(&ce);
        }

        debug(SystemLog_, "Cgi[%d]: worker exited", ce.Pid);

        StopCgiRead(&ce);
        StopCgiWrite(&ce);
        CgiPidMap::iterator del = it++;
        CgiPids_.erase(del);
    }
}

Cgi::CStringVec
HttpServer::FillCgiMetavars(SessionCtx* ss, const std::string& filepath) {
    Cgi::Metavars   metavar;    
    metavar.AddHttpHeaders(ss->Request.Headers);
    metavar.GetMapRef()["REMOTE_ADDR"] = std::string(ss->ConnectionSock.GetSockInfo().Addr_BE);
    metavar.GetMapRef()["CONTENT_TYPE"] = ss->Request.Headers.Get("Content-Type");
    metavar.GetMapRef()["CONTENT_LENGTH"] = ss->Request.Headers.Get("Content-Length");
    metavar.GetMapRef()["QUERY_STRING"] = ss->Request.Uri.QueryStr;
    metavar.GetMapRef()["GATEWAY_INTERFACE"] = "CGI/1.1";
    metavar.GetMapRef()["SERVER_PORT"] = Convert<std::string>(UInt16(Listeners_[ss->ListenerFileDesc].GetSockInfo().Port_BE));
    metavar.GetMapRef()["SERVER_PROTOCOL"] = Http::ProtocolVersionToString(ss->Request.Version);
    metavar.GetMapRef()["SERVER_SOFTWARE"] = "webserv/1.0.0";
    metavar.GetMapRef()["REQUEST_METHOD"] = Http::MethodToString(ss->Request.Method);
    metavar.GetMapRef()["SCRIPT_NAME"] = filepath;
    metavar.GetMapRef()["SERVER_NAME"] = ss->Request.Headers.Get("Host");
    // For php
    metavar.GetMapRef()["SCRIPT_FILENAME"] = filepath;
    metavar.GetMapRef()["REDIRECT_STATUS"] = "200";
    return metavar.ToEnvVector();
}

bool  HttpServer::AvaibleCgiDriver(const std::string& filepath) {
    std::string fileext = GetFileExt(filepath);
    return CgiDrivers_.count(fileext) > 0;
}

std::string  HttpServer::GetCgiDriver(const std::string& filepath) {
    std::string fileext = GetFileExt(filepath);

    if (CgiDrivers_.count(fileext) > 0) {
        return CgiDrivers_[fileext];
    }

    return "";
}

void  HttpServer::CgiWorker(Fd ipip[], Fd opip[], SessionCtx* ss, const std::string& filepath) {
    if (dup2(ipip[0], STDIN_FILENO) < 0)
        exit(1);
    if (dup2(opip[1], STDOUT_FILENO) < 0)
        exit(1);

    close(ipip[0]);
    close(ipip[1]);
    close(opip[0]);
    close(opip[1]);

    if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0)
        exit(1);

    std::string cgi_driver = GetCgiDriver(filepath);

    Cgi::CStringVec arg_vec;
    arg_vec.push_back(C::string(cgi_driver));
    arg_vec.push_back(C::string(filepath));
    arg_vec.push_back(NULL);

    Cgi::CStringVec env_vec = FillCgiMetavars(ss, filepath);

    execve(cgi_driver.c_str(), Cgi::CastToEnvs(arg_vec), Cgi::CastToEnvs(env_vec));
    exit(1);
}

void  HttpServer::HandleCgiRequest(SessionCtx* ss, const std::string& filepath) {
    CgiEntry    cgi;
    cgi.InBuffer = ss->Request.Body;

    Fd ipip[2] = {-1, -1};
    Fd opip[2] = {-1, -1};

    if (pipe(ipip) < 0)
        return ss->ResponseCode = 500, OnHttpError(ss);

    if (pipe(opip) < 0)
        return close(ipip[0]), close(ipip[1]),
               ss->ResponseCode = 500, OnHttpError(ss);

    cgi.Pid = fork();
    if (cgi.Pid == 0) {
        // cgi worker
        return CgiWorker(ipip, opip, ss, filepath);
    }
    // cgi watcher

    if (cgi.Pid < 0) {
        close(ipip[0]);
        close(ipip[1]);
        close(opip[0]);
        close(opip[1]);
        return ss->ResponseCode = 500, OnHttpError(ss);
    }

    cgi.FileDescIn = IO::File(ipip[1]);
    close(ipip[0]);
    cgi.FileDescOut = IO::File(opip[0]);
    close(opip[1]);
    debug(SystemLog_, "Cgi[%d]: forked: (input_fd: %d), (output_fd: %d)",
                         cgi.Pid,
                         cgi.FileDescIn.GetFd(),
                         cgi.FileDescOut.GetFd());

    /// Add cgi to our inner structures
    CgiPids_[cgi.Pid] = cgi;
    ss->CgiPtr = &CgiPids_[cgi.Pid];

    CgiSessions_[cgi.FileDescIn.GetFd()] = ss;
    CgiSessions_[cgi.FileDescOut.GetFd()] = ss;

    if (!cgi.InBuffer.empty())
        Poller_.AddFd(cgi.FileDescIn.GetFd(), IO::Poller::POLL_WRITE);
    Poller_.AddFd(cgi.FileDescOut.GetFd(), IO::Poller::POLL_READ);
}

void  HttpServer::StopCgiWorker(CgiEntry* ce) {
    kill(ce->Pid, SIGKILL);
}

void  HttpServer::StopCgiRead(CgiEntry* ce) {
    Poller_.RmFd(ce->FileDescOut.GetFd());
    CgiSessions_.erase(ce->FileDescOut.GetFd());
}

void  HttpServer::StopCgiWrite(CgiEntry* ce) {
    Poller_.RmFd(ce->FileDescIn.GetFd());
    CgiSessions_.erase(ce->FileDescIn.GetFd());
}

void  HttpServer::OnCgiResponse(SessionCtx* ss) {
    CgiEntry& ce = *ss->CgiPtr;
    StopCgiRead(&ce);
    debug(ss->AccessLog, "Cgi[%d]: response ready", ce.Pid);
    ss->ResponseWriter.Write(ce.CgiResponse.Body);
    ss->ResponseWriter.Header().SetMap(ce.CgiResponse.Headers.GetMap());
    return ss->ResponseCode = ce.CgiResponse.Code, OnHttpResponse(ss);
}

void  HttpServer::OnCgiHup(SessionCtx* ss) {
    CgiEntry& ce = *ss->CgiPtr;

    StopCgiRead(&ce);

    ce.CgiReader.EndRead();
    ce.CgiReader.Process();
    
    if (ce.CgiReader.HasError()) {
        Error err = ce.CgiReader.GetError();
        debug(ss->AccessLog, "Cgi[%d]: parse error: %s", ce.Pid, err.Description.c_str());
        return OnCgiError(ss);
    }

    if (ce.CgiReader.HasMessage()) {
        ce.CgiResponse = ce.CgiReader.GetMessage();
        return OnCgiResponse(ss);
    }

    debug(ss->AccessLog, "Cgi[%d]: no parsed output", ce.Pid);
    return OnCgiError(ss);
}

void  HttpServer::OnCgiOutput(SessionCtx* ss) {
    static const USize READ_BUF_SZ = 10000;
    CgiEntry& ce = *ss->CgiPtr;
    std::string portion = ce.FileDescOut.Read(READ_BUF_SZ);

    if (portion.empty()) {
        ce.CgiReader.EndRead();
    } else {
        ce.CgiReader.Read(portion);
    }

    ce.CgiReader.Process();

    if (ce.CgiReader.HasError()) {
        Error err = ce.CgiReader.GetError();
        debug(ss->AccessLog, "Cgi[%d]: parse error: %s", ce.Pid, err.Description.c_str());
        return OnCgiError(ss);
    }

    if (ce.CgiReader.HasMessage()) {
        ce.CgiResponse = ce.CgiReader.GetMessage();
        return OnCgiResponse(ss);
    }
}

void  HttpServer::OnCgiInput(SessionCtx* ss) {
    CgiEntry& ce = *ss->CgiPtr;
    static const USize  WRITE_BUFF_SZ = 10000;

    if (ce.InBuffer.empty()) {
        StopCgiWrite(ss->CgiPtr);
    } else {
        std::string  portion = ce.InBuffer.substr(0, WRITE_BUFF_SZ);
        ISize trasmitted_bytes = ce.FileDescIn.Write(portion);

        ce.InBuffer = ce.InBuffer.substr(trasmitted_bytes);
    }
}

void  HttpServer::OnCgiError(SessionCtx* ss) {
    StopCgiWrite(ss->CgiPtr);
    StopCgiRead(ss->CgiPtr);
    StopCgiWorker(ss->CgiPtr);

    return ss->ResponseCode = 500, OnHttpError(ss);
}

class HttpServer::EvCgiRead : public Event::IEvent {
 private:
    HttpServer* Server_;
    SessionCtx* SessionCtx_;

 public:
    EvCgiRead(HttpServer *srv, SessionCtx* ss)
    : Server_(srv)
    , SessionCtx_(ss)
    {
    }

    void  Handle() {
        Server_->OnCgiOutput(SessionCtx_);
    }
};

class HttpServer::EvCgiWrite : public Event::IEvent {
 private:
    HttpServer* Server_;
    SessionCtx* SessionCtx_;

 public:
    EvCgiWrite(HttpServer *srv, SessionCtx* ss)
    : Server_(srv)
    , SessionCtx_(ss)
    {
    }

    void  Handle() {
        Server_->OnCgiInput(SessionCtx_);
    }
};

class HttpServer::EvCgiError : public Event::IEvent {
 private:
    HttpServer* Server_;
    SessionCtx* SessionCtx_;

 public:
    EvCgiError(HttpServer *srv, SessionCtx* ss)
    : Server_(srv)
    , SessionCtx_(ss)
    {
    }

    void  Handle() {
        Server_->OnCgiError(SessionCtx_);
    }
};

class HttpServer::EvCgiHup : public Event::IEvent {
 private:
    HttpServer* Server_;
    SessionCtx* SessionCtx_;

 public:
    EvCgiHup(HttpServer *srv, SessionCtx* ss)
    : Server_(srv)
    , SessionCtx_(ss)
    {
    }

    void  Handle() {
        Server_->OnCgiHup(SessionCtx_);
    }
};

class HttpServer::EvCgiHook : public Event::IEvent {
 private:
    HttpServer* Server_;

 public:
    EvCgiHook(HttpServer *srv) : Server_(srv) {
    }

    void  Handle() {
        Server_->EvaluateCgiWorkers();
    }
};

Event::IEventPtr    HttpServer::SpawnCgiEvent(IO::Poller::PollEvent ev, SessionCtx* ss) {
    if (ev == IO::Poller::POLL_READ) {
        return new EvCgiRead(this, ss);
    } else if (ev == IO::Poller::POLL_WRITE) {
        return new EvCgiWrite(this, ss);
    } else if (ev == IO::Poller::POLL_ERROR) {
        return new EvCgiError(this, ss);
    } else if (ev == IO::Poller::POLL_CLOSE) {
        return new EvCgiHup(this, ss);
    } else {
        throw std::runtime_error("Unsupported event type for CgiReadEvent: " + Convert<std::string>(ev));
    }
}

Event::IEventPtr    HttpServer::SpawnCgiHook() {
    return new EvCgiHook(this);
}

}  // namespace Webserver
