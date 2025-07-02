#include "ns.h"

extern union Nsipc nsipcbuf; // let's not use it

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.

	sys_page_alloc(0, &nsipcbuf, PTE_PWU);
	int r, index = 0;
	while (1) {
		while ((r = sys_net_recv(nsipcbuf.pkt.jp_data, 2048)) < 0)
			if (r != -E_NET_FULL)
				panic("net input recv failed: %e\n", r);
		// cprintf("received %d bytes\n", r);
		nsipcbuf.pkt.jp_len = r;
		ipc_send(ns_envid, NSREQ_INPUT, &nsipcbuf, PTE_PWU);
		sys_page_alloc(0, &nsipcbuf, PTE_PWU);
		// smart: https://github.com/Babtsov/jos/blob/master/lab6/net/input.c
		// NOTE Previously, I used two buffers and switch buffer each time a packet is received to address the Hint
		// NOTE But this is such a janky heuristic. After a long debug, I finally gave in to search Github as found that people just remap the page
		// NOTE smart.
		// NOTE if I don't realloc the page each time, there will be some weird issue with accessing user current page table
		// NOTE I suspect it's just something in the tcp stack. I did not fully investigate. Reallocating the page just works
	}
}
