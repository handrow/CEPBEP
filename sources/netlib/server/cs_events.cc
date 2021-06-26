#include "netlib/server/io_events.h"
#include "netlib/server/cs_events.h"

namespace Server {

void EV_CS_NewHttpReq::Handle() {
    printf("EV_CS_NewHttpReq\n");

    Http::Response  res;

    res.version = Http::HTTP_1_1;
    res.code = 200;
    res.headers.__map["Content-type"] = "text/plain";

    if (__req.method == Http::METHOD_POST) {
        res.body = "post req was here\n";
    } else if (__req.method == Http::METHOD_GET) {
        res.body = "get req was here\n";
    } else {
        res.body = "wtf\n";
    }

    __evloop->PushEvent(new EV_CS_NewHttpRes(res, __session, __notifier, __evloop));
}

void EV_CS_NewHttpRes::Handle() {
    printf("EV_CS_NewHttpRes\n");
    __session->res_buff = __res.ToString();
    __notifier->AddPollerFlag(GetFd(*__session), IO::Poller::POLL_WRITE);
}

}  // namespace
