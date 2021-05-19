#ifndef SOCKET_SOCKET_H_
# define SOCKET_SOCKET_H_

# include "../logger/logger.h"

# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include <string>

namespace ft {


class Socket {
 public:
    Socket(uint16_t port, const char* host, Logger* logger);
    ~Socket();
    void Listen() const; // создание соединения, просмотр фд, вызов/создание воркера => воркеры сожно сдеелать потоками/процессами

 private:
    int __sock;
    Logger* __logger;
    Worker* __w;
};

class Worker
{
 public:
    Worker(int fd);
    // неуверен где должны быть эти методы
    // работает согласно данным обрабатываему Connection подробности в миро
    void AssReadind(); // читает в буфер connection.__c_buf, а пока читаем слушаем или обрабатываем другие соединения, когда заканчиватся вызывется Connection.Hendler();
    void AssWriting();

    void Spining(); // бегает по __Connections, если все читают/пишут sleep/lock, если поток один Listen() -> опять порождает вопросы по асинхронности сколько он должен слушать? или как он узнает о том что чтнение/запись у коннекта окончены.

 private:
    Connection* __Connections; // Connection* = new Connections[1000]; ?
    static const int MAX_CONNECTIONS;
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

    Connection(const struct sockaddr& clientname, int fd);
    void Handler(); // подробности на миро

    // я неуверен должны ли эти функции быть тут, если да то возникает ряд вопрос связанных с асинхронностью

    // void AssReadind();
    // void AssWriting();
    void CallHttpParser();
    void MakeBuf();
    void Work(); // будем предавать туда данные парсера и там уже делать дело согласно запросу

 private:
    static bool ENDING; // определяется после парсинга хттп, мб будет перекрываться с SttConnection 
    static int BODY_SIZE;
    static const int MAX_WAIT_TIME;
    static const int MAX_READ_TIME;
    static const int BUFF_SIZE;

    static const struct sockaddr __clientname;
    int __fd;
    long __start_time;
    char* __c_buf; // хз мб ненадо
    std::string __s_buf;
    SttConnection __stt;
};

}  // namespase ft

#endif  // SOCKET_SOCKET_H_