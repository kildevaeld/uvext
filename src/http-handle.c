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
  uv_http_t *handle_req = (uv_http_t *)tcp;

  if (nread == UV_EOF) {
    uv_close((uv_handle_t *)tcp, NULL);

    debug("closed req tcp connection due to unexpected EOF");
  } else if (nread > 0) {
    log_info("[ %3d ] req (len %ld)", handle_req->id, nread);

    parsed = uv_http_parser_execute(&handle_req->parse_req, buf->base, nread);
    if (parsed < nread) {
      log_err("parsing http req");
      // uv_close((uv_handle_t *)&handle_req->handle, on_res_end);
    }

    uv_close((uv_handle_t *)handle_req, on_res_end);
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
  uv_read_start((uv_stream_t *)req->handle, alloc_cb, on_req_read);
}

static void uv_http_cleanup(uv_http_t *handle_req) {
  uv_http_cleanup_parse_req(&handle_req->parse_req);
}

static void on_res_end(uv_handle_t *handle) {
  uv_http_t *handle_req = (uv_http_t *)handle;
  log_info("[ %3d ] connection closed", handle_req->id);
  uv_http_cleanup(handle_req);
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