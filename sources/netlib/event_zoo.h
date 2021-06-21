#ifndef NETLIB_EVENT_ZOO_H_
#define NETLIB_EVENT_ZOO_H_

#include "netlib/event.h"
#include "netlib/socket.h"

namespace Netlib {

/**
 *      Description:    Создание нового соединения.
 *
 *      Conditions:     Появление события POLLIN на слушающем сокете.
 * 
 *      Who raises:     IO_Watcher
 * 
 *      Logic:          Создание нового соединения c ссылкой(указателем) на структуру его виртуального сервера.
 *                      Добавление сокета соединения в очередь событий на чтение. Веремя обработки 0.
 *                      Обновление карты таймаута. Добавление ReadTimeout.
 * 
 */

struct NewSessionEvent : BaseEvent {
    void AcceptConnection(Error* err, const Socket* listen_sock);
    HttpSession AddServerPreferences(const ServerPreferences* sp);
    void AddTimeout(std::vector<pollfd>* pfds);
    void UpDateTimeout(map<int,HttpSession>* TimeoutMap);

    HttpSession __http_ses; //?
    Socket      __sock;
};

/**
 *      Description:    Поступление новых данных на сокет хттп сесии.
 * 
 *      Conditions:     Появление события POLLIN на сокете хттп сесии.
 * 
 *      Who raises:     IO_Watcher
 * 
 *      Logic:          Начало чтения, отправка на обработку, если буфер не пустой, обновление таймауте сессия.
 * 
 */

struct ReadEvent : BaseEven{
    void ReadSession();
    void CheckData();
    void AddTimeout(std::vector<pollfd>* pfds);
    void UpDateTimeout(map<int,HttpSession>* TimeoutMap);

    HttpSession* __http_ses;
};

/**
 *      Description:    Закончилась отправки данных.
 *
 *      Conditions:     Появление POLLOUT на сокете хттп сесии.
 * 
 *      Who raises:     IO_Watcher, Sender должен добавить флаг в pollfd.
 * 
 *      Logic:          Закрытие соединения, если сессия 1.0, Connetion: Close.  Иначе добавление эвента ReadTimeout.
 * 
 */

struct WriteEvent : BaseEvent {
    void CloseSesion();
    void AddTimeout(std::vector<pollfd>* pfds);
    void UpDateTimeout(map<int,HttpSession>* TimeoutMap);

    HttpSession* __http_ses;
};

/**
 *      Description:    Вышло время на ожидание данных.
 *
 *      Conditions:     Сработал таймаут на Poller.
 * 
 *      Who raises:     IO_Watcher, Events.
 * 
 *      Logic:          Проверка поступления данных, закрытие соединение, в случае 
 *                      если не появились новые данные c отправкой ошибки, оправка
 *                      на обработку если данные появились. Timeout 1s.
 * 
 */

struct ReadTimeout : BaseEvent {
    void CheckData();
    void CloseSesion();
    void AddTimeout(std::vector<pollfd>* pfds);
    void UpDateTimeout(map<int,HttpSession>* TimeoutMap);

    HttpSession* __http_ses;
};

/**
 *      Description:    Вышло время на отправку данных.
 *
 *      Conditions:     Сработал таймаут на Poller.
 * 
 *      Who raises:     IO_Watcher, Sender.
 * 
 *      Logic:          Закрытие соединения, если данные не отправляются. Timeout 1s.
 *                      Если отправляются продление таймаута. 
 * 
 *                      Может ставить один таймайт на отправку всех данных или сделать его зависимым от объема данных,
 *                      чтобы небыло слишком долгого ожидания при отправки большого количества инфы по плохому каналу?
 * 
 */

struct  WriteTimeout : BaseEvent {
    void CheckData();
    void CloseSesion();
    void AddTimeout(std::vector<pollfd>* pfds);
    void UpDateTimeout(map<int,HttpSession>* TimeoutMap);

    HttpSession* __http_ses;
};

/**
 *      Description:    Окончание передачи данных в cgi.
 *
 *      Conditions:     Появление события POLLIN на CGI фд.
 * 
 *      Who raises:     IO_Watcher, кто-то должен добавить pollfd.
 * 
 *      Logic:          После окончания передачи данных в CGI, начинаем считывать ответ. Обновляем таймаут
 * 
 */

struct CgiIn : BaseEvent {
    void ReadCgi();
    void AddTimeout(std::vector<pollfd>* pfds);
    void UpDateTimeout(map<int,HttpSession>* TimeoutMap);

    HttpSession* __http_ses;
};

/**
 *      Description:    Окончание чтение данных из CGI.
 *
 *      Conditions:     Появление события POLLOUT на CGI фд.
 * 
 *      Who raises:     IO_Watcher, CgiIn должен добавить pollfd.
 * 
 *      Logic:          После окончания передачи данных из CGI, начинаем отправку ответа. Обновляем таймаут.
 * 
 */

struct CgiOut : BaseEvent {
    void Send();
    void AddTimeout(std::vector<pollfd>* pfds);
    void UpDateTimeout(map<int,HttpSession>* TimeoutMap);

    HttpSession* __http_ses;
};

/**
 *      Description:    Окончание передачи данных в File.
 *
 *      Conditions:     Появление события POLLIN на File фд.
 * 
 *      Who raises:     IO_Watcher, кто-то должен добавить pollfd.
 * 
 *      Logic:          После окончания передачи данных в File, начинаем считывать ответ. Обновляем таймаут
 * 
 */

struct FileIn : BaseEvent {
    void ReadFile();
    void AddTimeout(std::vector<pollfd>* pfds);
    void UpDateTimeout(map<int,HttpSession>* TimeoutMap);

    HttpSession* __http_ses;
};

/**
 *      Description:    Окончание чтение данных из File.
 *
 *      Conditions:     Появление события POLLOUT на File фд.
 * 
 *      Who raises:     IO_Watcher, FileIn должен добавить pollfd.
 * 
 *      Logic:          После окончания передачи данных из File, начинаем отправку ответа. Обновляем таймаут.
 * 
 */

struct FileOut : BaseEvent {
    void Send();
    void AddTimeout(std::vector<pollfd>* pfds);
    void UpDateTimeout(map<int,HttpSession>* TimeoutMap);

    HttpSession* __http_ses;
};

}

#endif  // NETLIB_EVENT_ZOO_H_
