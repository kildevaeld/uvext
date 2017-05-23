# UvExt

Extra handlers for [libuv](https://github.com/libuv/libuv)

- HTTP client (doing)
- HTTP Server (todo)
- TLS Client (todo)
- TLS Server (todo)
- HTTPS Client (todo)
- HTTPS Server (todo)


## HTTP

```c

static void on_headers_complete(uv_http_request_t *req) {
    printf("Headers parsed");
}

static void on_data() {

}

uv_http_client_t client;

uv_http_client_req_t req;
uv_http_request_init((uv_http_req_t *)&req);
req.method = HTTP_POST;
req.port = 3000;
req.host = "127.0.0.1";


uv_http_request_settings_t settings = {
    .on_headers_complete = on_headers_complete,
    .on_data = on_data
};

uv_http_client_init(loop, &client, &settings);
uv_http_client_req(&client, &req, oc);



```