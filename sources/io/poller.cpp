#include "io/poller.h"

namespace IO {

Poller::Result Poller::Poll(Error* err) {
    Result result = {.FileDesc = -1, .EvSet = POLL_NONE};
    int rc = poll(FilePool_.data(), FilePool_.size(), TimeoutMs_);

    if (rc > 0) {
        USize i = FindEventFd();
        result.EvSet = EventSet(FilePool_[i].revents);
        result.FileDesc = FilePool_[i].fd;
        FilePool_[i].revents = 0;
    } else if (rc < 0) {
        *err = SystemError(errno);
    }
    return result;
}

USize Poller::FindPollFd(Fd fd) const {
    for (size_t i = 0; i < FilePool_.size(); ++i) {
        if (FilePool_[i].fd == fd)
            return i;
    }
    return npos;
}

USize Poller::FindEventFd() {
    for (size_t i = 0; i < FilePool_.size(); ++i) {
        USize rounded_index = (i + I_) % FilePool_.size();
        if (FilePool_[rounded_index].revents != 0x0)
            return I_ = rounded_index, I_;
    }
    return npos;
}

void Poller::AddFd(Fd fd, EventSet event_mask) {
    FilePool_.push_back((pollfd){
        .fd = fd,
        .events = event_mask,
        .revents = 0});
}

void Poller::RmFd(Fd fd) {
    USize off = FindPollFd(fd);
    if (off != npos)
        FilePool_.erase(FilePool_.begin() + off);
}

void Poller::SetEvMask(Fd fd, EventSet event_mask) {
    FilePool_[FindPollFd(fd)].events = event_mask;
}

void Poller::AddEvMask(Fd fd, EventSet event_mask) {
    FilePool_[FindPollFd(fd)].events |= event_mask;
}

void Poller::RmEvMask(Fd fd, EventSet event_mask) {
    FilePool_[FindPollFd(fd)].events &= ~event_mask;
}

Poller::EventSet Poller::GetEvMask(Fd fd) const {
    return FilePool_[FindPollFd(fd)].events;
}

void Poller::SetPollTimeout(UInt32 msec) {
    TimeoutMs_ = msec;
}

}  // namespace IO
