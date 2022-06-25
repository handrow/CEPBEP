#include "common/types.h"
#include "common/error.h"

#include "webserver/webserver.h"

#include "cgi/cgi.h"

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int ac, Cgi::Envs av, Cgi::Envs ev) {
    Webserver::HttpServer   server;
    Config::Category        config;
    std::string             configPath;
    try {
        if (ac != 2)
            configPath = "webserv.cfg";
        else
            configPath = av[1];
        Error err;
        config = Config::Category::ParseFromINI(configPath, &err);
        if (err.IsError())
            throw std::runtime_error("Config file parsing failed: " + err.Description);
        server.Config(config, ev);
    } catch (std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        exit(1);
    }
}
