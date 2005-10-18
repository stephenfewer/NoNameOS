#include <kernel/tasking/tasking.h>
#include <kernel/isr.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>
#include <kernel/gdt.h>

DWORD current_esp = 0x00000000;

DWORD tasking_ticks = 0;

struct task * tasking_currentTask = NULL;

struct task * tasking_queue[MAX_TASKS];

void ThreadTest1()
{
 unsigned char* VidMemChar = (unsigned char*)0xB8000;
 *VidMemChar='1';
 for(;;)*VidMemChar++;
}

void ThreadTest2()
{
 unsigned char* VidMemChar = (unsigned char*)0xB8004;
 *VidMemChar='1';
 for(;;)*VidMemChar++;
}

struct task * tasking_create( int id, void (*thread)() )
{
 DWORD * stack;
 struct task * tasking_newTask;
 
 tasking_newTask = mm_malloc( sizeof( struct task ) );
 
 tasking_newTask->id = id;
 
 tasking_newTask->tick_slice = 1;
 
 tasking_newTask->page_dir = paging_getCurrentPageDir();
 
 stack = (DWORD *)(mm_malloc( 4096 ) + 4096 ); //This allocates 4kb of memory, then puts the pointer at the end of it

 // First, this stuff is pushed by the processor
 *--stack = 0;		// ss
 *--stack = 0;		// user stack
 *--stack = 0x0202; // This is EFLAGS
 *--stack = 0x08;   // This is CS, our code segment
 *--stack = (DWORD)thread; // This is EIP
 
 // interrupt bytes
 *--stack = 0;		// error code
 *--stack = 32;		// int number
 
 // Next, the stuff pushed by 'pushad'
 *--stack = 0; //EAX
 *--stack = 0; //ESI
 *--stack = 0; //EBP
 *--stack = 0; //Just an offset, no value
 *--stack = 0; //EBX
 *--stack = 0; //EDX
 *--stack = 0; //ECX
 *--stack = 0; //EAX
 
 // Now these are the data segments pushed by the IRQ handler
 *--stack = 0x10; //DS
 *--stack = 0x10; //ES
 *--stack = 0x10; //FS
 *--stack = 0x10; //GS
 
 tasking_newTask->esp = (DWORD)stack; //Update the stack pointer
 
 tasking_queue[ tasking_newTask->id ] = tasking_newTask;
 
 return tasking_newTask;
}

DWORD tasking_schedule( struct REGISTERS * reg )
{
	kernel_lock();
	
	tasking_ticks++;
	
	if( tasking_ticks >= 12 )
		while(TRUE);
	
	if( tasking_currentTask == NULL )
	{
		// To-Do: create a kernel task 0 with current_esp
		tasking_currentTask = tasking_queue[ 0 ];
		kprintf("first task switch: current_esp = %x\n", current_esp );
		kernel_unlock();
		return tasking_currentTask->esp;
	}
	
	kprintf("Timer: [%d] ticks = %d  current_esp = %x\n", tasking_currentTask->id, tasking_ticks, current_esp );
	
	tasking_currentTask->esp = current_esp;
	
	// if the current task has reached the end of its tick slice
	// we must switch to a new task
	if( tasking_currentTask->tick_slice <= 0 )
	{
		// get the next task to switch to in a round robin fashine
		if( tasking_queue[ tasking_currentTask->id + 1 ] != NULL )
			tasking_currentTask = tasking_queue[ tasking_currentTask->id + 1 ];
		else
			tasking_currentTask = tasking_queue[ 0 ];
		
		// we could set this higher/lower depending on its priority: LOW, NORMAL, HIGH
		tasking_currentTask->tick_slice = 1;
		
		//paging_setCurrentPageDir( tasking_currentTask->page_dir );
	}	
	else
	{
		tasking_currentTask->tick_slice--;
	}
	
	kernel_unlock();
	
	return tasking_currentTask->esp;
}

void tasking_ltr( WORD selector )
{
	ASM( "ltr %0" : : "rm" (selector) );
}
   
void tasking_init()
{
	int i, interval;
	//struct tss * new_tss;
	
	// create the empty task queue
	for( i=0 ; i<MAX_TASKS ; i++ )
		tasking_queue[i] = NULL;

	// create a TSS for our software task switching (6.2)
/*	new_tss = mm_malloc( sizeof( struct tss ) );
	new_tss->cr3 = paging_getCurrentPageDir();
	new_tss->esp = tasking_getESP();
	
	// setup the TSS Descriptor (6.2.2)
	gdt_setEntry( KERNEL_TSS_SEL, (DWORD)new_tss, sizeof(struct tss)-1, 0x89, 0x00 );
	tasking_ltr( KERNEL_TSS_SEL );
*/

	tasking_create( 0, ThreadTest1 );
	
	tasking_create( 1, ThreadTest2 );
	
	// calculate the timer interval
	interval = 1193180 / 100000; //100Hz
	// square wave mode
	outportb( PIT_COMMAND_REG, 0x36);
	// set the low interval for timer 0 (mapped to IRQ0)
	outportb( PIT_TIMER_0, interval & 0xFF);
	// set the high interval for timer 0
	outportb( PIT_TIMER_0, interval >> 8);
	// set the scheduler to run via the timer interrupt
	isr_setHandler( IRQ0, tasking_schedule );
}
