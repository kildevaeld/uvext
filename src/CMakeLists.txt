

set(SOURCES
    header.c
    http-handle.c
    request-parser.c
    linkedlist.c
)


add_library(uvext ${SOURCES})
target_link_libraries(uvext  http_parser uv)