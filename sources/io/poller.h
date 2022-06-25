#ifndef NETLIB_IO_POLLER_H_
#define NETLIB_IO_POLLER_H_

#include <sys/poll.h>

#include "common/types.h"
#include "common/fast_vector.h"

#include "io/errors.h"

namespace IO {

class Poller {
 public:
    static const USize npos = 0ull - 1ull;

    typedef UInt8 EventSet;

    enum PollEvent {
        POLL_NONE       = 0x0,
        POLL_READ       = POLLIN,
        POLL_WRITE      = POLLOUT,
        POLL_ERROR      = POLLERR,
        POLL_CLOSE      = POLLHUP,
        POLL_NOT_OPEN   = POLLNVAL,
        POLL_PRIO       = POLLPRI,
    };

    struct Result {
        Fd FileDesc;
        EventSet EvSet;
    };

 public:
    Poller() : TimeoutMs_(0), I_(0) {}

    Result Poll(Error* err);

    void AddFd(Fd fd, EventSet event_mask = POLL_NONE);
    void RmFd(Fd fd);

    void SetEvMask(Fd fd, EventSet event_mask);
    void AddEvMask(Fd fd, EventSet event_mask);
    void RmEvMask(Fd fd, EventSet event_mask);
    EventSet GetEvMask(Fd fd) const;

    void SetPollTimeout(UInt32 msec);

 private:
    USize FindPollFd(Fd fd) const;
    USize FindEventFd();

 private:
    mut_std::vector<pollfd>  FilePool_;
    int                      TimeoutMs_;
    int                      I_;
};

}  // namespace IO

#endif  // NETLIB_IO_POLLER_H_
