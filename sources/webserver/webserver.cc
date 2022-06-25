#include <csignal>
#include "webserver/webserver.h"

namespace Webserver {

void  HttpServer::SetSystemLogger(Log::Logger* logger) {
    SystemLog_ = logger;
}

void  HttpServer::AddVritualServer(const IO::SockInfo& si, const VirtualServer& vs) {
    VirtualServerMap::iterator vit = VirtualServers_.begin();
    for (;vit != VirtualServers_.end(); ++vit) {
        Fd lfd = vit->first;

        if (Listeners_[lfd].GetSockInfo() == si) {
            // listener already exists
            VirtualServers_.insert(std::make_pair(lfd, vs));
            return;
        }
    }

    // create new listener
    Error err;
    IO::Socket sock = IO::Socket::CreateListenSocket(si, &err);
    if (err.IsError())
        throw std::runtime_error("Socket creation failed: " + err.Description);

    Listeners_[sock.GetFd()] = sock;
    Poller_.AddFd(sock.GetFd(), IO::Poller::POLL_READ);
    VirtualServers_.insert( std::make_pair(sock.GetFd(), vs) );
}

void  HttpServer::SetTimeout(UInt64 msec) {
    SessionTimeout_ = msec;
}

void  HttpServer::ServeForever() {
    signal(SIGPIPE, SIG_IGN);
    EventLoop_.AddDefaultEvent(SpawnPollerHook());
    EventLoop_.AddDefaultEvent(SpawnTimeoutHook());
    EventLoop_.AddDefaultEvent(SpawnCgiHook());
    EventLoop_.Run();
}



Log::Logger::LogLvl ReadLoggerLvl(const std::string& levelToken) {
    if (levelToken == "DEBUG")
        return Log::Logger::DEBUG;
    if (levelToken == "CRITICAL")
        return Log::Logger::CRITICAL;
    if (levelToken == "WARNING")
        return Log::Logger::WARNING;
    if (levelToken == "INFO")
        return Log::Logger::INFO;
    if (levelToken == "ERROR")
        return Log::Logger::ERROR;
    
    throw std::runtime_error("Config: " + levelToken + ": unknown logger lvl");
}

HttpServer::WebRedirect
ReadRedirect(const std::string& redirToken) {
    HttpServer::WebRedirect rd;
    rd.Enabled = true;
    rd.Code = 302;

    USize space = redirToken.find_first_of(" ");
    
    rd.Location = redirToken.substr(0, space);
    
    if (space != std::string::npos) {
        USize code_begin = redirToken.find_first_not_of(" ", space);
        USize code_end = redirToken.find_first_of(" ", code_begin);

        rd.Code = Convert<int>(redirToken.substr(code_begin, code_end - code_begin));
    }

    if (Trim(rd.Location, ' ').empty())
        throw std::runtime_error("Config: redirect location cant be empty");

    return rd;
}

HttpServer::MethodSet
ReadAllowedMethods(const std::string& mtds) {
    HttpServer::MethodSet ms;

    Tokenizator tkz(mtds);
    bool run = true;
    std::string token;

    while (token = tkz.Next(" \t", &run), run == true) {
        Http::Method method = Http::MethodFromString(token);
        if (method == Http::METHOD_UNKNOWN)
            throw std::runtime_error("Config: unkown HTTP method: " + token);
        ms.insert(method);
    }

    return ms;
}

bool ReadYes(const std::string& str) {
    return str == "YES" || str == "yes";
}

IO::SockInfo ReadListen(const std::string& str) {
    USize portD = str.find_first_of(":");

    if (portD == std::string::npos || portD + 1 > str.length())
        throw std::runtime_error("Config: Invalid listen field format: must be `D.D.D.D:P`");

    const std::string ipaddr_str = str.substr(0, portD);
    const std::string port_str = str.substr(portD + 1);

    IO::SockInfo si(ipaddr_str, Convert<UInt16>(port_str));

    if (si.Addr_BE == INADDR_NONE)
        throw std::runtime_error("Config: Bad ipv4 address");

    return si;
}

HttpServer::VirtualServer::HostnameList
ReadHostnames(const std::string& str) {

    HttpServer::VirtualServer::HostnameList hstn;

    Tokenizator tkz(str);
    bool run = true;
    std::string token;

    while (token = tkz.Next(" \t", &run), run == true) {
        hstn.push_back(token);
    }

    return hstn;
}


void  HttpServer::Config(const Config::Category& cat, Cgi::Envs evs) {

    Envs_ = evs;

    /// GLOBAL configs
    USize           gMaxBody = 10000; // 10 KB
    UInt64          gTimeout = 10000; // 10 seconds
    std::string     gCwd = "./";
    std::string     gLoggerKey;
    {
        if (cat.HasCategory("GLOBAL")) {
            const Config::Category& globalCtg = cat.GetSubcategoryRef("GLOBAL");

            if (globalCtg.HasField("cwd"))      gCwd = globalCtg.GetFieldValue("cwd");
            if (globalCtg.HasField("timeout"))  gTimeout = Convert<UInt64>(globalCtg.GetFieldValue("timeout"));
            if (globalCtg.HasField("max_body")) gMaxBody = Convert<UInt64>(globalCtg.GetFieldValue("max_body"));
            if (globalCtg.HasField("logger"))   gLoggerKey = globalCtg.GetFieldValue("logger");
        }

        if (gLoggerKey.empty())
            throw std::runtime_error("config: specify logger key");
        if (chdir(gCwd.c_str()))
            throw std::runtime_error("config: cwd - failed");

        SessionTimeout_ = gTimeout;
        MaxBodySize_ = gMaxBody;
    }

    SetTimeout(gTimeout);

    /// MIMES configs
    Mime::MimeTypesMap mimesMap;
    {
        if (cat.HasCategory("MIMES")) {
            const Config::Category& mimes_cat = cat.GetSubcategoryRef("MIMES");

            Config::Category::FieldsConstRange itrange = mimes_cat.GetFieldsIterRange();
            Config::Category::FieldsConstIter it1 = itrange.first;
            Config::Category::FieldsConstIter it2 = itrange.second;

            for (;it1 != it2; ++it1) {
                mimesMap[it1->first] = it1->second;
            }
        }
    }

    /// MIMES configs
    std::map< std::string, std::string>  cgiDrivers;
    {
        if (cat.HasCategory("CGI")) {
            const Config::Category& cgi_cat = cat.GetSubcategoryRef("CGI");

            Config::Category::FieldsConstRange itrange = cgi_cat.GetFieldsIterRange();
            Config::Category::FieldsConstIter it1 = itrange.first;
            Config::Category::FieldsConstIter it2 = itrange.second;

            for (;it1 != it2; ++it1) {
                cgiDrivers[it1->first] = it1->second;
            }
        }

        CgiDrivers_ = cgiDrivers;
    }

    HttpServer::ErrpageMap errorPages;
    {
        if (cat.HasCategory("ERRORS")) {
            const Config::Category& errs_cat = cat.GetSubcategoryRef("ERRORS");

            Config::Category::FieldsConstRange itrange = errs_cat.GetFieldsIterRange();
            Config::Category::FieldsConstIter it1 = itrange.first;
            Config::Category::FieldsConstIter it2 = itrange.second;

            for (;it1 != it2; ++it1) {
                errorPages[Convert<int>(it1->first)] = it1->second;
            }
        }
    }

    /// LOGGERS registry
    std::map<std::string, Log::Logger> loggers;
    {
        if (cat.HasCategory("LOGGERS")) {
            const Config::Category& loggerCtg = cat.GetSubcategoryRef("LOGGERS");

            Config::Category::SubcategoryConstRange itrange = loggerCtg.GetSubcatoryIterRange();
            Config::Category::SubcategoryConstIter  it1 = itrange.first;
            Config::Category::SubcategoryConstIter  it2 = itrange.second;

            for (; it1 != it2; ++it1) {
                const Config::Category& loggerCtg = it1->second;

                Log::Logger::LogLvl lvl = Log::Logger::INFO;
                std::string path = "/dev/stdout";

                if (loggerCtg.HasField("path"))    path = loggerCtg.GetFieldValue("path");
                if (loggerCtg.HasField("lvl"))     lvl = ReadLoggerLvl(loggerCtg.GetFieldValue("lvl"));

                loggers[it1->first] = Log::Logger(lvl, path.c_str());
            }
        }
    }

    if (loggers.count(gLoggerKey) <= 0)
        throw std::runtime_error("Config: " + gLoggerKey + ": no such logger");
    loggers[gLoggerKey].Open();
    SetSystemLogger(&loggers[gLoggerKey]);

    /// ROUTES registry
    std::map<std::string, WebRoute> webRoutes;
    {
        if (cat.HasCategory("ROUTES")) {
            const Config::Category& routes_cat = cat.GetSubcategoryRef("ROUTES");

            Config::Category::SubcategoryConstRange itrange = routes_cat.GetSubcatoryIterRange();
            Config::Category::SubcategoryConstIter  it1 = itrange.first;
            Config::Category::SubcategoryConstIter  it2 = itrange.second;

            for (; it1 != it2; ++it1) {
                std::string errMsgPre = "Config: Route \"" + it1->first + "\": ";

                std::string location;  // required
                std::string root;  // required
                // directory handling
                bool        cgi_enabled = false;
                bool        listing_enabled = false;
                bool        upload_enabled = false;
                std::string index_page = "";

                // redirection handling
                WebRedirect redirect = {.Enabled = false, .Location = "", .Code = 302};

                MethodSet   allowedMethods;

                /// parsing
                const Config::Category& routeCtg = it1->second;

                if (routeCtg.HasField("location"))
                    location = routeCtg.GetFieldValue("location");
                else
                    throw std::runtime_error(errMsgPre + "location field is required");

                if (routeCtg.HasField("redirect")) {
                    redirect = ReadRedirect(routeCtg.GetFieldValue("redirect"));
                } else {
                    if (routeCtg.HasField("root"))
                        root = routeCtg.GetFieldValue("root");
                    else
                        throw std::runtime_error(errMsgPre + "root field is required");

                    if (routeCtg.HasField("allowed_methods"))
                        allowedMethods = ReadAllowedMethods(routeCtg.GetFieldValue("allowed_methods"));
                    else
                        throw std::runtime_error(errMsgPre + "allowed_methods field is required");

                    if (routeCtg.HasField("index"))         index_page = routeCtg.GetFieldValue("index");
                    if (routeCtg.HasField("listing"))       listing_enabled = ReadYes(routeCtg.GetFieldValue("listing"));
                    if (routeCtg.HasField("upload"))        upload_enabled = ReadYes(routeCtg.GetFieldValue("upload"));
                    if (routeCtg.HasField("cgi"))           cgi_enabled = ReadYes(routeCtg.GetFieldValue("cgi"));
                }

                webRoutes[it1->first] = (WebRoute){
                    .Pattern = location,
                    .RootDir = root,
                    .IndexPage = index_page,
                    .Redirect = redirect,
                    .AllowedMethods = allowedMethods,
                    .CgiEnabled = cgi_enabled,
                    .UploadEnabled = upload_enabled,
                    .ListingEnabled = listing_enabled,
                };
            }

        }
    }

    /// SERVERS registry
    {
        bool oneServerFlag = false;
        if (cat.HasCategory("SERVERS")) {
            const Config::Category& serversCtg = cat.GetSubcategoryRef("SERVERS");

            Config::Category::SubcategoryConstRange itrange = serversCtg.GetSubcatoryIterRange();
            Config::Category::SubcategoryConstIter  it1 = itrange.first;
            Config::Category::SubcategoryConstIter  it2 = itrange.second;

            for (;it1 != it2; ++it1) {
                oneServerFlag = true;

                std::string errorMsgPre = "Config: Server \"" + it1->first + "\": ";
                const Config::Category&  serverCtg = it1->second;

                VirtualServer               vs;
                IO::SockInfo                si;

                if (serverCtg.HasField("listen"))
                    si = ReadListen(serverCtg.GetFieldValue("listen"));
                else
                    throw std::runtime_error(errorMsgPre + "listen field is required");

                if (serverCtg.HasField("names"))
                    vs.Hostnames = ReadHostnames(serverCtg.GetFieldValue("names"));
                else
                    throw std::runtime_error(errorMsgPre + "names field is required");

                if (serverCtg.HasField("routes")) {
                    std::string webRoutesStr = serverCtg.GetFieldValue("routes");

                    Tokenizator tkz(webRoutesStr);
                    bool run = true;
                    std::string token;

                    while (token = tkz.Next(" \t", &run), run) {
                        if (webRoutes.count(token) <= 0)
                            throw std::runtime_error(errorMsgPre + token + ": no such route");
                        vs.Routes.push_back(webRoutes[token]);
                    }
                }

                if (serverCtg.HasField("access_log")) {
                    std::string loggerKey = serverCtg.GetFieldValue("access_log");
                    if (loggers.count(loggerKey) <= 0)
                        throw std::runtime_error(errorMsgPre + loggerKey + ": no such logger");
                    loggers[loggerKey].Open();
                    vs.AccessLog = &loggers[loggerKey];
                }

                if (serverCtg.HasField("error_log")) {
                    std::string loggerKey = serverCtg.GetFieldValue("error_log");
                    if (loggers.count(loggerKey) <= 0)
                        throw std::runtime_error(errorMsgPre + loggerKey + ": no such logger");
                    loggers[loggerKey].Open();
                    vs.ErrorLog = &loggers[loggerKey];
                }

                vs.MimeMap = mimesMap;
                vs.ErrorPages = errorPages;
                AddVritualServer(si, vs);
            }
        }
        
        if (oneServerFlag == false) {
            throw std::runtime_error("Config: at least one server must be specified");
        }
    }

    /// RUN!!!
    ServeForever();
}

}  // namespace Webserver
