#ifndef UVEXT_SIMPLE_HTTP_H
#define UVEXT_SIMPLE_HTTP_H

#include "http-handle.h"

int uvext_http_request(uv_http_request_t *req,
                       uv_http_request_settings_t *settings);

#endif