/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(divid_handler, T_DIVIDE)	/*divide error*/
TRAPHANDLER_NOEC(debug_handler, T_DEBUG)	/*debug exception*/
TRAPHANDLER_NOEC(nmi_handler, T_NMI)	/*non-maskable interrupt*/
TRAPHANDLER_NOEC(breakpoint_handler, T_BRKPT)	/*breakpoint*/
TRAPHANDLER_NOEC(overflow_handler, T_OFLOW)	/*overflow*/
TRAPHANDLER_NOEC(bounds_check_handler, T_BOUND)	/*bounds check*/
TRAPHANDLER_NOEC(illegal_opcode_handler, T_ILLOP)	/*illegal opcode*/
TRAPHANDLER_NOEC(device_handler, T_DEVICE)	/*device not available*/
TRAPHANDLER(douvble_fault_handler, T_DBLFLT)	/*double fault*/
#TRAPHANDLER_NOEC(reserved_handler, 9)	/*reserved*/
TRAPHANDLER(tss_handler, T_TSS)	/*invalid task switch segment*/
TRAPHANDLER(segment_handler, T_SEGNP)	/*segment not present*/
TRAPHANDLER(stack_handler, T_STACK)	/*stack exception*/
TRAPHANDLER(gpf_handler, T_GPFLT)	/*general protection fault*/
TRAPHANDLER(page_fault_handler, T_PGFLT)	/*page fault*/
#TRAPHANDLER_NOEC(_handler1, 15)	/*reserved*/
TRAPHANDLER_NOEC(float_handler, T_FPERR)	/*floating point error*/
TRAPHANDLER_NOEC(aligment_handler, T_ALIGN)	/*aligment check*/
TRAPHANDLER_NOEC(machine_check_handler, T_MCHK)	/*machine check*/
TRAPHANDLER_NOEC(simd_handler,T_SIMDERR)/*SIMD floating point error*/
TRAPHANDLER(system_handler, T_SYSCALL)	/*system call*/
TRAPHANDLER_NOEC(catchall_handler, T_DEFAULT)	/*catchall*/
TRAPHANDLER_NOEC(timer_handler, IRQ_OFFSET + IRQ_TIMER)	/**/
TRAPHANDLER_NOEC(kbd_handler, IRQ_OFFSET + IRQ_KBD)	/**/
TRAPHANDLER_NOEC(serial_handler, IRQ_OFFSET + IRQ_SERIAL)	/**/
TRAPHANDLER_NOEC(spurious_handler, IRQ_OFFSET + IRQ_SPURIOUS)	/**/
TRAPHANDLER_NOEC(ide_handler, IRQ_OFFSET + IRQ_IDE)	/**/
TRAPHANDLER_NOEC(error_handler, IRQ_OFFSET + IRQ_ERROR)	/**/


/*
 * Lab 3: Your code here for _alltraps
 */

_alltraps:
	cli
	pushl %ds
	pushl %es
	pushal
	movl $(GD_KD),%eax
	mov %eax,%ds
	mov %eax,%es
	pushl %esp
	call trap
	popl %esp
	popal 
	popl %es
	popl %ds
	add 0x8,%esp /*clean up the pushed error code and ISR number*/
	sti
	iret   /*pops 5 things at once:CS,EIP,EFLAGS,SS,and ESP*/
