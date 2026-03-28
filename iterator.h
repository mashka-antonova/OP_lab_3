#ifndef ITERATOR_H
#define ITERATOR_H

#include "list.h"

typedef struct Iterator {
    LinkedNode* current;
} Iterator;

Iterator begin(LinkedList* list);
int hasNext(const Iterator* it);
void next(Iterator* it);
void* get(const Iterator* it);
void* getNext(const Iterator* it);

#endif // ITERATOR_H
