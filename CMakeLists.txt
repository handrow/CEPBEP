cmake_minimum_required(VERSION 3.15)
project(CEPBEP)

# Configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 	${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS 				"-Wall -Wextra -Werror")
set(CMAKE_CXX_STANDARD 				98)

set(OUR_LIBRARIES					logger http_parser)
set(OUR_INCLUDES					sources/)


# logger target
add_library(logger
	"sources/logger/logger.cc"
	"sources/logger/logger.h"
)

add_library(http_parser
	"sources/http/request.cc"
	"sources/http/response.cc"
	"sources/http/headers.cc"
	"sources/http/http.h"
)

# Main executable
add_executable(CEPBEP main.cc)
target_link_libraries(CEPBEP ${OUR_LIBRARIES})
target_include_directories(CEPBEP PUBLIC ${OUR_INCLUDES})

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
		target_link_libraries(${TNAME} gtest gtest_main ${OUR_LIBRARIES})
		target_include_directories(${TNAME} PUBLIC ${OUR_INCLUDES})
		add_test(NAME ${TNAME}
				COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TNAME} --gtest_print_time=0
				WORKING_DIRECTORY ${TEST_DIR})
	endmacro()

	# Our unit tests
	ADD_UNIT_GTEST(example_test "tests/example.cc")

endif()
