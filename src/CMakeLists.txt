

set(SOURCES
    header.c
    http-handle.c
    request-parser.c
    linkedlist.c
    url.c
    http.c
)


set(LIBRARIES http_parser uv ssl crypto)


add_library(uvext ${SOURCES})
target_link_libraries(uvext  ${LIBRARIES})