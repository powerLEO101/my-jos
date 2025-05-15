> At what point does the processor start executing 32-bit code? What exactly causes the switch from 16- to 32-bit mode?

at `ljmp    $PROT_MODE_CSEG, $protcseg`

> What is the last instruction of the boot loader executed, and what is the first instruction of the kernel it just loaded?

readseg or call entry(). 
first kernel instruction is `movw	$0x1234,0x472`

> Where is the first instruction of the kernel?

entry.S first line

> How does the boot loader decide how many sectors it must read in order to fetch the entire kernel from disk? Where does it find this information?

bootloader expects to find the kernel executable in elf format starting at the second sector

the elf file should contain the size of the kernel that needs to be loaded

> Exercise 5. Trace through the first few instructions of the boot loader again and identify the first instruction that would "break" or otherwise do the wrong thing if you were to get the boot loader's link address wrong. Then change the link address in boot/Makefrag to something wrong, run make clean, recompile the lab with make, and trace into the boot loader again to see what happens. Don't forget to change the link address back and make clean again afterward!

The code starts coding wrong thing at `ljmp    $PROT_MODE_CSEG, $protcseg`. the page table is only setup to expect execution at 0x7c00. when linking at a wrong place, this long jump will jump to some crazy places.

I did not fully understand the synatax of long jump. I will come back to it if needed later.

> Exercise 6. We can examine memory using GDB's x command. The GDB manual has full details, but for now, it is enough to know that the command x/Nx ADDR prints N words of memory at ADDR. (Note that both 'x's in the command are lowercase.) Warning: The size of a word is not a universal standard. In GNU assembly, a word is two bytes (the 'w' in xorw, which stands for word, means 2 bytes).
Reset the machine (exit QEMU/GDB and start them again). Examine the 8 words of memory at 0x00100000 at the point the BIOS enters the boot loader, and then again at the point the boot loader enters the kernel. Why are they different? What is there at the second breakpoint? (You do not really need to use QEMU to answer this question. Just think.)

Kernel is loaded at the address. I had some trouble setting the breakpoint because gdb uses physical address instead of the address represented entry in the symbol table (so low address)

> Exercise 7. single step through enable paging, what happened?

paging is enabled obviously. so at 0x00100000 and 0xf0100000, both have the code for the kernel

> Exercise 8, 9, 10, 11

code or trivial

> Exercise 12

ehhh, stabs files are not supported on my gcc version. apparently its been outdated for a while

--- Leoh
