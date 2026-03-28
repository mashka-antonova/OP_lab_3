#include "list.h"
#include <stdlib.h>
#include <string.h>

void insertAtHead(LinkedList* list, LinkedNode* newNode);
void insertAfter(LinkedList* list, LinkedNode* prevNode, LinkedNode* newNode);

LinkedList* initLinkedList(size_t dataSize) {

  LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));

  list->size = 0;
  list->dataSize = dataSize;
  list->head = NULL;
  list->tail = NULL;

  return list;
}

LinkedNode* createNode(LinkedList* list, const void* data) {
  LinkedNode* newNode = (LinkedNode*)malloc(sizeof(LinkedNode));
  if (newNode) {
    newNode->data = malloc(list->dataSize);
    if (newNode->data)
      memcpy(newNode->data, data, list->dataSize);
    newNode->next = NULL;
    newNode->prev = NULL;
  }
  return newNode;
}

void insertAtHead(LinkedList* list, LinkedNode* newNode) {
  if (list != NULL && newNode != NULL) {
    newNode->next = list->head;
    newNode->prev = NULL;

    if (list->head != NULL)
      list->head->prev = newNode;
    else
      list->tail = newNode;

    list->head = newNode;
  }
}

void insertAfter(LinkedList* list, LinkedNode* prevNode, LinkedNode* newNode) {
  if (prevNode != NULL && newNode != NULL) {
    newNode->next = prevNode->next;
    newNode->prev = prevNode;

    if (prevNode->next != NULL)
      prevNode->next->prev = newNode;

    else
      list->tail = newNode;

    prevNode->next = newNode;
  }
}

int insertSort(LinkedList* list, const void* data, int (*cmp)(const void*, const void*)) {
  int isCorrect = 0;
  LinkedNode* newNode = NULL;

  if (list && data && cmp && (newNode = createNode(list, data)) && newNode->data) {
    isCorrect = 1;
    if (!list->head || cmp(list->head->data, data) >= 0) {
      newNode->next = list->head;
      if (list->head)
        list->head->prev = newNode;
      else
        list->tail = newNode;
      list->head = newNode;
    } else {
        LinkedNode* cur = list->head;
        while (cur->next && cmp(cur->next->data, data) < 0)
          cur = cur->next;

        newNode->next = cur->next;
        newNode->prev = cur;

        if (cur->next)
          cur->next->prev = newNode;
        else
          list->tail = newNode;
        cur->next = newNode;
      }
      list->size++;
  }

  return isCorrect;
}

void clearList(LinkedList* list) {
  if (list) {
    LinkedNode* current = list->head;
    while (current) {
      LinkedNode* next = current->next;
      free(current->data);
      free(current);
      current = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
  }
}


void disposeList(LinkedList* list) {
  if (list) {
    clearList(list);
    free(list);
  }
}
