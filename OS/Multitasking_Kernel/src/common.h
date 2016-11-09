#ifndef _COMMON_H_
#define _COMMON_H_

/* Define */

#define NTCB 10
#define TL 1
#define NBUF 5
#define NPC 20
#define NTEXT 20
#define NMESSAGE 10

/* Task Define */

typedef struct Semaphore semaphore;
typedef struct TCB tcb;
typedef struct Buffer buffer;

struct TCB{
    int id;
    char *name;
    int status;
    unsigned level;
    unsigned old_level;
    unsigned char *stack;
    unsigned ss;
    unsigned sp;

    struct TCB * next;

    semaphore *mutex;
    semaphore *sm;

    buffer *mq;
};

struct Semaphore {
    int value;
    tcb **wq;
};

typedef struct TCB *TCBPointer;

/* Text Buffer Define */

struct Buffer {
    int sender;
    int size;
    char text[NTEXT];
    struct Buffer * next;
};

typedef struct Buffer *BufferPointer;

/* Min Heap Define */

typedef TCBPointer ElementType;

struct MinHeap {
    int capacity;
    int size;
    ElementType *elements; 
};

typedef struct MinHeap *MinPriorityQueue;

#endif