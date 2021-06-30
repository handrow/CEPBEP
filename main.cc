#include "common/types.h"
#include "common/error.h"
#include "http/writer.h"

#include "netlib/webserver/webserver.h"

#include <cstdio>
#include <iostream>
#include <string>

int main(void) {
    Webserver::HttpServer  server;
    IO::SockInfo  saddr;
    ft::Logger  logger(ft::Logger::DEBUG);

    try {
        server.SetLogger(&logger);
        server.AddListener(IO::SockInfo(std::string("192.168.24.34"), 9090));
        server.AddListener(IO::SockInfo(std::string("127.0.0.1"), 9090));
        server.ServeForever();
    } catch (std::exception& e) {
        critical(&logger, "Fatal error: ``%s''", e.what());
    }
}
