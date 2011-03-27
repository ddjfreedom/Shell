#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct l_node {
  void *val;
  struct l_node *next;
} l_node;

typedef l_node* linked_list;

linked_list linked_list_init();
void *l_remove(linked_list *list, int pos, int from_end);
void push(linked_list *stack, void *val);
void *pop(linked_list *stack);

#endif
