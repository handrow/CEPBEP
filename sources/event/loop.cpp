#include "event/loop.h"

namespace Event {

    Loop::~Loop() {
        while (!Hooks_.empty()) {
            delete Hooks_.back();
            Hooks_.pop_back();
        }
    }

    UInt64  Loop::GetTimeToNextEventMS() const {
        if (EventQueue_.empty())
            return INFINITE_TIME;
        return EventQueue_.top()->GetTimeToHandleMs();
    }

    void Loop::PushEvent(IEventPtr ev) {
        EventQueue_.push(ev);
    }

    void Loop::AddDefaultEvent(IEventPtr ev) {
        Hooks_.push_front(ev);
    }

    void Loop::Stop() {
        Run_ = false;
    }

    void Loop::Run() {
        while (Run_) {
            // Release all ready events
            while (GetTimeToNextEventMS() == 0) {
                EventQueue_.top()->Handle();
                delete EventQueue_.top();
                EventQueue_.pop();
            }

            // Proccess all hooks
            for (HookList::iterator hook_it =  Hooks_.begin();
                                    hook_it != Hooks_.end();
                                    ++hook_it) {
                (*hook_it)->Handle();
            }
        }
    }
}  // namespace Event
