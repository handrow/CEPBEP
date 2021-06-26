#include "netlib/server/io_events.h"
#include "netlib/server/cs_events.h"

namespace Server {

void  EV_IO_SessionAccept::Handle() {
    printf("EV_IO_SessionAccept\n");

    Error err;
    IO::Socket con_sock = IO::Socket::AcceptNewConnection(&__lstn->ios, &err);
    if (err.IsError()) {
        /// TODO(mcottomn): What should we do here?
    }

    SessionStream session = { .ios = con_sock };
    __notifier->WatchFor(session);
    __notifier->AddPollerFlag(GetFd(session), IO::Poller::POLL_READ);
}

void  EV_IO_SessionClose::Handle() {
    printf("EV_IO_SessionClose\n");

    SessionStream ss_copy = *__session;
    const fd_t session_fd = GetFd(*__session);

    __notifier->StopWatchFor(session_fd);
    ss_copy.ios.Close();
}

void  EV_IO_SessionRead::Handle() {
    printf("EV_IO_SessionRead\n");

    const fd_t session_fd = GetFd(*__session);

    Error err;
    std::string byte_portion = __session->ios.Read(SOCKET_READ_BUFF_SZ, &err);

    if (byte_portion.empty()) {
        __evloop->PushEvent(new EV_IO_SessionClose(__session, __notifier, __evloop));
        return ;
    }

    __session->req_rdr.Read(byte_portion);
    __session->req_rdr.Process();

    if (__session->req_rdr.HasMessage()) {
        __evloop->PushEvent(new EV_CS_NewHttpReq(__session->req_rdr.GetMessage(), __session, __notifier, __evloop));
    } else if (__session->req_rdr.HasError()) {
        __evloop->PushEvent(new EV_CS_NewHttpRes((Http::Response){
            .version = Http::HTTP_1_0,
            .code = 400,
            .body = "Bad request, sorry :("
        }, __session, __notifier, __evloop));
    }

    // save it for multithreading compabilities
    __notifier->AddPollerFlag(session_fd, IO::Poller::POLL_READ);
}

void  EV_IO_SessionWrite::Handle() {
    printf("EV_IO_SessionWrite\n");

    const fd_t session_fd = GetFd(*__session);

    Error err;
    if (__session->res_buff.empty()) {
        __notifier->RmPollerFlag(session_fd, IO::Poller::POLL_WRITE);
    } else {
        std::string byte_portion = __session->res_buff.substr(0, SOCKET_WRITE_BUFF_SZ);
        isize wrtd = __session->ios.Write(byte_portion, &err);

        __session->res_buff = __session->res_buff.substr(wrtd);
        // save it for multithreading compabilities
        __notifier->AddPollerFlag(session_fd, IO::Poller::POLL_WRITE);
    }

}

void  EV_IO_SessionErr::Handle() {
    printf("EV_IO_SessionErr\n");
}

}  // namespace Server
