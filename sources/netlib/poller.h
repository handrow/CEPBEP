#ifndef NETLIB_POLLER_H_
#define NETLIB_POLLER_H_

#include <sys/poll.h>

#include <vector>

#include "common/types.h"
#include "common/error.h"


namespace Netlib {

class Poller {
 public:
    static const usize npos = 0ull - 1ull;

    typedef u8 EventSet;

    enum PollEvent {
        POLL_NONE       = 0x0,
        POLL_READ       = POLLIN,
        POLL_WRITE      = POLLOUT,
        POLL_ERROR      = POLLERR,
        POLL_CLOSE      = POLLHUP,
        POLL_NOT_OPEN   = POLLNVAL
    };

    struct Result {
        fd_t fd;
        EventSet ev;
    };

 public:
    Poller() : __timeout_ms(0) {}

    Result Poll(Error* err);

    void AddFd(fd_t fd, EventSet event_mask = POLL_NONE);
    void RmFd(fd_t fd);

    void SetEvMask(fd_t fd, EventSet event_mask);
    void AddEvMask(fd_t fd, EventSet event_mask);
    void RmEvMask(fd_t fd, EventSet event_mask);
    EventSet GetEvMask(fd_t fd) const;

    void SetPollTimeout(u32 msec);

 private:
    usize __FindPollFd(fd_t fd) const;
    usize __FindEventFd() const;

 private:
    std::vector<pollfd> __pfds;
    int                 __timeout_ms;
};

}  // namespace Netlib

#endif  // NETLIB_POLLER_H_
