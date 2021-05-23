#ifndef SOCKET_SOCKET_H_
# define SOCKET_SOCKET_H_

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

long            timer_now(void);
void            pth_sleep(long time_to);
long            tv_to_usec(struct timeval tv);
struct timeval  usec_to_tv(long usec);

struct Data {
    fd_set __active_fd_set, __read_fd_set, __write_fd_set;
    struct timeval __live_time;
};

static const int BUFF_SIZE;
static const long MAX_READ_TIME;
static const long MAX_WAIT_TIME;
static const size_t MAX_MSG_SIZE;
Logger* __logger;

class Socket {
 public:
    Socket(uint16_t port, const char* host);
    ~Socket();
    void Listen(); // создание соединения, просмотр фд, вызов/создание воркера => воркеры можно сделать потоками/процессами
    void EventSelector(const int& event);
    void NewConnection();
    void DataUpload();
    void AddToQ(const int& fd, const Connection::SttConnection& stt);
    static const int NumPthreads;

 private:
    static const int MAX_CONNECTIONS;
    int __sock;
    
    struct sockaddr_in __clientname;
    std::vector<Worker> __w;
    std::queue<std::pair<int, Connection::SttConnection> > __queue;
    std::vector<Connection> __connections; // Connection* = new Connections[1000]; ?
};

class Worker
{
 public:
    Worker();

    void Handler(); // подробности на миро
    void AddConnection(const Connection& new_c);
    void Spining();
    void Kill();
    void StartRead();
    void Upload();
    void CheckRec();
    void CheckSend();
    
    int Find();
    std::queue<std::pair<int, Connection::SttConnection> >* __ptr_queue;
    std::vector<Connection>* __ptr_connections;
    std::pair<int, Connection::SttConnection> __task;

 private:
    int __busy;
};


class Connection {
 public:
    enum SttConnection {
        NEW,
        UPLOAD,
        READ,
        CHECK_READ,
        END_READ,
        READ_BODY,
        SEND,
        CHECK_SEND,
        END_READ,
        WORKING,
        KILL,
        ERROR
    };

    Connection(const struct sockaddr_in& clientname, int fd);
    ~Connection();
    void AssRead(); // читает в буфер connection.__c_buf, а пока читаем слушаем или обрабатываем другие соединения, когда заканчиватся вызывется Connection.Hendler();
    void AssWrite();
    void CallHttpParser();
    void MakeBuf();
    int CheckRead();
    int CheckWrite();
    void Work(); // будем предавать туда данные парсера и там уже делать дело согласно запросу
    long __start_time;
    long __time_to_die;
    int __fd;
    SttConnection __stt;
    const struct sockaddr_in __clientname;

 private:

    bool __ending;
    char* __c_buf;
    size_t __body_size;
    size_t __was_read;
    std::string __s_buf;
    std::string __send;
};

}  // namespase ft

#endif  // SOCKET_SOCKET_H_
