
#include <http-handle.h>
#include <uvext.h>

void on_complete(uv_http_parser_t *req) { printf("parse complete\n"); }

void on_headers(uv_http_parser_t *req) {

  printf("headers complete:\n");
  /*ll_foreach(item, req->headers) {
    uv_http_header_t *header = item->data;
    printf("  %s: %s\n", header->field, header->value);
  }*/
}

void on_header(uv_http_parser_t *req, const char *field, const char *value) {
  printf("header =  %s: %s\n", field, value);
}

void on_data(uv_http_parser_t *req, const char *data, size_t size) {
  printf("DATA %s\n", data);
}
/*
void _on_connect(uv_connect_t *tcp, int status) {
  uv_http_req_t *req = tcp->handle->data;

  printf("connected\n");

  uv_http_req_settings_t settings = {.on_parse_complete = on_complete,
                                         .on_data = on_data};

  uv_http_request(tcp->handle, req, &settings);
}

void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res) {
  if (status < 0) {
    fprintf(stderr, "getaddrinfo callback error %s\n", uv_err_name(status));
    return;
  }

  char addr[17] = {'\0'};
  uv_ip4_name((struct sockaddr_in *)res->ai_addr, addr, 16);
  fprintf(stderr, "%s\n", addr);

  uv_connect_t *connect_req = (uv_connect_t *)malloc(sizeof(uv_connect_t));
  uv_tcp_t *socket = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(resolver->loop, socket);
  socket->data = resolver->data;
  uv_tcp_connect(connect_req, socket, (const struct sockaddr *)res->ai_addr,
                 _on_connect);

  uv_freeaddrinfo(res);
}*/

static uv_http_req_settings_t settings = {.on_parse_complete = on_complete,
                                          .on_data = on_data,
                                          .on_headers_complete = on_headers,
                                          .on_header_complete = on_header};

void on_connect(uv_connect_t *c, int status) {
  if (status < 0) {
    fprintf(stderr, "getaddrinfo callback error %s\n", uv_err_name(status));
    return;
  }

  uv_write_t r;
  uv_buf_t buf;
  char *msg = "1B\r\nHello, World! Hello, World!\r\n";

  buf.base = msg;
  buf.len = strlen(msg) - 1;

  uv_http_request(c->handle, c->data, &settings);
  uv_write(&r, c->handle, &buf, 1, NULL);

  uv_http_request_end(c->handle, c->data);
}

int main() {

  /*uv_loop_t *loop = uv_default_loop();

  // 	172.217.6.163

  uv_http_t client;

  // loop への登録
  uv_http_init(loop, &client, 1);
  // uv_tcp_init(loop, &client);

  // アドレスの取得
  struct sockaddr_in req_addr;
  uv_ip4_addr("172.217.6.163", 80, &req_addr);

  // TCP コネクション用の構造体
  uv_connect_t connect_req;

  // 接続
  uv_tcp_connect(&connect_req, (uv_tcp_t *)&client,
                 (const struct sockaddr *)&req_addr, on_connect);

  uv_run(loop, UV_RUN_DEFAULT);*/

  uv_loop_t *loop = uv_default_loop();

  uv_http_req_t req;
  uv_http_request_init(&req);
  req.method = HTTP_POST;
  req.headers = uv_http_header_new();
  uv_http_header_set(req.headers, "Host", "localhost:3000");
  uv_http_header_set(req.headers, "Transfer-Encoding", "chunked");
  uv_http_header_set(req.headers, "Content-Type", "text/plain");

  struct sockaddr_in req_addr;
  uv_ip4_addr("127.0.0.1", 3000, &req_addr);
  // uv_ip4_addr("172.217.6.163", 80, &req_addr);
  uv_tcp_t client;
  uv_tcp_init(loop, &client);

  // TCP コネクション用の構造体
  uv_connect_t connect_req;
  connect_req.data = &req;

  // 接続
  uv_tcp_connect(&connect_req, (uv_tcp_t *)&client,
                 (const struct sockaddr *)&req_addr, on_connect);

  uv_run(loop, UV_RUN_DEFAULT);

  // uv_http_header_free(req.headers);

  /*struct addrinfo hints;
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  uv_getaddrinfo_t resolver;*/

  // resolver.data = &req;

  /*uv_http_request_init(
      loop, "http://api.livejazz.dk/concert?access=mobile&page=1", &req);

  req.headers = uv_header_new("Content-Type", "application/json");
  uv_add_header(req.headers, "Host", "api.livejazz.dk");
  // uv_add_header(req.headers, "Content-Type", "text/plain");


  url_data_inspect(req.url);

  uvext_http_request(&req, &settings);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_free_headers(req.headers);
  uv_http_request_cleanup(&req);*/

  return 0;
}