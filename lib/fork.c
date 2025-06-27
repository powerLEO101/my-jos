// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) ROUNDDOWN(utf->utf_fault_va, PGSIZE);
	uint32_t err = utf->utf_err;
	int perm = uvpt[(uintptr_t) addr >> 12] & PTE_SYSCALL;
	// NOTE TODO note that the uvpt refernce itself here can cause another page fault:
	// NOTE is an entire page dir entry is unmapped, then uvpt will also be unmapped (uvpd will be mapped)
	// NOTE in next nested page fault, we will finally panic because fault type is a READ
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	if (!(err & FEC_WR))
		panic("pgfault is not write: %p %p", addr, utf->utf_eip);
	if (!(perm & PTE_COW))
		// TODO double check this using gdb, is the page table refernce correct?
		panic("page is not copy on write: %p", addr);
	if ((r = sys_page_alloc(0, PFTEMP, perm ^ (PTE_W | PTE_COW))) < 0)
		panic("page alloc failed: %e", r);
	memmove((void *) PFTEMP, addr, PGSIZE);
	if ((r = sys_page_map(0, PFTEMP, 0, addr, perm ^ (PTE_W | PTE_COW))) < 0)
		panic("page map failed, is there alignment issue?: %e", r);
	if ((r = sys_page_unmap(0, PFTEMP)) < 0)
		panic("page unmap failed, PFTEMP may be trashed: %e", r);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function? Answer: This and another exercise question in the web page. 
// If we are mapping the stack, and we are also using the stack (we are in user space),
// our copy-on-write mark on the parent might be gone in the middle of duppage. So,
// we also need to care about the order: if parent map first, instead of child, then
// child can copy the mapping of a page from parent that parent actively uses and does
// not mark as copy-on-write. Bad!)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	int perm = uvpt[pn] & PTE_SYSCALL;

	// LAB 4: Your code here.
	// NOTE we are still in user space here. we are not even in the user exception stack
	// NOTE I searched through many online solutions, some answered the exercise question above
	// NOTE but no one seems to get it right. 
	// NOTE modifition of this code in lab5
	if (!(perm & PTE_SHARE) && perm & (PTE_W | PTE_COW))
		perm = (perm & ~PTE_W) | PTE_COW;
	if ((r = sys_page_map(0, (void *) (pn * PGSIZE), envid, (void *) (pn * PGSIZE), perm)) < 0)
		panic("cannot map child page: %e", r);
	if (perm & PTE_COW && (r = sys_page_map(0, (void *) (pn * PGSIZE), 0, (void *) (pn * PGSIZE), perm)) < 0)
		panic("cannot map parent page COW: %e", r);
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
extern void _pgfault_upcall(void);
envid_t
fork(void)
{
	// LAB 4: Your code here.
	set_pgfault_handler(pgfault);
	envid_t envid_c = sys_exofork();

	if (envid_c < 0)
		panic("cannot create new child");
	if (envid_c == 0) {
		// child
		thisenv = envs + ENVX(sys_getenvid());
		return 0;
	} else {
		// parent, child sys_exofork does not return until this routine finish setting up child
		for (unsigned i0 = 0; i0 < NPDENTRIES && i0 << 22 < UTOP; i0++)
			// NOTE the use of unsigned here is important. if use int, we might overflow to negative
			if ((uvpd[i0] | ~(PTE_P | PTE_U)) == ~0)
				for (unsigned i1 = 0; i1 < NPTENTRIES; i1++)
					if (((i0 * NPDENTRIES + i1) * PGSIZE != UXSTACKTOP - PGSIZE) && 
							((uvpt[i0 * NPDENTRIES + i1] | ~(PTE_U | PTE_P)) == ~0))
						duppage(envid_c, i0 * NPDENTRIES + i1);
		sys_page_alloc(envid_c, (void *) UXSTACKTOP - PGSIZE, PTE_PWU);
		sys_env_set_pgfault_upcall(envid_c, _pgfault_upcall); // same address space
		sys_env_set_status(envid_c, ENV_RUNNABLE);
		return envid_c;
	}
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
