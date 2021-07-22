#include "common/types.h"
#include "common/error.h"

#include "netlib/webserver/webserver.h"
#include "cgi/cgi.h"

#include <cstdio>
#include <iostream>
#include <string>

int main(int ac, Cgi::Envs av, Cgi::Envs) {
    Log::Logger             logger_stdout(Log::Logger::DEBUG);
    // Log::Logger             logger_null(Log::Logger::DEBUG, "/dev/null");
    IO::SockInfo            saddr;
    Webserver::HttpServer   server;

    Webserver::HttpServer::MethodSet ms_get;
    {
        ms_get.insert(Http::METHOD_GET);
    }

    Webserver::HttpServer::WebRoute route1 = {
        .pattern = "/gallery/*",
        .root_directory = "../www/images",
        .index_page = "",
        .reditect = { .enabled = false },
        .allowed_methods = ms_get,
        .listing_enabled = true
    };

    Webserver::HttpServer::WebRoute route2 = {
        .pattern = "/images/*",
        .reditect = {
            .enabled = true,
            .location = "/gallery/",
            .code = 301
        }
    };

    Webserver::HttpServer::WebRoute route_other = {
        .pattern = "/*",
        .root_directory = "../www/pages",
        .index_page = "index.html",
        .reditect = { .enabled = false },
        .allowed_methods = ms_get,
        .listing_enabled = true
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
        server.SetTimeout(4000);

        server.AddWebRoute(route1);
        server.AddWebRoute(route2);
        server.AddWebRoute(route_other);

        server.SetMimes(mimes);

        server.SetLogger(&logger_stdout, &logger_stdout, &logger_stdout);

        u16 port = (ac != 2) ? 9090
                             : Convert<u16>(av[1]);

        server.AddListener(IO::SockInfo(std::string("0.0.0.0"), port));
        server.ServeForever();
    } catch (std::exception& e) {
        critical(&logger_stdout, "Fatal error: ``%s''", e.what());
    }
}
