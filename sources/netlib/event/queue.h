#ifndef NETLIB_EVENT_QUEUE_H_
#define NETLIB_EVENT_QUEUE_H_

#include <queue>

#include "netlib/event/event.h"

namespace Event {

struct PriorityLess {
    inline bool
    operator()(const IEventPtr& ev1, const IEventPtr& ev2) const {
        return ev1->GetTimePointMs() > ev2->GetTimePointMs();
    }
};

typedef std::priority_queue<IEventPtr, std::vector<IEventPtr>, PriorityLess>   EventQueue;

}  // namespace Event

#endif  // NETLIB_EVENT_QUEUE_H_
