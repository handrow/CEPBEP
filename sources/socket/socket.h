#ifndef SOCKET_SOCKET_H_
# define SOCKET_SOCKET_H_

# include "../logger/logger.h"

# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include <string>
# include <vector>

namespace ft {

long	timer_now(void);
void	pth_sleep(long time_to);

class Socket {
 public:
    Socket(uint16_t port, const char* host, Logger* logger);
    ~Socket();
    void Listen(); // создание соединения, просмотр фд, вызов/создание воркера => воркеры можно сделать потоками/процессами
    void NewConnection();

 private:
    int __sock;
    fd_set __active_fd_set, __read_fd_set;
    Logger* __logger;
    struct sockaddr_in __clientname;
    std::vector<Worker> __w;
    static const int MAX_CONNECTIONS;
};

class Worker
{
 public:
    Worker();
    // неуверен где должны быть эти методы
    // работает согласно данным обрабатываему Connection подробности в миро
    void AssReadind(); // читает в буфер connection.__c_buf, а пока читаем слушаем или обрабатываем другие соединения, когда заканчиватся вызывется Connection.Hendler();
    void AssWriting();

    void Spining(); // бегает по __Connections, если все читают/пишут sleep/lock, если поток один Listen() -> опять порождает вопросы по асинхронности сколько он должен слушать? или как он узнает о том что чтнение/запись у коннекта окончены.
    void AddConnection(const Connection& new_c);

 private:
    static int BUSY;
    std::vector<Connection> __connections; // Connection* = new Connections[1000]; ?
};


class Connection {
 public:
    enum SttConnection {
        READING,
        READ_BODY,
        WORKING,
        RESPONSE,
        ERROR
    };

    Connection(const struct sockaddr_in& clientname, int fd);
    ~Connection();
    bool Handler(); // подробности на миро

    // я неуверен должны ли эти функции быть тут, если да то возникает ряд вопрос связанных с асинхронностью

    // void AssReadind();
    // void AssWriting();
    void CallHttpParser();
    void MakeBuf();
    void Work(); // будем предавать туда данные парсера и там уже делать дело согласно запросу

 private:
    static bool ENDING; // определяется после парсинга хттп, мб будет перекрываться с SttConnection 
    static int BODY_SIZE;
    //надо сделать глобальными константами
    static const int BUFF_SIZE;
    static const long MAX_WAIT_TIME;
    static const long MAX_READ_TIME;
    static const size_t MAX_MSG_SIZE;

    const struct sockaddr_in __clientname;
    int __fd;
    long __start_time;
    char* __c_buf; // хз мб ненадо
    std::string __s_buf;
    std::string __send;
    SttConnection __stt;
};

}  // namespase ft

#endif  // SOCKET_SOCKET_H_
