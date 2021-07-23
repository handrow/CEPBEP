#include "common/types.h"
#include "common/error.h"

#include "netlib/webserver/webserver.h"

#include "cgi/cgi.h"

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int ac, Cgi::Envs av, Cgi::Envs ev) {
    Webserver::HttpServer   server;
    Config::Category        config;
    std::string             config_path;
    try {
        if (ac != 2)
            config_path = "webserv.cfg";
        else
            config_path = av[1];
        Error err;
        config = Config::Category::ParseFromINI(config_path, &err);
        if (err.IsError())
            throw std::runtime_error("Config file parsing failed: " + err.message);
        server.Config(config, ev);
    } catch (std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        exit(1);
    }
}
