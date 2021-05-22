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
    DATA.__c_nmbr = 0;
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
        if (!DATA.__c_nmbr) {
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
    if (!event) {
        info(__logger, "select: Time to kill some connection");
        __w[0].FindKill();
    } else {
        int i;
        for (i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET (i, &DATA.__read_fd_set)) {
                if (i == __sock)
                    NewConnection();
                else {
                    FindConnection(i, Connection::CHECK_READ);
                }
                break ;
            }
            else if (FD_ISSET (i, &DATA.__write_fd_set)) {
                FindConnection(i, Connection::CHECK_SEND);
            }
        }
    }
}

void Socket::DataUpload() {
    DATA.__c_nmbr = 0;
    for (int i = 0; i < __w.size(); ++i)
        __w[i].DataTransfer();
}

void Socket::FindConnection(const int& i, const Connection::SttConnection& stt) {
    for (int i = 0; i < __w.size(); ++i) {
        for (int j = 0; j < __w[i].__connections.size(); ++j)
            if (__w[i].__connections[j].__fd == i) {
                __w[i].__connections[j].__stt = stt;
                return ;
            }
    }
}

void Socket::NewConnection() {
    info(__logger, "New Connection");
    if (DATA.__c_nmbr < MAX_CONNECTIONS) {
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
            __w.push_back(new_worker);
        }
        Connection a(__clientname, new_fd);
        __w[0].AddConnection(a);
    } else
        ; // ещё подумаю пока не критично
}

//Worker......................

Worker::Worker() {}

void Worker::AddConnection(const Connection& new_c) {
    __connections.push_back(new_c);
}

void Worker::Spining() {
    for (int i = 0; i < __connections.size(); ++i) {
        if (__connections[i].CHECK_READ)
            __connections[i].CheckRead();
    }
}

void Worker::FindKill() {
    for (int i = 0; i < __connections.size(); ++i) {
        if (timer_now() - __connections[i].__time_to_die <= 0) {
            __connections.erase(__connections.begin() + i);
        }
    }
}

void Worker::DataTransfer() {
    if (__connections.size()) {
        long min_time = tv_to_usec(DATA.__live_time);
        DATA.__c_nmbr += __connections.size();
        for (int i = 0; i < __connections.size(); ++i) {
            min_time = std::min(min_time, __connections[i].__time_to_die);
        }
        DATA.__live_time = usec_to_tv(min_time);
    }
}

//CONNECTION...................

Connection::Connection(const sockaddr_in& clientname, int fd) 
: __clientname(clientname)
, __fd(fd)
, __stt(NEW) {
    fcntl(__fd, F_SETFL, O_NONBLOCK);
    __s_buf.resize(BUFF_SIZE);
    __start_time = timer_now();
    __time_to_die = __start_time + MAX_WAIT_TIME;
}

void Connection::CheckRead() {
    if (__stt == NEW) {
        AssRead();
    }
    else if (__was_read)
        CallHttpParser();
}

Connection::~Connection() {
    close(__fd);
    FD_CLR(__fd, &(DATA.__active_fd_set));
}

void Connection::AssRead() {

}

}  // namespace ft