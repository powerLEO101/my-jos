// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>
#include <kern/pmap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line

extern uint16_t cga_color;

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display the backtrace of the current stack", mon_backtrace},
	{ "showmapping", "Show virtual memory mapping of [va_start, va_end) of kernel (currently)", mon_showmapping},
};

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	cprintf("Stack backtrace:\n");
	uint32_t *stack = (uint32_t *) read_ebp();
	while (stack != NULL) {
		cprintf("ebp %08x  eip %08x  args %08x %08x %08x %08x %08x\n", stack, stack[1], stack[2], stack[3], stack[4], stack[5], stack[6]);
		stack = (uint32_t *) stack[0];
	}
	return 0;
}

static void
print_pt_perm(pte_t pe)
{
	cprintf("%c%c%c",
			pe & PTE_PS ? 'M' : 'K',
			pe & PTE_W ? 'w' : '-',
			pe & PTE_U ? 'u' : '-'
			);
}

int
mon_showmapping(int argc, char **argv, struct Trapframe *tf)
{
	if (argc != 3) {
		cprintf("usage:\n  showmapping va_start va_end (in hex)\n");
		return 0;
	}
	uintptr_t va_start = strtol(argv[1], NULL, 16), va_end = strtol(argv[2], NULL, 16);
	if (va_start % PGSIZE || va_end % PGSIZE || va_end <= va_start) {
		cprintf("invalid va address\n");
		return 0;
	}
	// NOTE a lot of bandaid code here, because this code section is copied from a test in pmap.c
	extern pte_t *kern_pgdir;
	for (uintptr_t addr = va_start; addr < va_end; addr += PGSIZE) {
		cprintf("%p: ", addr);

		pte_t *pgdir = kern_pgdir;
		pte_t *p;
		pgdir = &pgdir[PDX(addr)];
		if (!(*pgdir & PTE_P)) {
			cprintf("unmapped1\n");
			continue;
		}
		if ((*pgdir & PTE_P) && (*pgdir & PTE_PS)) {
			cprintf("%08p ", PTE_ADDR(*pgdir) + (PTX(addr) << PGSHIFT));
			print_pt_perm(*pgdir);
			cprintf("\n");
			continue;
		}
		p = (pte_t*) KADDR(PTE_ADDR(*pgdir));
		if (!(p[PTX(addr)] & PTE_P)) {
			cprintf("unmapped2\n");
			continue;
		}
		cprintf("%08p ", PTE_ADDR(p[PTX(addr)]));
		print_pt_perm(p[PTX(addr)]);
		cprintf("\n");
	}

	return 0;
}

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		cga_color = 0x0b00;
		buf = readline("K> ");
		cga_color = 0x0000;
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
