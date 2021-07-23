#include "netlib/webserver/webserver.h"

#include "common/file.h"

namespace Webserver {

namespace {
//  Examples
//  /such/path/*.jpg        , /such/path/rel/path.jpg           -> (rel/path.jpg)
//  /such/path/*a/*.php     , /such/path/ab/c/d/e/hello.php     -> (ab/c/d/e/hello.php)
//  /such/path/roo.jpg      , /such/path/roo.jpg                -> (roo.jpg)
//  /such/path*             , /such/path/to/file.txt            -> (path/to/file.txt)
//  /*                      , /hello/world.cc                   -> (hello/world.cc)
//  *                       , /hello/abc/hi                     -> (hello/abc/hi)
//  /gaylor[d|c]/*          , /gaylord/abc/bde                  -> (abc/bde)
// std::string  GetRelativePathFromPattern(const std::string& pattern, const std::string& full_path) {
//     usize first_star_pos = pattern.find_first_of("*");
//     if (first_star_pos == std::string::npos)
//         first_star_pos = pattern.length();

//     usize last_path_sep_pos = pattern.find_last_of("/", first_star_pos);
//     if (last_path_sep_pos == std::string::npos)
//         last_path_sep_pos = 0;
//     else
//         last_path_sep_pos += 1;

//     return full_path.substr(last_path_sep_pos);
// }

usize        PathDiffer(const std::string& pattern, const std::string& path) {
    usize i;
    for (i = 0; i < pattern.length() && pattern[i] == path[i]; ++i) {
    }

    return i;
}

// /abc/adf, /abc/adf/abg/jhg -> abg/jhg
// /abc/adf/, /abc/adf/abg/jhg -> abg/jhg
// /abc/ad,   /abc/adf/abg/jhg -> adf/abg/jhg
std::string  GetRelativePathFromPattern(const std::string& pattern, const std::string& path) {
    usize diff = PathDiffer(pattern, path);

    if (path.length() == diff)
        return "";
    if (path[diff] == '/')
        return path.substr(diff + 1);
    
    diff = path.rfind('/', diff);
    if (diff == std::string::npos)
        return "";
    return path.substr(diff + 1);
}

bool  MatchPath(const std::string& pattern, const std::string& path) {
    usize diff = PathDiffer(pattern, path);

    return pattern.length() == diff;
}

}  // namespace

const HttpServer::WebRoute*
HttpServer::__FindWebRoute(const Http::Request& req, const WebRouteList& routes) {
    for (WebRouteList::const_iterator route_it  = routes.begin();
                                      route_it != routes.end();
                                      ++route_it) {
        const WebRoute& try_route = *route_it;
        if (MatchPath(try_route.pattern, req.uri.path) == false)
            continue;
        return &try_route;
    }

    return NULL;
}

void  HttpServer::__HandleBadMethod(SessionCtx* ss, const WebRoute& route) {
    debug(ss->access_log, "Session[%d]: request method (%s) isn't allowed",
                           ss->conn_sock.GetFd(),
                           Http::MethodToString(ss->http_req.method).c_str());

    ss->http_writer.Reset();
    for (MethodSet::iterator it = route.allowed_methods.begin();
                                it != route.allowed_methods.end();
                                ++it) {
        ss->http_writer.Header().Add("Allow", Http::MethodToString(*it));
    }
    return ss->res_code = 405, __OnHttpError(ss, false);
}

void  HttpServer::__HandleDirectoryResource(SessionCtx* ss,
                                            const WebRoute& route,
                                            const std::string& filepath) {

    debug(ss->access_log, "Session[%d]: resource (%s) found, it's a directory",
                          ss->conn_sock.GetFd(),
                          filepath.c_str());

    if (!route.index_page.empty()) {
        std::string indexpath = AppendPath(filepath, route.index_page);
        if (!IsDirectory(indexpath) && IsExist(indexpath)) {
            std::string redirect_link = AppendPath(ss->http_req.uri.path, route.index_page);
            debug(ss->access_log, "Session[%d]: Found index page (%s), redirecting to (%s)",
                                    ss->conn_sock.GetFd(),
                                    indexpath.c_str(),
                                    redirect_link.c_str());
            return __OnHttpRedirect(ss, redirect_link, 302);
        } else {
            return ss->res_code = 404, __OnHttpError(ss);
        }

    } else if (Back(filepath) != '/') {
        std::string full_dir_redirect = AppendPath(ss->http_req.uri.path, route.index_page);
        debug(ss->access_log, "Session[%d]: Found directory (%s), redirecting to (%s)",
                                ss->conn_sock.GetFd(),
                                filepath.c_str(),
                                full_dir_redirect.c_str());
        return __OnHttpRedirect(ss, full_dir_redirect, 302);

    } else if (route.listing_enabled) {
        debug(ss->access_log, "Session[%d]: Found directory (%s), sending directory listing",
                                ss->conn_sock.GetFd(),
                                filepath.c_str());
        return __SendDirectoryListing(filepath, ss);
    }

    debug(ss->access_log, "Session[%d]: Found directory (%s), unable to resolve: (index_page: %s, listing: %s)",
                        ss->conn_sock.GetFd(),
                        filepath.c_str(),
                        (route.index_page.empty() ? "NO" : "YES"),
                        (route.listing_enabled ? "YES" : "NO"));
    return ss->res_code = 404, __OnHttpError(ss);
}

void  HttpServer::__HandleStaticFile(SessionCtx* ss, const std::string& file_path) {
    Error err;
    IO::File  file = IO::File::OpenFile(file_path, O_RDONLY, &err);

    if (err.IsError()) {
        error(ss->error_log, "Session[%d]: static file \"%s\" couldn't be open: ``%s''",
                              ss->conn_sock.GetFd(),
                              file_path.c_str(),
                              err.message.c_str());
        return ss->res_code = 500, __OnHttpError(ss);
    }

    std::string mime_type = Mime::MapType(ss->server->mime_map, file_path);

    info(ss->access_log, "Session[%d]: sending static file (%s) with type \"%s\"",
                          ss->conn_sock.GetFd(),
                          file_path.c_str(),
                          mime_type.c_str());

    return ss->res_code = 200,
           ss->http_writer.Header().Set("Content-type", mime_type),
           __SendStaticFileResponse(file, ss);
}

void  HttpServer::__OnHttpRedirect(SessionCtx* ss, const std::string& location, int code) {
    ss->http_writer.Header().Set("Location", location);
    ss->res_code = code;
    return __OnHttpResponse(ss);
}

HttpServer::VirtualServer*
HttpServer::__GetVirtualServer(fd_t lfd, const std::string& hostname) {

    std::pair<VirtualServerMap::iterator, VirtualServerMap::iterator> vit_range = __vservers_map.equal_range(lfd);
    VirtualServerMap::iterator vit = vit_range.first;
    VirtualServerMap::iterator vit_e = vit_range.second;

    for (; vit != vit_e; ++vit) {
        VirtualServer* vs = &vit->second;
        for (VirtualServer::Hostnames::iterator it = vs->hostnames.begin();
                                                it != vs->hostnames.end();
                                                ++it) {
            if (Match(*it, hostname)) {
                return vs;
            }
        }
    }

    return NULL;
}

void  HttpServer::__OnHttpRequest(SessionCtx* ss) {
    ss->http_req = ss->req_rdr.GetMessage();

    /// Virtual server resolving
    VirtualServer* vs = __GetVirtualServer(ss->__listener_fd, ss->http_req.headers.Get("Host"));
    if (vs == NULL) {
        ss->server = NULL;
        ss->error_log = __system_log;
        ss->access_log = __system_log;
        info(__system_log, "Session[%d]: can't identify virtual server", ss->conn_sock.GetFd());
        return ss->res_code = 400, __OnHttpError(ss);
    }

    ss->server = vs;
    ss->access_log = vs->access_log;
    ss->error_log = vs->error_log;

    info(ss->access_log, "Session[%d]: new HTTP request:\n"
                         ">   http_method: %s\n"
                         ">  http_version: %s\n"
                         ">           uri: %s\n"
                         ">          host: %s",
                            ss->conn_sock.GetFd(),
                            Http::MethodToString(ss->http_req.method).c_str(),
                            Http::ProtocolVersionToString(ss->http_req.version).c_str(),
                            ss->http_req.uri.ToString().c_str(),
                            ss->http_req.headers.Get("Host").c_str());

    info(ss->access_log, "Session[%d]: virtual server identified", ss->conn_sock.GetFd());

    /// Resolving Web Route
    const WebRoute*  route = __FindWebRoute(ss->http_req, vs->routes);
    if (route == NULL) {
        debug(ss->access_log, "Session[%d]: no web route", ss->conn_sock.GetFd());
        return ss->res_code = 404, __OnHttpError(ss);
    }

    debug(ss->access_log, "Session[%d]: choosen web route with pattern \"%s\"",
                            ss->conn_sock.GetFd(),
                            route->pattern.c_str());

    /// Resolving redirections
    if (route->reditect.enabled) {
        debug(ss->access_log, "Session[%d]: redirection is enabled, redirecting to \"%s\"",
                                ss->conn_sock.GetFd(),
                                route->reditect.location.c_str());
        return __OnHttpRedirect(ss, route->reditect.location, route->reditect.code);
    }

    /// Check HTTP method is allowed
    if (route->allowed_methods.count(ss->http_req.method) <= 0) {
        return __HandleBadMethod(ss, *route);
    }

    if (__IsUpload(ss, *route))
        return __HandleUploadRequest(ss, *route);

    /// Getting pathes
    std::string  relpath = GetRelativePathFromPattern(route->pattern, ss->http_req.uri.path);
    std::string  filepath = AppendPath(route->root_directory, relpath);

    /// Check for existent
    if (!IsExist(filepath)) {
        debug(ss->access_log, "Session[%d]: resource (\"%s\") not found",
                              ss->conn_sock.GetFd(),
                              filepath.c_str());
        return ss->res_code = 404, __OnHttpError(ss);
    }

    if (ss->http_req.method == Http::METHOD_DELETE)
        return __HandleDeleteFile(ss, filepath);

    /// Handle directory accesses
    if (IsDirectory(filepath)) {
        return __HandleDirectoryResource(ss, *route, filepath);
    }

    if (route->cgi_enabled && __AvaibleCgiDriver(filepath)) {
        return __HandleCgiRequest(ss, filepath);
    }

    /// Handle Static file request
    return __HandleStaticFile(ss, filepath);
}

void  HttpServer::__HandleDeleteFile(SessionCtx* ss, const std::string& filepath) {
    if (std::remove(filepath.c_str()))
        return ss->res_code = 500, __OnHttpError(ss);
    return ss->res_code = 204, __OnHttpResponse(ss);
}

void  HttpServer::__OnHttpError(SessionCtx* ss, bool reset) {
    info(ss->access_log, "Session[%d]: sending HTTP error",
                         ss->conn_sock.GetFd(),
                         ss->res_code);
    if (reset) {
        ss->http_writer.Reset();
    }
    if (ss->server != NULL &&
        ss->server->errpages.find(ss->res_code) != ss->server->errpages.end()) {

        IO::File file = __GetErrPage(ss->res_code, ss);
        if (file.GetFd() != -1) {
            return __SendStaticFileResponse(file, ss);
        }
    }
    return __SendDefaultErrPage(ss);
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
    if (!ss->http_writer.Header().Has("Content-Type") && ss->http_writer.HasBody()) {
        ss->http_writer.Header().Set("Content-Type", "text/plain");
    }

    // Set server name
    ss->http_writer.Header().Set("Server", "not-ngnix/1.16");

    // Append to response buffer and enable writing in poller
    ss->res_buff += ss->http_writer.SendToString(ss->res_code, ss->http_req.version);
    ss->http_writer.Reset();

    info(ss->access_log, "Session[%d]: sending HTTP response (code: %d)",
                         ss->conn_sock.GetFd(),
                         ss->res_code);

    __poller.AddEvMask(ss->conn_sock.GetFd(), IO::Poller::POLL_WRITE);
}

}  // namespace Webserver
