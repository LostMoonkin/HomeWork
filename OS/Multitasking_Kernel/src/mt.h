#ifndef _MULTI_TASK_H_
#define _MULTI_TASK_H_
#include "common.h"


enum {
    FINISHED = 0, RUNNING, READY, BLOCKED
};

enum {
    TS = 0, HPF
};

void InitTCB();

typedef int (far * code_ptr)(void);
int CreateTask(char *name, code_ptr code, int stack_len, int level);
void DestroyTask(void);
void Over(void);

void interrupt NewInt8ForHPF(void);

void interrupt SelectSwitch(void);
void interrupt SwitchTS(void);
void interrupt SWitchHPF(void);

TCBPointer FindNextAvaliableTask(void);

int IsAllTaskFinished(void);

void ShowTCBStatus(void);

/* Task Mutex */
void Block(TCBPointer *p);
void WakeUp(TCBPointer *p);
void p(struct Semaphore *sem);
void v(struct Semaphore *sem);

/* send and receive */
void InitTextBuffer(void);
BufferPointer GetFreeBuffer();
void PutBuffer(BufferPointer buf);
void InsertBuffer(BufferPointer *mq, BufferPointer buf);
BufferPointer Remove(BufferPointer *mq, int sender);
void Send(char *receiver, char *str, int size);
int Receive(char *sender, char *str);

/* Regs Define */

struct int_regs{
    unsigned bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flags,off,seg;
};

/* Manage DOS Flags */

#define GET_INDOS 0x34
#define GET_CRIT_ERR 0x5d06

void InitDOS(void);
int IsDOSBusy(void);


/* Task */
void f1(void);
void f2(void);
void f3(void);
void f4(void);

void Producer(void);
void Customer(void);

void Sender(void);
void Receiver(void);

#endif