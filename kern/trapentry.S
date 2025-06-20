/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


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
	.data; \
	.long name; \
	.text; \
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
	.data; \
	.long name; \
	.text; \
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.data
	.globl trap_handlers
trap_handlers:
.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(T_DIVIDE_HANDLER, 0)
TRAPHANDLER_NOEC(T_DEBUG_HANDLER, 1)
TRAPHANDLER_NOEC(T_NMI_HANDLER, 2)
TRAPHANDLER_NOEC(T_BRKPT_HANDLER, 3)
TRAPHANDLER_NOEC(T_OFLOW_HANDLER, 4)
TRAPHANDLER_NOEC(T_BOUND_HANDLER, 5)
TRAPHANDLER_NOEC(T_ILLOP_HANDLER, 6)
TRAPHANDLER_NOEC(T_DEVICE_HANDLER, 7)
TRAPHANDLER(T_DBLFLT_HANDLER, 8)
TRAPHANDLER(T_COPROC_HANDLER, 9)
TRAPHANDLER(T_TSS_HANDLER, 10)
TRAPHANDLER(T_SEGNP_HANDLER, 11)
TRAPHANDLER(T_STACK_HANDLER, 12)
TRAPHANDLER(T_GPFLT_HANDLER, 13)
TRAPHANDLER(T_PGFLT_HANDLER, 14)
TRAPHANDLER(T_RES_HANDLER, 15)
TRAPHANDLER_NOEC(T_FPERR_HANDLER, 16)
TRAPHANDLER(T_ALIGN_HANDLER, 17)
TRAPHANDLER_NOEC(T_MCHK_HANDLER, 18)
TRAPHANDLER_NOEC(T_SIMDERR_HANDLER, 19)
TRAPHANDLER_NOEC(T_20_HANDLER, 20)
TRAPHANDLER_NOEC(T_21_HANDLER, 21)
TRAPHANDLER_NOEC(T_22_HANDLER, 22)
TRAPHANDLER_NOEC(T_23_HANDLER, 23)
TRAPHANDLER_NOEC(T_24_HANDLER, 24)
TRAPHANDLER_NOEC(T_25_HANDLER, 25)
TRAPHANDLER_NOEC(T_26_HANDLER, 26)
TRAPHANDLER_NOEC(T_27_HANDLER, 27)
TRAPHANDLER_NOEC(T_28_HANDLER, 28)
TRAPHANDLER_NOEC(T_29_HANDLER, 29)
TRAPHANDLER_NOEC(T_30_HANDLER, 30)
TRAPHANDLER_NOEC(T_31_HANDLER, 31)
TRAPHANDLER_NOEC(T_32_HANDLER, 32)
TRAPHANDLER_NOEC(T_33_HANDLER, 33)
TRAPHANDLER_NOEC(T_34_HANDLER, 34)
TRAPHANDLER_NOEC(T_35_HANDLER, 35)
TRAPHANDLER_NOEC(T_36_HANDLER, 36)
TRAPHANDLER_NOEC(T_37_HANDLER, 37)
TRAPHANDLER_NOEC(T_38_HANDLER, 38)
TRAPHANDLER_NOEC(T_39_HANDLER, 39)
TRAPHANDLER_NOEC(T_40_HANDLER, 40)
TRAPHANDLER_NOEC(T_41_HANDLER, 41)
TRAPHANDLER_NOEC(T_42_HANDLER, 42)
TRAPHANDLER_NOEC(T_43_HANDLER, 43)
TRAPHANDLER_NOEC(T_44_HANDLER, 44)
TRAPHANDLER_NOEC(T_45_HANDLER, 45)
TRAPHANDLER_NOEC(T_46_HANDLER, 46)
TRAPHANDLER_NOEC(T_47_HANDLER, 47)
TRAPHANDLER_NOEC(T_48_HANDLER, 48)

/*
 * Lab 3: Your code here for _alltraps
 */

	.globl _alltraps
_alltraps:
	pushl %ds
	pushl %es
	pushal
	movw $GD_KD, %ax
	movw %ax, %ds
	movw %ax, %es
	/* are these two move neccesary? since they refer to the same segment anyway. does privilege matter for segments other than cs? */
	pushl %esp
	call trap
