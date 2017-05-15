#include "http.h"

static void _on_connect(uv_connect_t *tcp, int status) {
  uv_http_request_t *req = tcp->handle->data;

  printf("connected\n");

  if (!get_header(req->headers, "Host")) {
    if (!req->headers)
      // FIXME: LEAK
      req->headers = uv_header_new("Host", req->url->host);
    else
      uv_add_header(req->headers, "Host", req->url->host);
  }

  uv_http_request(tcp->handle, req, req->data);
  free(tcp);
}

static void on_resolved(uv_getaddrinfo_t *resolver, int status,
                        struct addrinfo *res) {
  if (status < 0) {
    fprintf(stderr, "getaddrinfo callback error %s\n", uv_err_name(status));
    free(resolver);
    return;
  }

  struct addrinfo *tmp = res;

  uv_connect_t *connect_req = (uv_connect_t *)malloc(sizeof(uv_connect_t));
  uv_tcp_t *socket = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(resolver->loop, socket);
  socket->data = resolver->data;

  int rc;
  while (tmp) {

    rc = uv_tcp_connect(connect_req, socket,
                        (const struct sockaddr *)tmp->ai_addr, _on_connect);
    if (!rc) {
      break;
    }

    printf("could not connect %s", uv_err_name(rc));
    tmp = tmp->ai_next;
  }

  /*char addr[17] = {'\0'};
  uv_ip4_name((struct sockaddr_in *)res->ai_addr, addr, 16);
  fprintf(stderr, "%s\n", addr);*/

  uv_freeaddrinfo(res);
  free(resolver);
}

int uvext_http_request(uv_http_request_t *req,
                       uv_http_request_settings_t *settings) {

  struct addrinfo hints;
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  uv_getaddrinfo_t *resolver = malloc(sizeof(uv_getaddrinfo_t));

  resolver->data = req;
  req->data = settings;

  return uv_getaddrinfo(req->loop, resolver, on_resolved, req->url->host, "80",
                        &hints);
}