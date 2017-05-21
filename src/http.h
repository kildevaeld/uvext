#ifndef UVEXT_SIMPLE_HTTP_H
#define UVEXT_SIMPLE_HTTP_H

#include "http-handle.h"

int uvext_http_request(uv_http_req_t *req, uv_http_req_settings_t *settings);

#endif