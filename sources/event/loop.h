#ifndef NETLIB_EVENT_LOOP_H_
#define NETLIB_EVENT_LOOP_H_

#include <list>

#include "event/event.h"
#include "event/queue.h"

namespace Event {

class Loop {
 private:
    typedef std::list<IEventPtr> HookList;

 private:
    EventQueue      EventQueue_;
    bool            Run_;
    HookList        Hooks_;

 public:
    static const UInt64 INFINITE_TIME = 0x0ull - 0x1ull;

    Loop() : Run_(true) {
    }

    ~Loop();

    UInt64  GetTimeToNextEventMS() const;
    void PushEvent(IEventPtr ev);
    void AddDefaultEvent(IEventPtr ev);
    void Stop();
    void Run();
};

}  // namespace Event

#endif  // NETLIB_EVENT_LOOP_H_
