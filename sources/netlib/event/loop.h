#ifndef NETLIB_EVENT_LOOP_H_
#define NETLIB_EVENT_LOOP_H_

#include "netlib/event/event.h"
#include "netlib/event/queue.h"

namespace Event {

class Loop {
 private:
    EventQueue  __q;
    bool        __run;
    IEventPtr   __default_ev;

 public:
    static const u64 INFINITE_TIME = 0x0ull - 0x1ull;

    Loop() : __run(true), __default_ev(NULL) {
    }

    u64  GetTimeToNextEventMS() const;
    void PushEvent(IEventPtr ev);
    void SetDefaultEvent(IEventPtr ev);
    void Stop();
    void Run();
};

}  // namespace Event

#endif  // NETLIB_EVENT_LOOP_H_
