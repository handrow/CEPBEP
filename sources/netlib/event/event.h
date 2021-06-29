#ifndef NETLIB_EVENT_EVENT_H_
#define NETLIB_EVENT_EVENT_H_

#include "common/types.h"
#include "common/error.h"
#include "common/time.h"

namespace Event {

class IEvent {
 public:
    inline u64 GetTimeToHandleMs() const {
        u64 now = now_ms();

        if (__handle_time_ms <= now)
            return 0;
        return __handle_time_ms - now;
    }

    inline u64 GetTimePointMs() const {
        return __handle_time_ms;
    }

    explicit IEvent(u64 timepoint_ms = 0) : __handle_time_ms(timepoint_ms) {
    }

    virtual ~IEvent() {
    }

 public:
    virtual void Handle() = 0;

 private:
    const u64 __handle_time_ms;
};

typedef IEvent*   IEventPtr;

}  // namespace Event

#endif  // NETLIB_EVENT_EVENT_H_
