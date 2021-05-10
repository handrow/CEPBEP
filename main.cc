#include "logger/logger.h"

int main(int, char**) {
    ft::Logger SAYONARA("/dev/stdout");
    
    log(&SAYONARA, ft::Logger::INFO, "HELLO %s", "sssS");
    info(&SAYONARA, "MURAVEY");
}
