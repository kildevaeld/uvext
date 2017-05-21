#include "http-handle.h"
#include "debug.h"
#include <stdlib.h>

static void on_req_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf);
static void on_write_end(uv_write_t *req, int status);
static void on_res_end(uv_handle_t *handle);

#define CHECK(r, msg)                                                          \
  if (r) {                                                                     \
    log_err("%s: [%s(%d): %s]\n", msg, http_errno_name((r)), r,                \
            http_errno_description((r)));                                      \
    exit(1);                                                                   \
  }

#define UVERR(r, msg)                                                          \
  fprintf(stderr, "%s: [%s(%d): %s]\n", msg, uv_err_name((r)), r,              \
          uv_strerror((r)));

void uv_http_client_init(uv_loop_t *loop, uv_http_client_t *client) {
  uv_http_parser_init(&client->parser, NULL, NULL, NULL);
  client->loop = loop;
  client->data = NULL;
}

void uv_http_request_init(uv_http_req_t *req) {
  uv_http_parser_init(&req->parser, NULL, NULL, NULL);

  req->data = NULL;
  req->headers = NULL;
  req->major = 1;
  req->minor = 1;
  req->method = HTTP_GET;
  req->path = "/";
}

static void alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
  buf->base = malloc(size);
  buf->len = size;
}

static int write_method(uv_http_req_t *req, char *buf) {
  int i = 0;
  switch (req->method) {
  case HTTP_GET:
    memcpy(buf, "GET", 3);
    i += 3;
    break;
  case HTTP_POST:
    memcpy(buf, "POST", 4);
    i += 4;
    break;
  case HTTP_DELETE:
    memcpy(buf, "DELETE", 6);
    i += 6;
  case HTTP_HEAD:
    memcpy(buf, "HEAD", 4);
    i += 4;
  case HTTP_OPTIONS:
    memcpy(buf, "OPTIONS", 7);
    i += 7;
  default:
    return -1;
  }

  return i;
}

static int write_request(uv_http_req_t *req, char *buf) {

  int i = write_method(req, buf);

  int major = 1, minor = 1;
  if (req->major > -1)
    major = req->major;
  if (req->minor > -1)
    minor = req->minor;

  int ret = sprintf(buf + i, " %s HTTP/%d.%d\r\n", req->path, major, minor);
  if (ret < 0) {
    return -1;
  }
  i += ret;

  if (req->headers) {
    uv_http_header_foreach(h, req->headers) {
      int ret = sprintf(buf + i, "%s: %s\r\n", h->field, h->value);
      i += ret;
    }
  }

  memcpy(buf + i, "\r\n", 2);
  i += 2;

  return i;
}

static void on_write_end(uv_write_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "error on_write_end");
    return;
  }
  debug("write");
  debug("starting read");
  req->handle->data = req->data;
  // uv_read_start((uv_stream_t *)req->handle, alloc_cb, on_req_read);
}

static void on_req_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
  size_t parsed;
  uv_http_req_t *handle_req = (uv_http_req_t *)tcp->data;

  if (nread == UV_EOF) {

    uv_http_parser_execute(&handle_req->parser, buf->base, 0);
    uv_close((uv_handle_t *)tcp, on_res_end);
  } else if (nread > 0) {

    parsed = uv_http_parser_execute(&handle_req->parser, buf->base, nread);

    if (parsed < nread) {
      const char *nam =
          http_errno_name(HTTP_PARSER_ERRNO(&handle_req->parser.parser));
      const char *err =
          http_errno_description(HTTP_PARSER_ERRNO(&handle_req->parser.parser));

      log_err("parsing http req  %s: %s", nam, err);
      uv_close((uv_handle_t *)tcp, on_res_end);
    }
  } else {
    UVERR((int)nread, "reading req req");
  }
  if (buf->base)
    free(buf->base);
}

static void on_res_end(uv_handle_t *handle) {
  uv_http_req_t *handle_req = (uv_http_req_t *)handle;
  log_info("[ %3d ] connection closed", 1);
  // uv_http_cleanup(handle_req);
}

int uv_http_request(uv_stream_t *stream, uv_http_req_t *req,
                    uv_http_req_settings_t *settings) {

  req->parser.on_parse_complete = settings->on_parse_complete;
  req->parser.on_headers_complete = settings->on_headers_complete;
  req->parser.on_data = settings->on_data;
  req->parser.on_header_complete = settings->on_header_complete;

  uv_buf_t buf;
  char message[500];
  buf.len = write_request(req, message);
  buf.base = message;

  uv_write_t *write_req = malloc(sizeof(uv_write_t));
  write_req->data = req;
  // stream->data = req;

  return uv_write(write_req, stream, &buf, 1, on_write_end);
}

int uv_http_write(uv_http_client_t *client, const char *data, int size) {
  // O
  return 0;
}

int uv_http_request_end(uv_stream_t *stream, uv_http_req_t *req) {
  stream->data = req;
  uv_read_start(stream, alloc_cb, on_req_read);
}