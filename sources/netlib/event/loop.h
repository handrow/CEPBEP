#ifndef NETLIB_EVENT_LOOP_H_
#define NETLIB_EVENT_LOOP_H_

#include <list>

#include "netlib/event/event.h"
#include "netlib/event/queue.h"

namespace Event {

class Loop {
 private:
    typedef std::list<IEventPtr> HookList;

 private:
    EventQueue      __q;
    bool            __run;
    HookList        __hooks;

 public:
    static const u64 INFINITE_TIME = 0x0ull - 0x1ull;

    Loop() : __run(true) {
    }

    ~Loop();

    u64  GetTimeToNextEventMS() const;
    void PushEvent(IEventPtr ev);
    void AddDefaultEvent(IEventPtr ev);
    void Stop();
    void Run();
};

}  // namespace Event

#endif  // NETLIB_EVENT_LOOP_H_
