#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdbool.h>

typedef struct node {
  void *data;
  struct node *next;
  // struct node *prev;
} node_t;

#define ll_foreach(item, list)                                                 \
  for (node_t * (item) = (list); (item); (item) = (item)->next)

node_t *ll_new();

void ll_push(node_t *head, void *data);
void ll_unshift(node_t **head, void *data);
int ll_length(node_t *head);
void ll_free(node_t *head);
int ll_remove(node_t *head, int index);
void *ll_data(node_t *head, int index);
void ll_foreach_fn(node_t *head, void (*fn)(void *data, int index));
int ll_index_of(node_t *head, bool (*fn)(void *data, void *udata, int index),
                void *udata);

#endif