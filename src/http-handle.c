#include "http-handle.h"
#include "debug.h"
#include "request-parser.h"
#include <stdlib.h>

static void on_req_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf);
static void on_write_end(uv_write_t *req, int status);
static void on_res_end(uv_handle_t *handle);

static void alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
  buf->base = malloc(size);
  buf->len = size;
}

#define CHECK(r, msg)                                                          \
  if (r) {                                                                     \
    log_err("%s: [%s(%d): %s]\n", msg, http_errno_name((r)), r,                \
            http_errno_description((r)));                                      \
    exit(1);                                                                   \
  }

#define UVERR(r, msg)                                                          \
  fprintf(stderr, "%s: [%s(%d): %s]\n", msg, uv_err_name((r)), r,              \
          uv_strerror((r)));

void on_connect(uv_connect_t *req, int status) {

  if (status == -1) {
    log_err("error on_write_end");
    return;
  }

  int r;

  CHECK(status, "connecting");
  debug("connecting req");

  // the tcp handle points to our sws_handle_req_t which can store a bit extra
  // info

  // uv_read_start((uv_stream_t *)handle_req, alloc_cb, on_req_read);
  char *message = "GET / HTTP/1.1\r\n"
                  "Content-Type: text/html\r\n\r\n";
  int len = strlen(message);

  char buffer[100];
  uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));

  buf.len = len;
  buf.base = message;

  uv_write_t write_req;

  int buf_count = 1;

  dbg("writing:\n%s\n", message);
  uv_write(&write_req, req->handle, &buf, buf_count, on_write_end);
}

static void on_req_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
  size_t parsed;
  uv_http_request_t *handle_req = (uv_http_request_t *)tcp->data;

  if (nread == UV_EOF) {
    uv_http_parser_execute(&handle_req->parse_req, buf->base, 0);
    uv_close((uv_handle_t *)tcp, on_res_end);

  } else if (nread > 0) {

    parsed = uv_http_parser_execute(&handle_req->parse_req, buf->base, nread);
    if (parsed < nread) {
      const char *nam =
          http_errno_name(HTTP_PARSER_ERRNO(&handle_req->parse_req.parser));
      const char *err = http_errno_description(
          HTTP_PARSER_ERRNO(&handle_req->parse_req.parser));

      log_err("parsing http req  %s: %s", nam, err);
      uv_close((uv_handle_t *)tcp, on_res_end);
    }

  } else {
    UVERR((int)nread, "reading req req");
  }
  if (buf->base)
    free(buf->base);
}

static void on_write_end(uv_write_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "error on_write_end");
    return;
  }
  debug("starting read");
  uv_read_start((uv_stream_t *)req->handle, alloc_cb, on_req_read);
}

static void uv_http_cleanup(uv_http_t *handle_req) {
  uv_http_cleanup_parse_req(&handle_req->parse_req);
}

static void on_res_end(uv_handle_t *handle) {
  uv_http_request_t *handle_req = (uv_http_request_t *)handle;
  log_info("[ %3d ] connection closed", 1);
  // uv_http_cleanup(handle_req);
}

void uv_http_init(uv_loop_t *loop, uv_http_t *handle_req, int req_id) {
  uv_http_parser_init(&handle_req->parse_req, NULL, NULL, NULL);
  uv_tcp_init(loop, &handle_req->handle);
  handle_req->handle.data = NULL;

  handle_req->id = req_id;

  // All contained types point to parent type, except for the tcp type since its
  // data property needs
  // to be used for other pointers, i.e. inside pipe_file.
  // However since handle_req and tcp point to same address we can upcast tcp to
  // handle_req when needed.
  handle_req->parse_req.data = (void *)handle_req;
}

int uv_http_request_init(uv_loop_t *loop, char *u, uv_http_request_t *req) {
  uv_http_parser_init(&req->parse_req, NULL, NULL, NULL);
  uv_url_data_t *url = url_parse(u);
  if (!url)
    return 0;

  req->url = url;
  req->loop = loop;
  req->headers = NULL; // ll_new();
  req->method = HTTP_GET;

  return 1;
}

void uv_http_request_cleanup(uv_http_request_t *req) {
  if (!req)
    return;

  url_free(req->url);
}

static int write_method(uv_http_request_t *req, char *buf) {
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

static int write_request(uv_http_request_t *req, char *buf) {

  int i = write_method(req, buf);

  int major = 1, minor = 1;
  if (req->major > -1)
    major = req->major;
  if (req->minor > -1)
    minor = req->minor;

  int ret =
      sprintf(buf + i, " %s HTTP/%d.%d\r\n", req->url->path, major, minor);
  if (ret < 0) {
    return -1;
  }
  i += ret;

  uv_http_header_t *header;

  ll_foreach(item, req->headers) {
    if (!item->data)
      continue;
    header = (uv_http_header_t *)item->data;
    int ret = sprintf(buf + i, "%s: %s\r\n", header->field, header->value);
    if (ret < 0) {
      return -1;
    }
    i += ret;
  }

  memcpy(buf + i, "\r\n", 2);
  i += 2;
  return i;
}

int uv_http_request(uv_stream_t *stream, uv_http_request_t *req,
                    uv_http_request_settings_t *settings) {

  if (!req)
    return 0;

  req->parse_req.on_parse_complete = settings->on_parse_complete;
  req->parse_req.on_headers_complete = settings->on_headers_complete;
  req->parse_req.on_data = settings->on_data;

  char buffer[500];
  uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));
  char message[500];
  buf.len = write_request(req, message);
  buf.base = message;

  uv_write_t write_req;
  stream->data = req;
  return uv_write(&write_req, stream, &buf, 1, on_write_end);
}
