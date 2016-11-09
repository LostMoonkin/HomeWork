#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "mt.h"
#include "heap.h"

/* #define _DEBUG_ */

/* Define the global variable */
void interrupt (*OldInt8)(void);
char far *indos_ptr = 0;
char far *crit_err_ptr = 0;
int time_count = 0;
int is_main_running = 0;
int current_task = 0;
int switch_method = TS;
int in_buffer = 0, out_buffer = 0;
int category_buffer[NBUF];

MinPriorityQueue tcb_pqueue;
TCBPointer tcb_list[NTCB];
TCBPointer task;
TCBPointer main_task;

struct Semaphore mutex = {1, NULL};
struct Semaphore empty = {NBUF, NULL};
struct Semaphore full = {0, NULL};
struct Semaphore mutexfb = {1, NULL};
struct Semaphore sfb = {NBUF, NULL};

struct Buffer *text_buffer;

/* func implement */

void InitTCB()
{
    tcb_pqueue = Initialize(NTCB * 2);
    memset(tcb_list, 0, sizeof(tcb_list));
}

void InitDOS()
{
    union REGS regs;
    struct SREGS seg_regs;

    regs.h.ah = GET_INDOS;
    intdosx(&regs, &regs, &seg_regs);
    indos_ptr = MK_FP(seg_regs.es, regs.x.bx);
    if (_osmajor < 3) {
        crit_err_ptr = indos_ptr + 1;
    } else if (_osmajor == 3 && _osminor == 0) {
        crit_err_ptr = indos_ptr - 1;
    } else {
        regs.x.ax = GET_CRIT_ERR;
        intdosx(&regs, &regs, &seg_regs);
        crit_err_ptr = MK_FP(seg_regs.ds, regs.x.si);
    }

}

int IsDOSBusy()
{
    if (indos_ptr && crit_err_ptr) {
        return (*indos_ptr || *crit_err_ptr);
    } else {
        return -1;
    }
}

int CreateTask(char *name, code_ptr code, int stack_len, int level) 
{
    struct int_regs *regs_pt;
    int i;


    task = (ElementType)malloc(sizeof(struct TCB));
    if (!(task->stack = (char *)malloc(sizeof(char) * stack_len))) {
        printf("could not malloc(%d) for task stack!", stack_len);
        return -1;
    }
    
    regs_pt = (struct int_regs *)(task->stack + stack_len) - 1;
    regs_pt->cs = FP_SEG(code);
    regs_pt->ip = FP_OFF(code);
    regs_pt->flags = 0x200;
    regs_pt->ds = _DS;
    regs_pt->es = _DS;
    regs_pt->seg = FP_SEG(Over);
    regs_pt->off = FP_OFF(Over);

    if (!(task->name = (char *)malloc(sizeof(char) * strlen(name)))) {
        printf("could not malloc(%d) for task name", strlen(name));
        return -1;
    }
    memset(task->name, '\0', sizeof(task->name));
    strcpy(task->name, name);
    task->status = READY;
    task->next = NULL;
    task->ss = FP_SEG(regs_pt);
    task->sp = FP_OFF(regs_pt);
    task->old_level = task->level = level;
    task->mutex = (struct Semaphore *)malloc(sizeof(struct Semaphore));
    task->sm = (struct Semaphore *)malloc(sizeof(struct Semaphore));
    task->mutex->value = 1;
    task->mutex->wq = NULL;
    task->sm->value = 0;
    task->sm->wq = NULL;
    task->mq = NULL;

    Insert(tcb_pqueue, task);
    for (i = 1; i < NTCB; i++) {
        if (tcb_list[i] == NULL) {
            break;
        }
    }

    task->id = i;
    tcb_list[i] = task;

    printf("The thread %s has been created! level: %d\n", task->name, level);
}

void DestroyTask()
{
    if (task == NULL || task->status == FINISHED) {
        return;
    }
    disable();

    printf("Destroy Task:%s\n", task->name);

    if (task->stack) {
        free(task->stack);
    }
    task->stack = NULL;
    /*
    if (task->name) {
        free(task->name);
    }
    task->name = NULL;
    */
    task->status = FINISHED;
    /* free(task); */
    task = NULL;
}

