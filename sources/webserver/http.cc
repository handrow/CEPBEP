#include "webserver/webserver.h"

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

USize        PathDiffer(const std::string& pattern, const std::string& path) {
    USize i;
    for (i = 0; i < pattern.length() && pattern[i] == path[i]; ++i) {
    }

    return i;
}

// /abc/adf, /abc/adf/abg/jhg -> abg/jhg
// /abc/adf/, /abc/adf/abg/jhg -> abg/jhg
// /abc/ad,   /abc/adf/abg/jhg -> adf/abg/jhg
std::string  GetRelativePathFromPattern(const std::string& pattern, const std::string& path) {
    USize diff = PathDiffer(pattern, path);

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
    USize diff = PathDiffer(pattern, path);

    return pattern.length() == diff;
}

}  // namespace

const HttpServer::WebRoute*
HttpServer::__FindWebRoute(const Http::Request& req, const WebRouteList& routes) {
    for (WebRouteList::const_iterator route_it  = routes.begin();
                                      route_it != routes.end();
                                      ++route_it) {
        const WebRoute& try_route = *route_it;
        if (MatchPath(try_route.Pattern, req.Uri.Path) == false)
            continue;
        return &try_route;
    }

    return NULL;
}

void  HttpServer::__HandleBadMethod(SessionCtx* ss, const WebRoute& route) {
    debug(ss->AccessLog, "Session[%d]: request method (%s) isn't allowed",
                           ss->ConnectionSock.GetFd(),
                           Http::MethodToString(ss->Request.Method).c_str());

    ss->ResponseWriter.Reset();
    for (MethodSet::iterator it = route.AllowedMethods.begin();
                                it != route.AllowedMethods.end();
                                ++it) {
        ss->ResponseWriter.Header().Add("Allow", Http::MethodToString(*it));
    }
    return ss->ResponseCode = 405, __OnHttpError(ss, false);
}

void  HttpServer::__HandleDirectoryResource(SessionCtx* ss,
                                            const WebRoute& route,
                                            const std::string& filepath) {

    debug(ss->AccessLog, "Session[%d]: resource (%s) found, it's a directory",
                          ss->ConnectionSock.GetFd(),
                          filepath.c_str());

    if (!route.IndexPage.empty()) {
        std::string indexpath = AppendPath(filepath, route.IndexPage);
        if (!IsDirectory(indexpath) && IsExist(indexpath)) {
            std::string redirect_link = AppendPath(ss->Request.Uri.Path, route.IndexPage);
            debug(ss->AccessLog, "Session[%d]: Found index page (%s), redirecting to (%s)",
                                    ss->ConnectionSock.GetFd(),
                                    indexpath.c_str(),
                                    redirect_link.c_str());
            return __OnHttpRedirect(ss, redirect_link, 302);
        } else {
            return ss->ResponseCode = 404, __OnHttpError(ss);
        }

    } else if (Back(filepath) != '/') {
        std::string fullDirRedirect = AppendPath(ss->Request.Uri.Path, route.IndexPage);
        debug(ss->AccessLog, "Session[%d]: Found directory (%s), redirecting to (%s)",
                                ss->ConnectionSock.GetFd(),
                                filepath.c_str(),
                                fullDirRedirect.c_str());
        return __OnHttpRedirect(ss, fullDirRedirect, 302);

    } else if (route.ListingEnabled) {
        debug(ss->AccessLog, "Session[%d]: Found directory (%s), sending directory listing",
                                ss->ConnectionSock.GetFd(),
                                filepath.c_str());
        return __SendDirectoryListing(filepath, ss);
    }

    debug(ss->AccessLog, "Session[%d]: Found directory (%s), unable to resolve: (index_page: %s, listing: %s)",
                        ss->ConnectionSock.GetFd(),
                        filepath.c_str(),
                        (route.IndexPage.empty() ? "NO" : "YES"),
                        (route.ListingEnabled ? "YES" : "NO"));
    return ss->ResponseCode = 404, __OnHttpError(ss);
}

