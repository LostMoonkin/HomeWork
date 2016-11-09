#ifndef _HEAP_H_
#define _HEAP_H_
#include "common.h"

int Compare(ElementType a, ElementType b);

void PQueueNULLWarning();
void OutOfSpaceFatalError();

MinPriorityQueue Initialize(int maxElements);

void Destroy(MinPriorityQueue pqueue);

void MakeEmpty(MinPriorityQueue pqueue);

void Insert(MinPriorityQueue pqueue, ElementType x);

ElementType DeleteMin(MinPriorityQueue pqueue);

ElementType FindMin(MinPriorityQueue pqueue);

int IsEmpty(MinPriorityQueue pqueue);

int IsFull(MinPriorityQueue pqueue);

#endif