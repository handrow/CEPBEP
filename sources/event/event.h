#ifndef NETLIB_EVENT_EVENT_H_
#define NETLIB_EVENT_EVENT_H_

#include "common/types.h"
#include "common/error.h"
#include "common/time.h"

namespace Event {

class IEvent {
 public:
    inline UInt64 GetTimeToHandleMs() const {
        UInt64 now = now_ms();

        if (HandleTimeMs_ <= now)
            return 0;
        return HandleTimeMs_ - now;
    }

    inline UInt64 GetTimePointMs() const {
        return HandleTimeMs_;
    }

    explicit IEvent(UInt64 timepoint_ms = 0) : HandleTimeMs_(timepoint_ms) {
    }

    virtual ~IEvent() {
    }

 public:
    virtual void Handle() = 0;

 private:
    const UInt64 HandleTimeMs_;
};

typedef IEvent*   IEventPtr;

}  // namespace Event

#endif  // NETLIB_EVENT_EVENT_H_