void  HttpServer::__HandleStaticFile(SessionCtx* ss, const std::string& file_path) {
    Error err;
    IO::File  file = IO::File::OpenFile(file_path, O_RDONLY, &err);

    if (err.IsError()) {
        error(ss->ErrorLOg, "Session[%d]: static file \"%s\" couldn't be open: ``%s''",
                              ss->ConnectionSock.GetFd(),
                              file_path.c_str(),
                              err.Description.c_str());
        return ss->ResponseCode = 500, __OnHttpError(ss);
    }

    std::string mimeType = Mime::MapType(ss->Server->MimeMap, file_path);

    info(ss->AccessLog, "Session[%d]: sending static file (%s) with type \"%s\"",
                          ss->ConnectionSock.GetFd(),
                          file_path.c_str(),
                          mimeType.c_str());

    return ss->ResponseCode = 200,
           ss->ResponseWriter.Header().Set("Content-type", mimeType),
           __SendStaticFileResponse(file, ss);
}

void  HttpServer::__OnHttpRedirect(SessionCtx* ss, const std::string& location, int code) {
    ss->ResponseWriter.Header().Set("Location", location);
    ss->ResponseCode = code;
    return __OnHttpResponse(ss);
}

HttpServer::VirtualServer*
HttpServer::__GetVirtualServer(Fd lfd, const std::string& hostname) {

    std::pair<VirtualServerMap::iterator, VirtualServerMap::iterator> vit_range = VirtualServers_.equal_range(lfd);
    VirtualServerMap::iterator vit = vit_range.first;
    VirtualServerMap::iterator vit_e = vit_range.second;

    for (; vit != vit_e; ++vit) {
        VirtualServer* vs = &vit->second;
        for (VirtualServer::HostnameList::iterator it = vs->Hostnames.begin();
                                                   it != vs->Hostnames.end();
                                                   ++it) {
            if (Match(*it, hostname)) {
                return vs;
            }
        }
    }

    return NULL;
}

void  HttpServer::__OnHttpRequest(SessionCtx* ss) {
    /// Max body size check
    if (ss->Request.Body.size() > MaxBodySize_)
            return ss->ResponseCode = 413, __OnHttpError(ss);

    /// Virtual server resolving
    VirtualServer* vs = __GetVirtualServer(ss->ListenerFileDesc, ss->Request.Headers.Get("Host"));
    if (vs == NULL) {
        ss->Server = NULL;
        ss->ErrorLOg = SystemLog_;
        ss->AccessLog = SystemLog_;
        info(SystemLog_, "Session[%d]: can't identify virtual server", ss->ConnectionSock.GetFd());
        return ss->ResponseCode = 400, __OnHttpError(ss);
    }

    ss->Server = vs;
    ss->AccessLog = vs->AccessLog;
    ss->ErrorLOg = vs->ErrorLog;

    info(ss->AccessLog, "Session[%d]: new HTTP request:\n"
                         ">   http_method: %s\n"
                         ">  http_version: %s\n"
                         ">           uri: %s\n"
                         ">          host: %s",
                            ss->ConnectionSock.GetFd(),
                            Http::MethodToString(ss->Request.Method).c_str(),
                            Http::ProtocolVersionToString(ss->Request.Version).c_str(),
                            ss->Request.Uri.ToString().c_str(),
                            ss->Request.Headers.Get("Host").c_str());

    info(ss->AccessLog, "Session[%d]: virtual server identified", ss->ConnectionSock.GetFd());

    /// Resolving Web Route
    const WebRoute*  route = __FindWebRoute(ss->Request, vs->Routes);
    if (route == NULL) {
        debug(ss->AccessLog, "Session[%d]: no web route", ss->ConnectionSock.GetFd());
        return ss->ResponseCode = 404, __OnHttpError(ss);
    }

    debug(ss->AccessLog, "Session[%d]: choosen web route with pattern \"%s\"",
                            ss->ConnectionSock.GetFd(),
                            route->Pattern.c_str());

    /// Resolving redirections
    if (route->Redirect.Enabled) {
        debug(ss->AccessLog, "Session[%d]: redirection is enabled, redirecting to \"%s\"",
                                ss->ConnectionSock.GetFd(),
                                route->Redirect.Location.c_str());
        return __OnHttpRedirect(ss, route->Redirect.Location, route->Redirect.Code);
    }

    /// Check HTTP method is allowed
    if (route->AllowedMethods.count(ss->Request.Method) <= 0) {
        return __HandleBadMethod(ss, *route);
    }

    if (__IsUpload(ss, *route))
        return __HandleUploadRequest(ss, *route);

    /// Getting pathes
    std::string  relpath = GetRelativePathFromPattern(route->Pattern, ss->Request.Uri.Path);
    std::string  filepath = AppendPath(route->RootDir, relpath);

    /// Check for existent
    if (!IsExist(filepath)) {
        debug(ss->AccessLog, "Session[%d]: resource (\"%s\") not found",
                              ss->ConnectionSock.GetFd(),
                              filepath.c_str());
        return ss->ResponseCode = 404, __OnHttpError(ss);
    }

    /// Handle directory accesses
    if (IsDirectory(filepath)) {
        return __HandleDirectoryResource(ss, *route, filepath);
    }

    if (ss->Request.Method == Http::METHOD_DELETE)
        return __HandleDeleteFile(ss, filepath);

    if (route->CgiEnabled && __AvaibleCgiDriver(filepath)) {
        return __HandleCgiRequest(ss, filepath);
    }

    /// Handle Static file request
    return __HandleStaticFile(ss, filepath);
}

