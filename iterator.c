#include "iterator.h"

Iterator begin(LinkedList* list) {
  Iterator it;
  it.current = NULL;

  if (list != NULL && list->head != NULL)
    it.current = list->head;

  return it;
}

int hasNext(const Iterator* it) {
  int res = 0;
  if (it != NULL && it->current != NULL)
    res = 1;
  return res;
}

void next(Iterator* it) {
  if (it != NULL && it->current != NULL)
    it->current = it->current->next;
}

void* get(const Iterator* it) {
  return (it && it->current) ? it->current->data : NULL;
}

void* getNext(const Iterator* it) {
  return (it && it->current && it->current->next) ? it->current->next->data : NULL;
}
