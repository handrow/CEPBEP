#include "netlib/event/loop.h"

namespace Event {

    Loop::~Loop() {
        while (!__hooks.empty()) {
            delete __hooks.back();
            __hooks.pop_back();
        }
    }

    u64  Loop::GetTimeToNextEventMS() const {
        if (__q.empty())
            return INFINITE_TIME;
        return __q.top()->GetTimeToHandleMs();
    }

    void Loop::PushEvent(IEventPtr ev) {
        __q.push(ev);
    }

    void Loop::AddDefaultEvent(IEventPtr ev) {
        __hooks.push_front(ev);
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

            // Proccess all hooks
            for (HookList::iterator hook_it =  __hooks.begin();
                                    hook_it != __hooks.end();
                                    ++hook_it) {
                (*hook_it)->Handle();
            }
        }
    }
}  // namespace Event
