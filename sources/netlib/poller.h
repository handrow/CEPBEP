#ifndef NETLIB_POLLER_H_
#define NETLIB_POLLER_H_

#include "common/types.h"
#include "common/error.h"

#include <sys/poll.h>
#include <vector>

namespace NetLib {

class Poller {
 public:
    enum Event : u8 {
        POLL_NONE   	= 0b000000,
        POLL_READ	    = POLLIN,
        POLL_WRITE  	= POLLOUT,
        POLL_ERROR	    = POLLERR,
        POLL_CLOSE	    = POLLHUP,
        POLL_NOT_OPEN   = POLLNVAL
    };

    struct Result {
        fd_t fd;
        Event ev;
    };

    Result Poll(Error* err);

    void AddFd(fd_t, u8 event_mask = POLL_NONE);

    void SetEvMask(fd_t fd, u8 event_mask);
    void AddEvMask(fd_t fd, u8 event_mask);
    void RmEvMask(fd_t fd, u8 event_mask);
    u8 GetEvMask(fd_t fd) const;

    void SetPollTimeout(u32 msec);

 private:
    int __FindPollFd(fd_t fd) const;
    int __FindEventFd() const;

    std::vector<pollfd> __pfds;
    int                 __timeout;
};

}  // namespace NetLib

#endif  // NETLIB_POLLER_H_