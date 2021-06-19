#include "netlib/poller.h"

namespace Netlib {

Poller::Result Poller::Poll(Error* err) {
    Result result = {.ev = POLL_NONE, .fd = -1};

    int rc = poll(__pfds.data(), __pfds.size(), __timeout_ms);

    if (rc > 0) {
        usize i = __FindEventFd();
        result.ev = EventSet(__pfds[i].revents);
        result.fd = __pfds[i].fd;
        __pfds[i].revents = 0;
    } else if (rc < 0) {
        *err = SystemError(errno);
    }
    return result;
}

usize Poller::__FindPollFd(fd_t fd) const {
    for (size_t i = 0; i < __pfds.size(); ++i) {
        if (__pfds[i].fd == fd)
            return i;
    }
    return npos;
}

usize Poller::__FindEventFd() const {
    for (size_t i = 0; i < __pfds.size(); ++i) {
        if (__pfds[i].revents != 0x0)
            return i;
    }
    return npos;
}

void Poller::AddFd(fd_t fd, EventSet event_mask) {
    __pfds.push_back((pollfd){
        .events = event_mask,
        .fd = fd,
        .revents = 0});
}

void Poller::RmFd(fd_t fd) {
    __pfds.erase(__pfds.begin() + __FindPollFd(fd));
}

void Poller::SetEvMask(fd_t fd, EventSet event_mask) {
    __pfds[__FindPollFd(fd)].events = event_mask;
}

void Poller::AddEvMask(fd_t fd, EventSet event_mask) {
    __pfds[__FindPollFd(fd)].events |= event_mask;
}

void Poller::RmEvMask(fd_t fd, EventSet event_mask) {
    __pfds[__FindPollFd(fd)].events &= ~event_mask;
}

Poller::EventSet Poller::GetEvMask(fd_t fd) const {
    return __pfds[__FindPollFd(fd)].events;
}

void Poller::SetPollTimeout(u32 msec) {
    __timeout_ms = msec;
}

}  // namespace Netlib
