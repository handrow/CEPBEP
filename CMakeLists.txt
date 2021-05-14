cmake_minimum_required(VERSION 3.11)
project(CEPBEP)

# Configuration
set(CMAKE_CXX_FLAGS 				"-Wall -Wextra -Werror")
set(CMAKE_CXX_STANDARD 				"98")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 	"${CMAKE_SOURCE_DIR}/bin")

# logger target
add_library(logger
	"sources/logger/logger.cc"
	"sources/logger/logger.h"
)


# Main executable
add_executable(CEPBEP main.cc)
target_link_libraries(CEPBEP logger)
target_include_directories(CEPBEP PUBLIC "sources/")

# gtest

add_library(sum_integers
	"tests/sum_integers.cc"
	"tests/sum_integers.h"
)

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")

if (ENABLE_UNIT_TESTS)
	include(FetchContent)
	FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.8.0
	)
		
	FetchContent_MakeAvailable(googletest)
	FetchContent_GetProperties(googletest)
endif()

macro(STOLEN_FROM_HGRANULE TNAME)
    add_executable(${TNAME} ${ARGN})
    target_link_libraries(${TNAME} gtest gtest_main sum_integers)
    target_include_directories(${TNAME} PUBLIC ${GOOGLETEST_INCLUDE})
    add_test(NAME ${TNAME}
            COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TNAME} --gtest_print_time=0
            WORKING_DIRECTORY ${TEST_DIR})
endmacro()

STOLEN_FROM_HGRANULE(sum_int_test "tests/test.cc")