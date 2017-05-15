
#include <http.h>
#include <uvext.h>

void on_complete(uv_http_parse_req_t *req) { printf("parse complete\n"); }

void on_header(uv_http_parse_req_t *req) {

  printf("Headers:\n");
  ll_foreach(item, req->headers) {
    uv_http_header_t *header = item->data;
    printf("  %s: %s\n", header->field, header->value);
  }
}

void on_data(uv_http_parse_req_t *req, const char *data, size_t size) {
  // printf("DATA %s\n", data);
}

void _on_connect(uv_connect_t *tcp, int status) {
  uv_http_request_t *req = tcp->handle->data;

  printf("connected\n");

  uv_http_request_settings_t settings = {.on_parse_complete = on_complete,
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
}

static uv_http_request_settings_t settings = {.on_parse_complete = on_complete,
                                              .on_data = on_data,
                                              .on_headers_complete = on_header};
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

  /*struct addrinfo hints;
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  uv_getaddrinfo_t resolver;*/

  uv_http_request_t req;
  // resolver.data = &req;

  uv_http_request_init(
      loop, "http://api.livejazz.dk/concert?access=mobile&page=1", &req);

  req.headers = uv_header_new("Content-Type", "application/json");
  uv_add_header(req.headers, "Host", "api.livejazz.dk");
  // uv_add_header(req.headers, "Content-Type", "text/plain");

  /*int r =
      uv_getaddrinfo(loop, &resolver, on_resolved, req.url->host, "80", &hints);
*/
  url_data_inspect(req.url);

  uvext_http_request(&req, &settings);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_free_headers(req.headers);
  uv_http_request_cleanup(&req);

  return 0;
}