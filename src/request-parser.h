#ifndef UVEXT_HTTP_REQUEST_PARSER_H
#define UVEXT_HTTP_REQUEST_PARSER_H

#include "header.h"
#include "http_parser.h"

typedef struct uv_http_parse_req_s uv_http_parse_req_t;

// typedef struct sws_parse_result_s sws_parse_result_t;

typedef void (*uv_http_parse_complete_cb)(uv_http_parse_req_t *);
typedef void (*uv_http_parse_headers_complete_cb)(uv_http_parse_req_t *);
typedef void (*uv_http_parse_data_cb)(uv_http_parse_req_t *, const char *data,
                                      size_t length);

void uv_http_parser_init(uv_http_parse_req_t *parse_req,
                         uv_http_parse_headers_complete_cb on_header_complete,
                         uv_http_parse_data_cb on_data,
                         uv_http_parse_complete_cb on_parse_complete);

int uv_http_parser_execute(uv_http_parse_req_t *parse_req, char *buf,
                           ssize_t nread);

void uv_http_cleanup_parse_req(uv_http_parse_req_t *parse_req);

/* extends uv_http_parse_result_s */
struct uv_http_parse_req_s {

  int status_code;

  http_parser parser;
  uv_http_header_list_t *headers;
  // Callbacks
  uv_http_parse_complete_cb on_parse_complete;
  uv_http_parse_headers_complete_cb on_headers_complete;
  uv_http_parse_data_cb on_data;

  void *data;

  // Private
  uv_http_header_t *_c;
};

#endif