#include "heap.h"
#include <stdio.h>
#include <stdlib.h>

int Compare(ElementType a, ElementType b)
{
    return a->level <= b->level;
}

void PQueueNULLWarning()
{
    printf("Warning: Minimum Priority Queue is NULL");
}

void OutOfSpaceFatalError()
{
    printf("Fatal Error: Out of space");
    exit(-1);
}

MinPriorityQueue Initialize(int maxElements)
{
    MinPriorityQueue pqueue;
    if (maxElements <= 0) {
        printf("Fail to initialize: maxElements <= 0");
        exit(-1);
    }
     
    pqueue = malloc(sizeof(struct MinHeap));

    if (pqueue == NULL) {
        OutOfSpaceFatalError();
    }
     
    pqueue->size = 0;
    pqueue->capacity = maxElements;
    pqueue->elements = malloc(sizeof(ElementType) * (pqueue->capacity + 1));
    if (pqueue->elements == NULL){
        OutOfSpaceFatalError();
    }
    else {
        pqueue->elements[0] = (ElementType)malloc(sizeof(struct TCB));
        pqueue->elements[0]->level = 0;
    }
    return pqueue;
}

void Destroy(MinPriorityQueue pqueue)
{
    if (pqueue != NULL) {
        free(pqueue->elements);
        free(pqueue);
    }
}
 
void MakeEmpty(MinPriorityQueue pqueue)
{
    if (pqueue != NULL)
        pqueue->size = 0;
    else
        PQueueNULLWarning();
}

void Insert(MinPriorityQueue pqueue, ElementType x)
{
    int i;
    if (pqueue == NULL)
        PQueueNULLWarning();
     
    if (IsFull(pqueue))
    {
        printf("Fail to insert: Priority Queue is Full");
        return;
    }
    else
    {
        for (i = ++(pqueue->size); Compare(x, pqueue->elements[i / 2]); i /= 2)
            pqueue->elements[i] = pqueue->elements[i / 2]; 
        pqueue->elements[i] = x;
    }
}

ElementType DeleteMin(MinPriorityQueue pqueue)
{
    int i, child;
    ElementType min_element, last_element;
    if (pqueue == NULL){
        PQueueNULLWarning();
        return NULL;
    }
     
    if (IsEmpty(pqueue)) {
        printf("Fail to delete: Priority Queue is Empty");
        return NULL;
    }

    min_element = pqueue->elements[1];
    last_element = pqueue->elements[pqueue->size--];
    for (i = 1; i * 2 <= pqueue->size; i = child) {
        child = i * 2;
        
        if (child < pqueue->size && !Compare(pqueue->elements[child], pqueue->elements[child + 1])) {
            child++;
        }

        if (Compare(last_element, pqueue->elements[child])) {
            break;
        }
        else {
            pqueue->elements[i] = pqueue->elements[child];
        }
            
    }
    pqueue->elements[i] = last_element;
     
    return min_element; 
}

ElementType FindMin(MinPriorityQueue pqueue)
{
    if (pqueue == NULL) {
        PQueueNULLWarning();
        return NULL;
    }
    else {
        return pqueue->elements[1];
    }    
}

int IsEmpty(MinPriorityQueue pqueue)
{
    if (pqueue == NULL) {
        PQueueNULLWarning();
        return -1;
    } else {
        return (pqueue->size == 0);
    }
}
 
int IsFull(MinPriorityQueue pqueue)
{
    if (pqueue == NULL) {
        PQueueNULLWarning();
        return -1;
    } else {
        return (pqueue->size == pqueue->capacity);
    }
}
