#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void add_header(uv_http_header_list_t *head, uv_http_header_t *header) {
  ll_push(head, header);
}

void uv_add_header(uv_http_header_list_t *head, const char *field,
                   const char *value) {
  uv_http_header_t *header = malloc(sizeof(uv_http_header_t));

  if (!header) {
    printf("no mem\n");
    return;
  }

  strcpy(header->field, field);
  strcpy(header->value, value);

  ll_push(head, header);
}

static bool find(void *data, void *udata, int index) {
  const char *field = (const char *)udata;
  uv_http_header_t *header = (uv_http_header_t *)data;

  return strcmp(field, header->field) == 0;
}

const char *get_header(uv_http_header_list_t *head, const char *field) {
  if (!head)
    return NULL;
  uv_http_header_t *header = ll_data(head, ll_index_of(head, find, field));
  if (header)
    return header->value;
  return NULL;
}
void uv_free_headers(uv_http_header_list_t *head) {
  if (!head)
    return;
  uv_http_header_t *elm;
  ll_foreach(item, head) {
    elm = (uv_http_header_t *)item->data;
    // free(elm->field);
    // free(elm->value);
    free(elm);
  }
  ll_free(head);
}

void uv_header_print(uv_http_header_list_t *head) {
  uv_http_header_t *elm;

  ll_foreach(item, head) {
    elm = (uv_http_header_t *)item->data;
    if (!elm)
      printf("not elm");
    printf("%s: %s\n", elm->field, elm->value);
    // free(elm->field);
    // free(elm->value);
    // free(elm);
  }
}

uv_http_header_list_t *uv_header_new(const char *field, const char *value) {

  uv_http_header_t *header = malloc(sizeof(uv_http_header_t));

  strcpy(header->field, field);
  strcpy(header->value, value);

  return (uv_http_header_list_t *)ll_new(header);
}