void Over()
{
    DestroyTask();
    SelectSwitch();
    enable();
}

void interrupt SelectSwitch()
{
    if (switch_method == TS) {
        SwitchTS();
    } else if (switch_method == HPF) {
        SWitchHPF();
    }
}

TCBPointer FindNextAvaliableTask()
{
    int i = current_task + 1;
    for (; i <= NTCB; i++) {
        if (i == NTCB) {
            i = 0;
        }
        if (tcb_list[i]->status == READY) {
            current_task = i;
            return tcb_list[i];
        }
    }
    return NULL;
}

void interrupt SwitchTS() 
{
    TCBPointer temp_task;

    time_count = 0;
    disable();
    task->ss = _SS;
    task->sp = _SP;
    if (task->status == RUNNING) {
        task->status = READY;
    }
    temp_task = FindNextAvaliableTask();

    if (temp_task == NULL) {
        enable();
        return;
    }

    /* Debug!!!!!! */
    #ifdef _DEBUG_
    printf("\ntemp_task:%s  %d   %d\n", temp_task->name, temp_task->level, temp_task->status);
    printf("\ntask:%s    %d\n", task->name, task->level);
    #endif

    _SS = temp_task->ss;
    _SP = temp_task->sp;
    temp_task->status = RUNNING;
    task = temp_task;

    enable();

}

void interrupt SWitchHPF()
{
    int switch_method = 0;
    TCBPointer temp_task = NULL;
    disable();
    time_count = 0;
    if (is_main_running >= NTCB || IsEmpty(tcb_pqueue)) {
        temp_task = main_task;
        is_main_running = 0;
        switch_method = 1;
    } else {
        temp_task = DeleteMin(tcb_pqueue);
        if (task == NULL) {
            switch_method = 2;
        } else {
            switch_method = 3;
        }
    }

    /* Debug!!!!!! */
    #ifdef _DEBUG_
    printf("\nswitch method: %d\n", switch_method);
    printf("\ntemp_task:%s  %d   %d\n", temp_task->name, temp_task->level, temp_task->status);
    printf("\ntask:%s    %d\n", task->name, task->level);
    #endif

    if (switch_method) {
        if (switch_method == 3) {
            if (temp_task->level >= task->level) {
                Insert(tcb_pqueue, temp_task);
                enable();
                return;
            }
        }
        if (temp_task->status == BLOCKED) {
            Insert(tcb_pqueue, temp_task);
            enable();
            return;
        }
        if (task != NULL) {
            task->ss = _SS;
            task->sp = _SP;
            task->status = READY;
            if (strcmp(task->name, "main")) {
                Insert(tcb_pqueue, task);
            }
        }
        task = temp_task;
        temp_task = NULL;
        _SS = task->ss;
        _SP = task->sp;
        task->status = RUNNING;
    }
    enable();
}

int IsAllTaskFinished()
{
    TCBPointer p;
    int i;

    if (switch_method == HPF) {
        if (IsEmpty(tcb_pqueue)) {
            return 0;
        }
        return -1;
    } else {
        for (i = 1; i < NTCB; i++) {
            p = tcb_list[i];
            if (p) {
                if (p->status > FINISHED && p->status <= BLOCKED) {
                    return -1;
                }   
            }
        }
        return 0;
    }

}

void interrupt NewInt8ForHPF()
{
    (*OldInt8)();
    time_count++;
    if(time_count >= TL){
        is_main_running++;
        if(!IsDOSBusy()) {
            SelectSwitch();
        }
    }
}

void ShowTCBStatus()
{
    int id;
    printf("\n  **** The current Task's state ****\n");
    for(id = 0; id < NTCB; id++)
    {
        TCBPointer temp_task = tcb_list[id];
        
        if (temp_task) {
            printf("Task%d %9s state is ", id, temp_task->name);
            switch(temp_task->status)
            {
                case FINISHED: puts("FINISHED"); break;
                case RUNNING: puts("RUNNING"); break;
                case READY: puts("READY"); break;
                case BLOCKED: puts("BLOCKED"); break;
            }
        }
    }
    printf("\n");
}

