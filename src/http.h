#ifndef UVEXT_SIMPLE_HTTP_H
#define UVEXT_SIMPLE_HTTP_H

#include "http-handle.h"
#include <stdbool.h>

typedef struct uv_http_client_req_s {
  UV_HTTP_REQUEST_DEF
  const char *host;
  int port;
} uv_http_client_req_t;

typedef struct uv_http_client_s {
  uv_stream_t handle;
  uv_http_client_req_t *req;
  uv_http_req_settings_t *settings;
  uv_loop_t *loop;
  void *data;

  // Private
  bool headers_sent;
} uv_http_client_t;

typedef void (*uv_http_client_connect_cb)(uv_http_client_t *, int status);

void uv_http_client_init(uv_loop_t *loop, uv_http_client_t *client,
                         uv_http_req_settings_t *settings);

int uv_http_client_req(uv_http_client_t *client, uv_http_client_req_t *req,
                       uv_http_client_connect_cb cb);

int uv_http_client_write(uv_http_client_t *client, uv_buf_t *buf, uv_write_cb cb);

int uv_http_client_req_end(uv_http_client_t *client);


#endif