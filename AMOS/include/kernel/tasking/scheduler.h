#ifndef _KERNEL_TASKING_SCHEDULER_H_
#define _KERNEL_TASKING_SCHEDULER_H_

#include <sys/types.h>
#include <kernel/tasking/task.h>

#define MAX_TASKS	255

struct tss 
{
       WORD previous_tasklink, reserved0;
       DWORD esp0;
       WORD ss0, reserved1;
       DWORD esp1;
       WORD ss1, reserved2;
       DWORD esp2;
       WORD ss2, reserved3;
       struct PAGE_DIRECTORY * cr3;
       DWORD eip, eflags;
       DWORD eax, ecx, edx, ebx;
       DWORD esp, ebp, esi, edi;
       WORD es, reserved4, cs, reserved5;
       WORD ss, reserved6, ds, reserved7;
       WORD fs, reserved8, gs, reserved9;
       WORD ldt, reserved10;
       WORD reserved11, io_map_base;
};

void scheduler_addTask( struct TASK_INFO * );

void scheduler_removeTask( struct TASK_INFO * );

void scheduler_init();

#endif