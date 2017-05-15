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

uv_http_request_t req;

uv_http_request_init(loop, "http://libuv.org", &req);

uv_http_request_settings_t settings = {
    .on_headers_complete = on_headers_complete
};

uvext_http_request(&req, &settings);



```