void Block(TCBPointer *p)
{
    TCBPointer temp_ptr;
    disable();
    task->status = BLOCKED;
    task->old_level = task->level;
    task->level = 1000;
    printf("%s  has blocked!\n", task->name);

    if ((*p) == NULL) {
        (*p) = task;
    } else {
        temp_ptr = (*p);
        while (temp_ptr->next) {
            temp_ptr = temp_ptr->next;
        }
        temp_ptr->next = task;
    }

    enable();
    SelectSwitch();
}

void WakeUp(TCBPointer *p)
{
    TCBPointer temp_ptr;
    if (!(*p)) {
        return;
    }
    disable();
    temp_ptr = (*p);
    printf("%s   has wake up!\n", temp_ptr->name);
    if (temp_ptr) {
        temp_ptr->status = READY;
        temp_ptr->level = temp_ptr->old_level;
    }
    (*p) = (*p)->next;
    temp_ptr->next = NULL;
    enable();
}

void p(struct Semaphore *sem)
{
    TCBPointer *qp;
    disable();
    sem->value--;
    if (sem->value < 0) {
        qp = &(sem->wq);
        Block(qp);
    }

    enable();
}

void v(struct Semaphore *sem)
{
    TCBPointer *qp;
    disable();
    (sem->value)++;
    if(sem->value <= 0) {
        qp = &(sem->wq);
        WakeUp(qp);
    }
    enable();
}


/* Text Buffer */
void InitTextBuffer(void)
{
    BufferPointer buf_ptr, new_ptr;
    int i;
    buf_ptr = (BufferPointer)malloc(sizeof(struct Buffer));
    buf_ptr->sender = -1;
    buf_ptr->size = 0;
    memset(buf_ptr->text, '\0', sizeof(buf_ptr->text));
    text_buffer = new_ptr = buf_ptr;
    for (i = 1; i < NBUF; i++) {
        new_ptr = (BufferPointer)malloc(sizeof(struct Buffer));
        new_ptr->sender = -1;
        new_ptr->size = 0;
        memset(new_ptr->text, '\0', sizeof(new_ptr->text));
        buf_ptr->next = new_ptr;
        buf_ptr = new_ptr;
    }
    buf_ptr->next = NULL;
}

BufferPointer GetFreeBuffer()
{
    BufferPointer buf;
    buf = text_buffer;
    text_buffer = text_buffer->next;
    return buf;
}

void PutBuffer(BufferPointer buf)
{
    BufferPointer buf_ptr = text_buffer;
    
    if (text_buffer == NULL) {
        text_buffer = buf;
    } else {
        while (buf_ptr->next) {
            buf_ptr = buf_ptr->next;
        }
        buf_ptr->next = buf;
    }
    buf->next = NULL;
}

void InsertBuffer(BufferPointer *mq, BufferPointer buf)
{
    BufferPointer buf_ptr;

    if(buf == NULL)
        return;

    if((*mq) == NULL) {
        (*mq) = buf;
    } else {
        buf_ptr = (*mq);
        while(buf_ptr->next != NULL)
            buf_ptr = buf_ptr->next;
        buf_ptr->next = buf;
    }
    buf->next = NULL;
}

BufferPointer Remove(BufferPointer *mq, int sender)
{
    BufferPointer buf_ptr, buf;
    buf_ptr = (*mq);
    
    if(buf_ptr->sender == sender) {
        buf = buf_ptr;
        (*mq) = buf->next;  
        buf->next = NULL;
        return buf;
    }
    
    while(buf_ptr->next != NULL && buf_ptr->next->sender != sender) {
        buf_ptr = buf_ptr->next;
    }

    if(buf_ptr->next == NULL) {
        return NULL;
    }
    buf = buf_ptr->next;
    buf_ptr->next = buf->next;  
    buf->next = NULL;
    return buf;  
}

