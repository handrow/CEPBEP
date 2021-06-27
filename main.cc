#include "http/reader.h"
#include "netlib/event/loop.h"
#include "netlib/server/io_notifier.h"
#include <iostream>

struct Parent {
    int a;
    int b;
    int c;

    Parent(int _a, int _b, int _c) : a(_a), b(_b), c(_c) {
    }
};

struct A: public Parent {
    float GG;

    A(float g, const Parent& p) : Parent(p), GG(g) {
    }
};

struct B: public Parent {
    float BB;

    B(float b, const Parent& p) : Parent(p), BB(b) {
    }
};

int main() {
    // Event::Loop         lp;
    // Server::IoNotifier  ion;

    // ion.LinkWithEvLoop(&lp);

    // Server::ListenerStream  lstnr;

    // Error err;
    // lstnr.ios = IO::Socket::CreateListenSocket(IO::SockInfo(std::string("127.0.0.1"), 9090), &err);

    // if (err.IsOk()) {
    //     ion.WatchFor(lstnr);
    //     ion.AddPollerFlag(Server::GetFd(lstnr), IO::Poller::POLL_READ);

    //     lp.Run();
    // }

    Parent p(1, 2, 3);

    A pooo(6.16, p);

    B booo(7.13, pooo);

    (void)p;
    (void)pooo;
    (void)booo;
}
