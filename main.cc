#include "common/types.h"
#include "common/error.h"

#include "netlib/webserver/webserver.h"
#include "cgi/cgi.h"

#include <cstdio>
#include <iostream>
#include <string>

int main(int, Cgi::Envs, Cgi::Envs) {
    IO::SockInfo            saddr;
    Log::Logger              logger(Log::Logger::DEBUG);
    Webserver::HttpServer   server;

    try {
        server.SetLogger(&logger);
        server.AddListener(IO::SockInfo(std::string("0.0.0.0"), 9091));
        server.AddListener(IO::SockInfo(std::string("127.0.0.1"), 9090));
        server.ServeForever();
    } catch (std::exception& e) {
        critical(&logger, "Fatal error: ``%s''", e.what());
    }
}
