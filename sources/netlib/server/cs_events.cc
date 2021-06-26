#include "netlib/server/io_events.h"
#include "netlib/server/cs_events.h"

namespace Server {

void EV_CS_NewHttpReq::Handle() {

    Http::Response  res;

    res.code = 200;

    if (__req.method == Http::METHOD_POST) {
        res.body = "post req was here";
    } else if (__req.method == Http::METHOD_GET) {
        res.body = "get req was here";
    } else {
        res.body = "wtf";
    }

    __evloop->PushEvent(new EV_CS_NewHttpRes(res, __session, __notifier, __evloop));
}

void EV_CS_NewHttpRes::Handle() {
    __session->res_buff = __res.ToString();
    __notifier->AddPollerFlag(GetFd(__session), IO::Poller::POLL_WRITE);
}

}  // namespace
