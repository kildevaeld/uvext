
#include <uvext.h>

int main() {

  uv_loop_t *loop = uv_default_loop();

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

  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}