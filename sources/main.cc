#include "common/types.h"
#include "common/error.h"

#include "netlib/webserver/webserver.h"
#include "cgi/cgi.h"

#include <cstdio>
#include <iostream>
#include <string>

int main(int ac, Cgi::Envs av, Cgi::Envs env) {
    IO::SockInfo            saddr;
    Log::Logger             logger(Log::Logger::DEBUG);
    Webserver::HttpServer   server;
    
    server.SetEnviromentVariables(env);
    Webserver::HttpServer::MethodSet ms;
    ms.insert(Http::METHOD_GET);
    ms.insert(Http::METHOD_POST);

    Webserver::HttpServer::WebRoute route1 = {
        .pattern = "/gallery/*.[jpg|jpeg|gif|png|svg]",
        .root_directory = "../www/images",
        .index_page = "index.html",
        .cgi_enabled = false,
        .allowed_methods = ms
    };

    Webserver::HttpServer::WebRoute route2 = {
        .pattern = "/*",
        .root_directory = "../www/pages",
        .index_page = "index.html",
        .cgi_enabled = false,
        .allowed_methods = ms
    };

    Webserver::HttpServer::WebRoute route3 = {
        .pattern = "/cgi/*",
        .root_directory = "../www/cgi",
        .index_page = "p.py",
        .cgi_enabled = true,
        .allowed_methods = ms
    };

    Webserver::HttpServer::WebRoute route4 = {
        .pattern = "/*",
        .root_directory = "../www/pages",
        .index_page = "index.html",
        .cgi_enabled = false,
        .allowed_methods = ms
    };

    server.SetCGIOptions("/usr/bin/python");
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
        server.AddWebRoute(route3);
        server.AddWebRoute(route4);

        server.SetMimes(mimes);

        server.SetLogger(&logger, &logger, &logger);

        u16 port = (ac != 2) ? 8080
                             : Convert<u16>(av[1]);

        server.AddListener(IO::SockInfo(std::string("0.0.0.0"), port));
        server.ServeForever();
    } catch (std::exception& e) {
        critical(&logger, "Fatal error: ``%s''", e.what());
    }
}
