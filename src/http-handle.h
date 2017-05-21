#ifndef UVEXT_HTTP_HANDLE_H
#define UVEXT_HTTP_HANDLE_H

#include "request-parser.h"
#include <uv.h>

typedef enum http_method uv_http_method_t;

typedef struct uv_http_client_s {
  uv_stream_t handle;
  uv_http_parser_t parser;

  void *data;
  uv_loop_t *loop;
} uv_http_client_t;

typedef struct uv_http_req_s {
  uv_http_method_t method;
  const char *path;
  uv_http_header_t *headers;
  int major;
  int minor;

  uv_http_parser_t parser;
  void *data;
  uv_loop_t *loop;
} uv_http_req_t;

typedef void (*uv_http_request_err_cb)(uv_stream_t *stream, uv_http_req_t *req);
typedef void (*uv_http_parse_err_cb)(uv_http_parser_t *);

typedef struct uv_http_req_settings_s {
  uv_http_parse_complete_cb on_parse_complete;
  uv_http_parse_headers_complete_cb on_headers_complete;
  uv_http_parse_data_cb on_data;
  uv_http_parse_header_complete_cb on_header_complete;
  // uv_http_request_err_cb on_request_err;
  // uv_http_parse_er_cb on_parse_err;

} uv_http_req_settings_t;

void uv_http_client_init(uv_loop_t *loop, uv_http_client_t *client);

void uv_http_request_init(uv_http_req_t *req);
int uv_http_request(uv_stream_t *stream, uv_http_req_t *req,
                    uv_http_req_settings_t *settings);

int uv_http_write(uv_http_client_t *client, const char *data, int size);

int uv_http_request_end(uv_stream_t *stream, uv_http_req_t *req);

#endif