#ifndef UVEXT_HTTP_HANDLE_H
#define UVEXT_HTTP_HANDLE_H

#include "header.h"
#include "request-parser.h"
#include "uv.h"
/* extends uv_tcp_t (handle) */
struct uv_http_s {
  uv_tcp_t handle;
  int id;
  uv_http_parse_req_t parse_req;
  void *data;
  // sws_resource_info_t resource_info;
};

struct uv_http_request_s {
  uv_http_header_list_t *header;
};

typedef struct uv_http_s uv_http_t;

void on_connect(uv_connect_t *server, int status);

void uv_http_init(uv_loop_t *loop, uv_http_t *handle_req, int req_id);

#endif