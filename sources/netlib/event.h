#ifndef NETLIB_EVENT_H_
#define NETLIB_EVENT_H_

#include <queue>

#include "common/types.h"
#include "common/error.h"
#include "common/time.h"

///////////////////////////////////////////////////////////////////////////////
/// BASE EVENT SYSTEM                                                       ///
///////////////////////////////////////////////////////////////////////////////
namespace Netlib {

class BaseEvent {
 public:
    struct PriorityLess {
        inline bool
        operator()(const BaseEvent* ev1, const BaseEvent* ev2) const {
            return ev1->__handle_time_ms > ev2->__handle_time_ms;
        }
    };

 public:
    virtual void Handle() = 0;

 public:
    inline u64 GetTimeToHandleMs() const {
        u64 now = now_ms();
        
        if (__handle_time_ms <= now)
            return 0;
        return __handle_time_ms - now;
    }

    BaseEvent(u64 timepoint_ms) : __handle_time_ms(timepoint_ms) {
    }

    virtual ~BaseEvent() {
    }

 private:
    const u64 __handle_time_ms;
};

typedef std::priority_queue<BaseEvent*, std::vector<BaseEvent*>, BaseEvent::PriorityLess>   EventPQ;

class EventLoop {
 private:
    EventPQ     __q;
    bool        __run;

 public:
    static const u64 INFINITE_TIME = 0x0ull - 0x1ull;

    EventLoop() : __run(true) {}

    u64  GetTimeToNextEventMS() const {
        if (__q.empty())
            return INFINITE_TIME;
        return __q.top()->GetTimeToHandleMs();
    }

    void PushEvent(BaseEvent* ev) {
        __q.push(ev);
    }

    void Stop() {
        __run = false;
    }

    void Run() {
        while (__run) {
            // Release all ready events
            while (GetTimeToNextEventMS() == 0) {
                __q.top()->Handle();
                delete __q.top();
                __q.pop();
            }

            // TODO(mcottonm): ADD IO_WATCHER HERE
            usleep(500);
        }
    }
};

}  // namespace Netlib

#endif  // NETLIB_EVENT_H_
