cmake_minimum_required(VERSION 3.0.0)
project(CEPBEP)

# Configuration
set(CMAKE_CXX_FLAGS 				"-Wall -Wextra -Werror")
set(CMAKE_CXX_STANDARD 				"98")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 	"${CMAKE_SOURCE_DIR}/bin")

# Main executable
add_executable(CEPBEP main.cc)
target_include_directories(CEPBEP PUBLIC "sources/")