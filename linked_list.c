#include <stdlib.h>
#include "linked_list.h"

linked_list linked_list_init()
{
  linked_list list = malloc(sizeof(l_node));
  list->next = NULL;
  list->val = NULL;
  return list;
}
void *l_remove(linked_list *list, int pos, int frome_end)
{
  int len = 0, i;
  l_node *ptr, *del;
  void *val;
  ptr = (*list)->next;
  while (ptr) {
    len++;
    ptr = ptr->next;
  }
  if (frome_end) pos = len - pos - 1;
  if (pos < 0 || pos >= len)
    return NULL;
  for (i = 0, ptr = *list, del = ptr->next; i < pos; ++i) {
    del = del->next;
    ptr = ptr->next;
  }
  ptr->next = del->next;
  val = del->val;
  free(del);
  return val;
}
void push(linked_list *stack, void *val)
{
  l_node *new = malloc(sizeof(l_node));
  new->val = val;
  new->next = (*stack)->next;
  (*stack)->next = new;
}
void *pop(linked_list *stack)
{
  void *res = (*stack)->next->val;
  l_node *del = (*stack)->next;
  (*stack)->next = (*stack)->next->next;
  free(del);
  return res;
}
