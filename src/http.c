#include "http.h"
#include "debug.h"
#include "header.h"
#include <stdbool.h>
#include <stdlib.h>

static void on_connect(uv_connect_t *connect, int status);
static void on_resolved(uv_getaddrinfo_t *resolver, int status,
                        struct addrinfo *res);

static void on_req_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf);

static int maybe_write_headers(uv_http_client_t *client);

#define UVERR(r, msg)                                                          \
  fprintf(stderr, "%s: [%s(%d): %s]\n", msg, uv_err_name((r)), r,              \
          uv_strerror((r)));

static void alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
  buf->base = malloc(size);
  buf->len = size;
}

static inline bool is_ip(const char *ipAddress) {
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
  return result != 0;
}

static void on_resolved(uv_getaddrinfo_t *resolver, int status,
                        struct addrinfo *res) {
  if (status < 0) {
    fprintf(stderr, "getaddrinfo callback error %s\n", uv_err_name(status));
    free(resolver);
    return;
  }

  struct addrinfo *tmp = res;

  uv_connect_t *connect_req = malloc(sizeof(uv_connect_t));

  uv_http_client_t *client = resolver->data;
  connect_req->data = client->data;
  client->data = NULL;
  uv_tcp_init(resolver->loop, (uv_tcp_t *)&client->handle);

  // Set host header
  if (client->req->headers) {
    if (!uv_http_header_get(client->req->headers, "host")) {
      uv_http_header_set(client->req->headers, "host", client->req->host);
    }
  }

  int rc;
  char str[INET_ADDRSTRLEN];

  while (tmp) {

    inet_ntop(AF_INET, &(tmp->ai_addr), str, INET_ADDRSTRLEN);
    debug("connecting to %s on %s", client->req->host, str);
    rc = uv_tcp_connect(connect_req, (uv_tcp_t *)client,
                        (const struct sockaddr *)tmp->ai_addr, on_connect);
    if (!rc) {
      break;
    }

    printf("could not connect %s", uv_err_name(rc));
    tmp = tmp->ai_next;
  }

  uv_freeaddrinfo(res);
  free(resolver);
}

static void on_connect(uv_connect_t *connect, int status) {
  if (status < 0) {
    fprintf(stderr, "connect callback error %s\n", uv_err_name(status));
    goto connect_end;
  }

  uv_http_client_t *client = (uv_http_client_t *)connect->handle;

  if (connect->data) {
    uv_http_client_connect_cb cb = connect->data;
    cb(client, status);
    goto connect_end;
  }

  uv_http_request_end(connect->handle, (uv_http_req_t *)client->req);

connect_end:
  free(connect);
}

static inline bool is_chunked(uv_http_header_t *headers) {

  if (headers) {
    const char *t = uv_http_header_get(headers, "transfer-encoding");
    if (t && strcmp(t, "chunked") == 0) {
      return true;
    }
  }
  return false;
}

static void on_write_end(uv_write_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "error on_write_end");
    goto on_write_end_end;
  }
  if (req->data) {
    uv_write_cb cb = req->data;
    cb(req, status);
  }

on_write_end_end:
  free(req);
}

static int maybe_write_headers(uv_http_client_t *client) {

  if (client->headers_sent) {
    return 0;
  }

  uv_write_t *w = malloc(sizeof(uv_write_t));
  client->headers_sent = true;
  debug("writing headers to: %s", client->req->host);
  return uv_http_req_write_headers(
      w, &client->handle, (uv_http_req_t *)client->req, (uv_write_cb)free);
}

