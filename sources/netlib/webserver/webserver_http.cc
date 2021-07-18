#include "netlib/webserver/webserver.h"

#include "common/file.h"

namespace Webserver {

const HttpServer::WebRoute*
HttpServer::__FindWebRoute(const Http::Request& req, const WebRouteList& routes) {
    for (WebRouteList::const_iterator route_it  = routes.begin();
                                      route_it != routes.end();
                                      ++route_it) {

        const WebRoute& try_route = *route_it;
        if (Match(try_route.pattern, req.uri.path) == false)
            continue;
        if (try_route.allowed_methods.count(req.method) <= 0)
            continue;
        return &try_route;
    }

    return NULL;
}

namespace {
//  Examples
//  /such/path/*.jpg        , /such/path/rel/path.jpg           -> (rel/path.jpg)
//  /such/path/*a/*.php     , /such/path/ab/c/d/e/hello.php     -> (ab/c/d/e/hello.php)
//  /such/path/roo.jpg      , /such/path/roo.jpg                -> (roo.jpg)
//  /such/path*             , /such/path/to/file.txt            -> (path/to/file.txt)
//  /*                      , /hello/world.cc                   -> (hello/world.cc)
//  *                       , /hello/abc/hi                     -> (hello/abc/hi)
//  /gaylor[d|c]/*          , /gaylord/abc/bde                  -> (abc/bde)
std::string  GetRelativePathFromPattern(const std::string& pattern, const std::string& full_path) {
    usize first_star_pos = pattern.find_first_of("*");
    if (first_star_pos == std::string::npos)
        first_star_pos = pattern.length();

    usize last_path_sep_pos = pattern.find_last_of("/", first_star_pos);
    if (last_path_sep_pos == std::string::npos)
        last_path_sep_pos = 0;
    else
        last_path_sep_pos += 1;

    return full_path.substr(last_path_sep_pos);
}
}  // namespace

bool  HttpServer::__FindWebFile(const Http::Request& req,
                                const WebRoute& route,
                                std::string* res_path) {
    std::string relative_path = GetRelativePathFromPattern(route.pattern, req.uri.path);
    std::string root_path = (!route.root_directory.empty()) ? route.root_directory
                                                            : ".";

    if (Match("*/../*", req.uri.path) || Match("*/..", req.uri.path))
        return false;

    *res_path = root_path + "/" + relative_path;

    return IsExist(*res_path);
}

void  HttpServer::__OnStaticFileRequest(SessionCtx* ss, const WebRoute& route) {
    std::string  resource_path;
    bool         resource_found;
    resource_found = __FindWebFile(ss->http_req, route, &resource_path);

    if (!resource_found)
        return ss->res_code = 404, __OnHttpError(ss);

    if (IsDirectory(resource_path)) {

        if (Back(resource_path) != '/')
            resource_path += "/";
        std::string index_route = resource_path + route.index_page;

        if (!route.index_page.empty() && IsExist(index_route) && !IsDirectory(resource_path)) {
            resource_path = index_route;
        } else if (route.listing_enabled) {
            return __SendDirectoryListing(resource_path, ss);
        } else {
            return ss->res_code = 404, __OnHttpError(ss);
        }
    }

    Error err;
    IO::File  resource = IO::File::OpenFile(resource_path, O_RDONLY, &err);

    if (err.IsError())
        return ss->res_code = 500, __OnHttpError(ss);

    return ss->res_code = 200,
           ss->http_writer.Header().Set("Content-type", Mime::MapType(__mime_map, resource_path)),
           __SendStaticFileResponse(resource, ss);
}

void  HttpServer::__OnHttpRequest(SessionCtx* ss) {
    ss->http_req = ss->req_rdr.GetMessage();

    info(ss->access_log, "HTTP request:\n"
                         ">  session_fd: %d\n"
                         ">      method: %s\n"
                         ">     version: %s\n"
                         ">         uri: %s",
                            ss->conn_sock.GetFd(),
                            Http::MethodToString(ss->http_req.method).c_str(),
                            Http::ProtocolVersionToString(ss->http_req.version).c_str(),
                            ss->http_req.uri.ToString().c_str());

    const WebRoute*  route = __FindWebRoute(ss->http_req, __routes);
    if (route == NULL)
        return ss->res_code = 404, __OnHttpError(ss);
    return __OnStaticFileRequest(ss, *route);
}

void  HttpServer::__OnHttpError(SessionCtx* ss) {
    ss->http_writer.Reset();
    ss->http_writer.Write("Error occured: " + Convert<std::string>(ss->res_code) + ".\n");
    ss->http_writer.Write("Good luck with it!\n");
    __OnHttpResponse(ss);
}

void  HttpServer::__OnHttpResponse(SessionCtx* ss) {
    if (ss->http_req.version == Http::HTTP_1_1) {
        // Set default connection
        if (ss->conn_close == true) {
            ss->http_writer.Header().Set("Connection", "close");
        } else if (!ss->http_writer.Header().Has("Connection")) {
            ss->http_writer.Header().Set("Connection", "keep-alive");
        }
    } else if (ss->http_req.version == Http::HTTP_1_0) {
        ss->conn_close = true;
    }

    // Set default Content-Type
    if (!ss->http_writer.Header().Has("Content-Type")) {
        ss->http_writer.Header().Set("Content-Type", "text/plain");
    }

    // Set server name
    ss->http_writer.Header().Set("Server", "not-ngnix/1.16");

    // Append to response buffer and enable writing in poller
    ss->res_buff += ss->http_writer.SendToString(ss->res_code, ss->http_req.version);
    ss->http_writer.Reset();

    __poller.AddEvMask(ss->conn_sock.GetFd(), IO::Poller::POLL_WRITE);
}

}  // namespace Webserver
