#include "logger/logger.h"
#include "socket/socket.h"

int main(int, char**) {
    try {
    ft::Logger SAYONARA(ft::Logger::WARNING);
    
    log(&SAYONARA, ft::Logger::DEBUG, "HELLO %s", "sssS");
    log(&SAYONARA, ft::Logger::INFO, "HELLO %s", "sssS");
    info(&SAYONARA, "MURAVEY");
    error(&SAYONARA, "SUDO");
    warning(&SAYONARA, "MURAVEY %s %p", "iiiiiiii", &SAYONARA);
    critical(&SAYONARA, "III");

    ft::Socket s(5555, "127.0.0.1");
    s.Listen();
    }
    catch (std::runtime_error e) {
        std::cerr << e.what();
    }
}
