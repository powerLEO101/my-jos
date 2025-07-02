#include "ns.h"

extern union Nsipc nsipcbuf;
// NOTE the peculiar definition of nsipbuf:
// NOTE 90% sure that the nsipbuf here is a artifact of the fork in umain
// NOTE so output actually shared the same nsipcbuf COW with the parent
// NOTE but we don't care about that, we will overwrite the page table entry when doing ipc

void
output(envid_t ns_envid)
{
	int32_t r, whom, perm;
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	while (1) {
		if ((r = ipc_recv(&whom, &nsipcbuf, &perm)) < 0)
			panic("ipc_recv failed: %e", r);
		if (r != NSREQ_OUTPUT || !perm)
			continue;
		if (nsipcbuf.pkt.jp_len + sizeof(int) > PGSIZE) // buffer overflow, even though we don't expect such situation
			continue;
		// NOTE we are really stupid - we really just relay whatever server give us to the kernel
		// NOTE we need to do this part of the lab because lab doesn't know our syscall interface
		if ((r = sys_net_send(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)) < 0)
			cprintf("output: net send failed, buffer is not sent: %e\n", r);
	}
}
