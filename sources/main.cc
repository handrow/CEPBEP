#include "common/types.h"
#include "common/error.h"

#include "netlib/webserver/webserver.h"
#include "cgi/cgi.h"

#include <cstdio>
#include <iostream>
#include <string>

int main(int ac, Cgi::Envs av, Cgi::Envs env) {
    IO::SockInfo            saddr;
    Log::Logger             logger(Log::Logger::INFO);
    Webserver::HttpServer   server;
    
    server.__env.AddEnvs(env);
    Webserver::HttpServer::MethodSet ms;
    ms.insert(Http::METHOD_GET);

    Webserver::HttpServer::WebRoute route1 = {
        .pattern = "/gallery/*.[jpg|jpeg|gif|png|svg]",
        .root_directory = "../www/images",
        .index_page = "index.html",
        .exectr = "",
        .allowed_methods = ms
    };

    Webserver::HttpServer::WebRoute route2 = {
        .pattern = "/*",
        .root_directory = "../www/cgi",
        .index_page = "p.py",
        .exectr = "/usr/bin/python",
        .allowed_methods = ms
    };

    Mime::MimeTypesMap mimes;
    mimes["jpg"] = "image/jpeg";
    mimes["jpeg"] = "image/jpeg";
    mimes["png"] = "image/png";
    mimes["gif"] = "image/gif";
    mimes["svg"] = "image/svg";
    mimes["html"] = "text/html";
    mimes["htm"] = "text/html";
    mimes["css"] = "text/css";
    mimes["js"] = "text/javascript";
    mimes["webp"] = "image/webp";

    try {
        server.AddWebRoute(route1);
        server.AddWebRoute(route2);

        server.SetMimes(mimes);

        server.SetLogger(&logger, &logger, &logger);

        u16 port = (ac != 2) ? 9090
                             : Convert<u16>(av[1]);

        server.AddListener(IO::SockInfo(std::string("0.0.0.0"), port));
        server.ServeForever();
    } catch (std::exception& e) {
        critical(&logger, "Fatal error: ``%s''", e.what());
    }
}
