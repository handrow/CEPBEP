#include "netlib/webserver/webserver.h"

namespace Webserver {

void  HttpServer::SetSystemLogger(Log::Logger* logger) {
    __system_log = logger;
}

void  HttpServer::AddVritualServer(const IO::SockInfo& si, const VirtualServer& vs) {
    VirtualServerMap::iterator vit = __vservers_map.begin();
    for (;vit != __vservers_map.end(); ++vit) {
        fd_t lfd = vit->first;

        if (__listeners_map[lfd].GetSockInfo() == si) {
            // listener already exists
            __vservers_map.insert(std::make_pair(lfd, vs));
            return;
        }
    }

    // create new listener
    Error err;
    IO::Socket sock = IO::Socket::CreateListenSocket(si, &err);

    if (err.IsError())
        throw std::runtime_error("Socket creation failed: " + err.message);

    __listeners_map[sock.GetFd()] = sock;
    __poller.AddFd(sock.GetFd(), IO::Poller::POLL_READ);
    __vservers_map.insert( std::make_pair(sock.GetFd(), vs) );
}

void  HttpServer::SetTimeout(u64 msec) {
    __session_timeout = msec;
}

void  HttpServer::ServeForever() {
    __evloop.AddDefaultEvent(__SpawnPollerHook());
    __evloop.AddDefaultEvent(__SpawnTimeoutHook());
    __evloop.Run();
}



Log::Logger::LogLvl ReadLoggerLvl(const std::string& lvl_token) {
    if (lvl_token == "DEBUG")
        return Log::Logger::DEBUG;
    if (lvl_token == "CRITICAL")
        return Log::Logger::CRITICAL;
    if (lvl_token == "WARNING")
        return Log::Logger::WARNING;
    if (lvl_token == "INFO")
        return Log::Logger::INFO;
    if (lvl_token == "ERROR")
        return Log::Logger::ERROR;
    
    throw std::runtime_error("Config: " + lvl_token + ": unknown logger lvl");
}

HttpServer::WebRedirect
ReadRedirect(const std::string& redir_token) {
    HttpServer::WebRedirect rd;
    rd.enabled = true;
    rd.code = 302;

    usize space = redir_token.find_first_of(" ");
    
    rd.location = redir_token.substr(0, space);
    
    if (space != std::string::npos) {
        usize code_begin = redir_token.find_first_not_of(" ", space);
        usize code_end = redir_token.find_first_of(" ", code_begin);

        rd.code = Convert<int>(redir_token.substr(code_begin, code_end - code_begin));
    }

    if (Trim(rd.location, ' ').empty())
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
    usize port_d = str.find_first_of(":");

    if (port_d == std::string::npos || port_d + 1 > str.length())
        throw std::runtime_error("Config: Invalid listen field format: must be `D.D.D.D:P`");

    const std::string ipaddr_str = str.substr(0, port_d);
    const std::string port_str = str.substr(port_d + 1);

    IO::SockInfo si(ipaddr_str, Convert<u16>(port_str));

    if (si.addr_BE == INADDR_NONE)
        throw std::runtime_error("Config: Bad ipv4 address");

    return si;
}

HttpServer::VirtualServer::Hostnames
ReadHostnames(const std::string& str) {

    HttpServer::VirtualServer::Hostnames hstn;

    Tokenizator tkz(str);
    bool run = true;
    std::string token;

    while (token = tkz.Next(" \t", &run), run == true) {
        hstn.push_back(token);
    }

    return hstn;
}


void  HttpServer::Config(const Config::Category& cat) {
    /// GLOBAL configs
    usize           GLOBAL_max_body = 10000; // 10 KB
    u64             GLOBAL_timeout = 10000; // 10 seconds
    std::string     GLOBAL_cwd = "./";
    std::string     GLOBAL_logger_key;
    {
        if (cat.HasCategory("GLOBAL")) {
            const Config::Category& global_cat = cat.GetSubcategoryRef("GLOBAL");


            if (global_cat.HasField("cwd"))      GLOBAL_cwd = global_cat.GetFieldValue("cwd");
            if (global_cat.HasField("timeout"))  GLOBAL_timeout = Convert<u64>(global_cat.GetFieldValue("timeout"));
            if (global_cat.HasField("max_body")) GLOBAL_max_body = Convert<u64>(global_cat.GetFieldValue("max_body"));
            if (global_cat.HasField("logger"))   GLOBAL_logger_key = global_cat.GetFieldValue("logger");
        }

        if (GLOBAL_logger_key.empty())
            throw std::runtime_error("config: specify logger key");
        if (chdir(GLOBAL_cwd.c_str()))
            throw std::runtime_error("config: cwd - failed");

        __session_timeout = GLOBAL_timeout;
        // __max_body_size = GLOBAL_max_body;
    }

    SetTimeout(GLOBAL_timeout);

    /// MIMES configs
    Mime::MimeTypesMap MIMES_map;
    {
        if (cat.HasCategory("MIMES")) {
            const Config::Category& mimes_cat = cat.GetSubcategoryRef("MIMES");

            Config::Category::FieldsConstRange itrange = mimes_cat.GetFieldsIterRange();
            Config::Category::FieldsConstIter it1 = itrange.first;
            Config::Category::FieldsConstIter it2 = itrange.second;

            for (;it1 != it2; ++it1) {
                MIMES_map[it1->first] = it1->second;
            }
        }
    }

    HttpServer::ErrpageMap ERRORS_map;
    {
        if (cat.HasCategory("ERRORS")) {
            const Config::Category& errs_cat = cat.GetSubcategoryRef("ERRORS");

            Config::Category::FieldsConstRange itrange = errs_cat.GetFieldsIterRange();
            Config::Category::FieldsConstIter it1 = itrange.first;
            Config::Category::FieldsConstIter it2 = itrange.second;

            for (;it1 != it2; ++it1) {
                ERRORS_map[Convert<int>(it1->first)] = it1->second;
            }
        }
    }

    /// LOGGERS registry
    std::map<std::string, Log::Logger*> LOGGERS_registry;
    {
        if (cat.HasCategory("LOGGERS")) {
            const Config::Category& loggers_cat = cat.GetSubcategoryRef("LOGGERS");

            Config::Category::SubcategoryConstRange itrange = loggers_cat.GetSubcatoryIterRange();
            Config::Category::SubcategoryConstIter  it1 = itrange.first;
            Config::Category::SubcategoryConstIter  it2 = itrange.second;

            for (; it1 != it2; ++it1) {
                const Config::Category& logger_cat = it1->second;

                Log::Logger::LogLvl lvl = Log::Logger::INFO;
                std::string path = "/dev/stdout";

                if (logger_cat.HasField("path"))    path = logger_cat.GetFieldValue("path");
                if (logger_cat.HasField("lvl"))     lvl = ReadLoggerLvl(logger_cat.GetFieldValue("lvl"));

                LOGGERS_registry[it1->first] = new Log::Logger(lvl, path.c_str());
            }
        }
    }

    if (LOGGERS_registry.count(GLOBAL_logger_key) <= 0)
        throw std::runtime_error("Config: " + GLOBAL_logger_key + ": no such logger");
    SetSystemLogger(LOGGERS_registry[GLOBAL_logger_key]);

    /// ROUTES registry
    std::map<std::string, WebRoute> ROUTES_registry;
    {
        if (cat.HasCategory("ROUTES")) {
            const Config::Category& routes_cat = cat.GetSubcategoryRef("ROUTES");

            Config::Category::SubcategoryConstRange itrange = routes_cat.GetSubcatoryIterRange();
            Config::Category::SubcategoryConstIter  it1 = itrange.first;
            Config::Category::SubcategoryConstIter  it2 = itrange.second;

            for (; it1 != it2; ++it1) {
                std::string errmsg_pre = "Config: Route \"" + it1->first + "\": ";

                std::string location;  // required
                std::string root;  // required
                // directory handling
                bool        listing_enabled = false;
                std::string index_page = "";
                // redirection handling
                WebRedirect redirect = {.enabled = false};

                MethodSet   allowed_methods;

                /// parsing
                const Config::Category& route_cat = it1->second;

                if (route_cat.HasField("location"))
                    location = route_cat.GetFieldValue("location");
                else
                    throw std::runtime_error(errmsg_pre + "location field is required");

                if (route_cat.HasField("redirect")) {
                    redirect = ReadRedirect(route_cat.GetFieldValue("redirect"));
                } else {

                    if (route_cat.HasField("root"))
                        root = route_cat.GetFieldValue("root");
                    else
                        throw std::runtime_error(errmsg_pre + "root field is required");

                    if (route_cat.HasField("allowed_methods"))
                        allowed_methods = ReadAllowedMethods(route_cat.GetFieldValue("allowed_methods"));
                    else
                        throw std::runtime_error(errmsg_pre + "allowed_methods field is required");

                    if (route_cat.HasField("index_page"))    index_page = route_cat.GetFieldValue("index_page");
                    if (route_cat.HasField("listing"))       listing_enabled = ReadYes(route_cat.GetFieldValue("listing"));
                }

                ROUTES_registry[it1->first] = (WebRoute){
                    .pattern = location,
                    .root_directory = root,
                    .index_page = index_page,
                    .reditect = redirect,
                    .allowed_methods = allowed_methods,
                    .listing_enabled = listing_enabled
                };
            }

        }
    }

    /// SERVERS registry
    {
        bool one_server_flag = false;
        if (cat.HasCategory("SERVERS")) {
            const Config::Category& servers_cat = cat.GetSubcategoryRef("SERVERS");

            Config::Category::SubcategoryConstRange itrange = servers_cat.GetSubcatoryIterRange();
            Config::Category::SubcategoryConstIter  it1 = itrange.first;
            Config::Category::SubcategoryConstIter  it2 = itrange.second;

            for (;it1 != it2; ++it1) {
                one_server_flag = true;

                std::string errmsg_pre = "Config: Server \"" + it1->first + "\": ";
                const Config::Category&  server_cat = it1->second;

                VirtualServer               vs;
                IO::SockInfo                si;

                if (server_cat.HasField("listen"))
                    si = ReadListen(server_cat.GetFieldValue("listen"));
                else
                    throw std::runtime_error(errmsg_pre + "listen field is required");

                if (server_cat.HasField("names"))
                    vs.hostnames = ReadHostnames(server_cat.GetFieldValue("names"));
                else
                    throw std::runtime_error(errmsg_pre + "names field is required");

                if (server_cat.HasField("routes")) {
                    std::string routes_str = server_cat.GetFieldValue("routes");

                    Tokenizator tkz(routes_str);
                    bool run = true;
                    std::string token;

                    while (token = tkz.Next(" \t", &run), run) {
                        if (ROUTES_registry.count(token) <= 0)
                            throw std::runtime_error(errmsg_pre + token + ": no such route");
                        vs.routes.push_back(ROUTES_registry[token]);
                    }
                }

                if (server_cat.HasField("access_log")) {
                    std::string logger_key = server_cat.GetFieldValue("access_log");
                    if (LOGGERS_registry.count(logger_key) <= 0)
                        throw std::runtime_error(errmsg_pre + logger_key + ": no such logger");
                    vs.access_log = LOGGERS_registry[logger_key];
                }

                if (server_cat.HasField("error_log")) {
                    std::string logger_key = server_cat.GetFieldValue("error_log");
                    if (LOGGERS_registry.count(logger_key) <= 0)
                        throw std::runtime_error(errmsg_pre + logger_key + ": no such logger");
                    vs.access_log = LOGGERS_registry[logger_key];
                }

                vs.mime_map = MIMES_map;
                vs.errpages = ERRORS_map;

                AddVritualServer(si, vs);
            }
        }
        
        if (one_server_flag == false) {
            throw std::runtime_error("Config: at least one server must be specified");
        }
    }

    /// RUN!!!
    ServeForever();

    /// DEALLOCATION of logger registry
    {
        std::map<std::string, Log::Logger*>::iterator lit = LOGGERS_registry.begin();
        std::map<std::string, Log::Logger*>::iterator lend = LOGGERS_registry.end();

        for (;lit != lend; ++lit) {
            delete lit->second;
        }
    }
}

}  // namespace Webserver
