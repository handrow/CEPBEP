#include "netlib/poller.h"

namespace NetLib {

Poller::Result Poller::Poll(Error* err) {
    Result result;

    int rc = poll(__pfds.data(), __pfds.size(), __timeout);

    if (rc > 0) {
        pollfd pfd = *FindStruct();
        return result.ev = (Event)pfd.revents, result.fd = pfd.fd, result;
    }
    if (rc < 0)
        return *err = SystemError(errno), result;
    return result.ev = POLL_NON, result;
}

pollfd* Poller::FindStruct(fd_t fd) {
    if (!fd)
        for (size_t i = 0; i < __pfds.size(); ++i) {
            if (__pfds[i].revents)
                return &__pfds[i];
        }

    else
        for (size_t i = 0; i < __pfds.size(); ++i) {
            if (__pfds[i].fd == fd)
                return &__pfds[i];
        }
    return NULL;
}

void Poller::AddFd(fd_t fd, u16 event_mask) {
    pollfd pfd;
    pfd.events = event_mask;
    pfd.fd = fd;
    pfd.revents = 0;

    __pfds.push_back(pfd);
}

void Poller::SetEvMask(fd_t fd, u16 event_mask) {
    pollfd* pfd = FindStruct(fd);

    pfd->events = event_mask;
}

void Poller::AddEvMask(fd_t fd, u16 event_mask){
    pollfd* pfd = FindStruct(fd);

    pfd->events &= event_mask;
}

void Poller::RMEvMask(fd_t fd, u16 event_mask) {
    pollfd* pfd = FindStruct(fd);

    pfd->events |= ~event_mask;
}

u16 Poller::GetEvMask(fd_t fd) {
    pollfd* pfd = FindStruct(fd);
    return pfd->events;
}

void Poller::SetPollTimeout(u32 msec) {
    __timeout = msec;
}

}  // namespace NetLib