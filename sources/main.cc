#include "common/types.h"
#include "common/error.h"

#include "netlib/webserver/webserver.h"
#include "cgi/cgi.h"

#include <cstdio>
#include <iostream>
#include <string>

int main(int, Cgi::Envs, Cgi::Envs) {
    IO::SockInfo            saddr;
    Log::Logger             logger(Log::Logger::DEBUG);
    Webserver::HttpServer   server;

    Webserver::HttpServer::MethodSet ms;
    ms.insert(Http::METHOD_GET);

    Webserver::HttpServer::WebRoute route1 = {
        .pattern = "/hello/1/*",
        .root_directory = "/Users/hgranule/Desktop/webserv/bin/r1/",
        .index_page = "",
        .allowed_methods = ms
    };

    Webserver::HttpServer::WebRoute route2 = {
        .pattern = "/hello/2/*",
        .root_directory = "/Users/hgranule/Desktop/webserv/bin/r2/",
        .index_page = "index.html",
        .allowed_methods = ms
    };

    Webserver::HttpServer::WebRoute route3 = {
        .pattern = "*",
        .root_directory = "/Users/hgranule/Desktop/webserv/bin/all/",
        .index_page = "abc.htm",
        .allowed_methods = ms
    };

    Mime::MimeTypesMap mimes;
    mimes["jpg"] = "image/jpeg";
    mimes["jpeg"] = "image/jpeg";
    mimes["html"] = "text/html";
    mimes["htm"] = "text/html";

    try {
        server.AddWebRoute(route1);
        server.AddWebRoute(route2);
        server.AddWebRoute(route3);

        server.SetMimes(mimes);

        server.SetLogger(&logger, &logger, &logger);
        server.AddListener(IO::SockInfo(std::string("0.0.0.0"), 9003));
        server.ServeForever();
    } catch (std::exception& e) {
        critical(&logger, "Fatal error: ``%s''", e.what());
    }
}
