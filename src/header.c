#include "header.h"
#include <stdio.h>
#include <stdlib.h>
void add_header(uv_http_header_list_t *head, uv_http_header_t *header) {
  ll_push(head, header);
}

static bool find(node_t *node, const char *field) { return false; }

const char *get_header(uv_http_header_list_t *head, const char *field) {

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