> What is the purpose of having an individual handler function for each exception/interrupt? (i.e., if all exceptions/interrupts were delivered to the same handler, what feature that exists in the current implementation could not be provided?)

Trap hanlder don't know their trap number

> Did you have to do anything to make the user/softint program behave correctly? The grade script expects it to produce a general protection fault (trap 13), but softint's code says int $14. Why should this produce interrupt vector 13? What happens if the kernel actually allows softint's int $14 instruction to invoke the kernel's page fault handler (which is interrupt vector 14)?

User cannot software trigger int 14 because it's a page fault. Must be turned into a general protection fault. Otherwise user can mess with page table

> The break point test case will either generate a break point exception or a general protection fault depending on how you initialized the break point entry in the IDT (i.e., your call to SETGATE from trap_init). Why? How do you need to set it up in order to get the breakpoint exception to work as specified above and what incorrect setup would cause it to trigger a general protection fault?

The DPL should be 3 instead of 0. Because user need to be able to generate this interrupt via software (i.e. int)

> What do you think is the point of these mechanisms, particularly in light of what the user/softint test program does?

There are some interrupts that os allows user to directly generate. There are some that os does not.
