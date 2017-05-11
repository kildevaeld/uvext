
set(HTTPPARSERDIR ${PROJECT_ROOT}/libs/http-parser)

include_directories(${HTTPPARSERDIR})

add_library(http_parser STATIC
  ${HTTPPARSERDIR}/http_parser.c
)