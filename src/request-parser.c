#include "request-parser.h"
#include "debug.h"
#include <assert.h>
#include <stdlib.h>

static int on_message_begin(http_parser *parser);
static int on_url(http_parser *parser, const char *hdr, size_t length);
static int on_header_field(http_parser *parser, const char *hdr, size_t length);
static int on_header_value(http_parser *parser, const char *hdr, size_t length);
static int on_body(http_parser *parser, const char *hdr, size_t length);
static int on_headers_complete(http_parser *parser);
static int on_message_complete(http_parser *parser);
static int on_chunk_header(http_parser *parser);
static int on_chunk_complete(http_parser *parser);

static char *strslice(const char *s, size_t len) {
  char *slice = (char *)malloc(sizeof(char) * (len + 1));
  strncpy(slice, s, len);
  slice[len] = '\0';
  return slice;
}

static int on_chunk_header(http_parser *parser) {
  printf("on chunk header\n");
  return 0;
}
static int on_chunk_complete(http_parser *parser) {
  printf("on chunk complete\n");
  return 0;
}

static int on_message_begin(http_parser *parser) {
  debug("message begin %d", parser->status_code);
  return 0;
}
static int on_url(http_parser *parser, const char *hdr, size_t length) {
  debug("url");
  return 0;
}
static int on_header_field(http_parser *parser, const char *hdr,
                           size_t length) {

  if (length == -1) {
    return 0;
  }

  uv_http_parse_req_t *req = (uv_http_parse_req_t *)parser->data;

  req->_c = malloc(sizeof(uv_http_header_t));

  if (req->_c == NULL) {
    log_err("memory");
    return 1;
  }

  strncpy(req->_c->field, hdr, length);
  req->_c->field[length] = '\0';
  return 0;
}
static int on_header_value(http_parser *parser, const char *hdr,
                           size_t length) {

  uv_http_parse_req_t *req = (uv_http_parse_req_t *)parser->data;

  strncpy(req->_c->value, hdr, length);
  req->_c->value[length] = '\0';

  if (req->headers == NULL)
    req->headers = ll_new(req->_c);
  else
    add_header(req->headers, req->_c);
  req->_c = NULL;

  return 0;
}
static int on_headers_complete(http_parser *parser) {
  uv_http_parse_req_t *req = (uv_http_parse_req_t *)parser->data;
  assert(req);

  req->status_code = parser->status_code;

  if (req->on_headers_complete)
    req->on_headers_complete(req);
  return 0;
}
static int on_message_complete(http_parser *parser) {
  uv_http_parse_req_t *req = (uv_http_parse_req_t *)parser->data;
  assert(req);

  if (req->on_parse_complete)
    req->on_parse_complete(req);
  return 0;
}
static int on_body(http_parser *parser, const char *hdr, size_t length) {
  uv_http_parse_req_t *req = (uv_http_parse_req_t *)parser->data;
  assert(req);

  if (req->on_data)
    req->on_data(req, hdr, length);
  return 0;
}

static http_parser_settings parser_settings = {
    .on_message_begin = on_message_begin,
    .on_url = on_url,
    .on_header_field = on_header_field,
    .on_header_value = on_header_value,
    .on_headers_complete = on_headers_complete,
    .on_message_complete = on_message_complete,
    .on_body = on_body,
    .on_chunk_header = on_chunk_header,
    .on_chunk_complete = on_chunk_complete};

void uv_http_parser_init(uv_http_parse_req_t *parse_req,
                         uv_http_parse_headers_complete_cb on_header_complete,
                         uv_http_parse_data_cb on_data,
                         uv_http_parse_complete_cb on_parse_complete) {

  http_parser_init(&parse_req->parser, HTTP_RESPONSE);

  parse_req->parser.data = parse_req;
  parse_req->on_headers_complete = on_header_complete;
  parse_req->on_data = on_data;
  parse_req->on_parse_complete = on_parse_complete;

  parse_req->headers = NULL;
  parse_req->_c = NULL;
}

int uv_http_parser_execute(uv_http_parse_req_t *parse_req, char *buf,
                           ssize_t nread) {
  return http_parser_execute(&parse_req->parser, &parser_settings, buf, nread);
}

void uv_http_cleanup_parse_req(uv_http_parse_req_t *parse_req) {

  uv_free_headers(parse_req->headers);
  // free(parse_req->url);
  // free(parse_req->header_line.field);
  // free(parse_req->header_line.value);
}