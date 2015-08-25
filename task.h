
#ifndef TASK_H
#define TASK_H

#include "page.h"

#define KERNEL_STACK_SIZE		2048
/*
 * This structure defines a "task" or
 * a process.
 */
typedef struct task {
	/*
	 * Process ID.
	 */
	int id;
	/*
	 * Stack and base pointers.
	 */
	u32 esp, ebp;
	/*
	 * Instruction pointer.
	 */
	u32 eip;
	/*
	 * Page directory. Defines the address
	 * space of this process.
	 */
	page_directory_t *page_directory;
	/*
	 * Kernel stack location.
	 */
	u32 kernel_stack;
	/*
	 * The next task in a linked list.
	 */
	struct task *next;
} task_t;

/*
 * Initializes the tasking system.
 */
void initialize_tasking(void);

/*
 * Called by the timer hook, this changes the
 * running process.
 */
void task_switch(void);

/*
 * Forks the current process, spawning a new one
 * with a different memory space.
 */
int fork(void);

/*
 * Causes the current process stack to be forcibly
 * moved to a new location.
 */
void move_stack(void *new_stack_start, u32 size);

/*
 * Returns the PID of the process.
 */
int getpid(void);

#endif /* TASK_H */
