#ifndef SOCKET_NET_H_
# define SOCKET_NET_H_

# include "../logger/logger.h"

# include <fcntl.h>
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include <string>
# include <vector>
# include <queue>

namespace ft {

struct ServerDescriptor {
    uint16_t port;
    const char* host;
    const char* name;
};


struct ServerConf {
    const int BUFF_SIZE = 512;
    const size_t MAX_MSG_SIZE = 16384;
    const long MAX_WAIT_TIME = 100000000;
    const long MAX_READ_TIME = 100000000;

    const int MAX_CONNECTIONS = 100;
    const int NumPthreads = 0;
    std::vector<ServerDescriptor> __servers;
};


class Poller {
 public:
    enum Task {
        NEW_CONNECTION,
        CHECK_REC,
        CHECK_SEND,
        CHECK_EXCEP,
        KILL,
        UPLOAD
    };

    class Socket {
        Socket(uint16_t port, const char* host);
        int __sock;
    };

    struct Event {
        fd_set* __fd_set;
        int __fd;
        Task __task;
    };

    Poller(struct ServerConf conf);
    ~Poller();

    void EventSelector(int fd);
    Socket ListenSocket(int &sock);
    
    struct timeval __live_time;
    std::vector<Socket> __socks;
    std::queue<Event> __queue;
    std::vector<Worker> __w;
    std::vector<Connection> __connections;
};

class Worker
{
 public:
    Worker();
    ~Worker();

    std::queue<Poller::Event>* __ptr_queue;
    std::vector<Connection>* __ptr_connections;
    Poller::Task __task;

 private:
    int __busy;
};

class Connection {

};

}  // namespase ft

#endif  // SOCKET_NET_H_
