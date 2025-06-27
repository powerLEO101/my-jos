> Do you have to do anything else to ensure that this I/O privilege setting is saved and restored properly when you subsequently switch from one environment to another? Why?

No. Because eflags are automatically stored and restored by `int` and `iret`.
