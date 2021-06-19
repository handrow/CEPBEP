#ifndef NETLIB_POLLER_H_
#define NETLIB_POLLER_H_

#include "common/types.h"
#include "common/error.h"

#include <sys/poll.h>
#include <vector>

namespace NetLib {

class Poller {
 public:
	enum Event : u16 {
		POLL_NON 	= 0b000000,
		POLL_READ	= 0b000001,
		POLL_WRITE	= 0b000100,
		POLL_ERROR	= 0b001000,
		POLL_CLOSE	= 0b010000
	};

	struct Result {
		fd_t fd;
		Event ev;
	};

	Result Poll(Error* err);

	void AddFd(fd_t, u16 event_mask = POLL_NON);

	void SetEvMask(fd_t fd, u16 event_mask);
	void AddEvMask(fd_t fd, u16 event_mask);
	void RMEvMask(fd_t fd, u16 event_mask);
	u16 GetEvMask(fd_t fd);

	void SetPollTimeout(u32 msec);

 private:
	pollfd* FindStruct(fd_t fd = 0);

	std::vector<pollfd> __pfds;
	int					__timeout;
};

}  // namespace NetLib

#endif  // NETLIB_POLLER_H_