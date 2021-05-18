#include "socket.h"

namespace ft {

const int Socket::MAX_CONNECTIONS = 100;

Socket::Socket(uint16_t port, const char* host, Logger* logger) : __logger(logger) {
    struct sockaddr_in  socket_name;
    if ((__sock = socket (PF_INET, SOCK_STREAM, 0)) < 0) {
        error(__logger, "socket: %s", strerror(errno)); //  delete errno then done
        throw std::runtime_error("soсket");
    }
    socket_name.sin_family = AF_INET;
    socket_name.sin_port = htons(port);
    socket_name.sin_addr.s_addr = inet_addr(host);
    if (bind(__sock, reinterpret_cast<struct sockaddr*>(&socket_name), sizeof(socket_name)) < 0) {
        error(__logger, "bind: %s", strerror(errno));
        throw std::runtime_error("bind");
    }
}

void Socket::Listen() const {
    if (listen(__sock, MAX_CONNECTIONS) < 0) {
        error(__logger, "listen: %s", strerror(errno));
        throw std::runtime_error("listen");
    }
}

}  // namespace ft