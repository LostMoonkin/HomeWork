#include <dos.h>
#include "mt.h"
#include "heap.h"

extern MinPriorityQueue tcb_pqueue;
extern MinPriorityQueue tcb_list[NTCB];
extern TCBPointer task;
extern TCBPointer main_task;
extern int switch_method;
extern void interrupt (*OldInt8)(void);

int main(int argc, char **argv)
{
    int i, j;

    argc--;
    argv++;

    if (*((*argv) + 1) == 'H' || *(*(argv) + 1) == 'h') {
        switch_method = HPF;
    } else {
        switch_method = TS;
    }


    InitTCB();
    InitDOS();
    InitTextBuffer();
    OldInt8 = getvect(0x8);
    if (!(main_task = (TCBPointer)malloc(sizeof(struct TCB)))) {
        printf("could not malloc(%d) for MainTask !", sizeof(struct TCB));
        exit(-1);
    }
    if (!(main_task->name = (char *)malloc(sizeof(char) * 5))) {
        printf("could not malloc(%d) for MainTask name!", 5);
        exit(-1);
    }
    memset(main_task->name, '\0', sizeof(main_task->name));
    strcpy(main_task->name, "main");
    main_task->status = RUNNING;
    main_task->ss = _SS;
    main_task->sp = _SP;
    main_task->level = 1000;
    main_task->id = 0;
    *(tcb_list) = main_task;

    /* Task Scheduling*/

    CreateTask("f1", (code_ptr)f1, 1024, 5);
    CreateTask("f2", (code_ptr)f2, 1024, 2);
    CreateTask("f3", (code_ptr)f3, 1024, 1);
    CreateTask("f4", (code_ptr)f4, 1024, 4);

    ShowTCBStatus();
    task = main_task;
    setvect(0x8, NewInt8ForHPF);
    SelectSwitch();

    while (IsAllTaskFinished());

    /* Producer and Customer */

    setvect(0x8, OldInt8);
    CreateTask("producer", (code_ptr)Producer, 1024, 6);
    CreateTask("customer", (code_ptr)Customer, 1024, 6);

    ShowTCBStatus();

    task = main_task;
    setvect(0x8, NewInt8ForHPF);
    SelectSwitch();

    while (IsAllTaskFinished());

    setvect(0x8, OldInt8);
    CreateTask("sender", (code_ptr)Sender, 1024, 6);
    CreateTask("receiver", (code_ptr)Receiver, 1024, 6);

    ShowTCBStatus();
    task = main_task;
    setvect(0x8, NewInt8ForHPF);
    SelectSwitch();

    while (IsAllTaskFinished());
    ShowTCBStatus();

    printf("FINISHED!");
    setvect(0x8, OldInt8);
    return 0;
}