cmake_minimum_required(VERSION 3.15)
project(CEPBEP)

# Configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS                 "-Wall -Wextra -Werror")
set(CMAKE_CXX_STANDARD              98)
set(TEST_DIR                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

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
    "sources/netlib/webserver/webserver_cgi.cc"
    "sources/netlib/webserver/webserver_poll.cc"
    "sources/netlib/webserver/webserver_listener_event.cc"
    "sources/netlib/webserver/webserver_sessions.cc"
    "sources/netlib/webserver/webserver_static.cc"
    "sources/netlib/webserver/webserver_http.cc"
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
add_executable(CEPBEP sources/main.cc)
target_link_libraries(CEPBEP ${OUR_LIBRARIES})

# Google Test Framework

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")

if (ENABLE_UNIT_TESTS)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.8.0
    )
    FetchContent_MakeAvailable(googletest)
    FetchContent_GetProperties(googletest)


    macro(ADD_UNIT_GTEST TNAME)
        add_executable(${TNAME} ${ARGN})
        target_compile_options(${TNAME} PUBLIC -D UNIT_TESTING_ )
        target_link_libraries(${TNAME} gtest gtest_main ${OUR_LIBRARIES})
        add_test(NAME ${TNAME}
                COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TNAME} --gtest_print_time=0
                WORKING_DIRECTORY ${TEST_DIR})
    endmacro()

    # Our unit tests
    ADD_UNIT_GTEST(example_test "tests/example.cc")
    ADD_UNIT_GTEST(uri_test "tests/uri_test.cc")
    ADD_UNIT_GTEST(poller_test "tests/poller_test.cc")
    ADD_UNIT_GTEST(socket_test "tests/sock_test.cc")
    ADD_UNIT_GTEST(request_reader_test "tests/request_reader_test.cc")
    ADD_UNIT_GTEST(response_reader_test "tests/response_reader_test.cc")
    ADD_UNIT_GTEST(config_ini_test "tests/config_ini_test.cc")

endif()
