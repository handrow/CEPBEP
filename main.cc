#include "logger/logger.h"

int main(int, char**) {
    ft::Logger SAYONARA(NULL, ft::Logger::INFO);
    
    log(&SAYONARA, ft::Logger::INFO, "HELLO %s", "sssS");
    info(&SAYONARA, "MURAVEY");
    error(&SAYONARA, "SUDO");
    warning(&SAYONARA, "MURAVEY %s %p", "iiiiiiii", &SAYONARA);
    critical(&SAYONARA, "III");
}
