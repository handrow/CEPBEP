cmake_minimum_required(VERSION 3.15)
project(webserv)

# Configuration

set(CMAKE_CXX_FLAGS                 "-Wall -Wextra -Werror")
set(CMAKE_CXX_STANDARD              98)
set(OUR_LIBRARIES                   netlib config_parser cgi http logger ft)
set(OUR_INCLUDES                    sources/)

include_directories(PUBLIC ${OUR_INCLUDES})

# logger target
add_library(logger
    "sources/logger/logger.cc"
    "sources/logger/logger.h"
)

add_library(cgi
    "sources/cgi/cgi.h"
    "sources/cgi/metavars.cc"
    "sources/cgi/response_reader.h"
    "sources/cgi/response_reader.cc"
)

add_library(http
    "sources/http/uri.h"
    "sources/http/http.h"
    "sources/http/writer.h"
    "sources/http/reader.h"
    "sources/http/mime.h"
    "sources/http/mime.cc"
    "sources/http/uri.cc"
    "sources/http/http.cc"
    "sources/http/headers.cc"
    "sources/http/reader_request.cc"
    "sources/http/reader_response.cc"
    "sources/http/reader_common.cc"
    "sources/http/writer.cc"
)

add_library(netlib
    "sources/netlib/io/errors.h"
    "sources/netlib/io/file.h"
    "sources/netlib/io/poller.h"
    "sources/netlib/io/socket.h"
    "sources/netlib/io/file.cc"
    "sources/netlib/io/poller.cc"
    "sources/netlib/io/socket.cc"

    "sources/netlib/event/event.h"
    "sources/netlib/event/loop.h"
    "sources/netlib/event/queue.h"
    "sources/netlib/event/loop.cc"

    "sources/netlib/webserver/webserver.h"
    "sources/netlib/webserver/webserver.cc"
    "sources/netlib/webserver/webserver_listing.cc"
    "sources/netlib/webserver/webserver_poll.cc"
    "sources/netlib/webserver/webserver_listener_event.cc"
    "sources/netlib/webserver/webserver_sessions.cc"
    "sources/netlib/webserver/webserver_static.cc"
    "sources/netlib/webserver/webserver_http.cc"
    "sources/netlib/webserver/webserver_timeout.cc"
    "sources/netlib/webserver/webserver_err.cc"
    "sources/netlib/webserver/webserver_cgi.cc"
    "sources/netlib/webserver/webserver_upload.cc"
)

add_library(config_parser
    "sources/config/config.cc"
    "sources/config/config.h"
)

add_library(ft
    "sources/common/error.h"
    "sources/common/fast_vector.h"
    "sources/common/fsm.h"
    "sources/common/string_utils.h"
    "sources/common/tests.h"
    "sources/common/time.h"
    "sources/common/types.h"
    "sources/common/file.h"

    "sources/common/match.cc"
    "sources/common/tokenizer.cc"
    "sources/common/file.cc"
)

# Main executable
add_executable(webserv sources/main.cc)
target_link_libraries(webserv ${OUR_LIBRARIES})
