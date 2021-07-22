#include "common/types.h"
#include "common/error.h"

#include "netlib/webserver/webserver.h"

#include "cgi/cgi.h"

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int ac, Cgi::Envs av, Cgi::Envs) {
    Webserver::HttpServer   server;
    Config::Category        config;
    try {
        if (ac != 2)
            throw std::runtime_error("Invalid command line arguments");
        Error err;
        config = Config::Category::ParseFromINI(av[1], &err);\
        if (err.IsError())
            throw std::runtime_error("Config file parsing failed: " + err.message);
        server.Config(config);
    } catch (std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        exit(1);
    }
}