void  HttpServer::__HandleDeleteFile(SessionCtx* ss, const std::string& filepath) {
    if (std::remove(filepath.c_str()))
        return ss->ResponseCode = 500, __OnHttpError(ss);
    return ss->ResponseCode = 200, __OnHttpResponse(ss);
}

void  HttpServer::__OnHttpError(SessionCtx* ss, bool reset) {
    info(ss->AccessLog, "Session[%d]: sending HTTP error",
                         ss->ConnectionSock.GetFd(),
                         ss->ResponseCode);
    if (reset) {
        ss->ResponseWriter.Reset();
    }
    if (ss->Server != NULL &&
        ss->Server->ErrorPages.find(ss->ResponseCode) != ss->Server->ErrorPages.end()) {

        IO::File file = __GetErrPage(ss->ResponseCode, ss);
        if (file.GetFd() != -1) {
            return __SendStaticFileResponse(file, ss);
        }
    }
    return __SendDefaultErrPage(ss);
}

void  HttpServer::__OnHttpResponse(SessionCtx* ss) {
    if (ss->Request.Version != Http::HTTP_1_1 &&
        ss->Request.Version != Http::HTTP_1_0)
        ss->Request.Version = Http::HTTP_1_1;

    if (ss->Request.Version == Http::HTTP_1_1) {
        // Set default connection
        if (ss->IsConnectionClosed == true) {
            ss->ResponseWriter.Header().Set("Connection", "close");
        } else if (!ss->ResponseWriter.Header().Has("Connection")) {
            ss->ResponseWriter.Header().Set("Connection", "keep-alive");
        }
    } else if (ss->Request.Version == Http::HTTP_1_0) {
        ss->IsConnectionClosed = true;
    }

    // Set default Content-Type
    if (!ss->ResponseWriter.Header().Has("Content-Type") && ss->ResponseWriter.HasBody()) {
        ss->ResponseWriter.Header().Set("Content-Type", "text/plain");
    }

    // Set server name
    ss->ResponseWriter.Header().Set("Server", "not-ngnix/1.16");

    // Append to response buffer and enable writing in poller
    ss->ResultBuffer += ss->ResponseWriter.SendToString(ss->ResponseCode, ss->Request.Version);
    ss->ResponseWriter.Reset();

    info(ss->AccessLog, "Session[%d]: sending HTTP response (code: %d)",
                         ss->ConnectionSock.GetFd(),
                         ss->ResponseCode);

    Poller_.AddEvMask(ss->ConnectionSock.GetFd(), IO::Poller::POLL_WRITE);
}

}  // namespace Webserver
