#include "netlib/event/loop.h"

namespace Event {

    u64  Loop::GetTimeToNextEventMS() const {
        if (__q.empty())
            return INFINITE_TIME;
        return __q.top()->GetTimeToHandleMs();
    }

    void Loop::PushEvent(IEventPtr ev) {
        __q.push(ev);
    }

    void Loop::SetDefaultEvent(IEventPtr ev) {
        __default_ev = ev;
    }

    void Loop::Stop() {
        __run = false;
    }

     void Loop::Run() {
        while (__run) {
            // Release all ready events
            while (GetTimeToNextEventMS() == 0) {
                __q.top()->Handle();
                delete __q.top();
                __q.pop();
            }

            if (__default_ev != NULL)
                __default_ev->Handle();
        }
    }
}  // namespace Event
