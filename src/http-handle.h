#ifndef UVEXT_HTTP_HANDLE_H
#define UVEXT_HTTP_HANDLE_H

#include "header.h"
#include "request-parser.h"
#include "url.h"
#include "uv.h"

typedef enum http_method uv_http_method_t;

typedef void (*uv_http_request_err_cb)(uv_stream_t *stream,
                                       uv_http_request_t *req);
typedef void (*uv_http_parse_err_cb)(uv_http_parse_req_t *,
                                     uv_stream_t *stream);

/* extends uv_tcp_t (handle) */
struct uv_http_s {
  uv_tcp_t handle;
  int id;
  uv_http_parse_req_t parse_req;
  void *data;
  // sws_resource_info_t resource_info;
};

typedef struct uv_http_request_s {
  uv_http_method_t method;
  uv_url_data_t *url;
  uv_http_header_list_t *headers;
  int major;
  int minor;
  void *data;
  /* private */
  uv_http_parse_req_t parse_req;
  uv_loop_t *loop;
} uv_http_request_t;

typedef struct uv_http_request_settings_s {
  uv_http_parse_complete_cb on_parse_complete;
  uv_http_parse_headers_complete_cb on_headers_complete;
  uv_http_parse_data_cb on_data;
  uv_http_request_err_cb on_request_err;
  uv_http_parse_er_cb on_parse_err;

} uv_http_request_settings_t;

typedef struct uv_http_s uv_http_t;

void on_connect(uv_connect_t *server, int status);

int uv_http_request_init(uv_loop_t *loop, char *url, uv_http_request_t *req);

void uv_http_request_cleanup(uv_http_request_t *req);

int uv_http_request(uv_stream_t *stream, uv_http_request_t *req,
                    uv_http_request_settings_t *settings);

#endif