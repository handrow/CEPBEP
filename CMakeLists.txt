cmake_minimum_required(VERSION 3.0.0)
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

add_library(config_parser
	"sourses/config_parser/config_parser.cc"
	"sourses/config_parser/config_parser.h"	
)

# Main executable
add_executable(CEPBEP main.cc)
target_link_libraries(CEPBEP logger config_parser)
target_include_directories(CEPBEP PUBLIC "sources/")
