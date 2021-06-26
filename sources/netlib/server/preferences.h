#ifndef NETLIB_PREFERENCES_H_
#define NETLIB_PREFERENCES_H_

#include <bitset>
#include <map>
#include <list>

#include "common/types.h"

#include "logger/logger.h"
#include "http/http.h"
#include "netlib/io/socket.h"

namespace Server {
namespace Preferences {

struct LoggerPref {
    std::string                         log_path;
    ft::Logger::LogLvl                  log_lvl;
};

struct RoutePref {
    typedef std::bitset<Http::METHODS_NUM> MethodsSet;

    std::string                         path_pattern;
    std::string                         root_dir;
    std::string                         index_page;
    MethodsSet                          allowed_methods;

    /* CGI_allowed                      */
    /* Upload_preferences               */
    /* Listening_allowed                */
    /* Redirect_preferences             */
};

struct ServerPref {
    typedef std::list<RoutePref>        RoutesList;
    typedef std::list<std::string>      HostPatternList;
    typedef std::map<int, std::string>  ErrPagesMap;

    LoggerPref                          access_log;
    LoggerPref                          error_log;
    RoutesList                          routes;
    HostPatternList                     names;
    ErrPagesMap                         def_pages;
    IO::Port                            port;
    IO::IpAddrV4                        addr;
    u64                                 con_timeout_ms;

    /* CGI_preferences                  */
    /* Client_max_body                  */
};

}  // namespace Preferences
}  // namespace Server

#endif  // NETLIB_PREFERENCES_H_
