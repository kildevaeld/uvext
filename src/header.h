#ifndef UVEXT_HTTP_HEADER_H
#define UVEXT_HTTP_HEADER_H

#include "linkedlist.h"

#define MAX_HEADER 500

typedef struct uv_http_header_s {
  char field[MAX_HEADER];
  char value[MAX_HEADER];
} uv_http_header_t;

typedef node_t uv_http_header_list_t;

void add_header(uv_http_header_list_t *head, uv_http_header_t *header);
const char *get_header(uv_http_header_list_t *head, const char *field);
void uv_header_print(uv_http_header_list_t *head);
// void get_header(uv_http_header_t *headers, const char *field);

void uv_free_headers(uv_http_header_list_t *head);
#endif