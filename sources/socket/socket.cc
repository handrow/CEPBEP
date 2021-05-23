#include "socket.h"

namespace ft {

Data DATA;
const int BUFF_SIZE = 512;
const size_t MAX_MSG_SIZE = 16384;
const long MAX_WAIT_TIME = 100000000;
const long MAX_READ_TIME = 100000000;

const int Socket::MAX_CONNECTIONS = 100;
const int Socket::NumPthreads = 0;


long    tv_to_usec(timeval tv) {
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

timeval usec_to_tv(long usec) {
    timeval ret_time;
    ret_time.tv_sec = usec / 1000;
    ret_time.tv_usec = usec - ret_time.tv_sec * 1000;
    return ret_time;
}

long	timer_now(void)
{
	timeval	val;

	gettimeofday(&val, NULL);
	return tv_to_usec(val);
}

void	pth_sleep(long time_to)
{
	while (true)
	{
		if (timer_now() >= time_to)
			return ;
		usleep(50);
	}
}

Socket::Socket(uint16_t port, const char* host) {
    sockaddr_in  socket_name;
    if ((__sock = socket (PF_INET, SOCK_STREAM, 0)) < 0) {
        error(__logger, "socket: %s", strerror(errno)); //  delete errno then done
        throw std::runtime_error("soсket");
    }
    socket_name.sin_family = AF_INET;
    socket_name.sin_port = htons(port);
    socket_name.sin_addr.s_addr = inet_addr(host);
    if (bind(__sock, reinterpret_cast<sockaddr*>(&socket_name), sizeof(socket_name)) < 0) {
        error(__logger, "bind: %s", strerror(errno));
        throw std::runtime_error("bind");
    }
    if (listen(__sock, MAX_CONNECTIONS) < 0) {
        error(__logger, "listen: %s", strerror(errno));
        throw std::runtime_error("listen");
    }
    FD_ZERO(&(DATA.__active_fd_set));
    FD_SET(__sock, &(DATA.__active_fd_set));
}

void Socket::Listen() {
    while (true) {
        timeval* time_ptr;
        if (!__connections.size()) {
            time_ptr = NULL;
            DATA.__live_time = usec_to_tv(MAX_WAIT_TIME);
        } else
            *time_ptr = DATA.__live_time;
        DATA.__read_fd_set = DATA.__active_fd_set;
        DATA.__write_fd_set = DATA.__active_fd_set;
        FD_CLR(__sock, &(DATA.__write_fd_set));
        int event;
        if (event = select (FD_SETSIZE, &(DATA.__read_fd_set), &(DATA.__write_fd_set), NULL, time_ptr) < 0) {
            error(__logger, "select: %s", strerror(errno));
            throw std::runtime_error("select");
        }
        info(__logger, "select: some fd-event");
        EventSelector(event);
        DataUpload();
        if (!NumPthreads)
            __w[0].Spining();
    }
}

void Socket::EventSelector(const int& event) {
    int i = 0;
    Connection::SttConnection stt;
    if (!event) {
        info(__logger, "select: Time to kill some connection");
        stt = Connection::KILL;
    } else {
        for (; i < FD_SETSIZE; ++i) {
            if (FD_ISSET (i, &DATA.__read_fd_set)) {
                if (i == __sock) {
                    NewConnection();
                    stt = Connection::NEW;
                } else {
                    stt = Connection::CHECK_READ;
                }
                break ;
            }
            else if (FD_ISSET (i, &DATA.__write_fd_set)) {
                stt = Connection::CHECK_SEND;
            }
        }
    }
    AddToQ(i, stt);
}

void Socket::DataUpload() {
    AddToQ(0, Connection::UPLOAD);
}

void Socket::AddToQ(const int& fd, const Connection::SttConnection& stt) {
    std::pair<int, Connection::SttConnection> p(fd, stt);
     __queue.push(p);
}

void Socket::NewConnection() {
    info(__logger, "New Connection");
    if (__connections.size() < MAX_CONNECTIONS) {
        int new_fd;
        socklen_t size;
        if ((new_fd = accept(__sock, reinterpret_cast<sockaddr *>(&__clientname), &size)) < 0) {  // ШО ДЕЛАТЬ ЕСЛИ ТУТ ЗАВИСИНИТ ИЛИ ВЫКИНЕТ ОШИБКУ???
            error(__logger, "accept: %s", strerror(errno));
            throw std::runtime_error("accept");
        }
        info(__logger, "Server: connect from host %s, port %hd.\n",
                        inet_ntoa(__clientname.sin_addr),
                        ntohs(__clientname.sin_port));
        FD_SET(new_fd, &(DATA.__active_fd_set));
        // draft
        if (__w.size() == 0) {
            Worker new_worker;
            new_worker.__ptr_connections = &__connections;
            new_worker.__ptr_queue = &__queue;
            __w.push_back(new_worker);
        }
        Connection a(__clientname, new_fd);
        __connections.push_back(a);
    } else
        ; // ещё подумаю пока не критично
}


//Worker......................

Worker::Worker() {}

void Worker::Spining() {
    while (true){
        //mutex
        if ((*__ptr_queue).size()) {
            __task = ((*__ptr_queue).front());
            (*__ptr_queue).pop();
            Handler();
        }
    }
}

void Worker::Handler() {
    switch(__task.second) {
        case Connection::KILL :
            Kill();
            break;
        case Connection::NEW :
            CheckRec();
            break;
        case Connection::CHECK_READ :
            CheckRec();
            break;
        case Connection::CHECK_SEND :
            CheckSend();
            break;
        case Connection::UPLOAD :
            Upload();
            break;
    }
}

void Worker::CheckRec() {
    int i = Find(); 
    if ((*__ptr_connections)[i].CheckRead() < 0) {
        error(__logger, "Server: connect from host %s, port %hd.\n",
                        inet_ntoa((*__ptr_connections)[i].__clientname.sin_addr),
                        ntohs((*__ptr_connections)[i].__clientname.sin_port));
        (*__ptr_connections).erase((*__ptr_connections).begin() + i);
    }
}

void Worker::Kill() {
    __ptr_connections->erase(__ptr_connections->begin() + Find());
    DATA.__live_time = usec_to_tv(MAX_WAIT_TIME);
}

int Worker::Find() {
    int i;
    for (i = 0; i < (*__ptr_connections).size(); ++i) {
        if (__task.first == i)
            break;
    }
    return i;
}

void Worker::Upload() {
    if ((*__ptr_connections).size()) {
        long min_time = tv_to_usec(DATA.__live_time);
        for (int i = 0; i < (*__ptr_connections).size(); ++i) {
            min_time = std::min(min_time, (*__ptr_connections)[i].__time_to_die);
        }
        DATA.__live_time = usec_to_tv(min_time);
    }
}

//CONNECTION...................

Connection::Connection(const sockaddr_in& clientname, int fd) 
: __clientname(clientname)
, __fd(fd)
, __stt(NEW)
, __body_size(BUFF_SIZE) {
    fcntl(__fd, F_SETFL, O_NONBLOCK);
    __s_buf.resize(BUFF_SIZE);
    __start_time = timer_now();
    __time_to_die = __start_time + MAX_WAIT_TIME;
}

int Connection::CheckRead() {
    __time_to_die = MAX_WAIT_TIME;
    if (__stt == NEW) {
        AssRead();
    }
    else if (__was_read)
        CallHttpParser();
    else if (__was_read < 0)
        return -1;
    return 0;
}


int Connection::CheckWrite() {
    __time_to_die = MAX_WAIT_TIME;
    if (__ending) {
        __time_to_die = 0;
    } else {
        AssRead();
    }
}

Connection::~Connection() {
    close(__fd);
    FD_CLR(__fd, &(DATA.__active_fd_set));
}

void Connection::AssRead() {
    __was_read = read(__fd, const_cast<char*>(__s_buf.c_str()), __body_size);
}

}  // namespace ft