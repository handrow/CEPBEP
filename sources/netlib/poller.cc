#include "netlib/poller.h"

namespace NetLib {

Poller::Result Poller::Poll(Error* err) {
    Result result;

    int rc = poll(__pfds.data(), __pfds.size(), __timeout);

    if (rc > 0) {
        int i = __FindEventFd();
        result.ev = static_cast<Event>(__pfds[i].revents);
        result.fd = __pfds[i].fd;
        __pfds[i].revents = 0;
        return result;
    }
    if (rc < 0)
        return *err = SystemError(errno), result;
    return result.ev = POLL_NONE, result;
}

int Poller::__FindPollFd(fd_t fd) const {
    for (size_t i = 0; i < __pfds.size(); ++i) {
        if (__pfds[i].fd == fd)
            return i;
    }
    return -1;
}

int Poller::__FindEventFd() const {
    for (size_t i = 0; i < __pfds.size(); ++i) {
        if (__pfds[i].revents)
            return i;
    }
    return -1;
}

void Poller::AddFd(fd_t fd, u8 event_mask) {
    pollfd pfd;
    pfd.events = event_mask;
    pfd.fd = fd;
    pfd.revents = 0;

    __pfds.push_back(pfd);
}

void Poller::SetEvMask(fd_t fd, u8 event_mask) {
    __pfds[__FindPollFd(fd)].events = event_mask;
}

void Poller::AddEvMask(fd_t fd, u8 event_mask){

    __pfds[__FindPollFd(fd)].events |= event_mask;
}

void Poller::RmEvMask(fd_t fd, u8 event_mask) {
    __pfds[__FindPollFd(fd)].events &= ~event_mask;
}

u8 Poller::GetEvMask(fd_t fd) const {
    return __pfds[__FindPollFd(fd)].events;
}

void Poller::SetPollTimeout(u32 msec) {
    __timeout = msec;
}

}  // namespace NetLib