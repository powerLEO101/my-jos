#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

// LAB 6: Your driver code here

static volatile void *e1000;
// NOTE TODO what kind of memory are we accessing? void or uint32?
// NOTE a volatile pointer vs a pointer to a volatile:
// NOTE this is a pointer to a volatile, it says, the memory location
// NOTE pointed to by this pointer is volatile, but the pointer itself is not
// NOTE If I were to say "void volatile *e1000", then it is a volatile pointer
// NOTE ref: https://stackoverflow.com/questions/9935190/why-is-a-point-to-volatile-pointer-like-volatile-int-p-useful

// NOTE since this is part of kernel, we are promised that it is contiguous
// NOTE we are required to align this region to 16 bytes
static struct tx_desc tx_desc_list[E1000_MAX_TX_NDESC] __attribute__((__aligned__(16)));
static char tx_desc_data[E1000_MAX_TX_NDESC][E1000_MAX_BUFLEN];
static struct rx_desc rx_desc_list[E1000_MAX_RX_NDESC] __attribute__((__aligned__(16)));
static char rx_desc_data[E1000_MAX_RX_NDESC][E1000_MAX_BUFLEN];

static void
e1000_print_info()
{
	char testdata[100];
	for (int i = 0; i < 100; i++)
		testdata[i] = i;
	cprintf("e1000 info:\n");
	cprintf("  device status: %08x\n", *(uint32_t *) (e1000 + E1000_STATUS));
	// cprintf("  testing transmit retcode=%d\n", e1000_transmit(testdata, 48));
	// cprintf("  testing transmit retcode=%d\n", e1000_transmit("Packet 99", 9));
}

static inline void
e1000_set32(uint32_t offset, uint32_t value)
{
	*((uint32_t *) (e1000 + offset)) = value;
}

static inline uint32_t
e1000_read32(uint32_t offset)
{
	return *((uint32_t *) (e1000 + offset));
}

int 
e1000_transmit(void *buf, uint32_t len)
{
	uint32_t tail = e1000_read32(E1000_TDT);
	if (len > E1000_MAX_BUFLEN) // NOTE is the min buflen mandatory? NOTE no
		return -E_INVAL;
	if (!(tx_desc_list[tail].status & E1000_TXD_STAT_DD)) // descriptor is free iff dd bit is set (init set all dd bit)
		return -E_NET_FULL;

	// NOTE we dont need to set addr because they are preset
	memmove(tx_desc_data[tail], buf, len);
	tx_desc_list[tail].length = len;
	tx_desc_list[tail].cso = 0;
	tx_desc_list[tail].cmd = E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS; // we are lagacy, so DEXT is not set
	tx_desc_list[tail].status = 0;
	tx_desc_list[tail].css = 0;
	tx_desc_list[tail].special = 0;
	e1000_set32(E1000_TDT, (tail + 1) % E1000_MAX_TX_NDESC);
	return 0;
}

int
e1000_receive(void *buf, uint32_t len)
{
	uint32_t tail = (e1000_read32(E1000_RDT) + 1) % E1000_MAX_RX_NDESC, final_len;
	if (!(rx_desc_list[tail].status & E1000_RXD_STAT_DD))
		return -E_NET_FULL;
	final_len = len < rx_desc_list[tail].length ? len : rx_desc_list[tail].length;
	memmove(buf, rx_desc_data[tail], final_len);
	rx_desc_list[tail].status = 0;
	e1000_set32(E1000_RDT, tail);
	return final_len;
}

int
e1000_init(struct pci_func *pcif)
{
	pci_func_enable(pcif);
	e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
	for (uint32_t i = 0; i < E1000_MAX_TX_NDESC; i++) {
		tx_desc_list[i].addr = (uintptr_t) PADDR(tx_desc_data[i]);
		tx_desc_list[i].status = E1000_TXD_STAT_DD;
	}
	for (uint32_t i = 0; i < E1000_MAX_RX_NDESC; i++)
		rx_desc_list[i].addr = (uintptr_t) PADDR(rx_desc_data[i]);

	// transmit
	e1000_set32(E1000_TDBAL, (uintptr_t) PADDR(tx_desc_list));
	e1000_set32(E1000_TDLEN, sizeof(tx_desc_list));
	e1000_set32(E1000_TDH, 0);
	e1000_set32(E1000_TDT, 0);
	e1000_set32(E1000_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP | (0x10 << E1000_TCTL_CT_SHIFT) | (0x40 << E1000_TCTL_COLD_SHIFT));
	e1000_set32(E1000_TIPG, E1000_TIPG_DEFAULT);

	// receive
	e1000_set32(E1000_RA, 0x12005452); // hardcoded MAC
	e1000_set32(E1000_RA + 4, 0x80005634);
	e1000_set32(E1000_MTA, 0);
	e1000_set32(E1000_IMS, 0);
	e1000_set32(E1000_RDBAL, (uintptr_t) PADDR(rx_desc_list));
	e1000_set32(E1000_RDBAH, 0);
	e1000_set32(E1000_RDLEN, sizeof(rx_desc_list));
	e1000_set32(E1000_RDH, 0);
	e1000_set32(E1000_RDT, E1000_MAX_RX_NDESC - 1);
	e1000_set32(E1000_RCTL, E1000_RCTL_EN | E1000_RCTL_SECRC); 
	// enable receive function last because we want to configure the receive buffers first (they are full at start)

	e1000_print_info();
	
	return 0;
}
