
#include <http-handle.h>
#include <http.h>
#include <stdlib.h>
#include <uvext.h>
void on_complete(uv_http_parser_t *req) { printf("parse complete\n"); }

void on_headers(uv_http_parser_t *req) {

  printf("headers complete\n");
  /*ll_foreach(item, req->headers) {
    uv_http_header_t *header = item->data;
    printf("  %s: %s\n", header->field, header->value);
  }*/
}

void on_header(uv_http_parser_t *req, const char *field, const char *value) {
  printf("header =  %s: %s\n", field, value);
}

void on_data(uv_http_parser_t *req, const char *data, size_t size) {
  char buf[size + 1];
  strncpy(buf, data, size);
  buf[size] = '\0';
  printf("data = '%s'\n", buf);
}

static uv_http_req_settings_t settings = {.on_parse_complete = on_complete,
                                          .on_data = on_data,
                                          .on_headers_complete = on_headers,
                                          .on_header_complete = on_header};

static void on_write_end(uv_write_t *req, int status) { printf("write end"); }

void on_connect(uv_connect_t *c, int status) {
  if (status < 0) {
    fprintf(stderr, "getaddrinfo callback error %s\n", uv_err_name(status));
    return;
  }

  uv_write_t r;
  uv_buf_t buf;
  char *msg = "1B\r\nHello, World! Hello, World!\r\n0\r\n\r\n";

  buf.base = msg;
  buf.len = strlen(msg) - 1;

  uv_http_request(c->handle, c->data, &settings);
  uv_write(&r, c->handle, &buf, 1, NULL);

  uv_http_request_end(c->handle, c->data);
}

static void oc(uv_http_client_t *client, int status) {
  if (status < 0) {
    fprintf(stderr, "getaddrinfo callback error %s\n", uv_err_name(status));
    return;
  }
  printf("on oc\n");

  char *msg = "Hello, World! Hello, World!";
  uv_buf_t buf;
  buf.base = msg;
  buf.len = strlen(msg);

  uv_http_client_write(client, &buf, NULL);

  msg = "Test Mig i øret";
  buf.base = msg;
  buf.len = strlen(msg);

  uv_http_client_write(client, &buf, NULL);

  uv_http_client_req_end(client);
}

int main() {

  uv_loop_t *loop = uv_default_loop();

  uv_http_client_req_t req;
  uv_http_request_init((uv_http_req_t *)&req);
  req.method = HTTP_POST;
  req.headers = uv_http_header_new();
  req.port = 3000;
  req.host = "127.0.0.1";
  // uv_http_header_set(req.headers, "Host", "google.com");
  uv_http_header_set(req.headers, "Host", "localhost:3000");
  uv_http_header_set(req.headers, "transfer-encoding", "chunked");
  uv_http_header_set(req.headers, "content-type", "text/plain");
  uv_http_header_set(req.headers, "connection", "close");

  uv_http_client_t client;
  uv_http_client_init(loop, &client, &settings);

  uv_http_client_req(&client, &req, oc);

  // uv_http_client_req();

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