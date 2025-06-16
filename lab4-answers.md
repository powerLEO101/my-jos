> Compare kern/mpentry.S side by side with boot/boot.S. Bearing in mind that kern/mpentry.S is compiled and linked to run above KERNBASE just like everything else in the kernel, what is the purpose of macro MPBOOTPHYS? Why is it necessary in kern/mpentry.S but not in boot/boot.S? In other words, what could go wrong if it were omitted in kern/mpentry.S?

Because in `boot.S`, we specify to the linker that .text starts at 0x7c00. In constrast, we compile `mpentry.S` with the rest of the kernel, which makes it thinks it's at high address, which is no true. So we need to convert to physical address using macro within `mpentry.S` whenever we mess with `ip`

> Question within mpentry.S # Call mp_main().  (Exercise for the reader: why the indirect call?)

Reading the code logic from init.c:boot_aps, the code only copies mpentry.S compiled code into a low memory address. The mp_main that mpentry.S is trying to jump is not copied with mpentry.S. It resides in extended memory with the rest of the kernel. Direct call will be a relative address of the current address (this is even another level lower than the level I am working with - I just treat `call` as one instruction, while in fact it is many). The relative address will be calcualted as an address to low address, which does not exist. Therefore, we need a indirect call, which specifiy the exact address of the precedure in register.

Google: why use direct call instead of indirect call? direct call is much more performant because they make speculative execution eaiser.
