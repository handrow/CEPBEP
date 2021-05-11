#include "logger/logger.h"
int main(int, char**) {
    ft::Logger SAYONARA(ft::Logger::WARNING);
    
    log(&SAYONARA, ft::Logger::DEBUG, "HELLO %s", "sssS");
    log(&SAYONARA, ft::Logger::INFO, "HELLO %s", "sssS");
    info(&SAYONARA, "MURAVEY");
    error(&SAYONARA, "SUDO");
    warning(&SAYONARA, "MURAVEY %s %p", "iiiiiiii", &SAYONARA);
    critical(&SAYONARA, "III");
}
