#include "netlib/event/loop.h"

namespace Event {

void Loop::__ClearEventQ() {
    while (!__q.empty()) {
        IEventPtr ev = __q.top();
        __q.pop();
        delete ev;
    }
}

void Loop::__ClearLoopHooks() {
    while (!__loop_hooks.empty()) {
        IEventPtr ev = __loop_hooks.back();
        __loop_hooks.pop_back();
        delete ev;
    }
}

Loop::~Loop() {
    __ClearEventQ();
    __ClearLoopHooks();
}

u64  Loop::GetTimeToNextEventMS() const {
    if (__q.empty())
        return INFINITE_TIME;
    return __q.top()->GetTimeToHandleMs();
}

void Loop::PushEvent(IEventPtr ev) {
    __q.push(ev);
}

void Loop::AddLoopHook(IEventPtr ev) {
    __loop_hooks.push_back(ev);
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

        for (LoopHooks::iterator it = __loop_hooks.begin();
                                    it != __loop_hooks.end();
                                    ++it) {
            (*it)->Handle();
        }
    }
}

}