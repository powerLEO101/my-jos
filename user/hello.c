// hello, world
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("hello, world\n");
	cprintf("i am environment %08x\n", thisenv->env_id);
	int r;
	if ((r = sys_net_send("bogusbogusbogusbogusbogusbogusbogusbogusbogusbogus", 50)) < 0)
		cprintf("testing net failed: %e\n", r);
}
