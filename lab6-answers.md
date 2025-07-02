> How did you structure your transmit implementation? In particular, what do you do if the transmit ring is full?

In the syscall, I return an error to inform user that the queue is full. But in the implementation of output.c, I ignore such error and simply drop the packet. Since tcp should have the ability to recover the packet

> How did you structure your receive implementation? In particular, what do you do if the receive queue is empty and a user environment requests the next incoming packet?

I make the user wait.
