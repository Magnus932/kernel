
#include "task.h"
#include "irq.h"
#include "gdt.h"
/*
 * The currently running task.
 */
volatile task_t *current_task;
/*
 * The start of the task linked list.
 */
volatile task_t *ready_queue;

/*
 * Some externs are needed to access
 * members in paging.c
 */
extern page_directory_t *kernel_directory;
extern page_directory_t *current_directory;

/*
 * The next available process ID.
 */
u32 next_pid = 1;

extern unsigned int initial_esp;

void initialize_tasking(void)
{
	cli();

	move_stack((void *)0x800000, 0x4000);
	/*
	 * Initialize the first task ( Kernel task ).
	 */
	current_task = ready_queue = (task_t *)kmalloc(sizeof(task_t));
	current_task->id = next_pid++;
	current_task->esp = current_task->ebp = 0;
	current_task->eip = 0;
	current_task->page_directory = current_directory;
	current_task->next = 0;
	current_task->kernel_stack = kmalloc_a(KERNEL_STACK_SIZE);
	sti();
}

int fork(void)
{
	cli();
	/*
	 * Take a pointer to this process task struct
	 * for later reference.
	 */
	task_t *parent_task = (task_t *)current_task;
	/*
	 * Clone the address space.
	 */
	page_directory_t *directory = clone_directory(current_directory);
	/*
	 * Create a new process.
	 */
	task_t *new_task = (task_t *)kmalloc(sizeof(task_t));
	new_task->id = next_pid++;
	new_task->esp = new_task->ebp = 0;
	new_task->eip = 0;
	new_task->page_directory = directory;
	new_task->next = 0;

	/*
	 * Add it to the end of the ready queue.
	 */
	task_t *tmp_task = (task_t *)ready_queue;
	while (tmp_task->next)
		tmp_task = tmp_task->next;
	tmp_task->next = new_task;

	/*
	 * This will be the entry point for the new
	 * process.
	 */
	u32 eip = read_eip();
	/*
	 * We could be the parent or the child here, check.
	 */
	if (current_task == parent_task) {
		/*
		 * We are the parent, so set up the esp/ebp/eip
		 * for our child.
		 */
		u32 esp, ebp;
		__asm__ __volatile__ ("movl %%esp, %0" : "=r" (esp));
		__asm__ __volatile__ ("movl %%ebp, %0" : "=r" (ebp));
		new_task->esp = esp;
		new_task->ebp = ebp;
		new_task->eip = eip;
		sti();
		return new_task->id;
	}
	else {
		/*
		 * We are the child. By convention return 0.
		 */
		outb(MASTER_ICW, NON_SPEC_EOI);
		return 0;
	}
}

void switch_task(void)
{
	if (!current_task)
		return;
	/*
	 * Read esp, ebp now for saving later
	 * on.
	 */
	u32 esp, ebp, eip;
	__asm__ __volatile__ ("movl %%esp, %0" : "=r" (esp));
	__asm__ __volatile__ ("movl %%ebp, %0" : "=r" (ebp));

	eip = read_eip();
	if (eip == 0x12345)
		return;
	/*
	 * No we didnt switch tasks. Lets save some register
	 * values and switch.
	 */
	current_task->eip = eip;
	current_task->esp = esp;
	current_task->ebp = ebp;

	/*
	 * Get the next task to run.
	 */
	current_task = current_task->next;
	if (!current_task)
		current_task = ready_queue;

	eip = current_task->eip;
	esp = current_task->esp;
	ebp = current_task->ebp;

	/*
	 * Make sure the memory manager knows that we have
	 * changed directory.
	 */
	current_directory = current_task->page_directory;
	/*
	 * Change our kernel stack over.
	 */
	set_kernel_stack(current_task->kernel_stack + KERNEL_STACK_SIZE);
	__asm__ __volatile__ (
						  "cli;"
						  "movl %0, %%ecx;"
						  "movl %1, %%esp;"
						  "movl %2, %%ebp;"
						  "movl %3, %%cr3;"
						  "movl $0x12345, %%eax;"
						  "sti;"
						  "jmp *%%ecx"
						  : : "r" (eip), "a" (esp), "r" (ebp),
						  	  "r" (current_directory->physicalAddr));
}

int getpid(void)
{
	return current_task->id;
}

void move_stack(void *new_stack_start, u32 size)
{
	u32 i;
	/*
	 * Allocate some new space for the stack.
	 */
	for (i = (u32)new_stack_start;
		 i >= ((u32)new_stack_start - size);
		 i -= 0x1000) {
		alloc_frame(get_page(i, 1, current_directory), 0, 1);
	}
	u32 pd_addr;
	__asm__ __volatile__ ("movl %%cr3, %0" : "=r" (pd_addr));
	__asm__ __volatile__ ("movl %0, %%cr3" : : "r" (pd_addr));

	u32 old_stack_pointer, old_base_pointer;
	__asm__ __volatile__ ("movl %%esp, %0" : "=r" (old_stack_pointer));
	__asm__ __volatile__ ("movl %%ebp, %0" : "=r" (old_base_pointer));
	u32 offset = (u32)new_stack_start - initial_esp;

	u32 new_stack_pointer = old_stack_pointer + offset;
	u32 new_base_pointer = old_base_pointer + offset;

	/*
	 * Copy the stack.
	 */
	memcpy((void *)new_stack_pointer, (void *)old_stack_pointer, initial_esp - old_stack_pointer);

	for (i = (u32)new_stack_start; i > (u32)new_stack_start - size; i -= 4) {
		u32 tmp = *(u32 *)i;
		if ((old_stack_pointer < tmp) && (tmp < initial_esp)) {
			tmp = tmp + offset;
			u32 *tmp2 = (u32 *)i;
			*tmp2 = tmp;
		}
	}
	/*
	 * Lastly, change stacks..
	 */
	__asm__ __volatile__ ("movl %0, %%esp" : : "r" (new_stack_pointer));
	__asm__ __volatile__ ("movl %0, %%ebp" : : "r" (new_base_pointer));
}

void switch_to_user_mode()
{
   asm volatile("  \
     cli; \
     mov $0x23, %ax; \
     mov %ax, %ds; \
     mov %ax, %es; \
     mov %ax, %fs; \
     mov %ax, %gs; \
                   \
     mov %esp, %eax; \
     pushl $0x23; \
     pushl %eax; \
     pushf; \
     pop %eax; \
     or $0x200, %eax; \
     push %eax; \
     pushl $0x1b; \
     push $1f; \
     iret; \
   1: \
     ");
}