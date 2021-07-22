#include "common/types.h"
#include "common/error.h"

#include "netlib/webserver/webserver.h"
#include "cgi/cgi.h"

#include <cstdio>
#include <iostream>
#include <string>

Webserver::HttpServer::VirtualServer
Server1(Log::Logger* l) {
     Webserver::HttpServer::MethodSet ms_get;
    {
        ms_get.insert(Http::METHOD_GET);
    }

    Webserver::HttpServer::WebRoute route1 = {
        .pattern = "/gallery/*",
        .root_directory = "../www/s1/images",
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
        .root_directory = "../www/s1/pages",
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

    Webserver::HttpServer::ErrpageMap emap;
    emap[404] = "../www/errors/404.html";

    Webserver::HttpServer::VirtualServer::Hostnames hostnames;
    hostnames.push_back("*wp3d3p.site*");

    Webserver::HttpServer::WebRouteList routes;
    routes.push_back(route1);
    routes.push_back(route2);
    routes.push_back(route_other);

    return (Webserver::HttpServer::VirtualServer){
        .hostnames = hostnames,
        .mime_map = mimes,
        .routes = routes,
        .errpages = emap,
        .access_log = l,
        .error_log = l,
    };
}

Webserver::HttpServer::VirtualServer
Server2(Log::Logger* l) {
    Webserver::HttpServer::MethodSet ms_get;
    {
        ms_get.insert(Http::METHOD_GET);
    }

    Webserver::HttpServer::WebRoute route_other = {
        .pattern = "/*",
        .root_directory = "../www/s2",
        .index_page = "index.html",
        .reditect = { .enabled = false },
        .allowed_methods = ms_get,
        .listing_enabled = false
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

    Webserver::HttpServer::ErrpageMap emap;
    emap[404] = "../www/errors/404.html";

    Webserver::HttpServer::VirtualServer::Hostnames hostnames;
    hostnames.push_back("*www.wp3d3p.site*");

    Webserver::HttpServer::WebRouteList routes;
    routes.push_back(route_other);

    return (Webserver::HttpServer::VirtualServer){
        .hostnames = hostnames,
        .mime_map = mimes,
        .routes = routes,
        .errpages = emap,
        .access_log = l,
        .error_log = l,
    };
}

int main(int ac, Cgi::Envs av, Cgi::Envs) {
    Log::Logger             logger_stdout(Log::Logger::DEBUG);
    Log::Logger             logger_null(Log::Logger::DEBUG, "/dev/null");
    Webserver::HttpServer   server;

    try {
        server.SetTimeout(4000);
        server.SetSystemLogger(&logger_null);

        u16 port = (ac != 2) ? 9090
                             : Convert<u16>(av[1]);

        server.AddVritualServer(IO::SockInfo(std::string("0.0.0.0"), port), Server2(&logger_stdout));
        server.AddVritualServer(IO::SockInfo(std::string("0.0.0.0"), port), Server1(&logger_stdout));
        server.ServeForever();
    } catch (std::exception& e) {
        critical(&logger_stdout, "Fatal error: ``%s''", e.what());
    }
}
