cmake_minimum_required(VERSION 3.0)
project(uvext)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

option(UVEXT_SYSTEM_SSL "Use system ssl" off)
option(UVEXT_EXAMPLE "Build example" off)


include(cmake/http-parser.cmake)
include(cmake/uv.cmake)

if (NOT ${UVEXT_SYSTEM_SSL})
	message(STATUS "Using local boringssl")
	add_subdirectory(libs/boringssl)
	include_directories(libs/boringssl/include)
else()
	message(STATUS "Using system openssl")
	if (APPLE)
		include_directories(/usr/local/opt/openssl/lib)
		link_directories(/usr/local/opt/openssl/include)
	endif(APPLE)


endif(NOT ${UVEXT_SYSTEM_SSL})

add_subdirectory(src)
include_directories(src)

if (${UVEXT_EXAMPLE})
	add_subdirectory(example)
endif(${UVEXT_EXAMPLE})