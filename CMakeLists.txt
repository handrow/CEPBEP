cmake_minimum_required(VERSION 3.0.0)
project(CEPBEP)

set(COMPILE_FLAGS -Wall -Wextra -Werror)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(CEPBEP main.cc)