void Send(char *receiver, char *str, int size)
{
    BufferPointer buf;
    int i, id;
    disable(); 

    for(id = 0; id < NTCB; id++)
    {
        if(!strcmp(receiver, tcb_list[id]->name)) {
            break;
        }
    }
    if(id == NTCB)  
    {
        enable();
        return;
    }

    p(&sfb);  
    p(&mutexfb);  
    buf = GetFreeBuffer();
    v(&mutexfb);

    buf->sender = task->id;  
    buf->size = size;
    for(i = 0; i < buf->size; i++, str++)
        buf->text[i] = *str; 
    
    p(tcb_list[id]->mutex);
    InsertBuffer(&(tcb_list[id]->mq), buf);
    v(tcb_list[id]->mutex);
    v(tcb_list[id]->sm);  
    enable();  
}

int Receive(char *sender, char *str)
{
    BufferPointer buf;
    int i, id, size;
    disable();  
   
    for(id = 0; id < NTCB; id++)
    {
        if(strcmp(sender, tcb_list[id]->name) == 0)   
            break;
    }
    if(id == NTCB)  
    {
        enable(); 
        return -1;
    }
    
    p(task->sm);
    p(task->mutex);
    buf = Remove(&(task->mq), id);
    v(task->mutex);
    
    if(buf == NULL)
    {
        v(task->sm);
        enable();
        return 0;
    }

    size = buf->size;
    for(i = 0; i < buf->size; i++, str++)
        *str = buf->text[i];

    buf->sender = -1;
    buf->size = 0;
    buf->text[0] = '\0';

    p(&mutexfb);
    PutBuffer(buf);
    v(&mutexfb);
    v(&sfb);
    enable();   
    return size;  
}



/* Task */
void f1()
{
    int i,j,k;
    for(i = 0; i < 40;i++){
        putchar('a');
        for(j = 0;j < 1000; j++)
            for(k = 0;k < 100; k++);
    }
}

void f2()
{
    int i,j,k;
    for(i = 0; i < 10;i++){
        putchar('b');
        for(j = 0;j < 1000; j++)
            for(k = 0;k < 100; k++);
    }
}

void f3()
{
    int i,j,k;
    for(i = 0; i < 10;i++){
        putchar('c');
        for(j = 0;j < 1000; j++)
            for(k = 0;k < 100; k++);
    }
}

void f4()
{
    int i,j,k;
    for(i = 0; i < 10;i++){
        putchar('d');
        for(j = 0;j < 1000; j++)
            for(k = 0;k < 100; k++);
    }
}

void Producer(void)
{
    int i = 0, j, k, data;

    for (; i < NPC; i++) {
        data = i * i;
        p(&empty);
        p(&mutex);
        category_buffer[in_buffer] = data;
        in_buffer = (in_buffer + 1) % NBUF;
        printf("producer produces %d data\n", data);
        v(&mutex);
        v(&full);
    }
}

void Customer(void)
{
    int i = 0, j, k, data;

    for (; i < NPC; i++) {
        p(&full);
        p(&mutex);
        data = category_buffer[out_buffer];
        printf("customer get %d data\n", data);
        out_buffer = (out_buffer + 1) % NBUF;
        v(&mutex);
        v(&empty);
    }
}

void Sender()
{
    int i, j, k;
    char message[10] = "message";
    for(i = 0; i < NMESSAGE; i++)
    {
        message[7] = i + '0';
        message[8] = message[9] = '\0';
        Send("receiver", message, strlen(message));
        printf("message %s has been sent.\n", message);
    }
    Receive("receiver", message);
    if(message[0] == 'O' && message[1] == 'K') {
        printf("All the message have been received,Successful!\n");
    } else {
        printf("Not all the message have been received,failed!\n");
    }
}

void Receiver()
{
    int i, j, k;
    int size;
    char message[10];
    for(i = 0; i < NMESSAGE; i++)
    {
        memset(message, 0, sizeof(message));
        while((size = Receive("sender", message)) == 0);
        printf("message %s has been received.\n", message);
    }
    strcpy(message, "OK\0");
    Send("sender", message, strlen(message)); 
}