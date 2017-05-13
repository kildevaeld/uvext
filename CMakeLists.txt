cmake_minimum_required(VERSION 3.0)
project(uvext)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

include(cmake/http-parser.cmake)
include(cmake/uv.cmake)

add_subdirectory(libs/boringssl)
include_directories(libs/boringssl/include)
add_subdirectory(src)
include_directories(src)
add_subdirectory(example)