static void on_req_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
  size_t parsed;
  uv_http_client_t *handle = (uv_http_client_t *)tcp;
  uv_http_client_req_t *req = handle->req;
  if (nread == UV_EOF) {
    uv_http_parser_execute(&req->parser, buf->base, 0);
    uv_close((uv_handle_t *)tcp, NULL);
  } else if (nread > 0) {

    parsed = uv_http_parser_execute(&req->parser, buf->base, nread);

    if (parsed < nread) {
      const char *nam = http_errno_name(HTTP_PARSER_ERRNO(&req->parser.parser));
      const char *err =
          http_errno_description(HTTP_PARSER_ERRNO(&req->parser.parser));

      log_err("parsing http req  %s: %s", nam, err);
      uv_close((uv_handle_t *)tcp, NULL);
    }
  } else {
    UVERR((int)nread, "reading req req");
  }
  if (buf->base)
    free(buf->base);
}

void uv_http_client_init(uv_loop_t *loop, uv_http_client_t *client,
                         uv_http_req_settings_t *settings) {
  client->settings = settings;
  client->req = NULL;
  client->loop = loop;
}

int uv_http_client_req(uv_http_client_t *client, uv_http_client_req_t *req,
                       uv_http_client_connect_cb cb) {

  client->req = req;
  req->parser.on_parse_complete = client->settings->on_parse_complete;
  req->parser.on_headers_complete = client->settings->on_headers_complete;
  req->parser.on_data = client->settings->on_data;
  req->parser.on_header_complete = client->settings->on_header_complete;

  // TODO: Handle IP6
  if (is_ip(req->host)) {
    uv_tcp_init(client->loop, (uv_tcp_t *)&client->handle);

    struct sockaddr_in req_addr;

    uv_ip4_addr(req->host, req->port, &req_addr);

    uv_connect_t *con = malloc(sizeof(uv_connect_t));
    con->data = cb;

    return uv_tcp_connect(con, (uv_tcp_t *)client,
                          (const struct sockaddr *)&req_addr, on_connect);
  } else {
    struct addrinfo hints;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    uv_getaddrinfo_t *resolver = malloc(sizeof(uv_getaddrinfo_t));

    resolver->data = client;
    client->data = cb;

    char buf[20];
    sprintf(buf, "%i", req->port);
    return uv_getaddrinfo(client->loop, resolver, on_resolved, req->host, buf,
                          &hints);
  }

  return 0;
}

int uv_http_client_write(uv_http_client_t *client, uv_buf_t *buf,
                         uv_write_cb cb) {

  int rc;
  if ((rc = maybe_write_headers(client)) != 0)
    return rc;

  uv_http_method_t m = client->req->method;
  if (m != HTTP_POST && m != HTTP_PUT) {
    return 220;
  }
  uv_http_header_t *headers = client->req->headers;
  bool isc = is_chunked(headers);

  uv_write_t *write = malloc(sizeof(uv_write_t));
  write->data = cb;

  if (isc) {
    char str[18 + buf->len + 2];
    int i = sprintf(str, "%x\r\n", (int)buf->len);
    int l = i + buf->len + 2;
    memcpy(str + i, buf->base, buf->len);
    str[l - 2] = '\r';
    str[l - 1] = '\n';
    uv_buf_t buffer = uv_buf_init(str, l);
    return uv_write(write, (uv_stream_t *)client, &buffer, 1, on_write_end);
  }
  return uv_write(write, (uv_stream_t *)client, buf, 1, on_write_end);
}

int uv_http_client_req_end(uv_http_client_t *client) {

  int rc;
  if ((rc = maybe_write_headers(client)) != 0)
    return rc;

  uv_http_method_t m = client->req->method;
  if (m == HTTP_POST || m == HTTP_PUT) {
    uv_http_header_t *headers = client->req->headers;
    bool isc = is_chunked(headers);

    if (isc) {
      uv_buf_t buf;
      buf.base = "0\r\n\r\n";
      buf.len = 5;
      uv_write_t *write = malloc(sizeof(uv_write_t));
      write->data = NULL;

      int rc = uv_write(write, (uv_stream_t *)client, &buf, 1, on_write_end);
      if (rc != 0) {
        free(write);
        return rc;
      }
    }
  }

  return uv_read_start((uv_stream_t *)client, alloc_cb, on_req_read);
}
