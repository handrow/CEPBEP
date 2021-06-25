#include "http/reader.h"
#include <iostream>

#include "netlib/io/file.h"

#include "netlib/event/loop.h"
#include "netlib/event/queue.h"
#include "netlib/event/event.h"

class StopEvent: public Event::IEvent {
 private:
    IO::File     r;
    Event::Loop* loop_ptr;

 public:
    StopEvent(const IO::File& f, Event::Loop* loop) : r(f), loop_ptr(loop) {
    }

    void Handle() {
        Error err;
        std::string result = r.Read(1000, &err);

        std::cout << result << std::endl;
        loop_ptr->Stop();
    }

};

class WriteEvent : public Event::IEvent {
 private:
    IO::File     w;
    Event::Loop* loop_ptr;
    int          i;

 public:
    WriteEvent(int count, const IO::File& f, Event::Loop* loop) : w(f), loop_ptr(loop), i(count) {
    }

    void Handle() {
        if (i <= 0) {
            loop_ptr->PushEvent(new StopEvent(w, loop_ptr));
        } else {
            Error err;
            w.Write("Hello\n", &err);
            loop_ptr->PushEvent(new WriteEvent(i - 1, w, loop_ptr));
        }
    }
};


int main() {
    Event::Loop lp;

    lp.PushEvent(new WriteEvent(10, IO::File(open("lol.txt", O_RDWR | O_CREAT | O_TRUNC)), &lp));
    lp.Run();
}
