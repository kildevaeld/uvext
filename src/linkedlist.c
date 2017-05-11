#include "linkedlist.h"
#include <stdlib.h>

node_t *ll_new(void *data) {
  node_t *head = malloc(sizeof(node_t));
  head->data = data;
  head->next = NULL;
  return head;
}

void ll_push(node_t *head, void *data) {
  node_t *new_node = malloc(sizeof(node_t));
  new_node->next = NULL;
  new_node->data = data;
  while (head->next != NULL) {
    head = head->next;
  }
  head->next = new_node;
}

void ll_unshift(node_t **head, void *data) {
  node_t *new_node = malloc(sizeof(node_t));
  new_node->data = data;
  new_node->next = *head;
  *head = new_node;
}

int ll_length(node_t *head) {
  node_t *n = head;
  int c;
  while (n != NULL) {
    n = n->next;
    c++;
  }
  return c;
}

void ll_free(node_t *head) {
  node_t *n = head->next;
  while (n != NULL) {
    node_t *nn = n->next;
    free(nn);
    n = nn;
  }
  free(head);
}

void ll_foreach_fn(node_t *head, void (*fn)(void *data, int index)) {
  int index = 0;
  while (head != NULL) {
    fn(head->data, index++);
    head = head->next;
  }
}

int ll_remove(node_t *head, int index) {
  int i = 0, ret = -1;
  node_t *current = head;

  for (int i = 0; i < index - 1; i++) {
    if (current->next == NULL)
      return ret;
    current = current->next;
  }

  node_t *tmp = current->next;
  current->next = tmp->next;
  free(tmp);
  return 0;
}

void ll_data(node_t *head, int index) {}

int ll_index_of(node_t *head, bool (*fn)(void *data, int index)) {
  int index = 0;
  while (head != NULL) {
    if (fn(head->data, index++)) {
      return index;
    }
    head = head->next;
  }
  return -1;
}