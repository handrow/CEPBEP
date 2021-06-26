#include "http/reader.h"
#include "netlib/event/loop.h"
#include "netlib/server/io_notifier.h"
#include <iostream>

int main() {
    Event::Loop         lp;
    Server::IoNotifier  ion;

    ion.LinkWithEvLoop(&lp);

    Server::ListenerStream  lstnr;

    Error err;
    lstnr.ios = IO::Socket::CreateListenSocket(IO::SockInfo(std::string("127.0.0.1"), 9090), &err);

    if (err.IsOk()) {
        ion.WatchFor(lstnr);
        ion.AddPollerFlag(Server::GetFd(lstnr), IO::Poller::POLL_READ);

        lp.Run();
    }
}
